#include "TextField.h"

char* textFieldText = NULL;
unsigned long last_letter_added;
bool password_mode;

void initTextField(bool set_password_mode) {
  if (textFieldText != NULL) {
    free(textFieldText);
  }
  
  textFieldText = (char*)malloc(sizeof(char));
  *textFieldText = '\0';
  password_mode = set_password_mode;
}

void initTextField(char* init_text, int init_text_length, bool set_password_mode) {
  if (textFieldText != NULL) {
    free(textFieldText);
  }
  
  textFieldText = (char*)malloc(init_text_length + 1); // +1 for the null terminator
  strncpy(textFieldText, init_text, init_text_length);
  textFieldText[init_text_length] = '\0'; // Ensure null termination

  password_mode = set_password_mode;
}

//TODO: change to x2 resize?
void appendChar(char c) {
  int wordLength = strlen(textFieldText);
  if (wordLength < MAX_INPUT_LENGTH) {
    char* newTextFiedText = (char*)malloc((wordLength+2)*sizeof(char));
    *newTextFiedText = '\0';

    strncat(newTextFiedText, textFieldText, wordLength);
    strncat(newTextFiedText, &c, 1);
    
    free(textFieldText);
    textFieldText = newTextFiedText;
    last_letter_added = millis();
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
  last_letter_added = 0;
}

void textFieldLoop(Thumby* thumby) {
  drawRect(thumby, 0, 0, 71, 11, WHITE);

  int wordLength = strlen(textFieldText);
  if (wordLength <= MAX_DISPLAY_CHARS) {
    for (int i = 0; i < wordLength; i++) {
      if (!password_mode) {
        drawLetter(thumby, textFieldText[i], 3 + i*6, 2, WHITE, false);
      } else {
        bool is_last_char = i == wordLength-1;
        bool show_char = is_last_char && (last_letter_added + 500) > millis();
        drawLetter(thumby, show_char ? textFieldText[i] : '*', 3 + i*6, 2, WHITE, false);
      }
    }
  } else {
    int diff = wordLength - MAX_DISPLAY_CHARS;
    drawThreeDots(thumby, 3, 2, WHITE, false);
    for (int i = 1; i < MAX_DISPLAY_CHARS; i++) {
      if (!password_mode) {
        drawLetter(thumby, textFieldText[diff + i], 3 + i*6, 2, WHITE, false);
      } else {
        bool is_last_char = diff + i == wordLength-1;
        bool show_char = is_last_char && (last_letter_added + 500) > millis();
        drawLetter(thumby, show_char ? textFieldText[diff + i] : '*', 3 + i*6, 2, WHITE, false);
      }
    }
  }
}

char* getTextFieldText() {
  return textFieldText;
}