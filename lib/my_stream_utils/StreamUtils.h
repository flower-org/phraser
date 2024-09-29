#pragma once

#include <stdio.h>
#include "Arduino.h"

void moveCursor(void** cursor, size_t moveBy);

void streamLoadData(void** cursor, void* to, size_t size);

uint8_t streamLoadUint8_t(void** cursor);
uint16_t streamLoadUint16_t(void** cursor);
uint32_t streamLoadUint32_t(void** cursor);

void streamSaveData(void** cursor, void* from, size_t size);

void streamSaveUint8_t(void** cursor, uint8_t value);
void streamSaveUint16_t(void** cursor, uint16_t value);
void streamSaveUint32_t(void** cursor, uint32_t value);
