#include "USB.h"
#include "USBHIDKeyboard.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Preferences.h>

IPAddress local_IP(1, 1, 2, 1); // Установите желаемый IP-адрес
IPAddress gateway(1, 1, 2, 0);    // IP-адрес маршрутизатора (шлюз)
IPAddress subnet(255, 255, 255, 0);  // Маска подсети
IPAddress primaryDNS(1, 1, 2, 0);    // Основной DNS (например, Google)
IPAddress secondaryDNS(0, 0, 0, 0);

const char* ssid = "SSID";
const char* password = "PASSWORD";
const char* pcUrl = "http://1.1.2.2:5000/message";

USBHIDKeyboard Keyboard;
bool stop = false;
int tries = 0;
const int max_tries = 50; 

WebServer server(80);
Preferences preferences;

void sendMessageToServer(const String& message) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(pcUrl);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Content-Length", String(message.length()));

    String jsonPayload = "{\"message\":\"ESP:" + message + "\"}";
    Serial.println("Payload: " + jsonPayload);

    int httpResponseCode = http.POST(jsonPayload);

    if (httpResponseCode > 0)
      String response = http.getString();

    http.end();
  }
}

void handleStop() {
  sendMessageToServer("in stop");
  if (server.hasArg("plain")) { 
    String body = server.arg("plain");
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, body);

    if (!error) {
      if (doc.containsKey("stop")) {
        stop = doc["stop"];
        server.send(200, "application/json", "{\"status\":\"ok\"}");
        if (stop) sendMessageToServer("stop");
      }
    } else {
      server.send(400, "application/json", "{\"status\":\"error\"}");
    }
  } else {
    server.send(400, "application/json", "{\"status\":\"no body\"}");
  }
}

void handleReboot() {
  sendMessageToServer("in reboot");
  bool reboot = false;
  if (server.hasArg("plain")) { 
    String body = server.arg("plain");
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, body);

    if (!error) {
      if (doc.containsKey("reboot")) {
        reboot = doc["reboot"];
        server.send(200, "application/json", "{\"status\":\"ok\"}");
      }
    } else {
      server.send(400, "application/json", "{\"status\":\"error\"}");
    }
  } else {
    server.send(400, "application/json", "{\"status\":\"no body\"}");
  }
  if (reboot) rebootConsole();
}

void handleAwait() {
  sendMessageToServer("in await");
  bool await = false;
  if (server.hasArg("plain")) { 
    String body = server.arg("plain");
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, body);

    if (!error) {
      if (doc.containsKey("await")) {
        await = doc["await"];
        server.send(200, "application/json", "{\"status\":\"ok\"}");
      }
    } else {
      server.send(400, "application/json", "{\"status\":\"error\"}");
    }
  } else {
    server.send(400, "application/json", "{\"status\":\"no body\"}");
  }
  if (await) awaitFun();
}

void pressPSButton(){
  Keyboard.press(KEY_PAUSE);
  delay(100);
  Keyboard.releaseAll();
}

void holdPSButton(){
  Keyboard.press(KEY_PAUSE);
  delay(1500);
  Keyboard.releaseAll();
}

void pressEnterButton(){
  Keyboard.press(KEY_RETURN);
  delay(100);
  Keyboard.releaseAll();
}

void pressLeftButton(){
  Keyboard.press(KEY_LEFT_ARROW);
  delay(100);
  Keyboard.releaseAll();
}

void pressRightButton(){
  Keyboard.press(KEY_RIGHT_ARROW);
  delay(100);
  Keyboard.releaseAll();
}

void pressDownButton(){
  Keyboard.press(KEY_DOWN_ARROW);
  delay(100);
  Keyboard.releaseAll();
}

void pressUpButton(){
  Keyboard.press(KEY_UP_ARROW);
  delay(100);
  Keyboard.releaseAll();
}

void rebootConsole(){
  sendMessageToServer("REBOOT_CONSOLE");
  holdPSButton();
  delay(1000);
  pressRightButton();
  delay(100);
  pressDownButton();
  delay(100);
  pressDownButton();
  delay(100);
  pressEnterButton();
  delay(65000);
  pressRightButton();
  delay(100);
}

void awaitFun(){
  sendMessageToServer("AWAIT");
  while(true){}
}

void startJailbreak(){
  //sendMessageToServer("JAILBREAK");
  pressEnterButton();
  delay(7000);


  server.handleClient();
  if (stop) return;
  pressPSButton();
  delay(1000);
  server.handleClient();
}

void setup() {
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)){while(true){}}
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
  Keyboard.begin();
  USB.begin();
  server.on("/stop", HTTP_POST, handleStop);
  server.on("/reboot", HTTP_POST, handleReboot);
  server.on("/await", HTTP_POST, handleAwait);
  server.begin();
  sendMessageToServer("launch and wait...");
  preferences.begin("counter", false);
  int bootCount = preferences.getInt("boot_count", 0);
  bootCount++;
  preferences.putInt("boot_count", bootCount);
  sendMessageToServer(String(bootCount));
  delay(40000);
  if (bootCount % 2) rebootConsole();
  pressRightButton();
  sendMessageToServer("start");
  delay(5000);
  if (bootCount % 2) rebootConsole();
  pressEnterButton();
  delay(7000);
  server.handleClient();
  delay(8000);
  pressPSButton();
  delay(2000);
}

void loop() {
  server.handleClient();
  while(stop){
    server.handleClient();
    delay(1000);
  }
  if (tries > max_tries) rebootConsole();
  startJailbreak();
  ++tries;
  server.handleClient();
}