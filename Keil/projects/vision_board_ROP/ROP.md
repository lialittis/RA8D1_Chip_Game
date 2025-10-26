```c
/*
 * ROP Attack 返回地址改写详解：
 * 
 * 正常情况下的栈布局（vulnerable_function被调用时）：
 * 
 * 高地址
 * ┌─────────────────────────┐
 * │  demo_rop_attack的栈帧  │
 * ├─────────────────────────┤
 * │  返回地址（LR）         │  ← 应该返回到demo_rop_attack中调用vulnerable_function之后的位置
 * ├─────────────────────────┤
 * │  保存的寄存器           │
 * ├─────────────────────────┤
 * │  buffer[0-15]           │  ← buffer的起始位置
 * │  (16 bytes)             │
 * └─────────────────────────┘
 * 低地址
 * 
 * 
 * ROP攻击后的栈布局：
 * 
 * 高地址
 * ┌─────────────────────────┐
 * │  demo_rop_attack的栈帧  │
 * ├─────────────────────────┤
 * │  gadget_set_flag地址    │  ← 被攻击者改写！原本应该是返回到demo_rop_attack
 * ├─────────────────────────┤  ← 现在返回时会跳转到gadget_set_flag
 * │  gadget_leak_secret地址 │  ← 当gadget_set_flag执行完return时，会跳转到这里
 * ├─────────────────────────┤
 * │  gadget_dangerous_op地址│  ← 继续链式执行
 * ├─────────────────────────┤
 * │  normal_function地址    │  ← 最后跳转到这里（可选）
 * ├─────────────────────────┤
 * │  AAAAAAAAAAAAAAAA...    │  ← strcpy溢出的数据，覆盖了buffer和返回地址
 * └─────────────────────────┘
 * 低地址
 * 
 * 
 * 详细步骤：
 * 
 * 1. 原始返回地址是什么？
 *    - 是 demo_rop_attack 函数中调用 vulnerable_function() 之后的下一条指令地址
 *    - 例如：0x08001234（假设的地址）
 * 
 * 2. 应该改写成什么地址？
 *    - 改写成第一个gadget的地址，例如 gadget_set_flag 的地址
 *    - 例如：0x08001100（假设的gadget_set_flag地址）
 * 
 * 3. ROP链的执行流程：
 *    a) vulnerable_function 执行 strcpy，溢出覆盖返回地址
 *    b) vulnerable_function 执行 return 指令
 *    c) CPU从栈上pop返回地址到PC → 跳转到 gadget_set_flag
 *    d) gadget_set_flag 执行完毕，执行 return
 *    e) CPU再次pop栈上的下一个地址 → 跳转到 gadget_leak_secret
 *    f) 以此类推，形成gadget链
 * 
 * 4. 为什么PAC能防御？
 *    - 正常情况：demo_rop_attack调用vulnerable_function时，LR被签名
 *      签名后的LR = sign(返回地址, SP, 密钥)
 *    - 攻击时：strcpy覆盖的是未签名的地址
 *    - 返回时：AUTIASP指令验证签名失败 → 触发异常
 */

// 更新注释以反映实际的攻击流程：
    /*
     * ROP Attack Explanation:
     * 
     * 1. Attacker overflows the buffer in vulnerable_function
     * 2. The overflow overwrites the saved return address on the stack
     *    - 原本的返回地址：应该返回到 demo_rop_attack 中的下一条指令
     *    - 被改写成：gadget_set_flag 的地址（第一个ROP gadget）
     * 3. Instead of returning to demo_rop_attack, execution jumps to gadget_set_flag
     * 4. After gadget_set_flag returns, stack上的下一个地址是 gadget_leak_secret
     * 5. This creates a "ROP chain" executing unauthorized operations:
     *    vulnerable_function → gadget_set_flag → gadget_leak_secret → gadget_dangerous_op
     * 
     * Without PAC: The CPU has no way to verify if the return address
     *              has been tampered with, so the attack succeeds.
     * 
     * With PAC:    The return address would be signed (authenticated),
     *              and tampering would cause an authentication fault.
     */
```