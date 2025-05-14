/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author        Notes
 * 2025-05-15     Tianchi        first version
 *
 * Conclusion: Checked for PAC support macros and instruction emission → ❌ none found
 */

#include <rtthread.h>
#include <rtdevice.h>
#include "hal_data.h"

int secret_key = 0x12345678;

__attribute__((noinline)) int compute_something(int a, int b)
{
    volatile int temp = a + b + secret_key;
    return temp * 3;
}

__attribute__((noinline)) int intermediate_function(int x)
{
    return compute_something(x, 5);
}

void feature_check_test(void)
{
#if defined(__ARM_FEATURE_PAC_DEFAULT)
    rt_kprintf("✅ PAC is supported (macro __ARM_FEATURE_PAC_DEFAULT is defined)\n");
#else
    rt_kprintf("❌ PAC is NOT supported (macro __ARM_FEATURE_PAC_DEFAULT is NOT defined)\n");
#endif

#if defined(__ARM_FEATURE_BTI_DEFAULT)
    rt_kprintf("✅ BTI is supported (macro __ARM_FEATURE_BTI_DEFAULT is defined)\n");
#else
    rt_kprintf("❌ BTI is NOT supported (macro __ARM_FEATURE_BTI_DEFAULT is NOT defined)\n");
#endif
}

void hal_entry(void)
{
    rt_kprintf("== RT-Thread PAC/BTI Feature Detection Test ==\n");
    feature_check_test();
    rt_kprintf("== RT-Thread PAC/BTI Test Function Execution ==\n");
    int result = intermediate_function(10);
    rt_kprintf("Computation result: %d\n", result);
}