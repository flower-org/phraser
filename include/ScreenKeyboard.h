#pragma once

#include <Thumby.h>
#include <Keyboard.h>

#include "PhraserUtils.h"
#include "TextField.h"

void initOnScreenKeyboard(bool emulated_keyboard, bool password_mode);
char* keyboardLoop(Thumby* thumby);
