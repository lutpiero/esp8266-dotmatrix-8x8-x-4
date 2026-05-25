// PASTIKAN MENAMBAHKAN INI DI BARIS PALING ATAS (Di bawah #include <ESP8266WiFi.h>)
#include <WiFiClientSecure.h>

// ... (kode lainnya tetap sama) ...

void fetchPrayerTimes() {
  if (WiFi.status() == WL_CONNECTED) {
    // Menggunakan Client Secure agar kebal terhadap paksaan HTTPS dari server API
    WiFiClientSecure client;
    client.setInsecure(); // Abaikan validasi sertifikat SSL agar hemat memori
    
    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS); // Otomatis ikuti jika server pindah jalur
    
    String safeCity = city;
    safeCity.replace(" ", "%20");
    String safeCountry = country;
    safeCountry.replace(" ", "%20");
    
    // Gunakan https:// untuk memastikan koneksi tidak ditolak oleh server modern
    String url = "https://api.aladhan.com/v1/timingsByCity?city=" + safeCity + "&country=" + safeCountry + "&method=20";
    
    http.begin(client, url);
    int httpCode = http.GET();
    
    if (httpCode == 200) {
      String payload = http.getString();
      waktuSholat[0] = extractTime(payload, "Fajr");
      waktuSholat[1] = extractTime(payload, "Dhuhr");
      waktuSholat[2] = extractTime(payload, "Asr");
      waktuSholat[3] = extractTime(payload, "Maghrib");
      waktuSholat[4] = extractTime(payload, "Isha");
      lastHttpError = 200; 
    } else {
      // Jika masih error, simpan kode error-nya agar tampil di layar
      lastHttpError = httpCode; 
    }
    http.end();
  } else {
    lastHttpError = -2; // Indikator WiFi terputus
  }
}
