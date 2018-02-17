#include <arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define DEBUG true
#define PUBLISH_RATE 3600
#define DEEP_SLEEP true
#define ONE_WIRE_BUS 2

#ifdef DEBUG
 #define DEBUG_PRINT(x)  Serial.println (x)
#else
 #define DEBUG_PRINT(x)
#endif

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  WiFi.mode(WIFI_STA);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    DEBUG_PRINT(".");
  }
  DEBUG_PRINT(WiFi.localIP());
}

void setup() {
  if (DEBUG) Serial.begin(115200);
  setup_wifi();
  client.setServer(MQTT_SERVER, MQTT_PORT);
  sensors.begin();
}

void reconnect() {
  while (!client.connected()) {
    DEBUG_PRINT("Attempting MQTT connection...");
    if (client.connect(MQTT_CLIENT_NAME, MQTT_USERNAME, MQTT_PASSWORD)) {
      DEBUG_PRINT("connected");
    } else {
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
 if (!client.connected()) {
    reconnect();
  }
  client.loop();

  sensors.requestTemperatures();
 
  float number = sensors.getTempCByIndex(0);
  String payload = "{";
  payload += "\"temperature\":";
  payload += number;
  payload += "}";

  DEBUG_PRINT(payload);
 
  client.publish(MQTT_TOPIC, (char*) payload.c_str());
  delay(5000);
  
  if (DEEP_SLEEP) {
    DEBUG_PRINT("Deep Sleep");
    ESP.deepSleep(PUBLISH_RATE * 1e6);
  } else {
    delay(PUBLISH_RATE * 1e3);
  }
}