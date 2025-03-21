#pragma once

#include <Thumby.h>

#include "PhraserUtils.h"
#include "TextField.h"
#include "Schema_generated.h"

void listLoop(Thumby* thumby);

void drawIcon(Thumby* thumby, int lineIndex, phraser::Icon icon);
void drawLineSelectionBorder(Thumby* thumby, int lineIndex);
void printLineText(Thumby* thumby, int lineIndex, char* str);
void listLoop(Thumby* thumby);