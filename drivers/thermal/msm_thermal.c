/* Copyright (c) 2012, Code Aurora Forum. All rights reserved.
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

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/msm_tsens.h>
#include <linux/workqueue.h>
#include <linux/cpu.h>
#include <linux/cpufreq.h>
#include <linux/msm_thermal.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <mach/cpufreq.h>

/* Control interface
	-> temperatures in DegC
	-> CPU will throttle to set freqs at these temps */
#define DEFAULT_TEMP_MIN	45
#define DEFAULT_TEMP_MID	55
#define DEFAULT_TEMP_MAX	70

#define DEFAULT_TEMP_MIN_FREQ	1350000
#define DEFAULT_TEMP_MID_FREQ	918000
#define DEFAULT_TEMP_MAX_FREQ	486000

static unsigned int freq_debug = 0;
module_param_named(freq_limit_debug, freq_debug, uint, 0644);

#define dprintk(msg...)		\
do {				\
	if (freq_debug)		\
		pr_info(msg);	\
} while (0)

static unsigned int polling = HZ*2;
static unsigned int cpu = 0;

static unsigned int temp_max = DEFAULT_TEMP_MAX;
module_param(temp_max, int, 0644);
static unsigned int temp_mid = DEFAULT_TEMP_MIN;
module_param(temp_mid, int, 0644);
static unsigned int temp_min = DEFAULT_TEMP_MIN;
module_param(temp_min, int, 0644);

static unsigned int temp_max_freq = DEFAULT_TEMP_MAX_FREQ;
module_param(temp_max_freq, int, 0644);
static unsigned int temp_mid_freq = DEFAULT_TEMP_MIN_FREQ;
module_param(temp_mid_freq, int, 0644);
static unsigned int temp_min_freq = DEFAULT_TEMP_MIN_FREQ;
module_param(temp_min_freq, int, 0644);

static uint32_t freq_max;
static uint32_t freq_buffer;

static struct msm_thermal_data msm_thermal_info;
static struct delayed_work check_temp_work;

static void check_temp(struct work_struct *work)
{
	unsigned long temp = 0;
	struct tsens_device tsens_dev;

	if (freq_buffer == 0)
		freq_buffer = freq_max;

	tsens_dev.sensor_num = msm_thermal_info.sensor_id;
	tsens_get_temp(&tsens_dev, &temp);

	if (temp > temp_max) {
		freq_max = temp_max_freq;
		polling = HZ/8;

	} else if (temp > temp_mid) {
		freq_max = temp_mid_freq;
		polling = HZ/4;

	} else if (temp > temp_min) {
		freq_max = temp_min_freq;
		polling = HZ/2;

	} else if (temp > temp_min - 3) {
		polling = HZ;

	} else {
		polling = HZ*2;
	}

	if (freq_buffer != freq_max) {
		freq_buffer = freq_max;
		for_each_possible_cpu(cpu)
			msm_cpufreq_set_freq_limits(cpu, MSM_CPUFREQ_NO_LIMIT, freq_max);
		dprintk("msm_thermal: CPU temp: %luC, max: %dMHz, polling: %dms",
			temp, freq_max/1000, jiffies_to_msecs(polling));
	}

	schedule_delayed_work(&check_temp_work, polling);
}

int __devinit msm_thermal_init(struct msm_thermal_data *pdata)
{
	int ret = 0;

	BUG_ON(!pdata);
	BUG_ON(pdata->sensor_id >= TSENS_MAX_SENSORS);
	memcpy(&msm_thermal_info, pdata, sizeof(struct msm_thermal_data));

	dprintk("msm_thermal: Maximum cpu temp: %dC", temp_max);

	INIT_DELAYED_WORK(&check_temp_work, check_temp);
	schedule_delayed_work(&check_temp_work, HZ*20);

	return ret;
}

static int __devinit msm_thermal_dev_probe(struct platform_device *pdev)
{
	int ret = 0;
	char *key = NULL;
	struct device_node *node = pdev->dev.of_node;
	struct msm_thermal_data data;

	memset(&data, 0, sizeof(struct msm_thermal_data));
	key = "qcom,sensor-id";
	ret = of_property_read_u32(node, key, &data.sensor_id);
	if (ret)
		goto fail;
	WARN_ON(data.sensor_id >= TSENS_MAX_SENSORS);

fail:
	if (ret)
		pr_err("%s: Failed reading node=%s, key=%s\n",
		       __func__, node->full_name, key);
	else
		ret = msm_thermal_init(&data);

	return ret;
}

static struct of_device_id msm_thermal_match_table[] = {
	{.compatible = "qcom,msm-thermal"},
	{},
};

static struct platform_driver msm_thermal_device_driver = {
	.probe = msm_thermal_dev_probe,
	.driver = {
		.name = "msm-thermal",
		.owner = THIS_MODULE,
		.of_match_table = msm_thermal_match_table,
	},
};

int __init msm_thermal_device_init(void)
{
	return platform_driver_register(&msm_thermal_device_driver);
}
