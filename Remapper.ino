#include "Keyboard.h"
#include <hidboot.h>
#include <usbhub.h>

#include <SPI.h>

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

void KbdRptParser::OnKeyDown(uint8_t mod, uint8_t key)
{
  Serial.print("DN ");
  uint8_t c = oemToAscii[key];
  Keyboard.press(c);

  PrintKey(mod, key);
}

void KbdRptParser::OnKeyUp(uint8_t mod, uint8_t key)
{
  Serial.print("UP ");
  uint8_t c = oemToAscii[key];
  Keyboard.release(c);

  PrintKey(mod, key);
}

void printOut(char c) {
  for (char bits = 7; bits > -1; bits--) {
    // Compare bits 7-0 in byte
    if (c & (1 << bits)) {
      Serial.print("1");
    } else {
      Serial.print("0");
    }
  }
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

  Serial.print("before = ");
  printOut(before);
  Serial.println();
  Serial.println(beforeMod.bmLeftCtrl);
  Serial.print("after = ");
  printOut(after);
  Serial.println();
  Serial.println(afterMod.bmLeftCtrl);


  if (beforeMod.bmLeftCtrl != afterMod.bmLeftCtrl) {
    Serial.println("LeftCtrl changed");
    handleModifierChange(afterMod.bmLeftCtrl, KEY_LEFT_CTRL);
  }
  if (beforeMod.bmLeftShift != afterMod.bmLeftShift) {
    Serial.println("LeftShift changed");
    handleModifierChange(afterMod.bmLeftShift, KEY_LEFT_SHIFT);
  }
  if (beforeMod.bmLeftAlt != afterMod.bmLeftAlt) {
    Serial.println("LeftAlt changed");
    handleModifierChange(afterMod.bmLeftAlt, KEY_LEFT_ALT);
  }
  if (beforeMod.bmLeftGUI != afterMod.bmLeftGUI) {
    Serial.println("LeftGUI changed");
    handleModifierChange(afterMod.bmLeftGUI, KEY_LEFT_GUI);
  }

  if (beforeMod.bmRightCtrl != afterMod.bmRightCtrl) {
    Serial.println("RightCtrl changed");
    handleModifierChange(afterMod.bmRightCtrl, KEY_RIGHT_CTRL);
  }
  if (beforeMod.bmRightShift != afterMod.bmRightShift) {
    Serial.println("RightShift changed");
    handleModifierChange(afterMod.bmRightShift, KEY_RIGHT_SHIFT);
  }
  if (beforeMod.bmRightAlt != afterMod.bmRightAlt) {
    Serial.println("RightAlt changed");
    handleModifierChange(afterMod.bmRightAlt, KEY_RIGHT_ALT);
  }
  if (beforeMod.bmRightGUI != afterMod.bmRightGUI) {
    Serial.println("RightGUI changed");
    handleModifierChange(afterMod.bmRightGUI, KEY_RIGHT_GUI);
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

