# MINIL Machine-Code Monitor, Improved

This is very nearly all based on David Johnson-Davies' excellent [original](http://www.technoblogy.com/show?NNJ) with some adaptations: 

- Meant to run on Arduino Nano instead of ATtiny85
- Ported to Arduino Keypad library use
- Ported to non-TinySPI 7-segment display (normal SPI)
- Added inline instruction set documentation
- Modified Enter button handling for board layout
- Added instruction delay to behave more like ATtiny85
- Added [0-7]9 register display instruction
- Added [0-7]F delay instruction

![](led_blink.gif)

## Parts

- [Arduino Nano](https://www.arduino.cc/en/pmwiki.php?n=Main/ArduinoBoardNano)
- [4x4 keypad](https://www.adafruit.com/product/3844)
- [7-segment display](https://www.sparkfun.com/products/11442)
- Button
- LED
- 330Ω resistor

## Layout

| Pin(s) | Function       |
| ------ | -------------- |
| D9-D2  | Keypad         |
| D10    | LED            |
| D11    | SPI MOSI       |
| D13    | SPI SCK        |
| A0     | Button GND     | 
| A2     | Button trigger |
| A3     | SPI CS         |