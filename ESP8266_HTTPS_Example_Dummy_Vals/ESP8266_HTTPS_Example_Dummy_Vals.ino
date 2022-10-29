/*
 Simple sketch to connect to a dummy public URL and GET data from a public IP
 More information regarding IP and usage of API can be found here
 https://ipapi.co/api/#location-of-a-specific-ip
 
 You can also find your own IP and get all the information, find yours here and sub in your IP below
 https://www.whatismyip.com/
 
 This sketch is built and tested on an ESP8266
 
 */

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

ESP8266WiFiMulti WiFiMulti;
// Fingerprint for URL (not being used currently)
// const uint8_t fingerprint[20] = {0x40, 0xaf, 0x00, 0x6b, 0xec, 0x90, 0x22, 0x41, 0x8e, 0xa3, 0xad, 0xfa, 0x1a, 0xe8, 0x25, 0x41, 0x1d, 0x1a, 0x54, 0xb3};

// Variable decleration
// ----------------------------------------------------------------------------------
const char* yourSSID = "Your_Wifi_SSID_Name";
const char* youPassword = "Your_Wifi_Password_here";
String yourIP = "Your_IP_Here"; // Your IP address from https://www.whatismyip.com/, you can also use the public IP shown in usage below


DynamicJsonDocument doc(5000); // Set document for JSON object to be pulled from

void setup() {

  Serial.begin(115200);
  // Serial.setDebugOutput(true);

  // Flush serial port
  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA); // Connect to WiFi

  WiFiMulti.addAP(yourSSID, youPassword);
}

void loop() {
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

    // Note, not using SSL in this example, if you were, you would uncomment below and comment the setInsecure line below and use the fingerprint 
    // client->setFingerprint(fingerprint);
    // Or, if you happy to ignore the SSL certificate, then use the following line instead:
    client->setInsecure();

    HTTPClient https;

    Serial.print("[HTTPS] begin...\n");
    
    //if (https.begin(*client, "https://ipapi.co/8.8.8.8/json/")) {  // Public URL that can be used, to use your own comment this out and uncomment below
    
    if (https.begin(*client, "https://ipapi.co/" + yourIP + "/json/")) {  // Uses your public IP address (set in variable decleration) - to use the public IP comment this out and uncomment above
      
      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      int httpCode = https.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = https.getString(); // Set payload as server response
          
          Serial.println("The following is the raw payload");
          Serial.println("------------------------------------------------");
          Serial.println(payload);

        // JSON object manipulation
        // -------------------------------------------------------------------------------
        // The following is using the ArduinoJson library, the resources are here https://arduinojson.org/
        
          DeserializationError error = deserializeJson(doc, payload); // Put string payload into doc for JSON object usage
          JsonObject obj = doc.as<JsonObject>(); // Set obj as an object to be accessed
 
          // Test if parsing succeeds.
          if (error) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            return;
          }

          Serial.println("The following is the parsed data from JSON object");
          Serial.println("------------------------------------------------");

          // Set variables
          String city = obj["city"].as<String>(); // one way to set variable
          String region = obj["region"]; // other way to set variable
          String country_name = obj["country_name"];
          String postal = obj["postal"]; 
          String timezone = obj["timezone"]; 
          String utc_offset = obj["utc_offset"];
          String latitude = obj["latitude"];
          String longitude = obj["longitude"]; 


          // Print variables
          Serial.println("Country = " + country_name);
          Serial.println("Province/State = " + region);
          Serial.println("City = " + city);
          Serial.println("Postal = " + postal);
          Serial.println("Latitude = " + latitude);
          Serial.println("Longitude = " + longitude);
          Serial.println("Timezone = " + timezone);
          Serial.println("UTC Offset = " + utc_offset);
          
        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }

  // Delay for 10 seconds and then request again
  delay(10000);
}
