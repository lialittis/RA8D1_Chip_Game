恭喜编过啦！要“验证 PAC/BTI 是否真的生效”，可以从**编译期、二进制/反汇编、运行时**三层做确认。下面给你一套一步到位的做法（针对 Keil MDK + Arm Compiler 6 / armclang / Cortex-M85）：

# 1) 编译期：确保开关真的开了

在 Keil：**Options for Target… → Target**

* **Device** 选 Cortex-M85（或等价 CPU），确保是 **ARMv8.1-M Mainline**。
* **C/C++ (AC6)** → **Misc Controls** 里加（两条都加最稳）：

```
-mcpu=cortex-m85+pacbti
-mbranch-protection=pac-ret+bti
```

> 说明：
>
> * `+pacbti` 打开 M-profile 的 PAC/BTI 指令集支持；
> * `-mbranch-protection=pac-ret+bti` 让编译器在函数边界插入 BTI，并给返回地址做签名校验（PAC-RET）。

**快速自检（预处理宏）**
在任意 C 文件加一段（只为看到编译信息，编好后可删）：

```c
#if defined(__ARM_FEATURE_PAC_DEFAULT)
#warning "PAC default enabled"
#endif
#if defined(__ARM_FEATURE_BTI_DEFAULT)
#warning "BTI default enabled"
#endif
```

如果编译输出里能看到这两个 warning，就说明编译配置到位。

# 2) 二进制/反汇编：肉眼看得到 PAC/BTI 指令

用 Arm 的 fromelf 反汇编你的 `.\Objects\rtthread.axf`：

```
fromelf --text -c --output=rtthread.lst .\Objects\rtthread.axf
```

打开 `rtthread.lst` 搜索这些指令/标记（出现任意几个都算成功）：

* `BTI`（函数入口处的分支目标标识）
* `PAC` / `AUT` / `XPACLRI`（M-profile 的指针签名/认证/清除指令；不同版本工具链可能显示为 `PAC*`/`AUT*` 风格）
* 也常能看到编译器在**函数序言/尾声**处插入相应序列（BTI 通常在入口第一条）。

如果你习惯用 Keil 自带的反汇编窗口，也可以直接在 IDE 里点开任意函数，看首条是否是 `BTI`。

# 3) 运行时：做两个“小破坏”冒烟测试

> 注意：放在非优化或 `-O1` 也能观察到，出现错误预期是 **UsageFault/HardFault**（认证失败）。

**(A) 验证 PAC-RET：篡改返回地址应触发 Fault**

```c
__attribute__((noinline, naked))
void tamper_lr(void) {
  __asm volatile(
    "push {r0}\n"          // 保存现场
    "ldr r0, [sp, #4]\n"   // 取出栈上的 LR
    "adds r0, #4\n"        // 人为篡改返回地址
    "str r0, [sp, #4]\n"   // 写回
    "pop {r0}\n"
    "bx lr\n"              // 返回 —— 若启用 PAC-RET，认证失败将触发 Fault
  );
}

void demo_pac(void){
  tamper_lr();   // 期望进入 HardFault/UsageFault
}
```

在 `HardFault_Handler` 或 `UsageFault_Handler` 里打断点/打印，即可看到命中。

**(B) 验证 BTI：跳到“非落脚点”应触发 Fault**
构造一个函数，取其中**非入口**位置的标签，作为函数指针跳转目标（即“非 BTI 落脚点”）：

```c
typedef void (*fn_t)(void);

__attribute__((noinline))
void landing(void){
  __asm volatile (".align 2\n"
                  "mid_label:\n"   // 中间标签（非函数入口）
                  :::);
  __NOP(); __NOP();
}

void demo_bti(void){
  extern void mid_label(void);      // 声明上面的中间标签
  fn_t bad = (fn_t)&mid_label;      // 形成“非入口”目标
  bad();                            // 若启用 BTI，这里应触发 Fault
}
```

若 BTI 生效，直接跳到 `mid_label` 会被拦截。

# 4) 常见踩坑 & 排查

* **只选了 Cortex-M85，但没加 `+pacbti` / `-mbranch-protection`**
  → 能编译指令集，但不会自动插桩；反汇编看不到 BTI，运行时也不会报错。
* **LTO/高优化把测试样例优化没了**
  → 给测试函数加 `__attribute__((noinline))` 或把优化调低一点。
* **混用了 AC5/旧汇编语法**
  → 确认用的是 **Arm Compiler 6 (armclang)**，并且 fromelf 是同一套工具。
* **RTOS/中断封装影响观察**
  → 先在最小裸机工程里做冒烟测试，再迁回 RT-Thread。

# 5) 额外确认（可选，进阶）

* **检查链接映射/函数前缀**：某些库函数也会带 BTI，能在 `rtthread.lst` 里看到入口的 BTI。
* **编译器报告**：在 Keil 打开 “Generate MISRA/Build Log” 级别更详细的编译输出，确认 `-mbranch-protection` 已被传递。
* **单步验证**：在函数入口单步，看到第一条就是 `BTI`；在返回前单步，能看到返回地址认证序列（PAC/AUT/XPACLRI）。

如果你愿意，把 `rtthread.lst` 的一小段（任意 2-3 个函数的反汇编头部）贴我，我直接帮你判读是否插入了 BTI/PAC 序列。
