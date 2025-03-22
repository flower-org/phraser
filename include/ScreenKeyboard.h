#pragma once

#include <Thumby.h>
#include <Keyboard.h>

#include "PhraserUtils.h"
#include "TextField.h"

void initOnScreenKeyboard(bool emulated_keyboard);
char* keyboardLoop(Thumby* thumby);
