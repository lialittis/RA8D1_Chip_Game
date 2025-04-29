/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author        Notes
 * 2025-04-29     Tianchi        first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include "hal_data.h"

void vulnerable_function() {
    char buffer[8];
    rt_kprintf("Input Data (will overflow buffer): ");
    rt_device_read(rt_console_get_device(), 0, buffer, 16);
    rt_kprintf("Buffer Output: %s\n", buffer);
}

void test_stack_overflow() {
    rt_kprintf("=== Mock Stack Buffer Overflow ===\n");
    vulnerable_function();
    rt_kprintf("=== No Exception ===\n");
}

void hal_entry(void) {
    test_stack_overflow();
}
