#pragma once

#include <Thumby.h>

extern const uint16_t BANK_BLOCK_COUNT;
extern const uint16_t AES256_KEY_LENGTH;
extern const uint16_t AES256_IV_LENGTH;

void createNewDbInit();
void createNewDbLoop(Thumby* thumby);
