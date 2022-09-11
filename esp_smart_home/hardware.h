#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <AHT10.h>
#include <Wire.h>



//// ###### User configuration space for AC library classes ##########

#include <ir_Electra.h>  //  replace library based on your AC unit model, check https://github.com/crankyoldgit/IRremoteESP8266

#define AUTO_MODE kElectraAcAuto
#define COOL_MODE kElectraAcCool
#define DRY_MODE kElectraAcDry
#define HEAT_MODE kElectraAcHeat
#define FAN_MODE kElectraAcFan

#define FAN_AUTO kElectraAcFanAuto
#define FAN_MIN kElectraAcFanLow
#define FAN_MED kElectraAcFanMed
#define FAN_HI kElectraAcFanHigh



struct state {
  uint8_t temperature = 22, fan = 0;
  String operation = "cool";
  String powerStatus = "off";
};

// ESP8266 GPIO pin to use for IR blaster.
const uint16_t kIrLed = 14;//D5
// Library initialization, change it according to the imported library file.
IRElectraAc   ac(kIrLed);
state acState;

//init led variables
const int ledPin = 13; //D7
const int buttonPin = 12; //D6
int lightLevel = 0;
int buttonState = 0;
int ledValues[5] = {0, 10, 50, 500, 1000}; //analog levels for the lights


//init the temp sensor
AHT10 myAHT10(AHT10_ADDRESS_0X38);








void init_hardware() {
  pinMode(buttonPin, INPUT);//button to toggle light
  pinMode(ledPin, OUTPUT);
  analogWrite(ledPin, lightLevel);
  ac.begin();// init ac remote
  myAHT10.begin();//init the temp sensor
  delay(1000);
}

void updateLight(String Level) {
  lightLevel = ledValues[Level.toInt()];
  Serial.println("lightLevel: " + String(lightLevel));
  analogWrite(ledPin, lightLevel);
}

void updateAcState(String key, String value) {
  Serial.println("Key: " + key + " Value: " + value);
  if (key == "/power") {
    if (value == "on") {
      acState.powerStatus = value;
      ac.on();
      Serial.println("AC power = on");
    } else {
      acState.powerStatus = "off";
      ac.off();
      Serial.println("AC power = off");
    }

  } else if (key == "/temp") {
    acState.temperature = value.toInt();
    ac.setTemp(acState.temperature);
    Serial.println("AC temp = " + value);


  } else if (key == "/operation") {
    acState.operation = value;
    if (acState.operation == "cool") {
      ac.setMode(COOL_MODE);
    } else if (acState.operation == "heat") {
      ac.setMode(HEAT_MODE);
    }
    Serial.println("AC operation = " + value);


  } else if (key == "/fan") {
    acState.fan = value.toInt();
    if (acState.operation != 0) {
      if (acState.fan == 0) {
        ac.setFan(FAN_AUTO);
      } else if (acState.fan == 1) {
        ac.setFan(FAN_MIN);
      } else if (acState.fan == 2) {
        ac.setFan(FAN_MED);
      } else if (acState.fan == 3) {
        ac.setFan(FAN_HI);
      }
    }
    Serial.println("AC fan = " + value);
  }
  ac.send();
}

void handleButton() {
  buttonState = digitalRead(buttonPin);
  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonState == HIGH) {
    if (lightLevel < 10)
      lightLevel = ledValues[1];
    else if (lightLevel < 50)
      lightLevel = ledValues[2];
    else if (lightLevel < 500)
      lightLevel = ledValues[3];
    else if (lightLevel < 1000)
      lightLevel = ledValues[4];
    else
      lightLevel = ledValues[0];

    delay(300);
    Serial.println("lightLevel: " + String(lightLevel));
  }
  analogWrite(ledPin, lightLevel);
}
