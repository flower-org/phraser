#include "TextField.h"

char* textFieldText = NULL;

void initTextField() {
  if (textFieldText != NULL) {
    free(textFieldText);
  }
  
  textFieldText = (char*)malloc(sizeof(char));
  *textFieldText = '\0';
}

//TODO: change to x2 resize
void appendChar(char c) {
  int wordLength = strlen(textFieldText);
  if (wordLength < MAX_INPUT_LENGTH) {
    char* newTextFiedText = (char*)malloc((wordLength+2)*sizeof(char));
    *newTextFiedText = '\0';

    strncat(newTextFiedText, textFieldText, wordLength);
    strncat(newTextFiedText, &c, 1);
    
    free(textFieldText);
    textFieldText = newTextFiedText;
  }
}

void setChar(char c) {
    char* newTextFieldText = (char*)malloc(2 * sizeof(char));
    newTextFieldText[0] = c;   // Set the first character
    newTextFieldText[1] = '\0'; // Set the null terminator

    free(textFieldText);
    textFieldText = newTextFieldText;
}

void deleteLastChar() {
  int wordLength = strlen(textFieldText);
  if (wordLength > 0) {
    textFieldText[wordLength-1] = '\0';
  }
}

void textFieldLoop(Thumby* thumby) {
  drawRect(thumby, 0, 0, 71, 11, WHITE);

  int wordLength = strlen(textFieldText);
  if (wordLength <= MAX_DISPLAY_CHARS) {
    for (int i = 0; i <= wordLength; i++) {
      drawLetter(thumby, textFieldText[i], 3 + i*6, 2, WHITE, false);
    }
  } else {
    int diff = wordLength - MAX_DISPLAY_CHARS;
    drawThreeDots(thumby, 3, 2, WHITE, false);
    for (int i = 1; i < MAX_DISPLAY_CHARS; i++) {
      drawLetter(thumby, textFieldText[diff + i], 3 + i*6, 2, WHITE, false);
    }
  }
}

char* getTextFieldText() {
  return textFieldText;
}