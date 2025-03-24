#include "DbBackup.h"
#include "TextAreaDialog.h"
#include "UiCommon.h"

int backup_phase = 0;
void backupInit() {
  backup_phase = 0;
  char* text = "Backup DB?";
  initTextAreaDialog(text, strlen(text), DLG_YES_NO);
}

void backupLoop(Thumby* thumby) {
  if (backup_phase == 0) { // init YesNo dialog
    DialogResult result = textAreaLoop(thumby);
    if (result == DLG_RES_YES) {
      backup_phase = 1;
      char* text = "Backup DB\nnot implemented";
      initTextAreaDialog(text, strlen(text), DLG_OK);
    } else if (result == DLG_RES_NO) {
      switchToStartupScreen();
    }
  } else if (backup_phase == 1) { // YesNo dialog result Yes, start serial sequence 
    DialogResult result = textAreaLoop(thumby);
    if (result == DLG_RES_OK) {
      backup_phase = 2;
    }
  } else if (backup_phase == 2) {
    drawTurnOffMessage(thumby);
  }
}
