# Auto-decrement entered number

Enter a number and a delay (in tenths of a millisecond) between steps to watch the number auto-decrement to zero. 

```assembly
00 0E ; ENT R0 (number)
01 1E ; ENT R1 (delay)
02 0D ; LOOP: DEC R0
03 09 ; OUT R0
04 1F ; DLY R0
05 C2 ; JNZ 02
06 90 ; OUT R0 (output 0)
07 1F ; DLY R0
08 88 ; JZ 08 (halt)
```