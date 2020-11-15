#include <Keyboard.h>
#include "Joystick.h"

// ***********************************************************************
// **                                                                   **
// ** Joystick USB HID for Teensy or Pro Micro                          **
// ** Also includes Atari 520ST Keyboard                                **
// ** By DJ Sures (Synthiam.com) (c)2019                                **
// ** Heavily modified and influenced by Kevin Peat's 2017 code         **
// **                                                                   **
// ** Updated: November 13, 2020                                        **
// **                                                                   **
// ** NOTE                                                              **
// ** ~~~~                                                              **
// ** For this to work on Raspberry Pi (Linux), you need to add the     **
// ** the following to the /boot/cmdline.txt...                         **
// **                                                                   **
// ** usbhid.quirks=0x2341:0x8036:0x040    <- for Leonardo              **
// ** usbhid.quirks=0x2341:0x8037:0x040    <- for Micro                 **
// **                                                                   **
// ***********************************************************************
// uncomment to use for serial terminal debugging rather than usb hid device
// this is so you can see the ascii values to verify
// The DEBUG_JOY is for debugging joystick to serial
// The DEBUG is for debugging keyboard

//#define DEBUG_JOY
//#define DEBUG

// ***********************************************************************
// ** Joystick pins on the Arduino                                      **
// ***********************************************************************
#define J1UP 16
#define J1DOWN 10
#define J1LEFT 9
#define J1RIGHT 8
#define J1BUTTONA 14
#define J1BUTTONB 15

#define J2UP 2
#define J2DOWN 3
#define J2LEFT 4
#define J2RIGHT 5
#define J2BUTTONA 6
#define J2BUTTONB 7

// ***********************************************************************
// ** Joystick USB definitions                                          **
// ***********************************************************************
Joystick_ _j1 = Joystick_(
  0x06,
  JOYSTICK_TYPE_JOYSTICK,
  2,
  0,
  true,
  true,
  false,
  false,
  false,
  false,
  false,
  false,
  false,
  false,
  false
);

Joystick_ _j2 = Joystick_(
  0x03,
  JOYSTICK_TYPE_JOYSTICK,
  2,
  0,
  true,
  true,
  false,
  false,
  false,
  false,
  false,
  false,
  false,
  false,
  false
);

// ***********************************************************************
// ** Joystick global variables to keep track of last state             **
// ***********************************************************************
byte _lastJ1X = 1; // 0 left, 1 center, 2 right
byte _lastJ1Y = 1; // 0 up, 1 center, 2 down
byte _lastJ1A = 0; // 0 button released, 1 button pressed
byte _lastJ1B = 0; // 0 button released, 1 button pressed

byte _lastJ2X = 1; // 0 left, 1 center, 2 right
byte _lastJ2Y = 1; // 0 up, 1 center, 2 down
byte _lastJ2A = 0; // 0 button released, 1 button pressed
byte _lastJ2B = 0; // 0 button released, 1 button pressed

// ST keyboard reset pin
#define ST_KB_RESET A3

// Key scancodes
// Use ST scancode as index to find the corresponding USB scancode.
uint8_t scanCodes[] = {
  0x00, 
  KEY_ESC, // Esc
  '1', // 1
  '2', // 2
  '3', // 3
  '4', // 4
  '5', // 5
  '6', // 6
  '7', // 7
  '8', // 8
  '9', // 9
  '0', // 0
  '-', // -
  '=', // =
  KEY_BACKSPACE, // Backspace
  KEY_TAB, // Tab
  'q', // q
  'w', // w
  'e', // e
  'r', // r
  't', // t
  'y', // y
  'u', // u
  'i', // i
  'o', // o
  'p', // p
  '[', // [
  ']', // ]
  KEY_RETURN, // Enter
  KEY_LEFT_CTRL, // Control
  'a', // a
  's', // s
  'd', // d
  'f', // f
  'g', // g
  'h', // h
  'j', // j
  'k', // k
  'l', // l
  ';', // ;
  '\\', // "
  '`', // `
  KEY_LEFT_SHIFT, // Lshift
  '\'', // '
  'z', // z
  'x', // x
  'c', // c
  'v', // v
  'b', // b
  'n', // n
  'm', // m
  ',', // ,
  '.', // .
  '/', // /
  KEY_RIGHT_SHIFT, // Rshift
  0x37, 
  KEY_LEFT_ALT, // Alternate
  ' ', // Space
  KEY_CAPS_LOCK, // CapsLock
  KEY_F1, // F1
  KEY_F2, // F2
  KEY_F3, // F3
  KEY_F4, // F4
  KEY_F5, // F5
  KEY_F6, // F6
  KEY_F7, // F7
  KEY_F8, // F8
  KEY_F9, // F9
  KEY_F10, // F10
  0x45, 
  0x46, 
  KEY_HOME, // Clr/Home
  KEY_UP_ARROW, // Up Arrow
  0x49, 
  '-',  // Number pad -
  KEY_LEFT_ARROW, // Left Arrow
  0x4c, 
  KEY_RIGHT_ARROW, // Right Arrow
  '+',  // Number pad +
  0x4f, 
  KEY_DOWN_ARROW, // Down Arrow
  0x51, 
  KEY_INSERT, // Insert
  KEY_DELETE, // Delete
  0x54, 
  0x55, 
  0x56, 
  0x57, 
  0x58, 
  0x59, 
  0x5a, 
  0x5b, 
  0x5c, 
  0x5d, 
  0x5e, 
  0x5f, 
  0x5C, // ISO Key
  KEY_BACKSPACE, // Undo (Mapped to Backspace)
  KEY_F12, // Help (Mapped to F12 which is the Hatari menu key)
  '(', // Number pad (
  ')', // Number pad )
  '/', // Number pad /
  '*', // Number pad *
  '7', // Number pad 7
  '8', // Number pad 8
  '9', // Number pad 9
  '4', // Number pad 4
  '5', // Number pad 5
  '6', // Number pad 6
  '1', // Number pad 1
  '2', // Number pad 2
  '3', // Number pad 3
  '0', // Number pad 0
  '.', // Number pad .
  KEY_RETURN  // Number pad Enter
};

void setup(void) {

  // ***********************************************************************
  // ** Keyboard init stuff                                               **
  // ***********************************************************************

  // Initialize keyboard:
  Keyboard.begin();

  // Open serial port from Atari keyboard
  Serial1.begin(7812);

#ifdef DEBUG
  // Open serial port to PC
  Serial.begin(9600);
#endif

  // Reset ST keyboard
  delay(200);
  reset_st_keyboard();
  delay(200);

  // Empty serial buffer before starting
  while (Serial1.available())
    Serial1.read();

  // ***********************************************************************
  // ** Joystick init stuff                                               **
  // ***********************************************************************
  _j1.setXAxisRange(-127, 127);
  _j1.setYAxisRange(-127, 127);
  _j1.begin();

  _j2.setXAxisRange(-127, 127);
  _j2.setYAxisRange(-127, 127);
  _j2.begin();

  pinMode(J1UP, INPUT_PULLUP);
  pinMode(J1RIGHT, INPUT_PULLUP);
  pinMode(J1DOWN, INPUT_PULLUP);
  pinMode(J1LEFT, INPUT_PULLUP);
  pinMode(J1BUTTONA, INPUT_PULLUP);
  pinMode(J1BUTTONB, INPUT_PULLUP);

  pinMode(J2UP, INPUT_PULLUP);
  pinMode(J2RIGHT, INPUT_PULLUP);
  pinMode(J2DOWN, INPUT_PULLUP);
  pinMode(J2LEFT, INPUT_PULLUP);
  pinMode(J2BUTTONA, INPUT_PULLUP);
  pinMode(J2BUTTONB, INPUT_PULLUP);
}

void loop() {

  // ***********************************************************************
  // ** Process keyboard input                                            **
  // ***********************************************************************

  // Process incoming Atari keypresses
  if (Serial1.available())
    process_keypress(Serial1.read());

  // Handle keyboard auto-repeat
  //auto_repeat();

  // ***********************************************************************
  // ** Process joystick input                                            **
  // ***********************************************************************

#ifdef DEBUG_JOY
  Serial.print(digitalRead(J1UP));
  Serial.print(digitalRead(J1RIGHT));
  Serial.print(digitalRead(J1DOWN));
  Serial.print(digitalRead(J1LEFT));
  Serial.print(digitalRead(J1BUTTONA));
  Serial.print(digitalRead(J1BUTTONB));

  Serial.print(" ");

  Serial.print(digitalRead(J2UP));
  Serial.print(digitalRead(J2RIGHT));
  Serial.print(digitalRead(J2DOWN));
  Serial.print(digitalRead(J2LEFT));
  Serial.print(digitalRead(J2BUTTONA));
  Serial.print(digitalRead(J2BUTTONB));

  Serial.write("\r\n");
#endif

  // ---------------------------------------------------------
  // JOYSTICK 1
  // ---------------------------------------------------------
  byte j1y;

  if (!digitalRead(J1UP))
    j1y = 0;
  else if (!digitalRead(J1DOWN))
    j1y = 2;
  else
    j1y = 1;

  if (j1y != _lastJ1Y) {

    if (j1y == 0)
      _j1.setYAxis(-127);
    else if (j1y == 2)
      _j1.setYAxis(127);
    else
      _j1.setYAxis(0);

    _lastJ1Y = j1y;
  }

  byte j1x;

  if (!digitalRead(J1LEFT))
    j1x = 0;
  else if (!digitalRead(J1RIGHT))
    j1x = 2;
  else
    j1x = 1;

  if (j1x != _lastJ1X) {

    if (j1x == 0)
      _j1.setXAxis(-127);
    else if (j1x == 2)
      _j1.setXAxis(127);
    else
      _j1.setXAxis(0);

    _lastJ1X = j1x;
  }

  byte j1a = !digitalRead(J1BUTTONA);

  if (j1a != _lastJ1A) {

    _j1.setButton(0, j1a);

    _lastJ1A = j1a;
  }

  byte j1b = !digitalRead(J1BUTTONB);

  if (j1b != _lastJ1B) {

    _j1.setButton(1, j1b);

    _lastJ1B = j1b;
  }

  // ---------------------------------------------------------
  // JOYSTICK 2
  // ---------------------------------------------------------
  byte j2y;

  if (!digitalRead(J2UP))
    j2y = 0;
  else if (!digitalRead(J2DOWN))
    j2y = 2;
  else
    j2y = 1;

  if (j2y != _lastJ2Y) {

    if (j2y == 0)
      _j2.setYAxis(-127);
    else if (j2y == 2)
      _j2.setYAxis(127);
    else
      _j2.setYAxis(0);

    _lastJ2Y = j2y;
  }

  byte j2x;

  if (!digitalRead(J2LEFT))
    j2x = 0;
  else if (!digitalRead(J2RIGHT))
    j2x = 2;
  else
    j2x = 1;

  if (j2x != _lastJ2X) {

    if (j2x == 0)
      _j2.setXAxis(-127);
    else if (j2x == 2)
      _j2.setXAxis(127);
    else
      _j2.setXAxis(0);

    _lastJ2X = j2x;
  }

  byte j2a = !digitalRead(J2BUTTONA);

  if (j2a != _lastJ2A) {

    _j2.setButton(0, j2a);

    _lastJ2A = j2a;
  }

  byte j2b = !digitalRead(J2BUTTONB);

  if (j2b != _lastJ2B) {

    _j2.setButton(1, j2b);

    _lastJ2B = j2b;
  }

  delay(50);
}

// Reset ST Keyboard
void reset_st_keyboard(void) {

  Serial1.print(0x80);
  Serial1.print(1);
  pinMode(ST_KB_RESET, OUTPUT);
  digitalWrite(ST_KB_RESET, HIGH);
  delay(20);
  digitalWrite(ST_KB_RESET, LOW);
  delay(20);
  digitalWrite(ST_KB_RESET, HIGH);
}

// Process each keypress
void process_keypress(uint8_t key) {

#ifdef DEBUG
  Serial.println();
  Serial.print("Raw Value: ");
  Serial.println(key);
#endif

  if (key <= 114) {
    // key down

    int pc = scanCodes[key];

#ifdef DEBUG
    Serial.print("Press PC Code: ");
    Serial.println(pc);
#else
    Keyboard.press(pc);
#endif
  } else if (key >= 129 && key <= 129 + 114) {
    // key up

    int pc = scanCodes[key - 128];

#ifdef DEBUG
    Serial.print("Release PC Code: ");
    Serial.println(pc);
#else
    Keyboard.release(pc);
#endif
  }  
}
