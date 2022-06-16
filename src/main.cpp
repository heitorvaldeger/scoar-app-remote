#include <Arduino.h>
#include <WiFi.h>

// Firebase ESP32 Project
#include <FirebaseESP32.h>
#include <addons/TokenHelper.h>//Provide the token generation process info.
#include <addons/RTDBHelper.h>

// IR Remote ESP8266/ESP32
#include <IRremoteESP8266.h>
#include <IRutils.h>
#include <IRac.h>

//Define Firebase Data object
FirebaseData fbdo;
FirebaseData stream;

FirebaseConfig config;

String childPath[2] = {"/temp", "/status"};

const int kIrLed = 12;
IRac ac(kIrLed);

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

      if (path == "/temp") {
        String temp = stream.value.c_str();
        ac.next.degrees = temp.toInt();

        Serial.println(temp);
      }

      ac.sendAc();
    }
  }
}

void streamTimeoutCallback(bool timeout) {
  if (timeout)
    Serial.println("stream timed out, resuming...\n");

  if (!stream.httpConnected())
    Serial.printf("error code: %d, reason: %s\n\n", stream.httpCode(), stream.errorReason().c_str());
}

void setup() {
  Serial.begin(9600);
  
  // WiFi Configuration
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.println("Connecting to Wi-Fi...");

    uint32_t notConnectedCounter = 0;
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.print(".");
      notConnectedCounter++;
      if(notConnectedCounter > 30) { // Reset board if not connected after 30s
          Serial.println("Resetting due to Wifi not connecting...");
          ESP.restart();
      }
    }

    Serial.println("ESP32 connected! :)");
  /////////////////////////////////////////

  // Firebase Configuration
    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;
    config.token_status_callback = tokenStatusCallback;

    Firebase.begin(DATABASE_URL, API_KEY);
    Firebase.reconnectWiFi(true);

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
  ///////////////////////////////////////
}

void loop() {
  
}