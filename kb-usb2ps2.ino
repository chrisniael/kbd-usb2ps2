#include <hidboot.h>
#include <usbhub.h>
#include "FidPS2Keyboard.h"
#include "FidPS2Host.h"

uint8_t key_repeat = 0x0;

uint16_t key_repeat_interval = 360;
uint16_t key_repeat_ct = 0;

uint16_t before_key_repeat_interval = 8000;
uint16_t before_key_repeat_ct = 0;

// PS/2 normal scan code
uint8_t K[255];

// Ps/2 scan code suffix with E0
uint8_t K_E0[255];

void InitHidKeyMapPs2() {
  // A-Z
  K[0x04] = 0x1C;
  K[0x05] = 0x32;
  K[0x06] = 0x21;
  K[0x07] = 0x23;
  K[0x08] = 0x24;
  K[0x09] = 0x2B;
  K[0x0A] = 0x34;
  K[0x0B] = 0x33;
  K[0x0C] = 0x43;
  K[0x0D] = 0x3B;
  K[0x0E] = 0x42;
  K[0x0F] = 0x4B;
  K[0x10] = 0x3A;
  K[0x11] = 0x31;
  K[0x12] = 0x44;
  K[0x13] = 0x4D;
  K[0x14] = 0x15;
  K[0x15] = 0x2D;
  K[0x16] = 0x1B;
  K[0x17] = 0x2C;
  K[0x18] = 0x3C;
  K[0x19] = 0x2A;
  K[0x1A] = 0x1D;
  K[0x1B] = 0x22;
  K[0x1C] = 0x35;
  K[0x1D] = 0x1A;

  // 1-0
  K[0x1E] = 0x16;
  K[0x1F] = 0x1E;
  K[0x20] = 0x26;
  K[0x21] = 0x25;
  K[0x22] = 0x2E;
  K[0x23] = 0x36;
  K[0x24] = 0x3D;
  K[0x25] = 0x3E;
  K[0x26] = 0x46;
  K[0x27] = 0x45;

  K[0x28] = 0x5A;  // Return
  K[0x29] = 0x76;  // Escape
  K[0x2A] = 0x66;  // Backspace
  K[0x2B] = 0x0D;  // Tab
  K[0x2C] = 0x29;  // Space
  K[0x2D] = 0x4E;  // -_
  K[0x2E] = 0x55;  // =+
  K[0x2F] = 0x54;  // [{
  K[0x30] = 0x5B;  // ]}
  K[0x31] = 0x5D;  // \|
  K[0x32] = 0x5D;
  K[0x33] = 0x4C;  // ;:
  K[0x34] = 0x52;  // '"
  K[0x35] = 0x0E;  // `~
  K[0x36] = 0x41;  // ,<
  K[0x37] = 0x49;  // .>
  K[0x38] = 0x4A;  // /?
  K[0x39] = 0x58;  // Caps Lock

  // F1-F12
  K[0x3A] = 0x05;
  K[0x3B] = 0x06;
  K[0x3C] = 0x04;
  K[0x3D] = 0x0C;
  K[0x3E] = 0x03;
  K[0x3F] = 0x0B;
  K[0x40] = 0x83;
  K[0x41] = 0x0A;
  K[0x42] = 0x01;
  K[0x43] = 0x09;
  K[0x44] = 0x78;
  K[0x45] = 0x07;

  K[0x47] = 0x7E;  // Scroll Lock

  K[0x53] = 0x77;  // Num Lock

  K[0xE0] = 0x14;  // Left Control
  K[0xE1] = 0x12;  // Left Shift
  K[0xE2] = 0x11;  // Left Alt

  K[0xE5] = 0x59;  // Right Shift

  
  K_E0[0x46] = 0x7C;  // Print Screen
  K_E0[0x49] = 0x70;  // Insert
  K_E0[0x4A] = 0x6C;  // Home
  K_E0[0x4B] = 0x7D;  // Page Up
  K_E0[0x4C] = 0x71;  // Delete
  K_E0[0x4D] = 0x69;  // End
  K_E0[0x4E] = 0x7A;  // Page Down
  K_E0[0x4F] = 0x74;  // Right Arrow
  K_E0[0x50] = 0x6B;  // Left Arrow
  K_E0[0x51] = 0x72;  // Down Arrow
  K_E0[0x52] = 0x75;  // Up Arrow

  K_E0[0x54] = 0x4A;  // Keypad /
  K_E0[0x58] = 0x5A;  // Keypad Enter

  K_E0[0x65] = 0x2F;  // App

  K_E0[0xE3] = 0x1F;  // Left GUI
  K_E0[0xE4] = 0x14;  // Right Control
  K_E0[0xE6] = 0x11;  // Right Alt
  K_E0[0xE7] = 0x27;  // Right GUI
}


// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>

class KbdRptParser : public KeyboardReportParser
{
    void PrintKey(uint8_t mod, uint8_t key);

  protected:
    void OnControlKeysChanged(uint8_t before, uint8_t after);

    void OnKeyDown(uint8_t mod, uint8_t key);
    void OnKeyUp(uint8_t mod, uint8_t key);
    void OnKeyPressed(uint8_t key);
};

void KbdRptParser::PrintKey(uint8_t m, uint8_t key)
{
  MODIFIERKEYS mod;
  *((uint8_t*)&mod) = m;
  Serial.print((mod.bmLeftCtrl   == 1) ? "C" : " ");
  Serial.print((mod.bmLeftShift  == 1) ? "S" : " ");
  Serial.print((mod.bmLeftAlt    == 1) ? "A" : " ");
  Serial.print((mod.bmLeftGUI    == 1) ? "G" : " ");

  Serial.print(" >");
  PrintHex<uint8_t>(key, 0x80);
  Serial.print("< ");

  Serial.print((mod.bmRightCtrl   == 1) ? "C" : " ");
  Serial.print((mod.bmRightShift  == 1) ? "S" : " ");
  Serial.print((mod.bmRightAlt    == 1) ? "A" : " ");
  Serial.println((mod.bmRightGUI    == 1) ? "G" : " ");
};

void KbdRptParser::OnKeyDown(uint8_t mod, uint8_t key)
{
  Serial.print("DN ");
  PrintKey(mod, key);
  uint8_t c = OemToAscii(mod, key);

  if (c) {
    OnKeyPressed(c);
  }

  uint8_t ps2_key = K[key];
  Serial.print("Ps2: ");
  PrintHex<uint8_t>(ps2_key, 0x80);
  Serial.println("");
  fid_ps2h_write(ps2_key);
  key_repeat = ps2_key;
  key_repeat_ct = 0;
  before_key_repeat_ct = 0;
}

void KbdRptParser::OnControlKeysChanged(uint8_t before, uint8_t after) {

  MODIFIERKEYS beforeMod;
  *((uint8_t*)&beforeMod) = before;

  MODIFIERKEYS afterMod;
  *((uint8_t*)&afterMod) = after;

  Serial.print(beforeMod.bmLeftCtrl);
  Serial.print(beforeMod.bmRightCtrl);
  Serial.print(beforeMod.bmLeftShift);
  Serial.print(beforeMod.bmRightShift);
  Serial.print(beforeMod.bmLeftAlt);
  Serial.print(beforeMod.bmRightAlt);
  Serial.print(beforeMod.bmLeftGUI);
  Serial.println(beforeMod.bmRightGUI);

  Serial.print(afterMod.bmLeftCtrl);
  Serial.print(afterMod.bmRightCtrl);
  Serial.print(afterMod.bmLeftShift);
  Serial.print(afterMod.bmRightShift);
  Serial.print(afterMod.bmLeftAlt);
  Serial.print(afterMod.bmRightAlt);
  Serial.print(afterMod.bmLeftGUI);
  Serial.println(afterMod.bmRightGUI);


  if (beforeMod.bmLeftCtrl != afterMod.bmLeftCtrl) {
    Serial.println("LeftCtrl changed");
  }
  if (beforeMod.bmLeftShift != afterMod.bmLeftShift) {
    Serial.println("LeftShift changed");
  }
  if (beforeMod.bmLeftAlt != afterMod.bmLeftAlt) {
    Serial.println("LeftAlt changed");
  }
  if (beforeMod.bmLeftGUI != afterMod.bmLeftGUI) {
    Serial.println("LeftGUI changed");
  }

  if (beforeMod.bmRightCtrl != afterMod.bmRightCtrl) {
    Serial.println("RightCtrl changed");
  }
  if (beforeMod.bmRightShift != afterMod.bmRightShift) {
    Serial.println("RightShift changed");
  }
  if (beforeMod.bmRightAlt != afterMod.bmRightAlt) {
    Serial.println("RightAlt changed");
  }
  if (beforeMod.bmRightGUI != afterMod.bmRightGUI) {
    Serial.println("RightGUI changed");
  }

}

void KbdRptParser::OnKeyUp(uint8_t mod, uint8_t key)
{
  Serial.print("UP ");
  PrintKey(mod, key);

  fid_ps2h_write(0xF0);
  fid_ps2h_write(key_repeat);
  key_repeat = 0x0;
}

void KbdRptParser::OnKeyPressed(uint8_t key)
{
  Serial.print("ASCII: ");
  Serial.println((char)key);
};

USB     Usb;
//USBHub     Hub(&Usb);
HIDBoot<USB_HID_PROTOCOL_KEYBOARD>    HidKeyboard(&Usb);

KbdRptParser Prs;

void setup()
{
  Serial.begin( 115200 );
#if !defined(__MIPSEL__)
  while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif
  Serial.println("Start");

  InitHidKeyMapPs2();

  fid_ps2h_init(4,2);

  if (Usb.Init() == -1)
    Serial.println("OSC did not start.");

  delay( 200 );

  HidKeyboard.SetReportParser(0, &Prs);
}

void loop()
{
  Usb.Task();
  if (key_repeat != 0x0)
  {
    if (before_key_repeat_ct >= before_key_repeat_interval)
    {
      key_repeat_ct = (key_repeat_ct + 1) % key_repeat_interval;
      if (key_repeat_ct == 0)
      {
        fid_ps2h_write(key_repeat);
      }
    }
    else
    {
      ++before_key_repeat_ct;
    }

  }
}
