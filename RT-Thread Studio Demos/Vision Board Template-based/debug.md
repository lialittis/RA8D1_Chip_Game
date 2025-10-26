## Debug

```shell
0000909 W Overlapping memory regions in file packs\Keil.STM32L4xx_DFP.2.3.0-small.pack (STM32L412C8Tx); deleting outer region. Further warnings will be suppressed for this file. [cmsis_pack]
0001243 I Target type is r7fa8d1bh [board]
0002027 I DP IDR = 0x6ba02477 (v2 rev6) [dap]
0002036 I AHB-AP#0 IDR = 0x84770001 (AHB-AP var0 rev8) [discovery]
0002041 I APB-AP#1 IDR = 0x54770002 (APB-AP var0 rev5) [discovery]
0002045 I AHB-AP#0 Class 0x1 ROM table #0 @ 0xe00fe000 (designer=423 part=03b) [rom_table]
0002048 I [0]<e00ff000:ROM class=1 designer=43b:Arm part=4d4> [rom_table]
0002048 I   AHB-AP#0 Class 0x1 ROM table #1 @ 0xe00ff000 (designer=43b:Arm part=4d4) [rom_table]
0002052 I   [0]<e000e000:SCS M85 class=9 designer=43b:Arm part=d23 devtype=00 archid=2a04 devid=0:0:0> [rom_table]
0002055 I   [1]<e0001000:DWT M85 class=9 designer=43b:Arm part=d23 devtype=00 archid=1a02 devid=0:0:0> [rom_table]
0002057 I   [2]<e0002000:BPU M85 class=9 designer=43b:Arm part=d23 devtype=00 archid=1a03 devid=0:0:0> [rom_table]
0002059 I   [3]<e0000000:ITM M85 class=9 designer=43b:Arm part=d23 devtype=43 archid=1a01 devid=0:0:0> [rom_table]
0002062 I   [5]<e0041000:ETM M85 class=9 designer=43b:Arm part=d23 devtype=13 archid=4a13 devid=0:0:0> [rom_table]
0002064 I   [6]<e0003000:PMU M85 class=9 designer=43b:Arm part=d23 devtype=16 archid=0a06 devid=0:0:0> [rom_table]
0002066 I   [7]<e0042000:CTI M85 class=9 designer=43b:Arm part=d23 devtype=14 archid=1a14 devid=40800:0:0> [rom_table]
0002070 I [1]<e0040000:TPIU M85 class=9 designer=43b:Arm part=d23 devtype=11 archid=0000 devid=ca1:0:0> [rom_table]
0002072 I APB-AP#1 Class 0x1 ROM table #0 @ 0x80010000 (designer=423 part=03b) [rom_table]
0002075 I [0]<80011000:??? class=15 designer=423 part=004> [rom_table]
0002078 I [1]<80012000:CTI CS-400 class=9 designer=43b:Arm part=906 devtype=14 archid=0000 devid=40800:0:0> [rom_table]
0002080 I [2]<80013000:Trace Funnel CS-400 class=9 designer=43b:Arm part=908 devtype=12 archid=0000 devid=32:0:0> [rom_table]
0002082 I [3]<80014000:??? class=9 designer=43b:Arm part=961 devtype=21 archid=0000 devid=300:0:0> [rom_table]
0002084 I [4]<80015000:TSGEN class=15 designer=43b:Arm part=101> [rom_table]
0002094 I CPU core #0: Cortex-M85 r0p2, v8.1-M architecture [cortex_m]
0002094 I   Extensions: [DSP, FPU, FPU_DP, FPU_HP, FPU_V5, MPU, MVE, MVE_FP, PACBTI, PMU, RAS, SEC, SEC_V81, UDE] [cortex_m]
0002094 I   FPU present: FPv5-D16-M [cortex_m]
0002095 I Setting core #0 (Cortex-M85) default reset sequence to ResetSystem [pack_target]
0002096 I 8 hardware watchpoints [dwt]
0002103 I 8 hardware breakpoints, 1 literal comparators [fpb]
0002125 I Semihost server started on port 4444 (core 0) [server]
0002158 I GDB server started on port 3333 (core 0) [gdbserver]
0002956 I Client connected to port 3333! [gdbserver]
0003123 E Error while executing remote command 'reset init': unexpected value for reset_type option ('init') [gdbserver]
0003126 I Semihosting enabled [gdbserver]
0003134 I Attempting to load RTOS plugins [gdbserver]
[---|---|---|---|---|---|---|---|---|----]
[========================================]
0016130 I Erased 65536 bytes (8 sectors), programmed 65536 bytes (512 pages), skipped 256 bytes (16 pages) at 4.96 kB/s [loader]
```


### Core Info

Arm Cortex-M85 r0p2 (ARMv8.1-M)

- DSP
- FPU
- MVE/MVE-FP (Helium SIMD Extension)
- PACBTI (Pointer Authentication / Branch Target Identification)
- RAS (Reliability, Availability, Serviceability)
- SEC/SEC_V81 (TrustZone)


