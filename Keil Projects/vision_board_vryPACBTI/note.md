
== PAC/BTI minimal self-test ==
[BTI] OK indirect call (entry) -> should return
[BTI] OK call returned
[BTI] BAD indirect call (entry+2) -> expect Fault
psr: 0x01100000
r00: 0x02002ca3
r01: 0xc0010000
r02: 0x02005631
r03: 0x00000032
r04: 0x02002ca1
r05: 0xdeadbeef
r06: 0xdeadbeef
r07: 0xdeadbeef
r08: 0xdeadbeef
r09: 0xdeadbeef
r10: 0xdeadbeef
r11: 0xdeadbeef
r12: 0x00000000
 lr: 0x02003759
 pc: 0x0200a2fc
hard fault on thread: main

rt_thread_ thread       pri  status      sp     stack size max used left tick  error
---------- ------------ ---  ------- ---------- ----------  ------  ---------- ---
0x220078e8 tshell        20  ready   0x00000044 0x00001000    01%   0x0000000a OK
0x22000794 tidle0        31  ready   0x00000044 0x00000100    26%   0x00000020 OK
0x22000264 timer          4  suspend 0x00000044 0x00000200    13%   0x00000009 OK
0x22006df0 main          10  running 0x00000044 0x00000800    12%   0x00000005 OK
FPU active!
bus fault:
SCB_CFSR_BFSR:0x04 IMPRECISERR



PAC

== PAC/BTI minimal self-test ==
[PAC] tamper saved LR slot -> expect Fault on return
[PAC] in pac_victim()
[PAC] lr_slot=0xc0010000 old=0x02005769
psr: 0x21100000
r00: 0x0000005d
r01: 0x00280000
r02: 0x02005769
r03: 0x00000028
r04: 0xdeadbeef
r05: 0xdeadbeef
r06: 0xdeadbeef
r07: 0xdeadbeef
r08: 0xdeadbeef
r09: 0xdeadbeef
r10: 0xdeadbeef
r11: 0xdeadbeef
r12: 0x00000000
 lr: 0x0200a597
 pc: 0x0200a59e
hard fault on thread: main

rt_thread_ thread       pri  status      sp     stack size max used left tick  error
---------- ------------ ---  ------- ---------- ----------  ------  ---------- ---
0x220078e8 tshell        20  ready   0x00000044 0x00001000    01%   0x0000000a OK
0x22000794 tidle0        31  ready   0x00000044 0x00000100    26%   0x00000020 OK
0x22000264 timer          4  suspend 0x00000054 0x00000200    16%   0x00000009 OK
0x22006df0 main          10  running 0x00000044 0x00000800    14%   0x00000006 OK
FPU active!
bus fault:
SCB_CFSR_BFSR:0x04 IMPRECISERR
usage fault:
SCB_CFSR_UFSR:0x100 UNALIGNED