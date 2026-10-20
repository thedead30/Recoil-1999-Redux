// SUBSYSTEM: platform
// The three DirectInput data formats Recoil passes to IDirectInputDevice::SetDataFormat. The original links dinput.lib's copies
// into its .text section (0x004c73f0 / 0x004c7480 / 0x004c7ee0, 03_re/ledger/text_data_symbols.csv); the port uses the Windows
// SDK's (dinput8.lib, CMakeLists.txt). PLATFORM - tests/test_dinput_formats_native.cpp checks they equal the exe's tables.
// Declared with C linkage and an opaque type (the full DIDATAFORMAT needs <dinput.h>): only their addresses are taken.
#pragma once

extern "C" const unsigned int c_dfDIKeyboard[6];
extern "C" const unsigned int c_dfDIMouse[6];
extern "C" const unsigned int c_dfDIJoystick2[6];
