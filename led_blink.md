# Configurable LED blink count & delay

Enter a blink count, a maximum brightness (typically 255), and a delay (in tenths of a millisecond) between blinks.

```
00 0E ; ENT R0 (count)
01 1E ; ENT R1 (max brightness)
02 2E ; ENT R2 (delay)
03 3C ; CLR R3
04 8B ; JZ 0B
05 0D ; LOOP: DEC R0
06 1B ; BRI R1 (LED on)
07 2F ; DLY R2
08 3B ; BRI R3 (LED off)
09 2F ; DLY R2
0A C5 ; JNZ 5
0B 8B ; JZ 0B (halt)
```