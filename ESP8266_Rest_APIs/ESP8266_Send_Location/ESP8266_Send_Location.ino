/*
 * 
 The following sketch does the following
 - Gets the ESP IP address from one pulic API
 - Gets the ESP location based on it's public IP from another public API
 - Send the location data to a server using POST to a public POSTMAN API, which is echoed back to prove it was received
 
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
int x = 0; // used as a sequence step to only issue request once
String myIP; // ESP IP
String location; // ESP location
String response; // response from POST request

// Further location informaiton (packaged in JSON object and then parsed out)
const char* city;
const char* region;
const char* country_name;
const char* postal;
const char* timezone;
const char* utc_offset;
const char* latitude;
const char* longitude;

DynamicJsonDocument doc(5000); // set document for JSON object to be pulled from
JsonObject obj = doc.as<JsonObject>(); // set obj as an object to be accessed
boolean connectioWasAlive = true; // used for connection monitoring

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


// Function used to monitor WiFi connection status 
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

// Function used to send both GET requests (both IP and location)
String GET_Request(String url){

    // Setup HTTPS connection
    Serial.println("Connected!");
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    client->setInsecure();
    HTTPClient https;

    Serial.print("[HTTPS] begin...\n");

    if (https.begin(*client, url)) {  // use URL from function to provide GET request
      
      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      int httpCode = https.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {

          // if in step 1, first GET request
          if(x == 0) {
            myIP = https.getString(); // GET ESP IP
            Serial.println("IP Address Received!");
            return myIP;
          }
          
          // if in step 2, second GET request
          if (x == 1){
          location = https.getString(); // GET ESP location
          Serial.println("Location data received!"); 
          return location;
        }
        
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }
      // terminate HTTPS connection
      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }
}

// Function to send location data to server
String POST_Request(String urlPost, String payload){

    // setup HTTP connection
    WiFiClient client;
    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    http.begin(client, urlPost); // send POST to url
    http.addHeader("Content-Type", "application/json");

    Serial.print("[HTTP] POST...\n");
    // Start connection and send HTTP header and body
    
    // This will be echoed back in this example, but it is proving the payload was sent and received by a webserver
    int httpCode = http.POST(payload); // send location payload to server

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] POST... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        const String& response = http.getString(); 
        Serial.println("received payload:\n<<");
        Serial.println(response);
        Serial.println(">>");
        return response;
      }
    } else {
      Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    // terminate HTTP connection
    http.end();
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
        // The following is using the ArduinoJson library, the resources are here https://arduinojson.org/
        
          DeserializationError error = deserializeJson(doc, location); // Put string payload into doc for JSON object usage
          JsonObject obj = doc.as<JsonObject>(); // Set obj as an object to be accessed

           // Set variables based on JSON object elements
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

  else if ((WiFiMulti.run() == WL_CONNECTED) and (x == 2)) {
    Serial.println(location);
    POST_Request("http://postman-echo.com/post", location); // send location to server
    x = 3;
  }
  
  // Used to print ESP information once it's received from first two GET requests
  // Reduces the need to send GET requests to only two times (no duplication, which can cause connection denial since these API services are free (only a limited amount of requests))
  // This will continue to run to give you access to the stored data via the serial monitor
  
  else if ((WiFiMulti.run() == WL_CONNECTED) and (x == 3)){
    
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
