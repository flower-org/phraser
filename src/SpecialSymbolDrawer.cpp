#include "SpecialSymbolDrawer.h"

void symbolLoop(Thumby* thumby) {
  drawFolderIcon(thumby, 3+12*0, 3, WHITE);
  drawToParentFolder(thumby, 3+12*1, 3, WHITE);
  drawAsterisk(thumby, 3+12*2, 3, WHITE);
  drawKey(thumby, 3+12*3, 3, WHITE);
  drawAa(thumby, 3+12*4, 3, WHITE);
  drawLTTriangle(thumby, 3+12*5, 3, WHITE, false);
  
  drawSettings(thumby, 3+12*0, 13, WHITE);
  drawLock(thumby, 3+12*1, 13, WHITE);
  drawLogin(thumby, 3+12*2, 13, WHITE);
  drawStar(thumby, 3+12*3, 13, WHITE);
  drawLookingGlass(thumby, 3+12*4, 13, WHITE);
  drawGTTriangle(thumby, 3+12*5, 13, WHITE, false);

  drawTextOut(thumby, 3+12*0, 23, WHITE);
  drawLedger(thumby, 3+12*1, 23, WHITE);
  drawPlusMinus(thumby, 3+12*2, 23, WHITE);
  drawStars(thumby, 3+12*3, 23, WHITE);
  drawMessage(thumby, 3+12*4, 23, WHITE);
  drawQuote(thumby, 3+12*5, 23, WHITE);
  
  drawQuestion(thumby, 3+12*0, 33, WHITE);
  drawPlus(thumby, 3+12*1, 33, WHITE);
  drawMinus(thumby, 3+12*2, 33, WHITE);
  drawX(thumby, 3+12*3, 33, WHITE);
  drawCheck(thumby, 3+12*4, 33, WHITE);
  drawCopy(thumby, 3+12*5, 33, WHITE);
}
