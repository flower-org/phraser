#pragma once

#include "Arduino.h"
#include "BlockStore.h"
#include <stdarg.h>

#define ICON_FOLDER_ICON 1
#define ICON_TO_PARENT_FOLDER 2
#define ICON_ASTERISK 3
#define ICON_KEY 4
#define ICON_SETTINGS 5
#define ICON_LOCK 6
#define ICON_LOGIN 7
#define ICON_STAR 8
#define ICON_LOOKING_GLASS 9
#define ICON_AA 10
#define ICON_TEXT_OUT 11
#define ICON_LEDGER 12
#define ICON_PLUS_MINUS 13
#define ICON_STARS 14
#define ICON_MESSAGE 15
#define ICON_QUOTE 16
#define ICON_QUESTION 17
#define ICON_PLUS 18
#define ICON_MINUS 19
#define ICON_X 20
#define ICON_CHECK 21
#define ICON_COPY 22

#define FLAGS_TYPE 1
#define FLAGS_GENERATE 2
#define FLAGS_INPUT 4
#define FLAGS_VIEW 8

#define ROOT_FOLDER_ID 0
#define WEBSITES_FOLDER_ID 1
#define BANKING_FOLDER_ID 2
#define TERMINAL_FOLDER_ID 3

#define URL_LOGIN_PASSWORD_TEMPLATE_ID 1
#define SIMPLE_WORD_TEMPLATE_ID 2
#define LOGIN_PASSWORD_TEMPLATE_ID 3

void resetStoreAndFillBlocksWithRandomBytes();
void createAndSaveStandardInitialBlocks();
