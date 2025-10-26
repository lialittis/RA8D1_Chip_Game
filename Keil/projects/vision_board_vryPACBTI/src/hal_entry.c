/*
 * Copyright (c) 2006-2025
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2025-10-19     Tianchi YU        first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include "hal_data.h"
#include <stdint.h>

#define LED_PIN BSP_IO_PORT_01_PIN_02

/* ===== 1) BTI Test Target: Regular Function, the compiler will insert BTI at the entry ===== */
__attribute__((noinline))
static void bti_target(void)
{
    // Doesn't matter about the content
    __asm volatile ("nop");
}

/* ===== 2) PAC Test Target: use branch_protection("pac-ret+bti") to make sure BTI/PAC is used, not necessary ===== */
__attribute__((noinline, branch_protection("pac-ret+bti"))) // if doesn't work, replace it with "pac-ret+bti+leaf"
static void pac_victim(void)
{
    rt_kprintf("[PAC] in pac_victim()\n");
    volatile int sink = 0;
    sink++;
    (void)sink;
}

/* ===== BTI Test ===== */
static void test_bti(void)
{

	uintptr_t entry = (uintptr_t)bti_target;

    rt_kprintf("[BTI] OK indirect call (entry) -> should return\n");
    ((void (*)(void))entry)();
    rt_kprintf("[BTI] OK call returned\n");

    rt_kprintf("[BTI] BAD indirect call (entry+2) -> expect Fault\n");
    ((void (*)(void))(entry + 2))();
}

/* ===== PAC Test ===== */
__attribute__((noinline, branch_protection("pac-ret+bti"))) // to debug, could add `optimize("O0")` as one parameter as well
static void test_pac(void)
{
    rt_kprintf("[PAC] tamper saved LR slot -> expect Fault on return\n");
    pac_victim();

    __asm volatile ("" ::: "memory");   // Avoid re-order

    register uintptr_t spv __asm("sp"); // Get current SP(=sp0-16)
    uintptr_t *lr_slot = (uintptr_t *)(spv + 12); // = sp0-4, will POP to LR slot

    rt_kprintf("[PAC] lr_slot=%p old=0x%08lx\n",
               (void*)lr_slot, (unsigned long)*lr_slot);

    *lr_slot ^= 0x10;                   // Break slightly the LR (mismatch the signature)
    __asm volatile ("" ::: "memory");   // Return, then AUT will lead to Fault
}

/* ===== main ===== */
void hal_entry(void)
{
    rt_kprintf("\n== PAC/BTI minimal self-test ==\n");

    // Tests
    //test_bti();     // uncomment this line to test BTI
    test_pac();     // uncomment this line to test PAC
    rt_kprintf("[WARN] No Faults observed. Check compiler flags.\n");

    while (1)
    {
        rt_pin_write(LED_PIN, PIN_HIGH);
        rt_thread_mdelay(300);
        rt_pin_write(LED_PIN, PIN_LOW);
        rt_thread_mdelay(300);
    }
}
