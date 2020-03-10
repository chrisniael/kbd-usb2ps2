#include <hidboot.h>
#include <usbhub.h>

#include "FidPS2Host.h"
#include "FidPS2Keyboard.h"

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>


#define KEY_REPEAT_INTERVAL 360
#define KEY_REPEAT_DELAY 8000

// PS/2 Key type
// | hex  | describe    |
// | ---- | ----------- |
// | 0x00 | None type   |
// | 0x01 | Normal type |
// | 0x02 | E0 type     |
// | 0x04 | E1 type     |
#define PS2_KEY_TYPE_NONE 0x00
#define PS2_KEY_TYPE_NORMAL 0x01
#define PS2_KEY_TYPE_E0 0x02
#define PS2_KEY_TYPE_E1 0x04

struct Ps2Key {
  uint8_t key;
  uint8_t type;
};

// HID Usage ID Max = E7 (Usage Page 07), E8-FFFF Reserved
Ps2Key hid2ps2[0xE8];

const Ps2Key* last_press_key = nullptr;
uint16_t key_repeat_ct = 0;
uint16_t key_repeat_delay_ct = 0;

#define HID_KEY_LEFT_CONTROL 0xE0
#define HID_KEY_LEFT_SHIFT 0xE1
#define HID_KEY_LEFT_ALT 0xE2
#define HID_KEY_LEFT_GUI 0xE3
#define HID_KEY_RIGHT_CONTROL 0xE4
#define HID_KEY_RIGHT_SHIFT 0xE5
#define HID_KEY_RIGHT_ALT 0xE6
#define HID_KEY_RIGHT_GUI 0xE7

bool Ps2KeyTypeNormal(const uint8_t& type) {
  return type & PS2_KEY_TYPE_NORMAL;
}

bool Ps2KeyTypeE0(const uint8_t& type) { return type & PS2_KEY_TYPE_E0; }

bool Ps2KeyTypeE1(const uint8_t& type) { return type & PS2_KEY_TYPE_E1; }

void Ps2KeyPress(const Ps2Key& ps2_key) {
  Serial.print("Ps2 key press: ");
  PrintHex<uint8_t>(ps2_key.key, 0x80);
  Serial.print(", ");
  PrintHex<uint8_t>(ps2_key.type, 0x80);
  Serial.println("");

  if (Ps2KeyTypeNormal(ps2_key.type)) {
    fid_ps2h_write(ps2_key.key);
  } else if (Ps2KeyTypeE0(ps2_key.type)) {
    fid_ps2h_write(0xE0);
    fid_ps2h_write(ps2_key.key);
  } else if (Ps2KeyTypeE1(ps2_key.type)) {
    fid_ps2h_write(0xE1);
    fid_ps2h_write(0x14);
    fid_ps2h_write(0x77);
    fid_ps2h_write(0xE1);
    fid_ps2h_write(0xF0);
    fid_ps2h_write(0x14);
    fid_ps2h_write(0xF0);
    fid_ps2h_write(0x77);

    Serial.println("Ps2 key press: Pause/Break");
  } else {
    Serial.println("Ps2 key press: not recognized");
  }
}

void Ps2KeyRelease(const Ps2Key& ps2_key) {
  Serial.print("Ps2 key release: ");
  PrintHex<uint8_t>(ps2_key.key, 0x80);
  Serial.print(", ");
  PrintHex<uint8_t>(ps2_key.type, 0x80);
  Serial.println("");


  if (Ps2KeyTypeNormal(ps2_key.type)) {
    fid_ps2h_write(0xF0);
    fid_ps2h_write(ps2_key.key);
  } else if (Ps2KeyTypeE0(ps2_key.type)) {
    fid_ps2h_write(0xE0);
    fid_ps2h_write(0xF0);
    fid_ps2h_write(ps2_key.key);
  } else if (Ps2KeyTypeE1(ps2_key.type)) {
    Serial.println("Ps2 key relase: Pause/Break");
  } else {
    Serial.println("Ps2 key relase: not recognized");
  }
}

void InitHidKeyMapPs2() {
  //

  // PS/2 normal scan code
  // | HID | PS/2 Make | PS/2 Break |
  // | --- | --------- | ---------- |
  // | 04  | 1C        | F0 1C      |

  // A-Z
  hid2ps2[0x04] = {0x1C, PS2_KEY_TYPE_NORMAL};
  hid2ps2[0x05] = {0x32, PS2_KEY_TYPE_NORMAL};
  hid2ps2[0x06] = {0x21, PS2_KEY_TYPE_NORMAL};
  hid2ps2[0x07] = {0x23, PS2_KEY_TYPE_NORMAL};
  hid2ps2[0x08] = {0x24, PS2_KEY_TYPE_NORMAL};
  hid2ps2[0x09] = {0x2B, PS2_KEY_TYPE_NORMAL};
  hid2ps2[0x0A] = {0x34, PS2_KEY_TYPE_NORMAL};
  hid2ps2[0x0B] = {0x33, PS2_KEY_TYPE_NORMAL};
  hid2ps2[0x0C] = {0x43, PS2_KEY_TYPE_NORMAL};
  hid2ps2[0x0D] = {0x3B, PS2_KEY_TYPE_NORMAL};
  hid2ps2[0x0E] = {0x42, PS2_KEY_TYPE_NORMAL};
  hid2ps2[0x0F] = {0x4B, PS2_KEY_TYPE_NORMAL};
  hid2ps2[0x10] = {0x3A, PS2_KEY_TYPE_NORMAL};
  hid2ps2[0x11] = {0x31, PS2_KEY_TYPE_NORMAL};
  hid2ps2[0x12] = {0x44, PS2_KEY_TYPE_NORMAL};
  hid2ps2[0x13] = {0x4D, PS2_KEY_TYPE_NORMAL};
  hid2ps2[0x14] = {0x15, PS2_KEY_TYPE_NORMAL};
  hid2ps2[0x15] = {0x2D, PS2_KEY_TYPE_NORMAL};
  hid2ps2[0x16] = {0x1B, PS2_KEY_TYPE_NORMAL};
  hid2ps2[0x17] = {0x2C, PS2_KEY_TYPE_NORMAL};
  hid2ps2[0x18] = {0x3C, PS2_KEY_TYPE_NORMAL};
  hid2ps2[0x19] = {0x2A, PS2_KEY_TYPE_NORMAL};
  hid2ps2[0x1A] = {0x1D, PS2_KEY_TYPE_NORMAL};
  hid2ps2[0x1B] = {0x22, PS2_KEY_TYPE_NORMAL};
  hid2ps2[0x1C] = {0x35, PS2_KEY_TYPE_NORMAL};
  hid2ps2[0x1D] = {0x1A, PS2_KEY_TYPE_NORMAL};


  // 1-0
  hid2ps2[0x1E] = {0x16, PS2_KEY_TYPE_NORMAL};
  hid2ps2[0x1F] = {0x1E, PS2_KEY_TYPE_NORMAL};
  hid2ps2[0x20] = {0x26, PS2_KEY_TYPE_NORMAL};
  hid2ps2[0x21] = {0x25, PS2_KEY_TYPE_NORMAL};
  hid2ps2[0x22] = {0x2E, PS2_KEY_TYPE_NORMAL};
  hid2ps2[0x23] = {0x36, PS2_KEY_TYPE_NORMAL};
  hid2ps2[0x24] = {0x3D, PS2_KEY_TYPE_NORMAL};
  hid2ps2[0x25] = {0x3E, PS2_KEY_TYPE_NORMAL};
  hid2ps2[0x26] = {0x46, PS2_KEY_TYPE_NORMAL};
  hid2ps2[0x27] = {0x45, PS2_KEY_TYPE_NORMAL};

  hid2ps2[0x28] = {0x5A, PS2_KEY_TYPE_NORMAL};  // Return
  hid2ps2[0x29] = {0x76, PS2_KEY_TYPE_NORMAL};  // Escape
  hid2ps2[0x2A] = {0x66, PS2_KEY_TYPE_NORMAL};  // Backspace
  hid2ps2[0x2B] = {0x0D, PS2_KEY_TYPE_NORMAL};  // Tab
  hid2ps2[0x2C] = {0x29, PS2_KEY_TYPE_NORMAL};  // Space
  hid2ps2[0x2D] = {0x4E, PS2_KEY_TYPE_NORMAL};  // -_
  hid2ps2[0x2E] = {0x55, PS2_KEY_TYPE_NORMAL};  // =+
  hid2ps2[0x2F] = {0x54, PS2_KEY_TYPE_NORMAL};  // [{
  hid2ps2[0x30] = {0x5B, PS2_KEY_TYPE_NORMAL};  // ]}
  hid2ps2[0x31] = {0x5D, PS2_KEY_TYPE_NORMAL};  // \|

  hid2ps2[0x33] = {0x4C, PS2_KEY_TYPE_NORMAL};  // ;:
  hid2ps2[0x34] = {0x52, PS2_KEY_TYPE_NORMAL};  // '"
  hid2ps2[0x35] = {0x0E, PS2_KEY_TYPE_NORMAL};  // `~
  hid2ps2[0x36] = {0x41, PS2_KEY_TYPE_NORMAL};  // ,<
  hid2ps2[0x37] = {0x49, PS2_KEY_TYPE_NORMAL};  // ,>
  hid2ps2[0x38] = {0x4A, PS2_KEY_TYPE_NORMAL};  // /?
  hid2ps2[0x39] = {0x58, PS2_KEY_TYPE_NORMAL};  // Caps Lock

  // F1-F12
  hid2ps2[0x3A] = {0x05, PS2_KEY_TYPE_NORMAL};
  hid2ps2[0x3B] = {0x06, PS2_KEY_TYPE_NORMAL};
  hid2ps2[0x3C] = {0x04, PS2_KEY_TYPE_NORMAL};
  hid2ps2[0x3D] = {0x0C, PS2_KEY_TYPE_NORMAL};
  hid2ps2[0x3E] = {0x03, PS2_KEY_TYPE_NORMAL};
  hid2ps2[0x3F] = {0x0B, PS2_KEY_TYPE_NORMAL};
  hid2ps2[0x40] = {0x83, PS2_KEY_TYPE_NORMAL};
  hid2ps2[0x41] = {0x0A, PS2_KEY_TYPE_NORMAL};
  hid2ps2[0x42] = {0x01, PS2_KEY_TYPE_NORMAL};
  hid2ps2[0x43] = {0x09, PS2_KEY_TYPE_NORMAL};
  hid2ps2[0x44] = {0x78, PS2_KEY_TYPE_NORMAL};
  hid2ps2[0x45] = {0x07, PS2_KEY_TYPE_NORMAL};

  hid2ps2[0x47] = {0x7E, PS2_KEY_TYPE_NORMAL};  // Scroll Lock

  hid2ps2[0x53] = {0x77, PS2_KEY_TYPE_NORMAL};  // Num Lock

  hid2ps2[0x55] = {0x7C, PS2_KEY_TYPE_NORMAL};  // Keypad *
  hid2ps2[0x56] = {0x7B, PS2_KEY_TYPE_NORMAL};  // Keypad -
  hid2ps2[0x57] = {0x79, PS2_KEY_TYPE_NORMAL};  // Keypad +

  hid2ps2[0x63] = {0x71, PS2_KEY_TYPE_NORMAL};  // Keypad . Delete
  hid2ps2[0x67] = {0x0F, PS2_KEY_TYPE_NORMAL};  // Keypad =

  hid2ps2[0xE0] = {0x14, PS2_KEY_TYPE_NORMAL};  // Left Control
  hid2ps2[0xE1] = {0x12, PS2_KEY_TYPE_NORMAL};  // Left Shift
  hid2ps2[0xE2] = {0x11, PS2_KEY_TYPE_NORMAL};  // Left Alt

  hid2ps2[0xE5] = {0x59, PS2_KEY_TYPE_NORMAL};  // Right Shift


  // PS/2 scan code prefix with E0
  // | HID | PS/2 Make | PS/2 Break |
  // | --- | --------- | ---------- |
  // | 46  | E0 7C     | E0 F0 7C   |
  hid2ps2[0x46] = {0x7C, PS2_KEY_TYPE_E0};  // Print Screen

  hid2ps2[0x49] = {0x70, PS2_KEY_TYPE_E0};  // Insert
  hid2ps2[0x4A] = {0x6C, PS2_KEY_TYPE_E0};  // Home
  hid2ps2[0x4B] = {0x7D, PS2_KEY_TYPE_E0};  // Page Up
  hid2ps2[0x4C] = {0x71, PS2_KEY_TYPE_E0};  // Delete
  hid2ps2[0x4D] = {0x69, PS2_KEY_TYPE_E0};  // End
  hid2ps2[0x4E] = {0x7A, PS2_KEY_TYPE_E0};  // Page Down
  hid2ps2[0x4F] = {0x74, PS2_KEY_TYPE_E0};  // Right Arrow
  hid2ps2[0x50] = {0x6B, PS2_KEY_TYPE_E0};  // Left Arrow
  hid2ps2[0x51] = {0x72, PS2_KEY_TYPE_E0};  // Down Arrow
  hid2ps2[0x52] = {0x75, PS2_KEY_TYPE_E0};  // Up Arrow

  hid2ps2[0x54] = {0x4A, PS2_KEY_TYPE_E0};  // Keypad /

  hid2ps2[0x58] = {0x5A, PS2_KEY_TYPE_E0};  // Keypad Enter

  hid2ps2[0x65] = {0x2F, PS2_KEY_TYPE_E0};  // App

  hid2ps2[0xE3] = {0x1F, PS2_KEY_TYPE_E0};  // Left GUI
  hid2ps2[0xE4] = {0x14, PS2_KEY_TYPE_E0};  // Right Control

  hid2ps2[0xE6] = {0x11, PS2_KEY_TYPE_E0};  // Right Alt
  hid2ps2[0xE7] = {0x27, PS2_KEY_TYPE_E0};  // Right GUI


  // PS/2 scan code prefix with E1
  // | HID | PS/2 Make               | PS/2 Break |
  // | --- | ----------------------- | ---------- |
  // | 48  | E1 14 77 E1 F0 14 F0 77 | None       |
  hid2ps2[0x48] = {0x0, PS2_KEY_TYPE_E1};  // Pause/Break
}


class KbdRptParser : public KeyboardReportParser {
  void PrintKey(uint8_t mod, uint8_t key);

 protected:
  void OnControlKeysChanged(uint8_t before, uint8_t after);
  void OnKeyDown(uint8_t mod, uint8_t key);
  void OnKeyUp(uint8_t mod, uint8_t key);
  void OnKeyPressed(uint8_t key);
  void OnKeyDown(uint8_t key);
  void OnKeyUp(uint8_t key);
};

void KbdRptParser::PrintKey(uint8_t m, uint8_t key) {
  MODIFIERKEYS mod;
  *((uint8_t*)&mod) = m;
  Serial.print((mod.bmLeftCtrl == 1) ? "C" : " ");
  Serial.print((mod.bmLeftShift == 1) ? "S" : " ");
  Serial.print((mod.bmLeftAlt == 1) ? "A" : " ");
  Serial.print((mod.bmLeftGUI == 1) ? "G" : " ");

  Serial.print(" >");
  PrintHex<uint8_t>(key, 0x80);
  Serial.print("< ");

  Serial.print((mod.bmRightCtrl == 1) ? "C" : " ");
  Serial.print((mod.bmRightShift == 1) ? "S" : " ");
  Serial.print((mod.bmRightAlt == 1) ? "A" : " ");
  Serial.println((mod.bmRightGUI == 1) ? "G" : " ");
};

void KbdRptParser::OnKeyDown(uint8_t mod, uint8_t key) {
  Serial.print("DN ");
  PrintKey(mod, key);

  uint8_t c = OemToAscii(mod, key);
  if (c) {
    OnKeyPressed(c);
  }

  this->OnKeyDown(key);
}

void KbdRptParser::OnKeyDown(uint8_t key) {
  const Ps2Key& ps2_key = hid2ps2[key];
  Ps2KeyPress(ps2_key);

  last_press_key = &ps2_key;
  key_repeat_ct = 0;
  key_repeat_delay_ct = 0;
}

void KbdRptParser::OnControlKeysChanged(uint8_t before, uint8_t after) {
  MODIFIERKEYS beforeMod;
  *((uint8_t*)&beforeMod) = before;

  MODIFIERKEYS afterMod;
  *((uint8_t*)&afterMod) = after;

  if (beforeMod.bmLeftCtrl != afterMod.bmLeftCtrl) {
    Serial.println("LeftCtrl changed");

    if (afterMod.bmLeftCtrl) {
      this->OnKeyDown(HID_KEY_LEFT_CONTROL);
    } else {
      this->OnKeyUp(HID_KEY_LEFT_CONTROL);
    }
  }
  if (beforeMod.bmLeftShift != afterMod.bmLeftShift) {
    Serial.println("LeftShift changed");

    if (afterMod.bmLeftShift) {
      this->OnKeyDown(HID_KEY_LEFT_SHIFT);
    } else {
      this->OnKeyUp(HID_KEY_LEFT_SHIFT);
    }
  }
  if (beforeMod.bmLeftAlt != afterMod.bmLeftAlt) {
    Serial.println("LeftAlt changed");

    if (afterMod.bmLeftAlt) {
      this->OnKeyDown(HID_KEY_LEFT_ALT);
    } else {
      this->OnKeyUp(HID_KEY_LEFT_ALT);
    }
  }
  if (beforeMod.bmLeftGUI != afterMod.bmLeftGUI) {
    Serial.println("LeftGUI changed");

    if (afterMod.bmLeftGUI) {
      this->OnKeyDown(HID_KEY_LEFT_GUI);
    } else {
      this->OnKeyUp(HID_KEY_LEFT_GUI);
    }
  }

  if (beforeMod.bmRightCtrl != afterMod.bmRightCtrl) {
    Serial.println("RightCtrl changed");

    if (afterMod.bmRightCtrl) {
      this->OnKeyDown(HID_KEY_RIGHT_CONTROL);
    } else {
      this->OnKeyUp(HID_KEY_RIGHT_CONTROL);
    }
  }
  if (beforeMod.bmRightShift != afterMod.bmRightShift) {
    Serial.println("RightShift changed");

    if (afterMod.bmRightShift) {
      this->OnKeyDown(HID_KEY_RIGHT_SHIFT);
    } else {
      this->OnKeyUp(HID_KEY_RIGHT_SHIFT);
    }
  }
  if (beforeMod.bmRightAlt != afterMod.bmRightAlt) {
    Serial.println("RightAlt changed");

    if (afterMod.bmRightAlt) {
      this->OnKeyDown(HID_KEY_RIGHT_ALT);
    } else {
      this->OnKeyUp(HID_KEY_RIGHT_ALT);
    }
  }
  if (beforeMod.bmRightGUI != afterMod.bmRightGUI) {
    Serial.println("RightGUI changed");

    if (afterMod.bmRightGUI) {
      this->OnKeyDown(HID_KEY_RIGHT_GUI);
    } else {
      this->OnKeyUp(HID_KEY_RIGHT_GUI);
    }
  }

  key_repeat_ct = 0;
  key_repeat_delay_ct = 0;
}

void KbdRptParser::OnKeyUp(uint8_t mod, uint8_t key) {
  Serial.print("UP ");
  PrintKey(mod, key);

  this->OnKeyUp(key);
}

void KbdRptParser::OnKeyUp(uint8_t key) {
  const Ps2Key& ps2_key = hid2ps2[key];
  Ps2KeyRelease(ps2_key);

  if (last_press_key && last_press_key->key == ps2_key.key &&
      last_press_key->type == ps2_key.type) {
    last_press_key = nullptr;
  }

  key_repeat_ct = 0;
  key_repeat_delay_ct = 0;
}

void KbdRptParser::OnKeyPressed(uint8_t key) {
  Serial.print("ASCII: ");
  Serial.println((char)key);
};

USB Usb;
// USBHub     Hub(&Usb);
HIDBoot<USB_HID_PROTOCOL_KEYBOARD> HidKeyboard(&Usb);

KbdRptParser Prs;

void setup() {
  Serial.begin(115200);
#if !defined(__MIPSEL__)
  while (!Serial)
    ;  // Wait for serial port to connect - used on Leonardo, Teensy and other
       // boards with built-in USB CDC serial connection
#endif
  Serial.println("Start");

  InitHidKeyMapPs2();

  fid_ps2h_init(4, 2);

  if (Usb.Init() == -1) Serial.println("OSC did not start.");

  delay(200);

  HidKeyboard.SetReportParser(0, &Prs);
}

void loop() {
  Usb.Task();

  uint8_t b;
  while (fid_ps2h_read(&b)) {
    Serial.print("Ps2 read: ");
    PrintHex<uint8_t>(b, 0x80);
    Serial.println("");
  }

  if (last_press_key == nullptr) {
    return;
  }

  if (key_repeat_delay_ct >= KEY_REPEAT_DELAY) {
    key_repeat_ct = (key_repeat_ct + 1) % KEY_REPEAT_INTERVAL;
    if (key_repeat_ct == 0) {
      const Ps2Key& ps2_key = *last_press_key;
      Ps2KeyPress(ps2_key);
    }
  } else {
    ++key_repeat_delay_ct;
  }
}
