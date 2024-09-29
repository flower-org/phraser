#include "StreamUtils.h"

inline void moveCursorBy(void** cursor, size_t moveBy) {
  *cursor = (*cursor) + moveBy;
}

void moveCursor(void** cursor, size_t moveBy) {
  return moveCursorBy(cursor, moveBy);
}

void streamLoadData(void** cursor, void* to, size_t size) {
  memcpy(to, *cursor, size);
  moveCursorBy(cursor, size);
}

uint8_t streamLoadUint8_t(void** cursor) {
  uint8_t result;
  streamLoadData(cursor, &result, 1);
  
  return result;
}

uint16_t streamLoadUint16_t(void** cursor) {
  uint16_t result;
  streamLoadData(cursor, &result, 2);
  
  return result;
}

uint32_t streamLoadUint32_t(void** cursor) {
  uint32_t result;
  streamLoadData(cursor, &result, 4);
  
  return result;
}

void streamSaveData(void** cursor, void* from, size_t size) {
  memcpy(*cursor, from, size);
  moveCursorBy(cursor, size);
}

void streamSaveUint8_t(void** cursor, uint8_t value) {
  streamSaveData(cursor, &value, 1);
}

void streamSaveUint16_t(void** cursor, uint16_t value) {
  streamSaveData(cursor, &value, 2);
}

void streamSaveUint32_t(void** cursor, uint32_t value) {
  streamSaveData(cursor, &value, 4);
}
