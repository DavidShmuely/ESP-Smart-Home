#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "hardware.h"

/* 1. Define the WiFi credentials */
#define WIFI_SSID "Shmuely2.4"
#define WIFI_PASSWORD "0547615838"

/* 2. If work with RTDB, define the RTDB URL and database secret */
#define DATABASE_URL "intercom-21cfe-default-rtdb.firebaseio.com" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app
#define DATABASE_SECRET "BwAtJwlUFS2q9zgZGJMSDNHTDbn8xSo9T7fw1JYM"

/* 3. Define the Firebase Data object */
FirebaseData fbdo;
FirebaseData stream;

/* 4, Define the FirebaseAuth data for authentication data */
FirebaseAuth auth;

/* Define the FirebaseConfig data for config data */
FirebaseConfig config;

String deviceName = "esp1";
String parentPath = "/smartHome/"+deviceName;
String childPath[5] = {"/operation", "/power", "/temp", "/fan","/lightLevel"};
int count = 0;
volatile bool dataChanged = false;




//  wifi configuration
//----------------------------------------------------------------------
void init_wifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

}

//   Firebase Realtime Database Configuration.
//----------------------------------------------------------------------



//-----------------------------------------------------------------------


void streamCallback(MultiPathStream stream)
{
  size_t numChild = sizeof(childPath) / sizeof(childPath[0]);

  for (size_t i = 0; i < numChild; i++)
  {
    if (stream.get(childPath[i]))
    {
      Serial.printf("path: %s, event: %s, type: %s, value: %s%s", stream.dataPath.c_str(), stream.eventType.c_str(), stream.type.c_str(), stream.value.c_str(), i < numChild - 1 ? "\n" : "");
      Serial.println("");
      String str = stream.dataPath.c_str();
      if (str == "/power"||str=="/operation"||str=="/fan"||str=="/temp") {
        updateAcState(stream.dataPath.c_str(), stream.value.c_str());
      } else if (str=="/lightLevel") {
        updateLight(stream.value.c_str());
      }
    }
  }

  Serial.println();

  // This is the size of stream payload received (current and max value)
  // Max payload size is the payload size under the stream path since the stream connected
  // and read once and will not update until stream reconnection takes place.
  // This max value will be zero as no payload received in case of ESP8266 which
  // BearSSL reserved Rx buffer size is less than the actual stream payload.
  Serial.printf("Received stream payload size: %d (Max. %d)\n\n", stream.payloadLength(), stream.maxPayloadLength());

  // Due to limited of stack memory, do not perform any task that used large memory here especially starting connect to server.
  // Just set this flag and check it status later.
  dataChanged = true;
}

void streamTimeoutCallback(bool timeout)
{
  if (timeout)
    Serial.println("stream timed out, resuming...\n");

  if (!stream.httpConnected())
    Serial.printf("error code: %d, reason: %s\n\n", stream.httpCode(), stream.errorReason().c_str());
}



//TODO: check what to do if the stream timeout
void eventListener() {
  if (dataChanged) {
    dataChanged = false;
    // When stream data is available, do anything here...
  }
}






void updateTemp() {
  Firebase.RTDB.set(&fbdo, "/smartHome/"+deviceName+"/roomtemp", myAHT10.readTemperature()) ;
  Firebase.RTDB.set(&fbdo, "/smartHome/"+deviceName+"/roomhumid", myAHT10.readHumidity()) ;

}



//---------------------------------------------------------------------
void firebaseInit() {
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  config.database_url = DATABASE_URL;
  config.signer.tokens.legacy_token = DATABASE_SECRET;

  Firebase.reconnectWiFi(true);

  /* Initialize the library with the Firebase authen and config */
  Firebase.begin(&config, &auth);

  if (!Firebase.RTDB.beginMultiPathStream(&stream, parentPath))
    Serial.printf("sream begin error, %s\n\n", stream.errorReason().c_str());

  Firebase.RTDB.setMultiPathStreamCallback(&stream, streamCallback, streamTimeoutCallback);



  //  Firebase.setString("ipAddress/shitritIntercom",WiFi.localIP().toString()); //--> update the Firebase Realtime Database with intercom IP address
}
