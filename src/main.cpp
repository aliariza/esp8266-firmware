#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <WebSocketsClient.h>

const char *wsHost = "esp8266-dashboard.onrender.com";
const int wsPort = 443;

unsigned long lastTempLoop = 0;
unsigned long lastWsLoop = 0;
int ledPin = LED_BUILTIN;

WebSocketsClient webSocket;

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_CONNECTED:
    Serial.println("🔌 WebSocket connected");
    break;
  case WStype_DISCONNECTED:
    Serial.println("❌ WebSocket disconnected");
    break;
  case WStype_TEXT:
  {
    String message = (char *)payload;
    Serial.print("📥 Received message: ");
    Serial.println(message);
    if (message.indexOf("toggle") != -1)
    {
      digitalWrite(ledPin, !digitalRead(ledPin));
      Serial.println("💡 LED toggled");
    }
    break;
  }
  default:
    break;
  }
}

void setup()
{
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);

  WiFiManager wm;
  if (!wm.autoConnect("ESP-Setup"))
  {
    Serial.println("Failed to connect");
    ESP.restart();
  }

  Serial.println("✅ WiFi connected: " + WiFi.localIP().toString());

  webSocket.beginSSL(wsHost, wsPort, "/");
  webSocket.onEvent(webSocketEvent);
  webSocket.enableHeartbeat(30000, 3000, 2);
}

void loop()
{
  unsigned long now = millis();

  if (now - lastWsLoop > 20)
  {
    lastWsLoop = now;
    webSocket.loop();
  }

  if (WiFi.status() == WL_CONNECTED && now - lastTempLoop > 10000)
  {
    lastTempLoop = now;

    float temperature = random(200, 350) / 10.0;
    String payload = "{\"temperature\": " + String(temperature, 2) +
                     ", \"timestamp\": \"" + String(millis()) + "\"}";

    Serial.println("📡 Sending temperature over WebSocket...");
    webSocket.sendTXT(payload);
  }
}