#include <Arduino.h>
#include <WiFi.h>
#include "time.h"

// Firebase ESP32 Project
#include <FirebaseESP32.h>
#include <addons/TokenHelper.h>//Provide the token generation process info.
#include <addons/RTDBHelper.h>

// IR Remote ESP8266/ESP32
#include <IRremoteESP8266.h>
#include <IRutils.h>
#include <IRac.h>

// GPIO 2
#define LED 2

//Define Firebase Data object
FirebaseData fbdo;
FirebaseData stream;

FirebaseConfig config;

String childPath[2] = {"/temperature", "/status"};

const int kIrLed = 23;
IRac ac(kIrLed);

unsigned long epochTime;
const char* ntpServer = "pool.ntp.org";

unsigned long previousMillis = 0;
const long interval = 60000;

unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return(0);
  }
  time(&now);
  return now;
}

void streamCallback(MultiPathStreamData stream) {
  size_t numChild = sizeof(childPath) / sizeof(childPath[0]);

  for (size_t i = 0; i < numChild; i++)
  {
    if (stream.get(childPath[i]))
    {
      String path = stream.dataPath.c_str();
      
      if (path == "/status") {
        String status = stream.value.c_str();
        if(status == "true")
          ac.next.power = true;
        else
          ac.next.power = false;

        Serial.println(status);
      }

      if (path == "/temperature") {
        String temp = stream.value.c_str();
        ac.next.degrees = temp.toInt();

        Serial.println(temp);
      }

      ac.sendAc();
    }
  }

  // Firebase.setStringAsync(fbdo, SINC_TEMP, getTime());
}

void streamTimeoutCallback(bool timeout) {
  if (timeout)
    Serial.println("stream timed out, resuming...\n");

  if (!stream.httpConnected())
    Serial.printf("error code: %d, reason: %s\n\n", stream.httpCode(), stream.errorReason().c_str());
}

void setup() {
  Serial.begin(9600);
  
  // Set pin mode
  pinMode(LED, OUTPUT);

  // WiFi Configuration
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.println("Connecting to Wi-Fi...");

    uint32_t notConnectedCounter = 0;
    while (WiFi.status() != WL_CONNECTED) {
      digitalWrite(LED,HIGH);
      delay(1000);
      digitalWrite(LED,LOW);
      delay(1000);
      Serial.print(".");
      notConnectedCounter++;
      if(notConnectedCounter > 30) { // Reset board if not connected after 30s
          Serial.println("Resetting due to Wifi not connecting...");
          ESP.restart();
      }
    }

    digitalWrite(LED,HIGH);

    Serial.println("ESP32 connected! :)");
  /////////////////////////////////////////

  // Firebase Configuration
    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;
    config.token_status_callback = tokenStatusCallback;

    Firebase.begin(DATABASE_URL, API_KEY);
    Firebase.reconnectWiFi(true);

    while (!Firebase.ready()) {
      delay(1000);
      Serial.println("Connecting to Firebase Realtime Database...");
    }

    Serial.println("Connected to Firebase Realtime Database!");

    if(!Firebase.getInt(fbdo, String(PARENT_PATH) + "/protocol"))
      Serial.println(fbdo.errorReason().c_str());
    
    if (!Firebase.beginMultiPathStream(stream, PARENT_PATH))
      Serial.printf("sream begin error, %s\n\n", stream.errorReason().c_str());
  ////////////////////////////////////////

  // IR Configuration
    int protocolId = fbdo.to<int>();

    ac.next.protocol = (decode_type_t) protocolId;
    // ac.next.protocol = getProtocol(protocolId);

    Firebase.setMultiPathStreamCallback(stream, streamCallback, streamTimeoutCallback);
    ac.next.degrees = 16;
    ac.next.mode = stdAc::opmode_t::kCool;
    ac.next.light = true;
    ac.next.power = false;

    configTime(0, 0, ntpServer);
  ///////////////////////////////////////
}

void loop() {
    unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Save the data to Firebase Realtime Database
    Firebase.setStringAsync(fbdo, SINC_TEMP, getTime());
  }
}