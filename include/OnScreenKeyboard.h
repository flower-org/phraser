#pragma once

#include <Thumby.h>
#include <Keyboard.h>

#include "PhraserUtils.h"
#include "TextField.h"

void initOnScreenKeyboard();
void keyboardLoop(Thumby* thumby, bool is_emulated_keyboard);
