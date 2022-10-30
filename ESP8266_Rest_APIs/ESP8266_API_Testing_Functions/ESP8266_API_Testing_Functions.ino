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


const char* yourSSID = "Nf"; // SSID name 
const char* youPassword = "password"; // WiFi network password

// Set variables
int x = 0; // Used as a sequence step to only issue request once
String myIP; // ESP IP
String location; // ESP location

// Further location informaiton
const char* city;
const char* region;
const char* country_name;
const char* postal;
const char* timezone;
const char* utc_offset;
const char* latitude;
const char* longitude;

DynamicJsonDocument doc(5000); // Set document for JSON object to be pulled from
JsonObject obj = doc.as<JsonObject>(); // Set obj as an object to be accessed
boolean connectioWasAlive = true; // Used for connection monitoring

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


String GET_Request(String url){
    Serial.println("Connected!");
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    client->setInsecure();
    HTTPClient https;

    Serial.print("[HTTPS] begin...\n");

    if (https.begin(*client, url)) {  // Get ESPs IP address
      
      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      int httpCode = https.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          
          if(x == 0) {
            myIP = https.getString(); // GET ESP IP
            Serial.println("IP Address Received!");
            return myIP;
          }
          if (x == 1){
          location = https.getString(); // GET ESP location
          Serial.println("Location data received!"); 
          return location;
        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }
}


void loop() {
  
  monitorWiFi();
  
  if ((WiFiMulti.run() == WL_CONNECTED) and (x == 0)) {
    GET_Request("https://api.ipify.org"); // send first GET request to get ESP IP address
    x = 1; // go to next step
  }
  
  else if((WiFiMulti.run() == WL_CONNECTED) and (x == 1)){
     GET_Request("https://ipapi.co/" + myIP + "/json/"); // send second GET request to get ESP location
     x = 2; // go to next step
          
        // JSON object manipulation
        // -------------------------------------------------------------------------------
        // The following is using the ArduinoJson library, the resources are here https://arduinojson.org/
        
          DeserializationError error = deserializeJson(doc, location); // Put string payload into doc for JSON object usage
          JsonObject obj = doc.as<JsonObject>(); // Set obj as an object to be accessed

           city = obj["city"]; 
           region = obj["region"]; 
           country_name = obj["country_name"];
           postal = obj["postal"]; 
           timezone = obj["timezone"]; 
           utc_offset = obj["utc_offset"];
           latitude = obj["latitude"];
           longitude = obj["longitude"]; 
            
          if (error) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            return;
          }

        }

  // Used to print ESP information once it's received from first two GET requests
  else if ((WiFiMulti.run() == WL_CONNECTED) and (x == 2)){
    
        Serial.println("--------------------------------------------");
        Serial.println("Data present is, the ESPs IP is" + myIP + " and the location data is: ");
        Serial.println("Country = " + String(country_name));
        Serial.println("Province/State = " + String(region));
        Serial.println("City = " + String(city));
        Serial.println("Postal = " + String(postal));
        Serial.println("Latitude = " + String(latitude));
        Serial.println("Longitude = " + String(longitude));
        Serial.println("Timezone = " + String(timezone));
        Serial.println("--------------------------------------------");
  }
          
  // Delay for 10 seconds and then request again
  delay(10000);
}
