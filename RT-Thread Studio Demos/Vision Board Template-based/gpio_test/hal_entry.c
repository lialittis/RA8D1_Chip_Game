/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author        Notes
 * 2025-04-28    Tianchi        first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include "hal_data.h"

//#define LED_PIN    BSP_IO_PORT_01_PIN_02 /* Onboard LED pins */

// Port Control Reg Base
#define PORT1_BASE      0x40400020      // Port1 control base addr
#define PFS_BASE       0x40400800      // PFS register base addr

// PORT1 Registers
#define PORT1_PODR      (*(volatile uint32_t*)(PORT1_BASE + 0x00))  // Port Output Data Register
#define PORT1_PDR       (*(volatile uint32_t*)(PORT1_BASE + 0x04))  // Port Direction Register

// PFS Register Macro （Port m Pin n）
// PFS Register Addr = PFS_BASE + (port_num * 0x80) + (pin_num * 4)
#define PFS_PORT_PIN(port, pin)  (*(volatile uint32_t*)(PFS_BASE + (port)*0x80 + (pin)*4))
#define PFS_P1_2       PFS_PORT_PIN(1, 2)  // Port 1 Pin 2 PFS

/* Debug Macro ------------------------------------------------------------*/
#define DEBUG_PRINT_REG(reg, name) \
    rt_kprintf("[DEBUG] %s (0x%08X) = 0x%08X\n", name, (uint32_t)&reg, reg)


uint32_t read_control_reg(void)
{
    uint32_t result;
    __asm volatile ("MRS %0, CONTROL" : "=r" (result));
    return result;
}

uint32_t read_ipsr(void)
{
    uint32_t result;
    __asm volatile ("MRS %0, IPSR" : "=r" (result));
    return result;
}

uint32_t read_primask(void)
{
    uint32_t result;
    __asm volatile ("MRS %0, PRIMASK" : "=r" (result));
    return result;
}

void direct_gpio_test(void) {
    rt_kprintf("\n[STEP 1] Config GPIO...\n");

    R_BSP_PinAccessEnable();

    DEBUG_PRINT_REG(PFS_P1_2, "PFS_P1_2 (Before)");
    PFS_P1_2 = (PFS_P1_2 & ~0x0000001F) | 0x00000001;  // GPIO mode
    __DSB();
    DEBUG_PRINT_REG(PFS_P1_2, "PFS_P1_2 (After)");

    uint32_t pdr_val = PORT1_PDR;
    DEBUG_PRINT_REG(PORT1_PDR, "PORT1_PDR (Before)");
    PORT1_PDR = pdr_val | (1 << 2);  // Output Mode
    __DSB();
    DEBUG_PRINT_REG(PORT1_PDR, "PORT1_PDR (After)");

    uint32_t podr_val = PORT1_PODR;
    DEBUG_PRINT_REG(PORT1_PODR, "PORT1_PODR (Before)");
    PORT1_PODR = podr_val | (1 << 2); // LED UP
    __DSB();
    DEBUG_PRINT_REG(PORT1_PODR, "PORT1_PODR (After)");

    R_BSP_PinAccessDisable();
}

/**
 * @brief Main Entry
 */
void hal_entry(void)
{
    rt_kprintf("\nHello RT-Thread! This is Tianchi YU!\n");
    rt_kprintf("CONTROL register: 0x%08X\n", read_control_reg());
    rt_kprintf("IPSR: 0x%08X, PRIMASK: 0x%08X\n", read_ipsr(), read_primask());
    rt_kprintf("PORT1 Poor register: 0x%08X\n", PORT1_PODR);
    DEBUG_PRINT_REG(PORT1_PODR, "PORT1_PODR (Init)");

    direct_gpio_test();

    rt_kprintf("\n[STEP 2] Test LED Blink...\n");


    while (1)
    {
//        rt_pin_write(LED_PIN, PIN_HIGH);
//        rt_thread_mdelay(1000);
//        rt_pin_write(LED_PIN, PIN_LOW);
//        rt_thread_mdelay(1000);

        PORT1_PODR ^= (1 << 2);  // Switch LED Status
        __DSB();
        DEBUG_PRINT_REG(PORT1_PODR, "PORT1_PODR (Current)");
        rt_thread_mdelay(1000);
    }
}
