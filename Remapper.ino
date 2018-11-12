#include "Keyboard.h"
#include <hidboot.h>
#include <usbhub.h>

#include <SPI.h>

typedef uint8_t ModifierAction;

const ModifierAction NONE = 0;
const ModifierAction SHIFT = 1;
const ModifierAction NO_SHIFT = 2;
const ModifierAction ALT = 3;

typedef struct {
  uint8_t code;
  ModifierAction modifierAction;
} OutputKeystroke;

typedef struct {
  OutputKeystroke noShift;
  OutputKeystroke withShift;
} InputKeystroke;

typedef struct {
  boolean isPressed;
  uint8_t code;
} OutputModifier;

OutputModifier outputLeftShift = {false, KEY_LEFT_SHIFT};
OutputModifier outputRightShift = {false, KEY_RIGHT_SHIFT};
OutputModifier outputLeftAlt = {false, KEY_LEFT_ALT};
OutputModifier outputRightAlt = {false, KEY_RIGHT_ALT};
OutputModifier outputLeftCtrl = {false, KEY_LEFT_CTRL};
OutputModifier outputRightCtrl = {false, KEY_RIGHT_CTRL};
OutputModifier outputLeftGui = {false, KEY_LEFT_GUI};
OutputModifier outputRightGui = {false, KEY_RIGHT_GUI};

OutputModifier* leftShift = &outputLeftShift;
OutputModifier* rightShift = &outputRightShift;
OutputModifier* leftAlt = &outputLeftGui;
OutputModifier* rightAlt = &outputRightGui;
OutputModifier* leftCtrl = &outputLeftCtrl;
OutputModifier* rightCtrl = &outputRightCtrl;
OutputModifier* leftGui = &outputLeftAlt;
OutputModifier* rightGui = &outputRightAlt;

#include "OemToAscii"

class KbdRptParser : public KeyboardReportParser
{
  protected:
    void OnControlKeysChanged(uint8_t before, uint8_t after);

    void OnKeyDown	(uint8_t mod, uint8_t key);
    void OnKeyUp	(uint8_t mod, uint8_t key);
};

bool isShiftPressed() {
  return (leftShift->isPressed || rightShift->isPressed);
}

OutputKeystroke* convertOemToAscii(uint8_t key) {
  if (key > oemToAsciiSize) {
    return &(oemToAscii[0].noShift);
  }
  if (isShiftPressed()) {
    return &(oemToAscii[key].withShift);
  } else {
    return &(oemToAscii[key].noShift);
  }
}

void normal(uint8_t code) {
  Keyboard.press(code);
}

void withShift(uint8_t code) {
  if (!outputLeftShift.isPressed && !outputRightShift.isPressed) {
    Keyboard.press(outputLeftShift.code);
  }
  Keyboard.press(code);
  Keyboard.release(code);
  if (!outputLeftShift.isPressed && !outputRightShift.isPressed) {
    Keyboard.release(outputLeftShift.code);
  }
}

void withAlt(uint8_t code) {
  if (!outputLeftAlt.isPressed && !outputRightAlt.isPressed) {
    Keyboard.press(outputLeftAlt.code);
  }
  Keyboard.press(code);
  Keyboard.release(code);
  if (!outputLeftAlt.isPressed && !outputRightAlt.isPressed) {
    Keyboard.release(outputLeftAlt.code);
  }
}

void noShift(uint8_t code) {
  if (outputLeftShift.isPressed) {
    Keyboard.release(outputLeftShift.code);
  }
  if (outputRightShift.isPressed) {
    Keyboard.release(outputRightShift.code);
  }
  Keyboard.press(code);
  Keyboard.release(code);
  if (outputLeftShift.isPressed) {
    Keyboard.press(outputLeftShift.code);
  }
  if (outputRightShift.isPressed) {
    Keyboard.press(outputRightShift.code);
  }
}

void KbdRptParser::OnKeyDown(uint8_t mod, uint8_t key)
{
  OutputKeystroke* output = convertOemToAscii(key);

  switch (output->modifierAction) {
    case NONE: normal(output->code);
    break;
    case SHIFT: withShift(output->code);
    break;
    case NO_SHIFT: noShift(output->code);
    break;
    case ALT: withAlt(output->code);
    break;
  }
}

void KbdRptParser::OnKeyUp(uint8_t mod, uint8_t key)
{
  OutputKeystroke* output = convertOemToAscii(key);

  if (output->modifierAction == NONE) {
    Keyboard.release(output->code);
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

  if (beforeMod.bmLeftCtrl != afterMod.bmLeftCtrl) {
    handleModifierChange(afterMod.bmLeftCtrl, leftCtrl->code);
    leftCtrl->isPressed = afterMod.bmLeftCtrl;
  }
  if (beforeMod.bmLeftShift != afterMod.bmLeftShift) {
    handleModifierChange(afterMod.bmLeftShift, leftShift->code);
    leftShift->isPressed = afterMod.bmLeftShift;
  }
  if (beforeMod.bmLeftAlt != afterMod.bmLeftAlt) {
    handleModifierChange(afterMod.bmLeftAlt, leftAlt->code);
    leftAlt->isPressed = afterMod.bmLeftAlt;
  }
  if (beforeMod.bmLeftGUI != afterMod.bmLeftGUI) {
    handleModifierChange(afterMod.bmLeftGUI, leftGui->code);
    leftGui->isPressed = afterMod.bmLeftGUI;
  }

  if (beforeMod.bmRightCtrl != afterMod.bmRightCtrl) {
    handleModifierChange(afterMod.bmRightCtrl, rightCtrl->code);
    rightCtrl->isPressed = afterMod.bmRightCtrl;
  }
  if (beforeMod.bmRightShift != afterMod.bmRightShift) {
    handleModifierChange(afterMod.bmRightShift, rightShift->code);
    rightShift->isPressed = afterMod.bmRightShift;
  }
  if (beforeMod.bmRightAlt != afterMod.bmRightAlt) {
    handleModifierChange(afterMod.bmRightAlt, rightAlt->code);
    rightAlt->isPressed = afterMod.bmRightAlt;
  }
  if (beforeMod.bmRightGUI != afterMod.bmRightGUI) {
    handleModifierChange(afterMod.bmRightGUI, rightGui->code);
    rightGui->isPressed = afterMod.bmRightGUI;
  }

}

USB     Usb;
HIDBoot<USB_HID_PROTOCOL_KEYBOARD>    HidKeyboard(&Usb);

KbdRptParser Prs;

void setup()
{
  Usb.Init();
  delay( 200 );
  HidKeyboard.SetReportParser(0, &Prs);
  Keyboard.begin();
}

void loop()
{
  Usb.Task();
}

