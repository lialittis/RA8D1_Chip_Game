## Normal

![alt text](image-2.png)


## ROP

![alt text](image.png)

![alt text](image-1.png)

![alt text](image-3.png)

0x020033EF - > LR
Memory:
0x22007678： AAAAAAA...

Entre func:
LR: 0x020033F5
R4: 0x22007678
R5: 0x220005c8
R7: Deadbeef
SP: 0x22007678

PUSH 后：
SP: 0x22007668

![alt text](image-4.png)

copy 后：

![alt text](image-5.png)


Attack：

[WARNING] Launching ROP attack...

[SIMULATION] Executing ROP chain:
[VULN] Entering vulnerable_function
[VULN] Buffer address: 0x22007658
[VULN] Input length: 30
[VULN] Buffer content: AAAAAAAAAAAAAAAAAAAAAAAAAAAA¡1
[VULN] Returning from vulnerable_function
[GADGET] Setting privileged flag!
[GADGET] Setting privileged flag!
[GADGET] Executing dangerous operation!
psr: 0x60100000
r00: 0x220005c8
r01: 0xdeadbeef
r02: 0x020051b1
r03: 0x00000028
r04: 0x41414141
r05: 0x41414141
r06: 0x0200317d
r07: 0x02004afd
r08: 0x0200ba50
r09: 0x22007678
r10: 0x00000000
r11: 0x220005c8
r12: 0x020021a1
 lr: 0x020031b5
 pc: 0x0200ba00
hard fault on thread: main

rt_thread_ thread       pri  status      sp     stack size max used left tick  error
---------- ------------ ---  ------- ---------- ----------  ------  ---------- ---
0x22007908 tshell        20  ready   0x00000044 0x00001000    01%   0x0000000a OK
0x22000798 tidle0        31  ready   0x00000044 0x00000100    26%   0x00000020 OK
0x22000264 timer          4  suspend 0x00000044 0x00000200    13%   0x00000009 OK
0x22006e10 main          10  running 0x00000044 0x00000800    13%   0x00000007 OK
FPU active!
usage fault:
SCB_CFSR_UFSR:0x02 INVSTATE


## ROP攻击关键观察点

### 断点1：调用vulnerable_function之前
**地址：0x020033F0** `BL vulnerable_function`

#### 关键寄存器：
1. **LR (Link Register)** - 最重要！
   - 值应该是：`0x020033F4`（BL指令后的下一条指令地址）
   - 这就是**正常的返回地址**
   - 这个值会被保存到栈上

2. **SP (Stack Pointer)**
   - 记录当前栈顶位置
   - 例如：`0x20000100`
   - 调用函数后，返回地址会被压入栈

3. **R0 (参数寄存器)**
   - 包含传递给`vulnerable_function`的参数（字符串指针）
   - 查看这个地址的内容，确认输入数据

4. **PC (Program Counter)**
   - 应该是：`0x020033F0`（当前指令地址）

### 断点2：进入vulnerable_function
**地址：0x02009502** `SUB sp,sp,#0x10`

#### 关键寄存器：
1. **SP (Stack Pointer)** - 观察栈的变化
   ```
   断点1的SP: 0x20000100 (假设值)
   执行PUSH后: 0x200000F0 (SP - 0x10 = 减少16字节)
   SUB sp,#0x10后: 0x200000E0 (再减少16字节，分配buffer空间)
   ```

2. **查看栈内存（重点！）**
   ```
   Memory Window - 查看SP指向的地址：
   
   地址          值            说明
   ───────────────────────────────────────────
   0x200000F0:  [buffer起始]   ← 16字节buffer空间
   0x200000F4:  [buffer+4]
   0x200000F8:  [buffer+8]
   0x200000FC:  [buffer+12]
   ───────────────────────────────────────────
   0x20000100:  0xXXXXXXXX    ← 保存的r4
   0x20000104:  0xXXXXXXXX    ← 保存的r5
   0x20000108:  0xXXXXXXXX    ← 保存的r7
   0x2000010C:  0x020033F4    ← 保存的LR（返回地址）⭐⭐⭐
   ```

### 完整的观察流程

#### Step 1: 断点1处 (调用前)
```
寄存器值：
LR  = 0x020033F4  ← 正常返回地址（记住这个值！）
SP  = 0x20000110  
R0  = 0x200XXXXX  ← 指向输入字符串
PC  = 0x020033F0

栈内存 (SP = 0x20000110):
[0x20000110]: ????????  ← 栈顶
[0x20000114]: ????????
...
```

#### Step 2: 断点2处 (PUSH执行后)
```
寄存器值：
SP  = 0x20000100  ← 减少了16字节 (PUSH {r4,r5,r7,lr})
PC  = 0x02009502

栈内存 (SP = 0x20000100):
[0x20000100]: 0xXXXXXXXX  ← r4
[0x20000104]: 0xXXXXXXXX  ← r5
[0x20000108]: 0xXXXXXXXX  ← r7
[0x2000010C]: 0x020033F4  ← LR (返回地址) ⭐⭐⭐
[0x20000110]: ????????    ← 之前的栈内容
```

#### Step 3: SUB sp,#0x10 执行后
```
寄存器值：
SP  = 0x200000F0  ← 再减少16字节，为buffer分配空间

栈内存布局：
┌──────────────────┐ 0x200000F0 ← SP (buffer起始)
│  buffer[0-3]     │
│  buffer[4-7]     │
│  buffer[8-11]    │
│  buffer[12-15]   │ 0x200000FC
├──────────────────┤ 0x20000100
│  saved r4        │
│  saved r5        │
│  saved r7        │
│  saved LR        │ 0x2000010C ← 0x020033F4 ⭐目标地址
└──────────────────┘
```

### ROP攻击的关键观察

#### 执行strcpy之前：
```
Memory at 0x2000010C: 0x020033F4  ← 正常返回地址
```

#### 执行strcpy之后（如果发生溢出）：
```
Memory at 0x2000010C: 0x02009XXX  ← 被覆盖成gadget地址！
```

### Keil调试器具体操作

1. **观察寄存器窗口**
   - View → Registers
   - 重点看：LR, SP, PC, R0-R3

2. **观察栈内存窗口**
   ```
   View → Memory Windows → Memory 1
   Address: 输入SP寄存器的值
   显示格式：选择 "4 Byte Hex"
   ```

3. **观察Call Stack窗口**
   ```
   View → Call Stack
   会显示：
   ├─ vulnerable_function  @ 0x02009502
   └─ demo_rop_attack      @ 0x020033F4  ← 这就是返回地址
   ```

4. **观察变量窗口**
   ```
   View → Watch Windows → Watch 1
   添加：
   - &buffer
   - input
   - strlen(input)
   ```

### 重点关注的内存地址计算

```c
// 根据您的断点信息：
buffer起始地址 = SP值（SUB sp,#0x10后）
             = 0x200000F0 (假设)

返回地址位置 = buffer起始地址 + 0x1C
           = 0x200000F0 + 28 = 0x2000010C

// 验证公式：
// buffer(16) + r4(4) + r5(4) + r7(4) + LR(4) = 32 bytes
// 所以 LR的位置 = buffer地址 + 16 + 12 = buffer + 28
```

### 实际测试建议

```c
// 在vulnerable_function中添加打印：
rt_kprintf("SP after PUSH: 0x%08X\n", __get_SP());
rt_kprintf("Buffer addr:   0x%08X\n", (uint32_t)buffer);
rt_kprintf("LR saved at:   0x%08X\n", (uint32_t)(__get_SP() + 0x1C));
rt_kprintf("LR value:      0x%08X\n", *(uint32_t*)(__get_SP() + 0x1C));
```

### 关键检查点

✅ **断点1处必须记录**：
- LR的值 = 0x020033F4（这是正常返回地址）

✅ **断点2处必须验证**：
- 栈上保存的LR值 = 0x020033F4（应该相同）
- Buffer地址 = SP的值

✅ **strcpy后必须检查**：
- 如果输入超过16字节，栈上的LR值是否被覆盖
- 被覆盖成什么值（应该是gadget地址）

