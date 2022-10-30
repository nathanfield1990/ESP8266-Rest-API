/*
 Simple sketch to connect to a dummy public URL and GET data from a public IP
 More information regarding IP and usage of API can be found here
 https://ipapi.co/api/#location-of-a-specific-ip
 
 You can also find your own IP and get all the information, find yours here and sub in your IP below
 https://www.whatismyip.com/
 
 This sketch is built and tested on a ESP8266
 
 */

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

ESP8266WiFiMulti WiFiMulti;
// Fingerprint for demo URL, expires on June 2, 2021, needs to be updated well before this date
// const uint8_t fingerprint[20] = {0x40, 0xaf, 0x00, 0x6b, 0xec, 0x90, 0x22, 0x41, 0x8e, 0xa3, 0xad, 0xfa, 0x1a, 0xe8, 0x25, 0x41, 0x1d, 0x1a, 0x54, 0xb3};

// Variable decleration
// ----------------------------------------------------------------------------------
/*
const char* yourSSID = "Your_Wifi_SSID_Name";
const char* youPassword = "Your_Wifi_Password_here";
String yourIP = "Your_IP_Here"; // Your IP address from https://www.whatismyip.com/, you can also use the public IP shown in usage below
*/
//const char* yourSSID = "BELL871_2.4";
//const char* youPassword = "11111111";
//const char* yourSSID = "NachoWifi";
//const char* youPassword = "P-assword";

const char* yourSSID = "Nf";
const char* youPassword = "password";

String yourIP = "8.8.8.8"; // Your IP address from https://www.whatismyip.com/, you can also use the public IP shown in usage below
int x = 0;
String myIP = "8.8.8.8";
String location;
/*
String city;
String region;
String country_name;
String postal;
String timezone;
String utc_offset;
String latitude;
String longitude;
*/

DynamicJsonDocument doc(5000); // Set document for JSON object to be pulled from
JsonObject obj = doc.as<JsonObject>(); // Set obj as an object to be accessed
boolean connectioWasAlive = true;

void setup() {

  Serial.begin(115200);
  // Serial.setDebugOutput(true);

  // Flush serial port
  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }
  WiFiMulti.addAP(yourSSID, youPassword);
  //WiFi.mode(WIFI_STA); // Connect to WiFi
  
  
}

void monitorWiFi()
{
  if (WiFiMulti.run() != WL_CONNECTED and (x == 0))
  {
    if (connectioWasAlive == true)
    {
      connectioWasAlive = false;
      Serial.print("Looking for WiFi ");
    }
    Serial.print("...");
    delay(500);
  }
  else if (connectioWasAlive == false)
  {
    connectioWasAlive = true;
    Serial.printf(" connected to %s\n", WiFi.SSID().c_str());
  }
}

void loop() {
  monitorWiFi();
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED) and (x == 0)) {
    Serial.println("Connected!");
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    client->setInsecure();
    HTTPClient https;

    Serial.print("[HTTPS] begin...\n");

    if (https.begin(*client, "https://api.ipify.org")) {  // Get ESPs IP address
      
      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      int httpCode = https.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          myIP = https.getString(); // Set payload as server response
          Serial.println(myIP);
          x = 1;

        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }

    delay(1000);
    Serial.println("https://ipapi.co/" + myIP + "/json/");
  }
  else if((WiFiMulti.run() == WL_CONNECTED) and (x == 1)){
      std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
      client->setInsecure();
      HTTPClient https;
      if (https.begin(*client, "https://ipapi.co/" + myIP + "/json/")) {
      // Sub in your IP here and comment above (set variable in variable decleration at the top)
      Serial.println("The sent payload was https://ipapi.co/" + myIP + "/json/");
      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      int httpCode = https.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          location = https.getString(); // Set payload as server response
          
          Serial.println("The following is ESPs IP location");
          Serial.println("------------------------------------------------");
          Serial.println(location);

        // JSON object manipulation
        // -------------------------------------------------------------------------------
        // The following is using the ArduinoJson library, the resources are here https://arduinojson.org/
        
          DeserializationError error = deserializeJson(doc, location); // Put string payload into doc for JSON object usage
          JsonObject obj = doc.as<JsonObject>(); // Set obj as an object to be accessed

          String city = obj["city"].as<String>(); // one way to set variable
          String region = obj["region"]; // other way to set variable
          String country_name = obj["country_name"];
          String postal = obj["postal"]; 
          String timezone = obj["timezone"]; 
          String utc_offset = obj["utc_offset"];
          String latitude = obj["latitude"];
          String longitude = obj["longitude"]; 

          Serial.println("The following is the ESPs IP address");
          Serial.println("------------------------------------------------");
          Serial.println(myIP);
          
          Serial.println("The following is the parsed data from JSON object");
          Serial.println("------------------------------------------------");
          Serial.println("x =" + x);
          
          


          // Print variables
          Serial.println("Country = " + country_name);
          Serial.println("Province/State = " + region);
          Serial.println("City = " + city);
          Serial.println("Postal = " + postal);
          Serial.println("Latitude = " + latitude);
          Serial.println("Longitude = " + longitude);
          Serial.println("Timezone = " + timezone);
          x = 2;
          // Test if parsing succeeds.
          if (error) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            return;
          }

        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
      
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }
  
  else if ((WiFiMulti.run() == WL_CONNECTED) and (x == 2)){
  Serial.println("Data already accessed via GET request");
  }
          
  // Delay for 10 seconds and then request again
  delay(10000);
}
