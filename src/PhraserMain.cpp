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

Thumby* thumby = new Thumby();

void play_with_words() {
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
  
  playMessageSound(thumby);
}

int i = 0;
void loop() {
  //  Serial.printf("PSM-1 (Phraser) %d\n", i++);

  // Clear the screen to black
  thumby->clear();

  //symbolLoop(thumby);
  
  //keyboardLoop(thumby, true);

  listLoop(thumby);

  // Receive and display a message from link
  receive(thumby);

  // Update the screen
  thumby->writeBuffer(thumby->getBuffer(), thumby->getBufferSize());
}
