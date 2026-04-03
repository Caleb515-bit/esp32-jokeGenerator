#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ------------------------------------------------------------
//  CONFIG
// ------------------------------------------------------------
const char* WIFI_SSID     = "Your_WiFi_Name";
const char* WIFI_PASSWORD = "Your_WiFi_Password";
const char* API_URL       = "https://v2.jokeapi.dev/joke/Programming?type=single";
const int   FETCH_INTERVAL = 15000;

// ------------------------------------------------------------
//  OLED
// ------------------------------------------------------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found!");
    while (true);
  }

  showMessage("ESP32", "Joke Machine", "Starting...");
  delay(2000);

  connectToWiFi();
  fetchJoke();
}


void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    fetchJoke();
  } else {
    showMessage("WiFi", "Lost!", "Reconnecting");
    connectToWiFi();
  }
  delay(FETCH_INTERVAL);
}

//  Connect to WiFi

void connectToWiFi() {
  Serial.print("[WiFi] Connecting...");
  showMessage("WiFi", "Connecting", WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    attempts++;
    if (attempts > 20) {
      showMessage("WiFi", "Failed!", "Check password");
      return;
    }
  }

  Serial.println("\n[WiFi] Connected!");
  showMessage("WiFi", "Connected!", "");
  delay(1500);
}


//  Fetch joke from API

void fetchJoke() {
  Serial.println("[API] Fetching joke...");
  showMessage("Fetching", "joke...", "");

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.begin(client, API_URL);

  int httpCode = http.GET();

  if (httpCode == 200) {
    String response = http.getString();

    DynamicJsonDocument doc(1024);
    deserializeJson(doc, response);

    String joke = doc["joke"].as<String>();
    Serial.println("JOKE: " + joke);
    showJoke(joke);

  } else {
    Serial.printf("[API] Error: %d\n", httpCode);
    showMessage("API", "Error!", String(httpCode).c_str());
  }

  http.end();
}


//  Show 3 lines of status message

void showMessage(const char* line1, const char* line2, const char* line3) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println(line1);

  display.setTextSize(1);
  display.setCursor(0, 25);
  display.println(line2);

  display.setCursor(0, 40);
  display.println(line3);

  display.display();
}


//  Show joke with word wrap

void showJoke(String joke) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setTextWrap(true);
  display.setCursor(0, 0);
  display.println("JOKE:");
  display.println("");
  display.println(joke);
  display.display();
}
