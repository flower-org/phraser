#include "ThumbyUtils.h"

char sendBuf[7] = "Hello!";
uint8_t dataBuf[7];
uint8_t packedBuf[10];

// Removes all bytes from RX buffer (use after writing on Serial1)
void removeRxBytes() {
  delay(10);
  while(Serial1.available() > 0){
    Serial1.read();
  }
}

// Play a sound that sweeps a frequency range
void playMessageSound(Thumby* thumby) {
  for(uint16_t f=3000; f<10000; f+=250){
    thumby->play(f);
    delay(10);
  }

  // Stop the audio from playing forever
  thumby->stopPlay();
}

// Send a message over link
void send(Thumby* thumby) {
  // If packing does not fail, send over Serial1
  if(thumby->linkPack((uint8_t*)sendBuf, sizeof(sendBuf), packedBuf, sizeof(packedBuf)) != -1){

    // Write on Serial1 (this this initialzed in the Thumby Lib)
    Serial1.write(packedBuf, sizeof(packedBuf));

    // Remove echoed RX bytes
    removeRxBytes();
  }else{
    thumby->setCursor(0, 20);
    thumby->print("Packing Error!");
  }
}

// Get a message from over link
void receive(Thumby* thumby) {
  // If enough bytes in RX to fit packed message, read and try to unpack
  if (Serial1.available() >= 10){
    size_t read = Serial1.readBytes(packedBuf, sizeof(packedBuf));

    if(thumby->linkUnpack(packedBuf, read, dataBuf, sizeof(dataBuf)) != -1){
      thumby->setCursor(0, 20);
      thumby->print((char*)dataBuf);

      playMessageSound(thumby);
    }else{
      thumby->setCursor(0, 20);
      thumby->print("Unacking Error!");
      removeRxBytes();
    }
  }
}
