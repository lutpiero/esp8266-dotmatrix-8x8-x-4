#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

// Definisikan tipe hardware. Kebanyakan modul di pasaran adalah tipe FC16_HW.
// Jika teks terbalik atau berantakan, coba ganti ke GENERIC_HW atau PAROLA_HW.
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW

// Jumlah blok matrix 8x8 yang berjejer (biasanya 4 kalau pakai modul standar)
#define MAX_DEVICES 4 

// Definisikan pin CS sesuai yang kita obrolkan tadi (D8 pada NodeMCU)
#define CS_PIN D8 

// Inisialisasi object Parola menggunakan Hardware SPI
// (DIN otomatis ke D7, CLK otomatis ke D5)
MD_Parola myDisplay = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

void setup() {
  // Memulai display
  myDisplay.begin();
  
  // Mengatur tingkat kecerahan LED (0 sampai 15)
  myDisplay.setIntensity(5);
  
  // Membersihkan layar sebelum mulai
  myDisplay.displayClear();

  // Mengatur parameter teks berjalan:
  // (Teks, Perataan, Kecepatan, Jeda (ms), Efek Masuk, Efek Keluar)
  // Kecepatan: Semakin kecil angkanya, semakin cepat jalannya (misal 50).
  myDisplay.displayText("Selamat Iedul Adha 1447 H", PA_CENTER, 75, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
}

void loop() {
  // Menjalankan animasi secara terus menerus
  if (myDisplay.displayAnimate()) {
    // Kalau animasi selesai, reset agar mengulang dari awal
    myDisplay.displayReset();
  }
}
