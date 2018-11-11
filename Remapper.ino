#include "Keyboard.h"
#include <hidboot.h>
#include <usbhub.h>

#include <SPI.h>

typedef struct {
  uint8_t code;
  bool shift;
  bool alt;
} OutputKeystroke;

typedef struct {
  OutputKeystroke noShift;
  OutputKeystroke withShift;
} InputKeystroke;

typedef struct {
  boolean isPressed;
  uint8_t code;
} OutputModifier;

OutputModifier leftShift = {false, KEY_LEFT_SHIFT};
OutputModifier rightShift = {false, KEY_RIGHT_SHIFT};
OutputModifier leftAlt = {false, KEY_LEFT_ALT};
OutputModifier rightAlt = {false, KEY_RIGHT_ALT};
OutputModifier leftCtrl = {false, KEY_LEFT_CTRL};
OutputModifier rightCtrl = {false, KEY_RIGHT_CTRL};
OutputModifier leftGui = {false, KEY_LEFT_GUI};
OutputModifier rightGui = {false, KEY_RIGHT_GUI};

#include "OemToAscii"

class KbdRptParser : public KeyboardReportParser
{
    void PrintKey(uint8_t mod, uint8_t key);

  protected:
    void OnControlKeysChanged(uint8_t before, uint8_t after);

    void OnKeyDown	(uint8_t mod, uint8_t key);
    void OnKeyUp	(uint8_t mod, uint8_t key);
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

bool isShiftPressed() {
  return (leftShift.isPressed || rightShift.isPressed);
}

OutputKeystroke* convertOemToAscii(uint8_t key) {
  if (isShiftPressed()) {
    return &(oemToAscii[key].withShift);
  } else {
    return &(oemToAscii[key].noShift);
  }
}

void prepareModifier(bool left, bool right, uint8_t leftKey, uint8_t rightKey) {
}

void KbdRptParser::OnKeyDown(uint8_t mod, uint8_t key)
{
  Serial.print("DN ");
  OutputKeystroke* output = convertOemToAscii(key);
  if (output->shift) {
    if (!leftShift.isPressed && !rightShift.isPressed) {
      Keyboard.press(leftShift.code);
    }
  } else {
    if (leftShift.isPressed) {
      Keyboard.release(leftShift.code);
    }
    if (rightShift.isPressed) {
      Keyboard.release(rightShift.code);
    }
  }
  Keyboard.press(output->code);
  if (output->shift) {
    if (!leftShift.isPressed && !rightShift.isPressed) {
      Keyboard.release(leftShift.code);
    }
  } else {
    if (leftShift.isPressed) {
      Keyboard.press(leftShift.code);
    }
    if (rightShift.isPressed) {
      Keyboard.press(rightShift.code);
    }
  }

  PrintKey(mod, key);
}

void KbdRptParser::OnKeyUp(uint8_t mod, uint8_t key)
{
  Serial.print("UP ");
  OutputKeystroke* output = convertOemToAscii(key);
  Keyboard.release(output->code);

  PrintKey(mod, key);
}

void handleModifierChange(uint8_t modifierState, uint8_t modifierCode) {
  if (modifierState == 1) {
    Keyboard.press(modifierCode);
  } else {
    Keyboard.release(modifierCode);
  }
}

void KbdRptParser::OnControlKeysChanged(uint8_t before, uint8_t after) {

  MODIFIERKEYS beforeMod;
  *((uint8_t*)&beforeMod) = before;

  MODIFIERKEYS afterMod;
  *((uint8_t*)&afterMod) = after;

  if (beforeMod.bmLeftCtrl != afterMod.bmLeftCtrl) {
    Serial.println("LeftCtrl changed");
    handleModifierChange(afterMod.bmLeftCtrl, leftCtrl.code);
    leftCtrl.isPressed = afterMod.bmLeftCtrl;
  }
  if (beforeMod.bmLeftShift != afterMod.bmLeftShift) {
    Serial.println("LeftShift changed");
    handleModifierChange(afterMod.bmLeftShift, leftShift.code);
    leftShift.isPressed = afterMod.bmLeftShift;
  }
  if (beforeMod.bmLeftAlt != afterMod.bmLeftAlt) {
    Serial.println("LeftAlt changed");
    handleModifierChange(afterMod.bmLeftAlt, leftAlt.code);
    leftAlt.isPressed = afterMod.bmLeftAlt;
  }
  if (beforeMod.bmLeftGUI != afterMod.bmLeftGUI) {
    Serial.println("LeftGUI changed");
    handleModifierChange(afterMod.bmLeftGUI, leftGui.code);
    leftGui.isPressed = afterMod.bmLeftGUI;
  }

  if (beforeMod.bmRightCtrl != afterMod.bmRightCtrl) {
    Serial.println("RightCtrl changed");
    handleModifierChange(afterMod.bmRightCtrl, rightCtrl.code);
    rightCtrl.isPressed = afterMod.bmRightCtrl;
  }
  if (beforeMod.bmRightShift != afterMod.bmRightShift) {
    Serial.println("RightShift changed");
    handleModifierChange(afterMod.bmRightShift, rightShift.code);
    rightShift.isPressed = afterMod.bmRightShift;
  }
  if (beforeMod.bmRightAlt != afterMod.bmRightAlt) {
    Serial.println("RightAlt changed");
    handleModifierChange(afterMod.bmRightAlt, rightAlt.code);
    rightAlt.isPressed = afterMod.bmRightAlt;
  }
  if (beforeMod.bmRightGUI != afterMod.bmRightGUI) {
    Serial.println("RightGUI changed");
    handleModifierChange(afterMod.bmRightGUI, rightGui.code);
    rightGui.isPressed = afterMod.bmRightGUI;
  }

}

USB     Usb;
//USBHub     Hub(&Usb);
HIDBoot<USB_HID_PROTOCOL_KEYBOARD>    HidKeyboard(&Usb);

KbdRptParser Prs;

void setup()
{
  Serial.begin( 9600 );
#if !defined(__MIPSEL__)
  while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif
  Serial.println("Start");

  if (Usb.Init() == -1)
    Serial.println("OSC did not start.");

  delay( 200 );

  HidKeyboard.SetReportParser(0, &Prs);
  Keyboard.begin();
}

void loop()
{
  Usb.Task();
}

