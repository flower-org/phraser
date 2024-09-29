#include "TextField.h"

char* showWord = 0;

void initTextField() {
  if (showWord != 0) {
    free(showWord);
  }
  
  showWord = (char*)malloc(sizeof(char));
  *showWord = '\0';
}

//TODO: change to x2 resize
void appendChar(char c) {
  int wordLength = strlen(showWord);
  if (wordLength < MAX_INPUT_LENGTH) {
    char* newShowWord = (char*)malloc((wordLength+2)*sizeof(char));
    *newShowWord = '\0';

    strncat(newShowWord, showWord, wordLength);
    strncat(newShowWord, &c, 1);
    
    free(showWord);
    showWord = newShowWord;
  }
}

void deleteLastChar() {
  int wordLength = strlen(showWord);
  if (wordLength > 0) {
    showWord[wordLength-1] = '\0';
  }
}

void textFieldLoop(Thumby* thumby) {
  drawRect(thumby, 0, 0, 71, 11, WHITE);

  int wordLength = strlen(showWord);
  if (wordLength <= MAX_DISPLAY_CHARS) {
    for (int i = 0; i <= wordLength; i++) {
      drawLetter(thumby, showWord[i], 3 + i*6, 2, WHITE, false);
    }
  } else {
    int diff = wordLength - MAX_DISPLAY_CHARS;
    drawThreeDots(thumby, 3, 2, WHITE, false);
    for (int i = 1; i < MAX_DISPLAY_CHARS; i++) {
      drawLetter(thumby, showWord[diff + i], 3 + i*6, 2, WHITE, false);
    }
  }
}
