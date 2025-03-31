#include "MainDbUi.h"

#include "TextAreaDialog.h"

void phraserDbUiInit() {
  char* text = "DB decryption done.\nThis is Main DB UI.";
  initTextAreaDialog(text, strlen(text), TEXT_AREA);
}

void phraserDbUiLoop(Thumby* thumby) {
  textAreaLoop(thumby);
}
