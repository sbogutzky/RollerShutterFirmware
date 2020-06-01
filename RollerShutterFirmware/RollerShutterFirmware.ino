/*
  Roller Shutter Firmware

  The MIT License (MIT)
  Copyright (c) 2020 Simon Bogutzky

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
  
  v1.0.1
*/

#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

const char* ssid = "";
const char* password = "";

ESP8266WebServer server(80);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

const char* vers = "1.2.0";
const int dwn = 12;
const int stp = 13;
const int up = 14;

String formattedTime;
String dayStamp;
String timeStamp;
long timer = 0;
long timeout = 1000;

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

  timeClient.begin();
  
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(7200);
}

void resetCmd() {
  digitalWrite(dwn, HIGH);
  digitalWrite(stp, HIGH);
  digitalWrite(up, HIGH);
  delay(2000);
}

void loop(void){
  server.handleClient();
  if (millis() > timeout + timer) {
    timer = millis();
    
    while(!timeClient.update()) {
      timeClient.forceUpdate();
    }

    formattedTime = timeClient.getFormattedTime();
    Serial.println(formattedTime);

    if (formattedTime == "11:30:00") {
      resetCmd();
      digitalWrite(dwn, LOW);
    }
  }
}
