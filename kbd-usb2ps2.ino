#include <hidboot.h>
#include <usbhub.h>

#include "FidPS2Host.h"
#include "FidPS2Keyboard.h"

#define KEY_REPEAT_INTERVAL 360
#define KEY_REPEAT_DELAY 8000

// flag: repeat & type
// For saving memory
// | bin       | hex  | describe                |
// | --------- | ---- | ----------------------- |
// | 0000 0000 | 0x00 | No repeat & None type   |
// | 0000 0001 | 0x01 | No repeat & Normal type |
// | 0000 0010 | 0x02 | No repeat & E0 type     |
// | 0000 0100 | 0x04 | No repeat & E1 type     |
// | 0001 0001 | 0x11 | repeat & Normal type    |
// | 0001 0010 | 0x12 | repeat & E0 type        |
#define PS2_KEY_REPEAT_NO 0x00
#define PS2_KEY_REPEAT_YES 0x10
#define PS2_KEY_TYPE_NONE 0x00
#define PS2_KEY_TYPE_NORMAL 0x01
#define PS2_KEY_TYPE_E0 0x02
#define PS2_KEY_TYPE_E1 0x04

struct Ps2Key {
  uint8_t key;
  uint8_t flag;
};

Ps2Key hid2ps2[0xE8];

Ps2Key* last_press_key = nullptr;
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

bool Ps2KeyTypeNormal(const uint8_t& flag) {
  return flag & 0x0F & PS2_KEY_TYPE_NORMAL;
}

bool Ps2KeyTypeE0(const uint8_t& flag) { return flag & 0x0F & PS2_KEY_TYPE_E0; }

bool Ps2KeyTypeE1(const uint8_t& flag) { return flag & 0x0F & PS2_KEY_TYPE_E1; }

bool Ps2KeyRepeatYes(const uint8_t& flag) {
  return flag & 0xF0 & PS2_KEY_REPEAT_YES;
}

void Ps2KeyPress(const Ps2Key& ps2_key) {
  if (Ps2KeyTypeNormal(ps2_key.flag)) {
    fid_ps2h_write(ps2_key.key);

    Serial.print("Ps2: ");
    PrintHex<uint8_t>(ps2_key.key, 0x80);
    Serial.println("");
  } else if (Ps2KeyTypeE0(ps2_key.flag)) {
    fid_ps2h_write(0xE0);
    fid_ps2h_write(ps2_key.key);

    Serial.print("Ps2: ");
    PrintHex<uint8_t>(ps2_key.key, 0x80);
    Serial.println("");
  } else if (Ps2KeyTypeE1(ps2_key.flag)) {
    fid_ps2h_write(0xE1);
    fid_ps2h_write(0x14);
    fid_ps2h_write(0x77);
    fid_ps2h_write(0xE1);
    fid_ps2h_write(0xF0);
    fid_ps2h_write(0x14);
    fid_ps2h_write(0xF0);
    fid_ps2h_write(0x77);

    Serial.println("Ps2: Pause/Break");
  } else {
    Serial.println("Ps2: not recognized");
  }
}

void Ps2KeyRelease(const Ps2Key& ps2_key) {
  if (Ps2KeyTypeNormal(ps2_key.flag)) {
    fid_ps2h_write(0xF0);
    fid_ps2h_write(ps2_key.key);

    Serial.print("Ps2: ");
    PrintHex<uint8_t>(ps2_key.key, 0x80);
    Serial.println("");
  } else if (Ps2KeyTypeE0(ps2_key.flag)) {
    fid_ps2h_write(0xE0);
    fid_ps2h_write(0xF0);
    fid_ps2h_write(ps2_key.key);

    Serial.print("Ps2: ");
    PrintHex<uint8_t>(ps2_key.key, 0x80);
    Serial.println("");
  } else if (Ps2KeyTypeE1(ps2_key.flag)) {
    Serial.println("Ps2: Pause/Break");
  } else {
    Serial.println("Ps2: not recognized");
  }
}

void InitHidKeyMapPs2() {
  //

  // PS/2 normal scan code
  // | HID | PS/2 Make | PS/2 Break |
  // | --- | --------- | ---------- |
  // | 04  | 1C        | F0 1C      |

  // A-Z
  hid2ps2[0x04] = {0x1c, PS2_KEY_REPEAT_YES & PS2_KEY_TYPE_NORMAL};

  // PS/2 scan code prefix with E0
  // | HID | PS/2 Make | PS/2 Break |
  // | --- | --------- | ---------- |
  // | 46  | E0 7C     | E0 F0 7C   |
  hid2ps2[0x46] = {0x7c, PS2_KEY_REPEAT_NO & PS2_KEY_TYPE_E0};

  // PS/2 scan code prefix with E1
  // | HID | PS/2 Make               | PS/2 Break |
  // | --- | ----------------------- | ---------- |
  // | 48  | E1 14 77 E1 F0 14 F0 77 | None       |

  hid2ps2[0x48] = {0x0, PS2_KEY_REPEAT_NO & PS2_KEY_TYPE_E1};  // Pause/Break

  //  // A-Z
  //  k[0x04] = 0x1C;
  //  k[0x05] = 0x32;
  //  k[0x06] = 0x21;
  //  k[0x07] = 0x23;
  //  k[0x08] = 0x24;
  //  k[0x09] = 0x2B;
  //  k[0x0A] = 0x34;
  //  k[0x0B] = 0x33;
  //  k[0x0C] = 0x43;
  //  k[0x0D] = 0x3B;
  //  k[0x0E] = 0x42;
  //  k[0x0F] = 0x4B;
  //  k[0x10] = 0x3A;
  //  k[0x11] = 0x31;
  //  k[0x12] = 0x44;
  //  k[0x13] = 0x4D;
  //  k[0x14] = 0x15;
  //  k[0x15] = 0x2D;
  //  k[0x16] = 0x1B;
  //  k[0x17] = 0x2C;
  //  k[0x18] = 0x3C;
  //  k[0x19] = 0x2A;
  //  k[0x1A] = 0x1D;
  //  k[0x1B] = 0x22;
  //  k[0x1C] = 0x35;
  //  k[0x1D] = 0x1A;
  //
  //  // 1-0
  //  k[0x1E] = 0x16;
  //  k[0x1F] = 0x1E;
  //  k[0x20] = 0x26;
  //  k[0x21] = 0x25;
  //  k[0x22] = 0x2E;
  //  k[0x23] = 0x36;
  //  k[0x24] = 0x3D;
  //  k[0x25] = 0x3E;
  //  k[0x26] = 0x46;
  //  k[0x27] = 0x45;
  //
  //  k[0x28] = 0x5A;  // Return
  //  k[0x29] = 0x76;  // Escape
  //  k[0x2A] = 0x66;  // Backspace
  //  k[0x2B] = 0x0D;  // Tab
  //  k[0x2C] = 0x29;  // Space
  //  k[0x2D] = 0x4E;  // -_
  //  k[0x2E] = 0x55;  // =+
  //  k[0x2F] = 0x54;  // [{
  //  k[0x30] = 0x5B;  // ]}
  //  k[0x31] = 0x5D;  // \|
  //  k[0x32] = 0x5D;
  //  k[0x33] = 0x4C;  // ;:
  //  k[0x34] = 0x52;  // '"
  //  k[0x35] = 0x0E;  // `~
  //  k[0x36] = 0x41;  // ,<
  //  k[0x37] = 0x49;  // .>
  //  k[0x38] = 0x4A;  // /?
  //  k[0x39] = 0x58;  // Caps Lock
  //
  //  // F1-F12
  //  k[0x3A] = 0x05;
  //  k[0x3B] = 0x06;
  //  k[0x3C] = 0x04;
  //  k[0x3D] = 0x0C;
  //  k[0x3E] = 0x03;
  //  k[0x3F] = 0x0B;
  //  k[0x40] = 0x83;
  //  k[0x41] = 0x0A;
  //  k[0x42] = 0x01;
  //  k[0x43] = 0x09;
  //  k[0x44] = 0x78;
  //  k[0x45] = 0x07;
  //
  //  k[0x47] = 0x7E;  // Scroll Lock
  //
  //  k[0x53] = 0x77;  // Num Lock
  //
  //  k[0xE0] = 0x14;  // Left Control
  //  k[0xE1] = 0x12;  // Left Shift
  //  k[0xE2] = 0x11;  // Left Alt
  //
  //  k[0xE5] = 0x59;  // Right Shift
  //
  //
  //  k_e0[0x46] = 0x7C;  // Print Screen
  //  k_e0[0x49] = 0x70;  // Insert
  //  k_e0[0x4A] = 0x6C;  // Home
  //  k_e0[0x4B] = 0x7D;  // Page Up
  //  k_e0[0x4C] = 0x71;  // Delete
  //  k_e0[0x4D] = 0x69;  // End
  //  k_e0[0x4E] = 0x7A;  // Page Down
  //  k_e0[0x4F] = 0x74;  // Right Arrow
  //  k_e0[0x50] = 0x6B;  // Left Arrow
  //  k_e0[0x51] = 0x72;  // Down Arrow
  //  k_e0[0x52] = 0x75;  // Up Arrow
  //
  //  k_e0[0x54] = 0x4A;  // Keypad /
  //  k_e0[0x58] = 0x5A;  // Keypad Enter
  //
  //  k_e0[0x65] = 0x2F;  // App
  //
  //  k_e0[0xE3] = 0x1F;  // Left GUI
  //  k_e0[0xE4] = 0x14;  // Right Control
  //  k_e0[0xE6] = 0x11;  // Right Alt
  //  k_e0[0xE7] = 0x27;  // Right GUI
}

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>

class KbdRptParser : public KeyboardReportParser {
  void PrintKey(uint8_t mod, uint8_t key);

 protected:
  void OnControlKeysChanged(uint8_t before, uint8_t after);

  void OnKeyDown(uint8_t mod, uint8_t key);
  void OnKeyUp(uint8_t mod, uint8_t key);
  void OnKeyPressed(uint8_t key);
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

  Ps2Key& ps2_key = hid2ps2[key];
  Ps2KeyPress(ps2_key);

  if (Ps2KeyRepeatYes(ps2_key.flag)) {
    last_press_key = &ps2_key;
  } else {
    last_press_key = nullptr;
  }

  key_repeat_ct = 0;
  key_repeat_delay_ct = 0;
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

    if (afterMod.bmLeftCtrl) {
      Ps2Key& ps2_key = hid2ps2[HID_KEY_LEFT_CONTROL];
      Ps2KeyPress(ps2_key);

      last_press_key = nullptr;
    } else {
      Ps2Key& ps2_key = hid2ps2[HID_KEY_LEFT_CONTROL];
      Ps2KeyRelease(ps2_key);
    }
  }
  if (beforeMod.bmLeftShift != afterMod.bmLeftShift) {
    Serial.println("LeftShift changed");

    if (afterMod.bmLeftShift) {
      Ps2Key& ps2_key = hid2ps2[HID_KEY_LEFT_SHIFT];
      Ps2KeyPress(ps2_key);

      last_press_key = nullptr;
    } else {
      Ps2Key& ps2_key = hid2ps2[HID_KEY_LEFT_SHIFT];
      Ps2KeyRelease(ps2_key);
    }
  }
  if (beforeMod.bmLeftAlt != afterMod.bmLeftAlt) {
    Serial.println("LeftAlt changed");

    if (afterMod.bmLeftAlt) {
      Ps2Key& ps2_key = hid2ps2[HID_KEY_LEFT_ALT];
      Ps2KeyPress(ps2_key);

      last_press_key = nullptr;
    } else {
      Ps2Key& ps2_key = hid2ps2[HID_KEY_LEFT_ALT];
      Ps2KeyRelease(ps2_key);
    }
  }
  if (beforeMod.bmLeftGUI != afterMod.bmLeftGUI) {
    Serial.println("LeftGUI changed");

    if (afterMod.bmLeftGUI) {
      Ps2Key& ps2_key = hid2ps2[HID_KEY_LEFT_GUI];
      Ps2KeyPress(ps2_key);

      last_press_key = nullptr;
    } else {
      Ps2Key& ps2_key = hid2ps2[HID_KEY_LEFT_GUI];
      Ps2KeyRelease(ps2_key);
    }
  }

  if (beforeMod.bmRightCtrl != afterMod.bmRightCtrl) {
    Serial.println("RightCtrl changed");

    if (afterMod.bmRightCtrl) {
      Ps2Key& ps2_key = hid2ps2[HID_KEY_RIGHT_CONTROL];
      Ps2KeyPress(ps2_key);

      last_press_key = nullptr;
    } else {
      Ps2Key& ps2_key = hid2ps2[HID_KEY_RIGHT_CONTROL];
      Ps2KeyRelease(ps2_key);
    }
  }
  if (beforeMod.bmRightShift != afterMod.bmRightShift) {
    Serial.println("RightShift changed");

    if (afterMod.bmRightShift) {
      Ps2Key& ps2_key = hid2ps2[HID_KEY_RIGHT_SHIFT];
      Ps2KeyPress(ps2_key);

      last_press_key = nullptr;
    } else {
      Ps2Key& ps2_key = hid2ps2[HID_KEY_RIGHT_SHIFT];
      Ps2KeyRelease(ps2_key);
    }
  }
  if (beforeMod.bmRightAlt != afterMod.bmRightAlt) {
    Serial.println("RightAlt changed");

    if (afterMod.bmRightAlt) {
      Ps2Key& ps2_key = hid2ps2[HID_KEY_RIGHT_ALT];
      Ps2KeyPress(ps2_key);

      last_press_key = nullptr;
    } else {
      Ps2Key& ps2_key = hid2ps2[HID_KEY_RIGHT_ALT];
      Ps2KeyRelease(ps2_key);
    }
  }
  if (beforeMod.bmRightGUI != afterMod.bmRightGUI) {
    Serial.println("RightGUI changed");

    if (afterMod.bmRightGUI) {
      Ps2Key& ps2_key = hid2ps2[HID_KEY_RIGHT_GUI];
      Ps2KeyPress(ps2_key);

      last_press_key = nullptr;
    } else {
      Ps2Key& ps2_key = hid2ps2[HID_KEY_RIGHT_GUI];
      Ps2KeyRelease(ps2_key);
    }
  }

  key_repeat_ct = 0;
  key_repeat_delay_ct = 0;
}

void KbdRptParser::OnKeyUp(uint8_t mod, uint8_t key) {
  Serial.print("UP ");
  PrintKey(mod, key);

  Ps2Key& ps2_key = hid2ps2[key];
  Ps2KeyRelease(ps2_key);

  if (last_press_key && last_press_key->key == ps2_key.key &&
      last_press_key->flag == ps2_key.flag) {
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

  if (last_press_key == nullptr) {
    return;
  }

  if (key_repeat_delay_ct >= KEY_REPEAT_DELAY) {
    key_repeat_ct = (key_repeat_ct + 1) % KEY_REPEAT_INTERVAL;
    if (key_repeat_ct == 0) {
      Ps2Key& ps2_key = *last_press_key;
      Ps2KeyPress(ps2_key);
    }
  } else {
    ++key_repeat_delay_ct;
  }
}
