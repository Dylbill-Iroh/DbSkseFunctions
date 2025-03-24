#pragma once

namespace input {
RE::INPUT_DEVICE GetInputDeviceForKeyCode(int keyCode);

void TapKey(int keyCode);

void TapKey(std::string keyString);

void HoldKey(int keyCode, int holdTimeMilliSeconds);

void HoldKey(std::string keyString, int holdTimeMilliSeconds);
}