/*
 Roller Shutter Firmware
 Created by Simon Bogutzky.
 Copyright © 2020 Simon Bogutzky. All rights reserved.
 v1.0.0
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const char* ssid = "";
const char* password = "";

ESP8266WebServer server(80);


const char* vers = "1.0.0";
const int dwn = 12;
const int stp = 13;
const int up = 14;

void handleRoot() {
  server.send(200, "text/plain; charset=utf-8", "Roller shutter webserver");
}

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup(void){
  pinMode(dwn, OUTPUT);
  pinMode(stp, OUTPUT);
  pinMode(up, OUTPUT);

  digitalWrite(dwn, HIGH);
  digitalWrite(stp, HIGH);
  digitalWrite(up, HIGH);
  
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/down", [](){
    resetCmd();
    digitalWrite(dwn, LOW);
    server.send(200, "text/plain", "down");
  });

  server.on("/stop", [](){
    resetCmd();
    digitalWrite(stp, LOW);
    server.send(200, "text/plain", "stop");
  });

  server.on("/up", [](){
    resetCmd();
    digitalWrite(up, LOW);
    server.send(200, "text/plain", "up");
  });

  server.on("/version", [](){
    server.send(200, "text/plain", vers);
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void resetCmd() {
  digitalWrite(dwn, HIGH);
  digitalWrite(stp, HIGH);
  digitalWrite(up, HIGH);
  delay(2000);
}

void loop(void){
  server.handleClient();
}
