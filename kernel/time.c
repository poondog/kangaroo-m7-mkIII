/*
 *  linux/kernel/time.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 *
 *  This file contains the interface functions for the various
 *  time related system calls: time, stime, gettimeofday, settimeofday,
 *			       adjtime
 */

#include <linux/export.h>
#include <linux/timex.h>
#include <linux/capability.h>
#include <linux/clocksource.h>
#include <linux/errno.h>
#include <linux/syscalls.h>
#include <linux/security.h>
#include <linux/fs.h>
#include <linux/math64.h>
#include <linux/ptrace.h>

#include <asm/uaccess.h>
#include <asm/unistd.h>

#include "timeconst.h"

struct timezone sys_tz;

EXPORT_SYMBOL(sys_tz);

#ifdef __ARCH_WANT_SYS_TIME

SYSCALL_DEFINE1(time, time_t __user *, tloc)
{
	time_t i = get_seconds();

	if (tloc) {
		if (put_user(i,tloc))
			return -EFAULT;
	}
	force_successful_syscall_return();
	return i;
}


SYSCALL_DEFINE1(stime, time_t __user *, tptr)
{
	struct timespec tv;
	int err;

	if (get_user(tv.tv_sec, tptr))
		return -EFAULT;

	tv.tv_nsec = 0;

	err = security_settime(&tv, NULL);
	if (err)
		return err;

	do_settimeofday(&tv);
	return 0;
}

#endif 

SYSCALL_DEFINE2(gettimeofday, struct timeval __user *, tv,
		struct timezone __user *, tz)
{
	if (likely(tv != NULL)) {
		struct timeval ktv;
		do_gettimeofday(&ktv);
		if (copy_to_user(tv, &ktv, sizeof(ktv)))
			return -EFAULT;
	}
	if (unlikely(tz != NULL)) {
		if (copy_to_user(tz, &sys_tz, sizeof(sys_tz)))
			return -EFAULT;
	}
	return 0;
}

static inline void warp_clock(void)
{
	struct timespec adjust;

	adjust = current_kernel_time();
	adjust.tv_sec += sys_tz.tz_minuteswest * 60;
	do_settimeofday(&adjust);
}


int do_sys_settimeofday(const struct timespec *tv, const struct timezone *tz)
{
	static int firsttime = 1;
	int error = 0;

	if (tv && !timespec_valid(tv))
		return -EINVAL;

	error = security_settime(tv, tz);
	if (error)
		return error;

	if (tz) {
		sys_tz = *tz;
		update_vsyscall_tz();
		if (firsttime) {
			firsttime = 0;
			if (!tv)
				warp_clock();
		}
	}
	if (tv)
		return do_settimeofday(tv);
	return 0;
}

SYSCALL_DEFINE2(settimeofday, struct timeval __user *, tv,
		struct timezone __user *, tz)
{
	struct timeval user_tv;
	struct timespec	new_ts;
	struct timezone new_tz;

	if (tv) {
		if (copy_from_user(&user_tv, tv, sizeof(*tv)))
			return -EFAULT;
		new_ts.tv_sec = user_tv.tv_sec;
		new_ts.tv_nsec = user_tv.tv_usec * NSEC_PER_USEC;
	}
	if (tz) {
		if (copy_from_user(&new_tz, tz, sizeof(*tz)))
			return -EFAULT;
	}

	return do_sys_settimeofday(tv ? &new_ts : NULL, tz ? &new_tz : NULL);
}

SYSCALL_DEFINE1(adjtimex, struct timex __user *, txc_p)
{
	struct timex txc;		
	int ret;

	if(copy_from_user(&txc, txc_p, sizeof(struct timex)))
		return -EFAULT;
	ret = do_adjtimex(&txc);
	return copy_to_user(txc_p, &txc, sizeof(struct timex)) ? -EFAULT : ret;
}

struct timespec current_fs_time(struct super_block *sb)
{
	struct timespec now = current_kernel_time();
	return timespec_trunc(now, sb->s_time_gran);
}
EXPORT_SYMBOL(current_fs_time);

/*
 * Convert jiffies to milliseconds and back.
 */
unsigned int __jiffies_to_msecs(const unsigned long j)
{
	return __inline_jiffies_to_msecs(j);
}
EXPORT_SYMBOL(__jiffies_to_msecs);

unsigned int __jiffies_to_usecs(const unsigned long j)
{
	return __inline_jiffies_to_usecs(j);
}
EXPORT_SYMBOL(__jiffies_to_usecs);

/**
 * timespec_trunc - Truncate timespec to a granularity
 * @t: Timespec
 * @gran: Granularity in ns.
 *
 * Truncate a timespec to a granularity. gran must be smaller than a second.
 * Always rounds down.
 *
 * This function should be only used for timestamps returned by
 * current_kernel_time() or CURRENT_TIME, not with do_gettimeofday() because
 * it doesn't handle the better resolution of the latter.
 */
struct timespec timespec_trunc(struct timespec t, unsigned gran)
{
	/*
	 * Division is pretty slow so avoid it for common cases.
	 * Currently current_kernel_time() never returns better than
	 * jiffies resolution. Exploit that.
	 */
	if (gran <= jiffies_to_usecs(1) * 1000) {
		/* nothing */
	} else if (gran == 1000000000) {
		t.tv_nsec = 0;
	} else {
		t.tv_nsec -= t.tv_nsec % gran;
	}
	return t;
}
EXPORT_SYMBOL(timespec_trunc);

/* Converts Gregorian date to seconds since 1970-01-01 00:00:00.
 * Assumes input in normal date format, i.e. 1980-12-31 23:59:59
 * => year=1980, mon=12, day=31, hour=23, min=59, sec=59.
 *
 * [For the Julian calendar (which was used in Russia before 1917,
 * Britain & colonies before 1752, anywhere else before 1582,
 * and is still in use by some communities) leave out the
 * -year/100+year/400 terms, and add 10.]
 *
 * This algorithm was first published by Gauss (I think).
 *
 * WARNING: this function will overflow on 2106-02-07 06:28:16 on
 * machines where long is 32-bit! (However, as time_t is signed, we
 * will already get problems at other places on 2038-01-19 03:14:08)
 */
unsigned long
mktime(const unsigned int year0, const unsigned int mon0,
       const unsigned int day, const unsigned int hour,
       const unsigned int min, const unsigned int sec)
{
	unsigned int mon = mon0, year = year0;

	/* 1..12 -> 11,12,1..10 */
	if (0 >= (int) (mon -= 2)) {
		mon += 12;	/* Puts Feb last since it has leap day */
		year -= 1;
	}

	return ((((unsigned long)
		  (year/4 - year/100 + year/400 + 367*mon/12 + day) +
		  year*365 - 719499
	    )*24 + hour /* now have hours */
	  )*60 + min /* now have minutes */
	)*60 + sec; /* finally seconds */
}

EXPORT_SYMBOL(mktime);

/**
 * set_normalized_timespec - set timespec sec and nsec parts and normalize
 *
 * @ts:		pointer to timespec variable to be set
 * @sec:	seconds to set
 * @nsec:	nanoseconds to set
 *
 * Set seconds and nanoseconds field of a timespec variable and
 * normalize to the timespec storage format
 *
 * Note: The tv_nsec part is always in the range of
 *	0 <= tv_nsec < NSEC_PER_SEC
 * For negative values only the tv_sec field is negative !
 */
void set_normalized_timespec(struct timespec *ts, time_t sec, s64 nsec)
{
	while (nsec >= NSEC_PER_SEC) {
		/*
		 * The following asm() prevents the compiler from
		 * optimising this loop into a modulo operation. See
		 * also __iter_div_u64_rem() in include/linux/time.h
		 */
		asm("" : "+rm"(nsec));
		nsec -= NSEC_PER_SEC;
		++sec;
	}
	while (nsec < 0) {
		asm("" : "+rm"(nsec));
		nsec += NSEC_PER_SEC;
		--sec;
	}
	ts->tv_sec = sec;
	ts->tv_nsec = nsec;
}
EXPORT_SYMBOL(set_normalized_timespec);

/**
 * ns_to_timespec - Convert nanoseconds to timespec
 * @nsec:       the nanoseconds value to be converted
 *
 * Returns the timespec representation of the nsec parameter.
 */
struct timespec ns_to_timespec(const s64 nsec)
{
	struct timespec ts;
	s32 rem;

	if (!nsec)
		return (struct timespec) {0, 0};

	ts.tv_sec = div_s64_rem(nsec, NSEC_PER_SEC, &rem);
	if (unlikely(rem < 0)) {
		ts.tv_sec--;
		rem += NSEC_PER_SEC;
	}
	ts.tv_nsec = rem;

	return ts;
}
EXPORT_SYMBOL(ns_to_timespec);

/**
 * ns_to_timeval - Convert nanoseconds to timeval
 * @nsec:       the nanoseconds value to be converted
 *
 * Returns the timeval representation of the nsec parameter.
 */
struct timeval ns_to_timeval(const s64 nsec)
{
	struct timespec ts = ns_to_timespec(nsec);
	struct timeval tv;

	tv.tv_sec = ts.tv_sec;
	tv.tv_usec = (suseconds_t) ts.tv_nsec / 1000;

	return tv;
}
EXPORT_SYMBOL(ns_to_timeval);

unsigned long __msecs_to_jiffies(const unsigned int m)
{
	return __inline_msecs_to_jiffies(m);
}
EXPORT_SYMBOL(__msecs_to_jiffies);

unsigned long __usecs_to_jiffies(const unsigned int u)
{
	return __inline_usecs_to_jiffies(u);
}
EXPORT_SYMBOL(__usecs_to_jiffies);

unsigned long
timespec_to_jiffies(const struct timespec *value)
{
	unsigned long sec = value->tv_sec;
	long nsec = value->tv_nsec + TICK_NSEC - 1;

	if (sec >= MAX_SEC_IN_JIFFIES){
		sec = MAX_SEC_IN_JIFFIES;
		nsec = 0;
	}
	return (((u64)sec * SEC_CONVERSION) +
		(((u64)nsec * NSEC_CONVERSION) >>
		 (NSEC_JIFFIE_SC - SEC_JIFFIE_SC))) >> SEC_JIFFIE_SC;

}
EXPORT_SYMBOL(timespec_to_jiffies);

void
jiffies_to_timespec(const unsigned long jiffies, struct timespec *value)
{
	u32 rem;
	value->tv_sec = div_u64_rem((u64)jiffies * TICK_NSEC,
				    NSEC_PER_SEC, &rem);
	value->tv_nsec = rem;
}
EXPORT_SYMBOL(jiffies_to_timespec);

unsigned long
timeval_to_jiffies(const struct timeval *value)
{
	unsigned long sec = value->tv_sec;
	long usec = value->tv_usec;

	if (sec >= MAX_SEC_IN_JIFFIES){
		sec = MAX_SEC_IN_JIFFIES;
		usec = 0;
	}
	return (((u64)sec * SEC_CONVERSION) +
		(((u64)usec * USEC_CONVERSION + USEC_ROUND) >>
		 (USEC_JIFFIE_SC - SEC_JIFFIE_SC))) >> SEC_JIFFIE_SC;
}
EXPORT_SYMBOL(timeval_to_jiffies);

void jiffies_to_timeval(const unsigned long jiffies, struct timeval *value)
{
	u32 rem;

	value->tv_sec = div_u64_rem((u64)jiffies * TICK_NSEC,
				    NSEC_PER_SEC, &rem);
	value->tv_usec = rem / NSEC_PER_USEC;
}
EXPORT_SYMBOL(jiffies_to_timeval);

clock_t jiffies_to_clock_t(unsigned long x)
{
#if (TICK_NSEC % (NSEC_PER_SEC / USER_HZ)) == 0
# if HZ < USER_HZ
	return x * (USER_HZ / HZ);
# else
	return x / (HZ / USER_HZ);
# endif
#else
	return div_u64((u64)x * TICK_NSEC, NSEC_PER_SEC / USER_HZ);
#endif
}
EXPORT_SYMBOL(jiffies_to_clock_t);

unsigned long clock_t_to_jiffies(unsigned long x)
{
#if (HZ % USER_HZ)==0
	if (x >= ~0UL / (HZ / USER_HZ))
		return ~0UL;
	return x * (HZ / USER_HZ);
#else
	
	if (x >= ~0UL / HZ * USER_HZ)
		return ~0UL;

	
	return div_u64((u64)x * HZ, USER_HZ);
#endif
}
EXPORT_SYMBOL(clock_t_to_jiffies);

u64 jiffies_64_to_clock_t(u64 x)
{
#if (TICK_NSEC % (NSEC_PER_SEC / USER_HZ)) == 0
# if HZ < USER_HZ
	x = div_u64(x * USER_HZ, HZ);
# elif HZ > USER_HZ
	x = div_u64(x, HZ / USER_HZ);
# else
	
# endif
#else
	x = div_u64(x * TICK_NSEC, (NSEC_PER_SEC / USER_HZ));
#endif
	return x;
}
EXPORT_SYMBOL(jiffies_64_to_clock_t);

u64 nsec_to_clock_t(u64 x)
{
#if (NSEC_PER_SEC % USER_HZ) == 0
	return div_u64(x, NSEC_PER_SEC / USER_HZ);
#elif (USER_HZ % 512) == 0
	return div_u64(x * USER_HZ / 512, NSEC_PER_SEC / 512);
#else
	return div_u64(x * 9, (9ull * NSEC_PER_SEC + (USER_HZ / 2)) / USER_HZ);
#endif
}

u64 nsecs_to_jiffies64(u64 n)
{
#if (NSEC_PER_SEC % HZ) == 0
	
	return div_u64(n, NSEC_PER_SEC / HZ);
#elif (HZ % 512) == 0
	
	return div_u64(n * HZ / 512, NSEC_PER_SEC / 512);
#else
	return div_u64(n * 9, (9ull * NSEC_PER_SEC + HZ / 2) / HZ);
#endif
}

unsigned long nsecs_to_jiffies(u64 n)
{
	return (unsigned long)nsecs_to_jiffies64(n);
}

struct timespec timespec_add_safe(const struct timespec lhs,
				  const struct timespec rhs)
{
	struct timespec res;

	set_normalized_timespec(&res, lhs.tv_sec + rhs.tv_sec,
				lhs.tv_nsec + rhs.tv_nsec);

	if (res.tv_sec < lhs.tv_sec || res.tv_sec < rhs.tv_sec)
		res.tv_sec = TIME_T_MAX;

	return res;
}
