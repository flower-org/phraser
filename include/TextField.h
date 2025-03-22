#pragma once

#include <Thumby.h>
#include <Arduino.h>

#include "PhraserUtils.h"

const int MAX_DISPLAY_CHARS = 11;
const int MAX_INPUT_LENGTH = 256;

void textFieldLoop(Thumby* thumby);

void initTextField();
void appendChar(char c);
void deleteLastChar();
void setChar(char c);
