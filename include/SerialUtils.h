#pragma once

#include <Thumby.h>

#define HELLO 1
#define BYE 2
#define START_BACKUP 3
#define START_BACKUP_CONFIRMATION 4
#define START_RESTORE 5
#define START_RESTORE_CONFIRMATION 6
#define BACKUP_BLOCK 7
#define BACKUP_BLOCK_RECEIVED  8
#define RESTORE_BLOCK 9
#define RESTORE_BLOCK_RECEIVED 10

#define DATA_CORRECTNESS_ERROR 254

extern const uint8_t* magic_number;

void sendHelloSerial();
void sendStartBackupSerial();
void sendBye();
void sendUInt32Serial(uint32_t value);
void sendBlockSerial(uint32_t block_number);
bool canReadUInt32Serial();
uint32_t readUInt32Serial();
bool canReadOperationSerial();
uint8_t readOperationSerial();
void sendStartRestoreSerial();
void sendRestoreBlockReceivedSerial(uint32_t block_number);
