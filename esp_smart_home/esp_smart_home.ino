#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "network.h"

long previousMillis = 0;
long interval = 120000;
bool sensorRefresh = false;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();
  init_wifi();
  init_hardware();
  firebaseInit();
  
}

void loop() {
//  analogWrite(ledPin,lightLevel);
  eventListener();
//  handleButton();
  
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis > interval) {
    previousMillis = currentMillis;
    sensorRefresh = !sensorRefresh;
  }
  if (sensorRefresh) {
    Serial.println("updateTemp");
    updateTemp();
    sensorRefresh = false; /*after its done set sensorRefresh to false so it wont be done again until one second is gone*/
  }


}
