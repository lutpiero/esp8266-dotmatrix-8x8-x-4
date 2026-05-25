#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

// Definisikan tipe hardware. Kebanyakan modul di pasaran adalah tipe FC16_HW.
// Jika teks terbalik atau berantakan, coba ganti ke GENERIC_HW atau PAROLA_HW.
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW

#define MAX_DEVICES 4 

// Gunakan angka pin yang persis sama dengan kode lama Anda
#define DATA_PIN 13
#define CLK_PIN  14
#define CS_PIN   15

// Inisialisasi menggunakan mode Software SPI (menyebutkan DATA, CLK, dan CS secara spesifik)
MD_Parola myDisplay = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

void setup() {
  myDisplay.begin();
  myDisplay.setIntensity(5); // Tingkat kecerahan
  myDisplay.displayClear();  // Membersihkan layar

  // Mengatur teks berjalan
  myDisplay.displayText("Selamat Iedul Adha 1447 H", PA_CENTER, 75, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
}

void loop() {
  if (myDisplay.displayAnimate()) {
    myDisplay.displayReset();
  }
}
