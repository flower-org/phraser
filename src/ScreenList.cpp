#include "ScreenList.h"

int lines[4] = { 0, 10, 20, 30 };

void drawIcon(Thumby* thumby, int lineIndex, phraser::Icon icon) {
  int icon_offset_x = 1;
  int icon_offset_y = 2;
  int line = lines[lineIndex];

  drawFolderIcon(thumby, icon_offset_x, line+icon_offset_y, WHITE);

  switch (icon) {
    case phraser::Icon_Key: 
      drawKey(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser::Icon_Login: 
      drawLogin(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser::Icon_Asterisk: 
      drawAsterisk(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser::Icon_Lock: 
      drawLock(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser::Icon_Aa: 
      drawAa(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser::Icon_Star: 
      drawStar(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser::Icon_Settings: 
      drawSettings(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser::Icon_Folder: 
      drawFolderIcon(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser::Icon_ToParentFolder: 
      drawToParentFolder(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser::Icon_LookingGlass: 
      drawLookingGlass(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser::Icon_LTTriangle: 
      drawLTTriangle(thumby, icon_offset_x, line+icon_offset_y, WHITE, true);
      break;
    case phraser::Icon_GTTriangle: 
      drawGTTriangle(thumby, icon_offset_x, line+icon_offset_y, WHITE, true);
      break;
    case phraser::Icon_TextOut: 
      drawTextOut(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser::Icon_Ledger: 
      drawLedger(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser::Icon_PlusMinus: 
      drawPlusMinus(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser::Icon_Stars: 
      drawStars(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser::Icon_Message: 
      drawMessage(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser::Icon_Quote: 
      drawQuote(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser::Icon_Question: 
      drawQuestion(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser::Icon_Plus: 
      drawPlus(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser::Icon_Minus: 
      drawMinus(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser::Icon_X: 
      drawX(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser::Icon_Check: 
      drawCheck(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    case phraser::Icon_Copy: 
      drawCopy(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
    default: 
      drawFolderIcon(thumby, icon_offset_x, line+icon_offset_y, WHITE);
      break;
  }
}

void drawLineSelectionBorder(Thumby* thumby, int lineIndex) {
  int selection_border_x1 = 11;
  int selection_border_x2 = 71;
  int selection_border_offset_y1 = 0;
  int selection_border_offset_y2 = 10;

  int line = lines[lineIndex];
  drawRect(thumby, selection_border_x1, line + selection_border_offset_y1, 
    selection_border_x2, line + selection_border_offset_y2, WHITE);
}

// TODO: scroll text horizontally if it's too long
void printLineText(Thumby* thumby, int lineIndex, char* str) {
  int text_offset = 13;
  int text_offset_y = 1;
  int line = lines[lineIndex];

  printAt(thumby, text_offset, line + text_offset_y, str);
}

int r = 0;
void listLoop(Thumby* thumby) {
  int line1 = 0;
  int line2 = 10;
  int line3 = 20;
  int line4 = 30;

  drawIcon(thumby, 0, phraser::Icon_Folder);
  drawIcon(thumby, 1, phraser::Icon_Folder);
  drawIcon(thumby, 2, phraser::Icon_Folder);
  drawIcon(thumby, 3, phraser::Icon_Folder);

  switch (r%4) {
    case 0:
      drawLineSelectionBorder(thumby, 0);
      break;
    case 1:
      drawLineSelectionBorder(thumby, 1);
      break;
    case 2:
      drawLineSelectionBorder(thumby, 2);
      break;
    case 3:
      drawLineSelectionBorder(thumby, 3);
      break;
  }
  r++;
  delay(2500);

  printLineText(thumby, 0, "Unseal");
  printLineText(thumby, 1, "Download DB");
  printLineText(thumby, 2, "Upload DB");
  printLineText(thumby, 3, "Unseal (show password)");
}
