/* Copyright (c) 2009-2012, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _ARCH_ARM_MACH_MSM_SOCINFO_H_
#define _ARCH_ARM_MACH_MSM_SOCINFO_H_

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/of_fdt.h>
#include <linux/of.h>

#include <asm/cputype.h>
#include <asm/mach-types.h>
#define SOCINFO_VERSION_MAJOR(ver) ((ver & 0xffff0000) >> 16)
#define SOCINFO_VERSION_MINOR(ver) (ver & 0x0000ffff)

#ifdef CONFIG_OF
#define early_machine_is_msm8974()	\
	of_flat_dt_is_compatible(of_get_flat_dt_root(), "qcom,msm8974")
#define machine_is_msm8974()		\
	of_machine_is_compatible("qcom,msm8974")
#define machine_is_msm8974_sim()		\
	of_machine_is_compatible("qcom,msm8974-sim")
#define machine_is_msm8974_rumi()	\
	of_machine_is_compatible("qcom,msm8974-rumi")
#define early_machine_is_msm9625()	\
	of_flat_dt_is_compatible(of_get_flat_dt_root(), "qcom,msm9625")
#define machine_is_msm9625()		\
	of_machine_is_compatible("qcom,msm9625")
#else
#define early_machine_is_msm8974()	0
#define machine_is_msm8974()		0
#define machine_is_msm8974_sim()	0
#define machine_is_msm8974_rumi()	0
#define early_machine_is_msm9625()	0
#define machine_is_msm9625()		0
#endif

#define PLATFORM_SUBTYPE_SGLTE	6
#define PLATFORM_SUBTYPE_DSDA	7

enum msm_cpu {
	MSM_CPU_UNKNOWN = 0,
	MSM_CPU_7X01,
	MSM_CPU_7X25,
	MSM_CPU_7X27,
	MSM_CPU_8X50,
	MSM_CPU_8X50A,
	MSM_CPU_7X30,
	MSM_CPU_8X55,
	MSM_CPU_8X60,
	MSM_CPU_8960,
	MSM_CPU_8960AB,
	MSM_CPU_7X27A,
	FSM_CPU_9XXX,
	MSM_CPU_7X25A,
	MSM_CPU_7X25AA,
	MSM_CPU_7X25AB,
	MSM_CPU_8064,
	MSM_CPU_8064AB,
	MSM_CPU_8930,
	MSM_CPU_8930AA,
	MSM_CPU_7X27AA,
	MSM_CPU_9615,
	MSM_CPU_8974,
	MSM_CPU_8627,
	MSM_CPU_8625,
	MSM_CPU_9625
};

enum pmic_model {
	PMIC_MODEL_PM8058	= 13,
	PMIC_MODEL_PM8028	= 14,
	PMIC_MODEL_PM8901	= 15,
	PMIC_MODEL_PM8027	= 16,
	PMIC_MODEL_ISL_9519	= 17,
	PMIC_MODEL_PM8921	= 18,
	PMIC_MODEL_PM8018	= 19,
	PMIC_MODEL_PM8015	= 20,
	PMIC_MODEL_PM8014	= 21,
	PMIC_MODEL_PM8821	= 22,
	PMIC_MODEL_PM8038	= 23,
	PMIC_MODEL_PM8922	= 24,
	PMIC_MODEL_PM8917	= 25,
	PMIC_MODEL_UNKNOWN	= 0xFFFFFFFF
};

enum msm_cpu socinfo_get_msm_cpu(void);
uint32_t socinfo_get_id(void);
uint32_t socinfo_get_version(void);
uint32_t socinfo_get_raw_id(void);
char *socinfo_get_build_id(void);
uint32_t socinfo_get_platform_type(void);
uint32_t socinfo_get_platform_subtype(void);
uint32_t socinfo_get_platform_version(void);
enum pmic_model socinfo_get_pmic_model(void);
uint32_t socinfo_get_pmic_die_revision(void);
int __init socinfo_init(void) __must_check;
const int read_msm_cpu_type(void);
const int get_core_count(void);
const int cpu_is_krait(void);
const int cpu_is_krait_v1(void);
const int cpu_is_krait_v2(void);
const int cpu_is_krait_v3(void);

static inline int cpu_is_msm7x01(void)
{
#ifdef CONFIG_ARCH_MSM7X01A
	enum msm_cpu cpu = socinfo_get_msm_cpu();

	BUG_ON(cpu == MSM_CPU_UNKNOWN);
	return cpu == MSM_CPU_7X01;
#else
	return 0;
#endif
}

static inline int cpu_is_msm7x25(void)
{
#ifdef CONFIG_ARCH_MSM7X25
	enum msm_cpu cpu = socinfo_get_msm_cpu();

	BUG_ON(cpu == MSM_CPU_UNKNOWN);
	return cpu == MSM_CPU_7X25;
#else
	return 0;
#endif
}

static inline int cpu_is_msm7x27(void)
{
#if defined(CONFIG_ARCH_MSM7X27) && !defined(CONFIG_ARCH_MSM7X27A)
	enum msm_cpu cpu = socinfo_get_msm_cpu();

	BUG_ON(cpu == MSM_CPU_UNKNOWN);
	return cpu == MSM_CPU_7X27;
#else
	return 0;
#endif
}

static inline int cpu_is_msm7x27a(void)
{
#ifdef CONFIG_ARCH_MSM7X27A
	enum msm_cpu cpu = socinfo_get_msm_cpu();

	BUG_ON(cpu == MSM_CPU_UNKNOWN);
	return cpu == MSM_CPU_7X27A;
#else
	return 0;
#endif
}

static inline int cpu_is_msm7x27aa(void)
{
#ifdef CONFIG_ARCH_MSM7X27A
	enum msm_cpu cpu = socinfo_get_msm_cpu();

	BUG_ON(cpu == MSM_CPU_UNKNOWN);
	return cpu == MSM_CPU_7X27AA;
#else
	return 0;
#endif
}

static inline int cpu_is_msm7x25a(void)
{
#ifdef CONFIG_ARCH_MSM7X27A
	enum msm_cpu cpu = socinfo_get_msm_cpu();

	BUG_ON(cpu == MSM_CPU_UNKNOWN);
	return cpu == MSM_CPU_7X25A;
#else
	return 0;
#endif
}

static inline int cpu_is_msm7x25aa(void)
{
#ifdef CONFIG_ARCH_MSM7X27A
	enum msm_cpu cpu = socinfo_get_msm_cpu();

	BUG_ON(cpu == MSM_CPU_UNKNOWN);
	return cpu == MSM_CPU_7X25AA;
#else
	return 0;
#endif
}

static inline int cpu_is_msm7x25ab(void)
{
#ifdef CONFIG_ARCH_MSM7X27A
	enum msm_cpu cpu = socinfo_get_msm_cpu();

	BUG_ON(cpu == MSM_CPU_UNKNOWN);
	return cpu == MSM_CPU_7X25AB;
#else
	return 0;
#endif
}

static inline int cpu_is_msm7x30(void)
{
#ifdef CONFIG_ARCH_MSM7X30
	enum msm_cpu cpu = socinfo_get_msm_cpu();

	BUG_ON(cpu == MSM_CPU_UNKNOWN);
	return cpu == MSM_CPU_7X30;
#else
	return 0;
#endif
}

static inline int cpu_is_qsd8x50(void)
{
#ifdef CONFIG_ARCH_QSD8X50
	enum msm_cpu cpu = socinfo_get_msm_cpu();

	BUG_ON(cpu == MSM_CPU_UNKNOWN);
	return cpu == MSM_CPU_8X50;
#else
	return 0;
#endif
}

static inline int cpu_is_msm8x55(void)
{
#ifdef CONFIG_ARCH_MSM7X30
	enum msm_cpu cpu = socinfo_get_msm_cpu();

	BUG_ON(cpu == MSM_CPU_UNKNOWN);
	return cpu == MSM_CPU_8X55;
#else
	return 0;
#endif
}

static inline int cpu_is_msm8x60(void)
{
#ifdef CONFIG_ARCH_MSM8X60
	return read_msm_cpu_type() == MSM_CPU_8X60;
#else
	return 0;
#endif
}

static inline int cpu_is_msm8960(void)
{
#ifdef CONFIG_ARCH_MSM8960
	return read_msm_cpu_type() == MSM_CPU_8960;
#else
	return 0;
#endif
}

static inline int cpu_is_msm8960ab(void)
{
#ifdef CONFIG_ARCH_MSM8960
	return read_msm_cpu_type() == MSM_CPU_8960AB;
#else
	return 0;
#endif
}

static inline int cpu_is_apq8064(void)
{
#ifdef CONFIG_ARCH_APQ8064
	return read_msm_cpu_type() == MSM_CPU_8064;
#else
	return 0;
#endif
}

static inline int cpu_is_apq8064ab(void)
{
#ifdef CONFIG_ARCH_APQ8064
	return read_msm_cpu_type() == MSM_CPU_8064AB;
#else
	return 0;
#endif
}

static inline int cpu_is_msm8930(void)
{
#ifdef CONFIG_ARCH_MSM8930
	return read_msm_cpu_type() == MSM_CPU_8930;
#else
	return 0;
#endif
}

static inline int cpu_is_msm8930aa(void)
{
#ifdef CONFIG_ARCH_MSM8930
	return read_msm_cpu_type() == MSM_CPU_8930AA;
#else
	return 0;
#endif
}

static inline int cpu_is_msm8627(void)
{
#ifdef CONFIG_ARCH_MSM8930
	return read_msm_cpu_type() == MSM_CPU_8627;
#else
	return 0;
#endif
}

static inline int cpu_is_fsm9xxx(void)
{
#ifdef CONFIG_ARCH_FSM9XXX
	enum msm_cpu cpu = socinfo_get_msm_cpu();

	BUG_ON(cpu == MSM_CPU_UNKNOWN);
	return cpu == FSM_CPU_9XXX;
#else
	return 0;
#endif
}

static inline int cpu_is_msm9615(void)
{
#ifdef CONFIG_ARCH_MSM9615
	enum msm_cpu cpu = socinfo_get_msm_cpu();

	BUG_ON(cpu == MSM_CPU_UNKNOWN);
	return cpu == MSM_CPU_9615;
#else
	return 0;
#endif
}

static inline int cpu_is_msm8625(void)
{
#ifdef CONFIG_ARCH_MSM8625
	enum msm_cpu cpu = socinfo_get_msm_cpu();

	BUG_ON(cpu == MSM_CPU_UNKNOWN);
	return cpu == MSM_CPU_8625;
#else
	return 0;
#endif
}

#endif
