#include <Arduino.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <TinyPICO.h>
#include <ESP32Servo.h>

#include "ESPAsyncWebServer.h"

#define DOTSTAR_PWR 13
#define DOTSTAR_DATA 2
#define DOTSTAR_CLK 12

// Config
const int zoneCount = 3;
int zonePins[zoneCount] = {25, 26, 27};
Servo servos[zoneCount];
const int servoOffAngle = 90;
const int servoOnAngle = 0;

int zoneStates[zoneCount];

AsyncWebServer server(80);
TinyPICO tp = TinyPICO();

void setZoneState(int zone, int value)
{
  zoneStates[zone] = value;

  servos[zone].write(value == 1 ? servoOnAngle : servoOffAngle);
  delay(900);

  Serial.print("Zone ");
  Serial.print(zone + 1);
  Serial.print(" is now ");
  Serial.println(value ? "on" : "off");
}

void initializeServos()
{
  // Allow allocation of all timers
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  for (int i = 0; i < zoneCount; i++)
  {
    servos[i].setPeriodHertz(50); // standard 50 hz servo
    // using SG90 servo min/max of 500us and 2400us
    // for MG995 large servo, use 1000us and 2000us,
    // which are the defaults.
    servos[i].attach(zonePins[i], 500, 2500);
  }
}

void setup()
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  if (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.printf("WiFi Failed!\n");
    return;
  }

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP().toString());

  esp_err_t err = mdns_init();
  if (err)
  {
    printf("MDNS Init failed: %d\n", err);
    return;
  }
  mdns_hostname_set(DNS_NAME);

  initializeServos();

  // Initialize zones
  for (int i = 0; i < zoneCount; i++)
  {
    setZoneState(i, 0);
  }

  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              size_t capacity = JSON_ARRAY_SIZE(zoneCount) + zoneCount * JSON_OBJECT_SIZE(2);

              DynamicJsonDocument doc(capacity);

              for (int i = 0; i < zoneCount; i++)
              {
                JsonObject list = doc.createNestedObject();
                list["zone"] = i + 1;
                list["state"] = zoneStates[i];
              }

              String json;
              serializeJson(doc, json);
              request->send(200, "application/json", json);
            });

  server.on("/setState", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              int zone = request->arg("zone").toInt();
              int value = request->arg("value").toInt();

              setZoneState(zone - 1, value);
              request->send(200);
            });

  server.begin();
}

void loop()
{

  // put your main code here, to run repeatedly:
}
