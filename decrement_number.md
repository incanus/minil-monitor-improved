# Decrement entered number

Enter a number, then press Enter each time to watch it decrement to zero.

```
00 0E ; ENT R0
01 0D ; DEC R0
02 C0 ; JNZ 00
03 09 ; OUT R0 (output 0)
04 84 ; JZ 04  (halt)
```