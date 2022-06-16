# scoar-app-remote

Repositório que armazena os sketchs de comunicação entre o Realtime Database (Firebase), ESP32 e o módulo emissor InfraVermelho KY-005. O projeto utiliza o ambiente PlatformIO

## Configuração do arquivo platformio.ini
```
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
build_flags = 
	-D WIFI_SSID=\"WIFI_SSID\"
	-D WIFI_PASSWORD=\"WIFI_PASSWORD\"
	-D DATABASE_URL=\"FIREBASE_DATABASE_URL\"
	-D API_KEY=\"FIREBASE_DATABASE_API_KEY\"
	-D PARENT_PATH=\"PARENT_PATH\"
lib_deps = 
	mobizt/Firebase ESP32 Client@^3.17.5
	crankyoldgit/IRremoteESP8266@^2.8.2
```

## Protocolos mapeados
O repositório utiliza os protocolos suportados pelo projeto: [IRremoteESP8266](https://github.com/crankyoldgit/IRremoteESP8266)

Foi realizado um [fork](https://github.com/crankyoldgit/IRremoteESP8266) do projeto para oferecer suporte para aparelhos nacionais

| **PROTOCOL ID** | **PROTOCOL NAME** |
|:---------------:|:-----------------:|
| 6               | SONY              |
| 5               | PANASONIC         |
| 7               | SAMSUNG           |
| 10              | LG                |
| 24              | GREE              |
| 46              | SAMSUNG_AC        |
| 49              | PANASONIC_AC      |
