/**
 * ANGLE LIGHT 電角度
 * M.Phillips, 2018.
 * 
 * Simulates a display similar to the NanoLeaf
 * 
 * Responds on MQTT to:
 * devices/triangle/set/brightness [int]
 * devices/triangle/set/pattern [int]
 * 
 */

#include <FS.h>
#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <DNSServer.h>
#include <PubSubClient.h>
#include <ArduinoJson.h> // dont use the v6 beta
#include "Panel.h"
#include "Pattern.h"

#define numberOfPanels 12
#define pixelsPerPanel 6
#define pinLED D2
#define pinButton D1
#define totalPatterns 6
#define autoConfigSSID "angleLight"
#define autoConfigPassword "password"

char mqtt_server[40];
char mqtt_username[40];
char mqtt_password[40];
int total_pixels = pixelsPerPanel * numberOfPanels;
int delayAmount;
int ledTimer = 0;
int buttonTimer = 0;
unsigned long mqttTimer = 0;
int activePattern = 1;
bool buttonPressed = false;
bool shouldSaveConfig = false;

Panel panels[numberOfPanels];
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
Adafruit_NeoPixel Strip = Adafruit_NeoPixel(total_pixels, pinLED, NEO_GRB + NEO_KHZ800);

void setup() {
  Strip.begin();
  Strip.setBrightness(200);
  turnOffAllPanels();

  pinMode(pinButton, INPUT_PULLUP);
  Serial.begin(115200);
  Serial.println ("start");
  connectWifi();
  setPattern();
  mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setCallback(mqttCallback);
}

void loop() {
  if (millis() - ledTimer > delayAmount) {
    ledTimer = millis();
    render();
    advance();
  }

  if (digitalRead(pinButton) == LOW && (millis() - buttonTimer > 200) )  {
    buttonTimer = millis();
    buttonPressed = true;
  }

  if (digitalRead(pinButton) == HIGH && buttonPressed) {
    buttonPressed = false;
    activePattern = totalPatterns > activePattern ? activePattern + 1 : 1;
    setPattern();
  }

  if (mqttClient.connected()) {
    mqttClient.loop();
  } else {
    if (millis() - mqttTimer > 5000) {
      mqttTimer = mqttReconnect() ? 0 : millis();
    }
  }
}

int limitCheck(int input, int minimum, int maximum) {
  return (input > maximum ? maximum : input < minimum ? minimum : input);
}

void setPattern() {
  Serial.println (activePattern);
  Pattern pattern = Pattern(activePattern);
  delayAmount = pattern.delayAmount;
  initPanels(pattern);
}

void initPanels(Pattern pattern) {
  uint32_t colorStart = pattern.colorStart;
  uint32_t colorEnd = pattern.colorEnd;
  int patternWidth = pattern.width > 0 ? pattern.width : numberOfPanels ;
  int bufferStart = pattern.bufferStart;
  int bufferEnd = pattern.bufferEnd;
  bool bounce = pattern.bounce;
  bool randomise = pattern.randomise;
  
  patternWidth = bounce ? patternWidth - 1 : patternWidth;
  int patternStep = 255 / patternWidth;
  int currentStep = 0;

  for (int panel = 0; panel < numberOfPanels; panel++) {
    panels[panel] = Panel(colorStart, colorEnd, bufferStart, bufferEnd, currentStep, bounce, randomise);
    currentStep = currentStep > 255 ? 0 : currentStep + patternStep; 
  }
}

void advance() {
  for (int panel = 0; panel < numberOfPanels; panel++) {
    panels[panel].advance();
  }
}

void render() {
  for (int panel = 0; panel < numberOfPanels; panel++) {
    uint32_t color = panel[panels].getNewColor();
    for (int pixel = (pixelsPerPanel * panel); pixel < (pixelsPerPanel * panel) + pixelsPerPanel; pixel++) {
      Strip.setPixelColor(pixel, color);
    }
  }
  Strip.show();
}

void turnOffAllPanels() {
  for (int pixel = 0; pixel < total_pixels; pixel++) {
    Strip.setPixelColor(pixel, 0);
  }
  Strip.show();
}


// -----------
// WIFIMANAGHER
void saveConfigCallback () {
  shouldSaveConfig = true;
}

void connectWifi() {
  if (SPIFFS.begin()) {
    if (SPIFFS.exists("/config.json")) {
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        size_t size = configFile.size();
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          strcpy(mqtt_server, json["mqtt_server"]);
          strcpy(mqtt_username, json["mqtt_username"]);
          strcpy(mqtt_password, json["mqtt_password"]);
        } else {
          Serial.println("failed to load json config");
        }
        configFile.close();
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
  WiFiManagerParameter custom_mqtt_username("username", "mqtt username", mqtt_username, 40);
  WiFiManagerParameter custom_mqtt_password("password", "mqtt password", mqtt_password, 40);
  WiFiManager wifiManager;
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_username);
  wifiManager.addParameter(&custom_mqtt_password);
  wifiManager.setMinimumSignalQuality();
  wifiManager.setDebugOutput(false);
  wifiManager.setTimeout(120);

  if (!wifiManager.autoConnect(autoConfigSSID, autoConfigPassword)) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    ESP.reset();
    delay(5000);
  }

  strcpy(mqtt_server, custom_mqtt_server.getValue());
  strcpy(mqtt_username, custom_mqtt_username.getValue());
  strcpy(mqtt_password, custom_mqtt_password.getValue());

  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["mqtt_server"] = mqtt_server;
    json["mqtt_username"] = mqtt_username;
    json["mqtt_password"] = mqtt_password;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
  }

  Serial.println("local ip");
  Serial.println(WiFi.localIP());
}


// ----
// MQTT
void mqttCallback(char* topic, byte* message, unsigned int length) {
  if (strcmp(topic, "devices/angleLight/set/pattern") == 0) { // TOFIX: Make topic name via variable
    message[length] = '\0';
    String s = String((char*)message);
    int i = limitCheck (s.toInt(), 1, totalPatterns);
    activePattern = i;
    setPattern();
    Serial.print ("pattern");
  }

  if (strcmp(topic, "devices/angleLight/set/brightness") == 0) { // TOFIX: Make topic name via variable
    message[length] = '\0';
    String s = String((char*)message);
    int i = limitCheck (s.toInt(), 0, 255);
    Strip.setBrightness(i);
  }
}

boolean mqttReconnect() {
  Serial.println("Attempting MQTT connection...");
  if (mqttClient.connect("angleLight", mqtt_username, mqtt_password, "devices/angleLight/status", 1, 0, "offline")) {
    mqttClient.subscribe("devices/angleLight/set/#");
    mqttClient.publish("devices/angleLight/status", "online", true);
    // TOFIX: Make topic name via variable
    Serial.println("MQTT connected.");
  }
  return mqttClient.connected();
}
