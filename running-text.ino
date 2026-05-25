#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4

#define DATA_PIN 13
#define CLK_PIN  14
#define CS_PIN   15

MD_Parola myDisplay = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

void setup() {
  myDisplay.begin();
  myDisplay.setIntensity(3); 
  myDisplay.displayClear();
  myDisplay.displayText("Selamat Iedul Adha 1447 H", PA_CENTER, 75, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
}

void loop() {
  if (myDisplay.displayAnimate()) {
    myDisplay.displayReset();
  }
}
