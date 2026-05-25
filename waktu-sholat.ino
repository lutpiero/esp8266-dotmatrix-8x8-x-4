#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <EEPROM.h>

// --- KONFIGURASI MATRIX ---
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define DATA_PIN 13 
#define CLK_PIN  14 
#define CS_PIN   15 
MD_Parola myDisplay = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

// --- GLOBAL VARIABLES ---
ESP8266WebServer server(80);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 7 * 3600); 

bool isAPMode = false;
String userSSID = "";
String userPass = "";
String city = "Jakarta";
String country = "Indonesia";

int waktuSholat[5] = {0, 0, 0, 0, 0}; 
String namaSholat[5] = {"Subuh", "Dzuhur", "Ashar", "Maghrib", "Isya"};
char displayBuffer[150];
int lastHourFetched = -1;
int lastHttpError = 0; // Menyimpan kode error untuk ditampilkan

// --- FUNGSI BANTUAN EEPROM ---
void writeEEPROM(int startAdr, int maxLength, String writeString) {
  for (int i = 0; i < maxLength; i++) {
    if (i < writeString.length()) {
      EEPROM.write(startAdr + i, writeString[i]);
    } else {
      EEPROM.write(startAdr + i, 0); 
    }
  }
  EEPROM.commit();
}

String readEEPROM(int startAdr, int maxLength) {
  String dest = "";
  for (int i = 0; i < maxLength; i++) {
    char c = char(EEPROM.read(startAdr + i));
    if (c == 0) break;
    dest += c;
  }
  return dest;
}

// --- FUNGSI WEB SERVER ---
void handleRoot() {
  String html = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>body{font-family:sans-serif; padding:20px; background:#f4f4f4;} .card{background:#fff; padding:20px; border-radius:8px; box-shadow:0 2px 4px rgba(0,0,0,0.1);} input[type=text], input[type=password]{width:100%; padding:10px; margin:8px 0; border:1px solid #ccc; border-radius:4px;} button{width:100%; padding:12px; background:#28a745; color:white; border:none; border-radius:4px; font-size:16px; margin-bottom:10px;}</style></head>";
  html += "<body><div class='card'><h2>Pengaturan NodeMCU</h2>";
  html += "<form action='/save' method='POST'>";
  html += "<label>WiFi SSID:</label><input type='text' name='ssid' value='" + userSSID + "'>";
  html += "<label>WiFi Password:</label><input type='password' name='pass'>";
  html += "<label>Nama Kota:</label><input type='text' name='city' value='" + city + "'>";
  html += "<button type='submit'>Simpan & Restart</button>";
  html += "</form>";
  html += "<hr><form action='/reset' method='GET'><button type='submit' style='background:#dc3545;'>Reset ke Pengaturan Pabrik</button></form>";
  html += "</div></body></html>";
  server.send(200, "text/html", html);
}

void handleSave() {
  String newSSID = server.arg("ssid");
  String newPass = server.arg("pass");
  String newCity = server.arg("city");

  writeEEPROM(0, 32, newSSID);
  writeEEPROM(32, 64, newPass);
  writeEEPROM(96, 32, newCity);

  String html = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'></head><body style='font-family:sans-serif; text-align:center; padding:50px;'><h2>Tersimpan!</h2><p>NodeMCU sedang di-restart.</p></body></html>";
  server.send(200, "text/html", html);

  delay(2000);
  ESP.restart(); 
}

void handleReset() {
  writeEEPROM(0, 32, "");
  writeEEPROM(32, 64, "");
  writeEEPROM(96, 32, "");
  
  String html = "<!DOCTYPE html><html><body style='font-family:sans-serif; text-align:center; padding:50px;'><h2>Memori Dihapus!</h2><p>Alat sedang di-restart ke mode AP.</p></body></html>";
  server.send(200, "text/html", html);
  
  delay(2000);
  ESP.restart();
}

void setup() {
  Serial.begin(115200);
  EEPROM.begin(512); 
  
  myDisplay.begin();
  myDisplay.setIntensity(3);
  myDisplay.displayClear();

  userSSID = readEEPROM(0, 32);
  userPass = readEEPROM(32, 64);
  String savedCity = readEEPROM(96, 32);
  if (savedCity.length() > 0) city = savedCity;

  // PERBAIKAN: Selalu inisialisasi route web server agar bisa diakses kapan saja
  server.on("/", HTTP_GET, handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.on("/reset", HTTP_GET, handleReset);

  if (userSSID.length() > 0) {
    myDisplay.print("Conn..");
    WiFi.begin(userSSID.c_str(), userPass.c_str());
    
    int timeout = 30; // Tunggu maksimal 15 detik
    while (WiFi.status() != WL_CONNECTED && timeout > 0) {
      delay(500);
      timeout--;
    }
  }

  if (WiFi.status() != WL_CONNECTED) {
    isAPMode = true;
    WiFi.mode(WIFI_AP);
    IPAddress apIP(192, 168, 2, 1);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP("shalat", "ABC123456");

    server.begin(); // Mulai web server di AP Mode
    
    myDisplay.displayText("Setup WiFi: 'shalat' - IP: 192.168.2.1", PA_CENTER, 40, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  } 
  else {
    isAPMode = false;
    server.begin(); // Mulai web server di Station Mode (Konek Wi-Fi rumah)
    
    String ipMsg = "IP: " + WiFi.localIP().toString();
    myDisplay.displayClear();
    myDisplay.displayText(ipMsg.c_str(), PA_CENTER, 45, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
    
    while (!myDisplay.displayAnimate()) {
      server.handleClient(); // Jaga server tetap responsif selama animasi IP
      yield(); 
    }
    myDisplay.displayReset();

    timeClient.begin();
    myDisplay.print("Sync..");
    timeClient.update();
    fetchPrayerTimes();
  }
}

void loop() {
  server.handleClient(); // PERBAIKAN: Halaman web terus dipantau, baik di mode AP maupun konek Wi-Fi
  
  if (isAPMode) {
    if (myDisplay.displayAnimate()) {
      myDisplay.displayReset(); 
    }
  } 
  else {
    timeClient.update();

    if (myDisplay.displayAnimate()) {
      updateDisplayString();
      if (strlen(displayBuffer) <= 8) { 
        myDisplay.displayText(displayBuffer, PA_CENTER, 0, 500, PA_PRINT, PA_NO_EFFECT);
      } else {
        myDisplay.displayText(displayBuffer, PA_CENTER, 40, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
      }
      myDisplay.displayReset();
    }
    
    if (timeClient.getHours() == 1 && lastHourFetched != 1) {
      fetchPrayerTimes();
      lastHourFetched = 1;
    }
    if (timeClient.getHours() == 2) { 
      lastHourFetched = -1;
    }
  }
}

void updateDisplayString() {
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();
  int currentSecond = timeClient.getSeconds();
  int currentMins = (currentHour * 60) + currentMinute;

  // Jika gagal, tampilkan jam beserta kode error dari server
  if (waktuSholat[0] == 0) {
    sprintf(displayBuffer, "API Err: %d", lastHttpError);
    return;
  }

  int nextPrayerIndex = -1;
  for (int i = 0; i < 5; i++) {
    int diff = waktuSholat[i] - currentMins;
    if (diff >= 0 && diff <= 5) {
      nextPrayerIndex = i;
      break; 
    }
  }

  if (nextPrayerIndex != -1) {
    int diff = waktuSholat[nextPrayerIndex] - currentMins;
    if (diff == 0) {
      sprintf(displayBuffer, "Waktu Sholat %s", namaSholat[nextPrayerIndex].c_str());
    } else {
      int detikSisa = 60 - currentSecond;
      int menitSisa = diff - 1;
      if (detikSisa == 60) {
         detikSisa = 0;
         menitSisa = diff;
      }
      sprintf(displayBuffer, "Waktu Sholat %s %d m %d s lagi", namaSholat[nextPrayerIndex].c_str(), menitSisa, detikSisa);
    }
  } else {
    sprintf(displayBuffer, "%02d:%02d:%02d", currentHour, currentMinute, currentSecond);
  }
}

void fetchPrayerTimes() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;
    
    String safeCity = city;
    safeCity.replace(" ", "%20");
    String safeCountry = country;
    safeCountry.replace(" ", "%20");
    
    String url = "http://api.aladhan.com/v1/timingsByCity?city=" + safeCity + "&country=" + safeCountry + "&method=20";
    http.begin(client, url);
    int httpCode = http.GET();
    
    if (httpCode == 200) {
      String payload = http.getString();
      waktuSholat[0] = extractTime(payload, "Fajr");
      waktuSholat[1] = extractTime(payload, "Dhuhr");
      waktuSholat[2] = extractTime(payload, "Asr");
      waktuSholat[3] = extractTime(payload, "Maghrib");
      waktuSholat[4] = extractTime(payload, "Isha");
      lastHttpError = 200; // Sukses
    } else {
      // Simpan kode error (misal -1 gagal koneksi jaringan, 400 Bad Request)
      lastHttpError = httpCode; 
    }
    http.end();
  } else {
    lastHttpError = -2; // Error indikator WiFi terputus
  }
}

int extractTime(String payload, String key) {
  int keyIndex = payload.indexOf("\"" + key + "\"");
  if (keyIndex > -1) {
    int colonIndex = payload.indexOf(":", keyIndex);
    if (colonIndex > -1) {
      int startQuote = payload.indexOf("\"", colonIndex);
      if (startQuote > -1) {
        String timeStr = payload.substring(startQuote + 1, startQuote + 6); 
        int h = timeStr.substring(0, 2).toInt();
        int m = timeStr.substring(3, 5).toInt();
        return (h * 60) + m;
      }
    }
  }
  return 0;
}
