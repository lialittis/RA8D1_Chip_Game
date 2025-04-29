/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author        Notes
 * 2025-04-29    Tianchi        first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include "hal_data.h"

void test_general_registers() {
    uint32_t val_r0, val_r1;

    __asm volatile (
        "mov r0, #0x1234\n"
        "mov %0, r0\n"
        "mov r1, #0x5678\n"
        "mov %1, r1\n"
        : "=r" (val_r0), "=r" (val_r1)
        :
        : "r0", "r1"
    );

    rt_kprintf("R0 = 0x%08X, R1 = 0x%08X\n", val_r0, val_r1);
}

void test_stack_pointer() {
    uint32_t original_sp, modified_sp;

    __asm volatile (
        "mov %0, sp\n"
        "sub sp, sp, #256\n"
        "mov %1, sp\n"
        "mov sp, %0\n"
        : "=r" (original_sp), "=r" (modified_sp)
        :
        : "sp"
    );

    rt_kprintf("Original SP = 0x%08X, Modified SP = 0x%08X\n", original_sp, modified_sp);
}

void test_stack_memory() {
    uint32_t buffer[4] = {0};
    uint32_t *ptr = buffer;

    ptr[0] = 0xDEADBEEF;
    ptr[1] = 0xCAFEBABE;

    rt_kprintf("Stack[0] = 0x%08X, Stack[1] = 0x%08X\n", ptr[0], ptr[1]);
}

void hal_entry(void) {
    rt_kprintf("General Registers Testing...\n");
    test_general_registers();
    rt_kprintf("Stack Register Testing...\n");
    test_stack_pointer();
    rt_kprintf("Stack Memory Testing...\n");
    test_stack_memory();
}
