#include <Thumby.h>
#include <Keyboard.h>

#include "ThumbyUtils.h"
#include "PhraserUtils.h"
#include "ScreenKeyboard.h"
#include "ScreenList.h"
#include "SpecialSymbolDrawer.h"
#include "TextField.h"
#include "Registry.h"

#include "Schema_generated.h"
#include "RootTypeFinishingMethods.h"

using namespace phraser;

typedef enum {
  INIT_SCREEN,
  UNSEAL,
  BACKUP,
  RESTORE,
  TEST_KEYBOARD,
  UNSEAL_SHOW_PASS,
  CREATE_NEW_DB
} Mode;


Mode currentMode;
Thumby* thumby = new Thumby();

ListItem** init_screen_items = NULL;
const int init_screen_item_count = 6;
void mainScreenInit() {
  init_screen_items = (ListItem**)malloc(init_screen_item_count * sizeof(ListItem*));

  init_screen_items[0] = createListItem("Unseal", phraser::Icon_Lock);
  init_screen_items[1] = createListItem("Backup DB", phraser::Icon_Download);
  init_screen_items[2] = createListItem("Restore DB", phraser::Icon_Upload);
  init_screen_items[3] = createListItem("Test Keyboard", phraser::Icon_TextOut);
  init_screen_items[4] = createListItem("Unseal (show password)", phraser::Icon_Lock);
  init_screen_items[5] = createListItem("Create New DB", phraser::Icon_Settings);

  initList(init_screen_items, init_screen_item_count);
}

void releaseMainScreenContext() {
  freeList(init_screen_items, init_screen_item_count);
  init_screen_items = NULL;
}

void playWithWords() {
  flatbuffers::FlatBufferBuilder builder(1024);
  auto name = builder.CreateString("worrd_name");
  auto word = builder.CreateString("worrd_tmpl");
  short word_id = 3;

  //auto name2 = builder.CreateString("worrd_tmpl_2");
  //short word_id2 = 23;

  auto word1 = CreateWord(builder, word_id, 0, name, word, 15, phraser::Icon_GTTriangle);

  builder.Finish(word1);

  uint8_t *buf = builder.GetBufferPointer();
  int size = builder.GetSize();

  Serial.printf("serialized address %d\n", buf);
  Serial.printf("serialized size %d\n\n", size);

  const phraser::Word* word_r = GetWord(buf);

  const flatbuffers::String* wordName = word_r->word();
  const char* wordCstr = wordName->c_str();

  Serial.printf("deserialized template_id %d\n", word_r->word_template_id());
  Serial.printf("deserialized word %s\n", wordCstr);
}

void setup() {
  // Sets up buttons, audio, link pins, and screen
  thumby->begin();

  currentMode = INIT_SCREEN;
  mainScreenInit();

/*
  // Init duplex UART for Thumby to PC comms
  Keyboard.begin(KeyboardLayout_en_US);

  //TODO: Disable serial in the final version
  Serial.begin(115200);
  delay(1000);

  Serial.printf("PSM-1 (Phraser)\n");
  Serial.printf("Thumby (Pi Pico) USB Password Manager\n\n");

  play_with_words();

  // Make sure RX buffer is empty
  removeRxBytes();

  drawLoadingScreen(thumby);

  loadRegistryFromFlash();

  initOnScreenKeyboard();
  
  mainScreenInit();

  playMessageSound(thumby);*/
}

void initScreenLoop() {
  ListItem* chosenItem = listLoop(thumby);
  if (chosenItem != NULL) {
    if (chosenItem == init_screen_items[0]) { currentMode = UNSEAL; } //Unseal
    else if (chosenItem == init_screen_items[1]) { currentMode = BACKUP; } //Backup DB
    else if (chosenItem == init_screen_items[2]) { currentMode = RESTORE; } //Restore DB
    else if (chosenItem == init_screen_items[3]) { currentMode = TEST_KEYBOARD; } //Test Keyboard
    else if (chosenItem == init_screen_items[4]) { currentMode = UNSEAL_SHOW_PASS; } //Unseal (show password)
    else if (chosenItem == init_screen_items[5]) { currentMode = CREATE_NEW_DB; } //New DB
    releaseMainScreenContext();
  }
}

void backupLoop() {
  drawMessage(thumby, "backupLoop");
}

void restoreLoop() {
  drawMessage(thumby, "restoreLoop");
}

void testKeyboardLoop() {
  drawMessage(thumby, "testKeyboardLoop");
}

void createNewDbLoop() {
  drawMessage(thumby, "createNewDbLoop");
}

void unsealLoop() {
  drawMessage(thumby, "unsealLoop");
}

void unsealShowPassLoop() {
  drawMessage(thumby, "unsealShowPassLoop");
}

void loop() {
  // Clear the screen to black
  thumby->clear();

  switch (currentMode) {
    case INIT_SCREEN: initScreenLoop(); break;
    case UNSEAL: unsealLoop(); break;
    case BACKUP: backupLoop(); break;
    case RESTORE: restoreLoop(); break;
    case TEST_KEYBOARD: testKeyboardLoop(); break;
    case UNSEAL_SHOW_PASS: unsealShowPassLoop(); break;
    case CREATE_NEW_DB: createNewDbLoop(); break;
  }

  // Update the screen
  thumby->writeBuffer(thumby->getBuffer(), thumby->getBufferSize());


  //symbolLoop(thumby);
  
  //keyboardLoop(thumby, true);

  //listLoop(thumby);

  //Receive and display a message from link
  //receive(thumby);
}
