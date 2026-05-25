#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

// Kita coba ganti tipe hardware-nya ke GENERIC
#define HARDWARE_TYPE MD_MAX72XX::GENERIC_HW 
#define MAX_DEVICES 4 

#define DATA_PIN 13
#define CLK_PIN  14
#define CS_PIN   15

MD_Parola myDisplay = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

void setup() {
  myDisplay.begin();
  myDisplay.setIntensity(5); // Kecerahan sedang
  myDisplay.displayClear();
  
  // Menampilkan teks diam tanpa animasi untuk uji coba
  myDisplay.print("Halo");
}

void loop() {
  // Biarkan kosong untuk tes ini
}
