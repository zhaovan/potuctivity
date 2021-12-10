#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <FS.h>   // Include the SPIFFS library
//#include "SPIFFS.h"
#include <ezButton.h>

ESP8266WiFiMulti wifiMulti;     // Create an instance of the ESP8266WiFiMulti class, called 'wifiMulti'

ESP8266WebServer server(80);    // Create a webserver object that listens for HTTP request on port 80

String getContentType(String filename); // convert the file extension to the MIME type
bool handleFileRead(String path);       // send the right file to the client (if it exists)

const char* ssid = "Ivan’s iPhone"; //wifi ssid / wifi name
const char* pass = "31415926"; //wifi password

ezButton limitSwitch(13);

void setup() {
  Serial.begin(9600);         // Start the Serial communication to send messages to the computer
  delay(10);
  Serial.println('\n');
  //
  //  wifiMulti.addAP("Ivan's iPhone", "31415926");   // add Wi-Fi networks you want to connect to
  ////  wifiMulti.addAP("ssid_from_AP_2", "your_password_for_AP_2");
  ////  wifiMulti.addAP("ssid_from_AP_3", "your_password_for_AP_3");
  //
  //  Serial.println("Connecting ...");
  //  int i = 0;
  //  while (wifiMulti.run() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
  //    delay(250);
  //    Serial.print('.');
  //  }
  //  Serial.println('\n');
  //  Serial.print("Connected to ");
  //  Serial.println(WiFi.SSID());              // Tell us what network we're connected to
  //  Serial.print("IP address:\t");
  //  Serial.println(WiFi.localIP());           // Send the IP address of the ESP8266 to the computer

  Serial.print(F("Connecting to "));  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println(F("[CONNECTED]"));
  Serial.print("[IP ");
  Serial.print(WiFi.localIP());
  Serial.println("]");

  if (MDNS.begin("esp8266")) {              // Start the mDNS responder for esp8266.local
    Serial.println("mDNS responder started");
  } else {
    Serial.println("Error setting up MDNS responder!");
  }

  if (!SPIFFS.begin()) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  } else {
    Serial.println("Mounted SPIFFS");
  }
  // Start the SPI Flash Files System

 
// If the client requests any URI
  server.onNotFound([]() {
    if (limitSwitch.getState() != LOW) {
      server.send(200, "text/plain", "Connect your phone to get started"); // otherwise, respond with a 404 (Not Found) error
    } else {
      if (!handleFileRead(server.uri()))                  // send it if it exists
        server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
    }
  });

  server.begin();                           // Actually start the server
  Serial.println("HTTP server started");
}

void loop(void) {
  limitSwitch.loop();
  server.handleClient();
}

String getContentType(String filename) { // convert the file extension to the MIME type
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  return "text/plain";
}

bool handleFileRead(String path) { // send the right file to the client (if it exists)
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "home.html";         // If a folder is requested, send the index file
  String contentType = getContentType(path);            // Get the MIME type
  if (SPIFFS.exists(path)) {                            // If the file exists
    File file = SPIFFS.open(path, "r");                 // Open it
    size_t sent = server.streamFile(file, contentType); // And send it to the client
    file.close();                                       // Then close the file again
    return true;
  }
  Serial.println("\tFile Not Found");
  return false;                                         // If the file doesn't exist, return false
}
