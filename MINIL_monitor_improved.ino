/* MINIL Machine-Code Monitor

   Justin Miller - justinmiller.io - 4th October 2021
   
   Based on original by:
   David Johnson-Davies - www.technoblogy.com - 19th June 2014

   - Meant to run on Arduino Nano instead of ATtiny85
   - Ported to Arduino Keypad library use
   - Ported to non-TinySPI 7-segment display (normal SPI)
   - Added inline instruction set documentation
   - Modified Enter button handling for board layout
   - Added instruction delay to behave more like ATtiny85
   - Added [0-7]9 register display instruction
   - Added [0-7]F delay instruction
   
   CC BY 4.0
   Licensed under a Creative Commons Attribution 4.0 International license: 
   http://creativecommons.org/licenses/by/4.0/
*/

/* Instruction Set

   = Loads [0-7][0-7]
   45 R4 = R5
   07 R0 = R7

   = Jumps [JZ 80-BF, JNZ C0-FF]
   C3 JNZ 3
   84 JZ 4

   = Special [0-7][9-F]
   9 OUT Display register
   A ADD Add 1 to register
   B BRI Set LED brightness to register value (0-255)
   C CLR Clear register
   D DEC Decrement register
   E ENT Enter value to register
   F DLY Wait register value * 10 milliseconds
*/

// EEPROM library
#include <EEPROM.h>

// Matrix keypad setup
#include <Keypad.h>
const byte rows = 4;
const byte cols = 4;
char keys[rows][cols] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[rows] = { 5, 4, 3, 2 };
byte colPins[cols] = { 9, 8, 7, 6 };
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, rows, cols);

// Button & LED setup
const int ButtonGnd = A0;
const int ButtonTrg = A2;
const int LED = 10;

// Seven segment display setup
#include <SPI.h>
const int SegCS = A3;
const int Clear_Display = 0x76;
const int Decimal_Control = 0x77;
const int Cursor_Control = 0x79;
const int Brightness_Control = 0x7A;
                     
// MINIL setup
unsigned int Register[8];

void setup(void) {
  pinMode(ButtonGnd, OUTPUT);
  digitalWrite(ButtonGnd, LOW);
  pinMode(ButtonTrg, INPUT_PULLUP);

  pinMode(SegCS, OUTPUT);
  digitalWrite(SegCS, HIGH);
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV64);

  ClearDisplay();

  delay(1000);
}

// Display utilities
void DisplayBegin() {
  digitalWrite(SegCS, LOW);
}

void DisplayEnd() {
  digitalWrite(SegCS, HIGH);
}

// Clear display
void ClearDisplay() {
  DisplayBegin();
  SPI.transfer(Brightness_Control);
  delay(1);
  SPI.transfer(255);
  delay(1);
  SPI.transfer(Clear_Display);
  DisplayEnd();
}

// Display a 2 digit hex number
void DisplayTwo(int n, int offset) {
  DisplayBegin();
  SPI.transfer(Cursor_Control);
  delay(1);
  SPI.transfer(offset);
  DisplayEnd();
  DisplayBegin();
  SPI.transfer((n>>4) & 0xFF);
  delay(1);
  SPI.transfer(n & 0x0F);
  DisplayEnd();
}

// Display a four digit hex number
void Display(int n) {
  DisplayBegin();
  SPI.transfer(Decimal_Control);
  delay(1);
  SPI.transfer(0);
  delay(1);
  SPI.transfer(Cursor_Control);
  delay(1);
  SPI.transfer(0);
  delay(1);
  int i=16;
  do {
    i = i - 4;
    SPI.transfer((n>>i) & 0x0F);
    delay(1);
  } while (i != 0);
  DisplayEnd();
}

// Display colon
void DisplayColon() {
  DisplayBegin();
  SPI.transfer(Decimal_Control);
  delay(1);
  SPI.transfer(0x10);
  DisplayEnd();
}

// Display running
void DisplayRunning() {
  DisplayBegin();
  SPI.transfer(Clear_Display);
  delay(1);
  SPI.transfer(Decimal_Control);
  delay(1);
  SPI.transfer(0x0F);
  DisplayEnd();
}

// Display Go
void DisplayGo() {
  DisplayBegin();
  SPI.transfer(Cursor_Control);
  delay(1);
  SPI.transfer(0);
  delay(1);
  SPI.transfer(Decimal_Control);
  delay(1);
  SPI.transfer(0x10);
  delay(1);
  SPI.transfer('G');
  delay(1);
  SPI.transfer('o');
  delay(1);
  SPI.transfer(' ');
  delay(1);
  SPI.transfer(' ');
  DisplayEnd();
}

// Display Error
void DisplayError() {
  DisplayBegin();
  SPI.transfer(Cursor_Control);
  delay(1);
  SPI.transfer(0);
  DisplayEnd();
  DisplayBegin();
  SPI.transfer('E');
  delay(1);
  SPI.transfer('r');
  DisplayEnd();
}

// Returns the keypad character or -1 if no button pressed
char ReadKeypad() {
  char key = keypad.getKey();
  if (key != '\0') {
    int keyNum = key - '0';
    if (keyNum < 0 || keyNum > 9) {
      if (key == 'A') return 10;
      if (key == 'B') return 11;
      if (key == 'C') return 12;
      if (key == 'D') return 13;
      if (key == '*') return 14;
      if (key == '#') return 15;
    } else {
      return keyNum;
    }
  }
  return -1;
}

// Wait until release keypad key
void WaitReleaseKey() {
  do delay(100); while (ReadKeypad() != -1);
}

// Read button
boolean ReadButton() {
  return (digitalRead(ButtonTrg) == LOW); 
}

// Waits until button up, or 1 second
boolean LongPress() {
  long time = millis();
  do {
    delay(100);
    if (millis()-time > 1000) return true;
  } while (ReadButton());
  return false;
}

// Reads a specified number of key presses and display them
// Returns -1 immediately if button was pressed, or result
// Uses long so can return an unsigned int or -1
long GetData(int keys) {
  long Input = 0;
  int Nibble;
  boolean Press;
  for (int i=0; i<keys; i++) {
    Input = Input << 4;
    do {
      Nibble = ReadKeypad();
      if (ReadButton()) return -1;
    } while (Nibble < 0);
    DisplayBegin();
    if (i==0) {
      SPI.transfer(Cursor_Control);
      delay(1);
      SPI.transfer(4-keys);
      delay(1);
      for (int i=0;i<keys;i++) {
        SPI.transfer('_');
        delay(1);
      }
      SPI.transfer(Cursor_Control);
      delay(1);
      SPI.transfer(4-keys);
      delay(1);
    }
    SPI.transfer(Nibble);
    DisplayEnd();
    WaitReleaseKey();
    Input = Input | Nibble;
  }
  return Input;
}

// MINIL Interpreter - Run the user's program
void Run() {
  long Word;
  int Sign;
  unsigned int PC, Value, Jump;
  char Inst, Reg, Special, Type;
  boolean ZeroFlag = false;
  PC = 0;
  DisplayRunning();
  do {
    // Get next Instruction 
    Inst = EEPROM.read(PC++);
    delayMicroseconds(500);
    Type = Inst & 0x88;
    Reg = (Inst >> 4) & 0x07;
    if ((Inst & 0x80) != 0) {
      // Jump instructions
      Jump = Inst & 0x3F;
      if (((Inst & 0x40) == 0) == ZeroFlag) PC = Jump;
    } else if (Type == 0) {
      // Load Instruction
      Register[Reg] = Register[Inst & 0x07];
    } else if (Type == 0x08) {
      // Special Instructions
      Special = Inst & 0x0F;
      Value = Register[Reg];
      if (Special == 0x09) {
        // Display
        Display(Value);
      } else if ((Special == 0x0A) || (Special == 0x0D)) {
        // Decrement or Add1 and convert to BCD
        if (Special == 0x0A) Sign = 1; else Sign = -1;
        Value = Value + Sign;
        for (int i=0; i<16; i=i+4) {
          // Binary-coded decimal correction - must be a neater way!
          if ((Value & (unsigned int)(0xF<<i)) > (unsigned int)(0x9<<i)) {
            Value = Value + Sign * (unsigned int)(0x6<<i);
          }
        }
        Register[Reg] = Value;
        ZeroFlag = (Value == 0);
      } else if (Special == 0x0B) {
        // Brightness - Convert value from BCD
        Value = ((Value>>8) & 0x0F)*100 + ((Value>>4) & 0x0F)*10 + (Value & 0x0F);
        analogWrite(LED,255-Value);
      } else if (Special == 0x0C) {
        // Clear
        Register[Reg] = 0;
      } else if (Special == 0x0E) {
        // Display and enter
        Display(Value);
        do {
          Word = GetData(4);
          if (Word != -1) Register[Reg] = Word;
        } while (Word != -1);
        LongPress();
        DisplayRunning();
      } else if (Special == 0x0F) {
        // Delay - Convert value from BCD
        Value = ((Value>>8) & 0x0F)*100 + ((Value>>4) & 0x0F)*10 + (Value & 0x0F);
        delay(Value*10);
      } else {
        // Invalid special instruction
        ClearDisplay();
        DisplayError();
        DisplayTwo(Special, 2);
        do ; while (!ReadButton());
      }
    }
  } while (!ReadButton());
}

void loop() { 
  unsigned int PC, Value;
  boolean Command;
  long Word;
  char Reg;
  analogWrite(LED,255);
  DisplayGo();
  do ; while (ReadButton());
  Word = GetData(1);
  WaitReleaseKey();
  if (Word <= 7) {
    Reg = Word;
    Value = Register[Reg];
    // Display register contents
    Display(Value);
    do {
      Word = GetData(4);
      if (Word != -1) Register[Reg] = Word;
    } while (Word != -1);
  } else if (Word == 14) {
    // * = Data input
    PC = 0;
    do {
      do {
        Word = EEPROM.read(PC);
        ClearDisplay();
        DisplayTwo(PC, 0);
        DisplayTwo(Word, 2);
        DisplayColon();
        // Wait for key
        Word = GetData(2);
        if (Word != -1) EEPROM.write(PC, Word);
      } while (Word != -1);
      Command = LongPress();
      PC++;
    } while (!Command);
  } else if (Word == 15) {
    // # = Run Program
    Run();
  }
}
