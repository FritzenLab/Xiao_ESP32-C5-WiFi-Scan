// reference: https://wiki.seeedstudio.com/xiao_esp32c5_getting_started/

#define LED 27 // onboard LED pin for Xiao ESP32-C5
#include <HTTPClient.h>   
#include <WiFiClientSecure.h>
#include <WiFi.h>

WiFiClientSecure client;
HTTPClient http;

long ledTime = 0;
bool ledStatus = false;
long scanTime = 0;
bool scanPending = false;

void blinkLED(){
  if(millis() - ledTime > 300){
    ledTime += 300;
    if(ledStatus == false){
      ledStatus= true;
      digitalWrite(LED, HIGH);
    }else{
      ledStatus= false;
      digitalWrite(LED, LOW);
    }
  }
}
void startWiFiScan() {
  // https://docs.espressif.com/projects/arduino-esp32/en/latest/api/wifi.html#scannetworks
  // passing true as first argument makes the scan asynchronous
  WiFi.scanNetworks(true);
  scanPending = true;
  Serial.println("\n--- 5 GHz scan started (async) ---");
}

void checkWiFiScan() {
  // https://docs.espressif.com/projects/arduino-esp32/en/latest/api/wifi.html#scancomplete
  // returns WIFI_SCAN_RUNNING (-1) while busy, WIFI_SCAN_FAILED (-2) on error, or number of found networks
  int found = WiFi.scanComplete();

  if (found == WIFI_SCAN_RUNNING) return;  // still scanning, come back later

  scanPending = false;

  if (found == WIFI_SCAN_FAILED || found == 0) {
    Serial.println("No networks found.");
  } else {
    int count5G = 0;
    for (int i = 0; i < found; i++) {
      if (WiFi.channel(i) >= 36) {
        count5G++;
        Serial.printf("  [%d] SSID: %-30s | RSSI: %d dBm | Ch: %d\n",
          count5G,
          WiFi.SSID(i).c_str(),
          WiFi.RSSI(i),
          WiFi.channel(i));
      }
    }
    if (count5G == 0) Serial.println("No 5 GHz networks found.");
    else Serial.printf("  %d 5 GHz network(s) found.\n", count5G);
  }

  WiFi.scanDelete();
}

void setup() {
  // put your setup code here, to run once:
  pinMode(LED, OUTPUT);
  // http.begin() sets the target URL
  
  Serial.begin(115200);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin("Your5Gssid", "YourPassword");

  Serial.print("Connecting to WiFi");
  unsigned long wifiStart = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - wifiStart > 15000) {
      Serial.println("\n[ERROR] WiFi timeout. Check SSID/password.");
      return;
    }
    
  }
  Serial.printf("\n[OK] Connected — IP: %s | RSSI: %d dBm\n",
  WiFi.localIP().toString().c_str(), WiFi.RSSI());
}

void loop() {
  blinkLED();

  if (!scanPending && millis() - scanTime > 20000) {
    scanTime = millis();
    startWiFiScan();
  }

  if (scanPending) {
    checkWiFiScan();
  }
}