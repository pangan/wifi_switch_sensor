#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <ArduinoOTA.h>
//needed for library
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager

#include <StreamString.h>

ESP8266WebServer server(8095);

String header;

// Auxiliar variables to store the current output state
String output5State = "off";
String output4State = "off";

// Assign output variables to GPIO pin
const int output5 = 4; //D2
const int resetwifi = 14;  //D5

const int IN_A0 = A0; 


const char* www_username = "pangan";
const char* www_password = "OnlyGod54";

int value_A0;


void handleRoot() {
 Serial.println("You called root page");


String status = "OFF";
if (digitalRead(output5) == HIGH){
  Serial.println("Switch is ON");
  status = "ON";
}

String sensor_value = "?";

StreamString conta;
conta.reserve(500);  // Preallocate a large chunk to avoid memory fragmentation
conta.printf("\
<!DOCTYPE html>\
<html>\
<head>\
<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\
         .button_on { background-color: #195B6A; border: none; color: white; padding: 16px 40px;\
            text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}\
          .button_off { background-color:red; border: none; color: white; padding: 16px 40px;\
            text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}\
            .button_sensor { background-color:orange; border: none; color: white; padding: 16px 40px;\
            text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}\
            .value_div {font_size: 20px;}\
</style>\
</head>\
<body>\
<center>\
<h1>WiFi Switch</h1><br>\
<a href='pon' target='myIframe' class='button_on'>TURN ON</a><br><br><br><br><br>\
<a href='poff' target='myIframe' class='button_off'>TURN OFF</a><br><br><br>\
State:<iframe name='myIframe' width='100' height='25' frameBorder='0' srcdoc='%s'></iframe><br>\
<br>\
Sensor value: <iframe name='sensor' width='100', height='25' frameborder='0' srcdoc='%s' class='value_div'></iframe> <a href ='psensor' target='sensor' class='button_sensor'>Read</a><br><br><br>\
<hr>\
<br>\
<a href='logout'>Logout</a>\
</center>\
</body>\
</html>", status, sensor_value);

 server.send(200, "text/html", conta.c_str());

}

void handleLEDon() { 
 Serial.println("LED on page");
  digitalWrite(output5, HIGH);
   //String s = MAIN_pumpoff; //Read HTML contents
 server.send(200, "text/html", "On"); //Send ADC value only to client ajax request
}
 
void handleLEDoff() { 
 Serial.println("LED off page");
 digitalWrite(output5, LOW);
 //String s = MAIN_pumpon; //Read HTML contents
 server.send(200, "text/html", "Off"); //Send ADC value only to client ajax request
}

void handleSensor() {
  int sensor_value = 0;
  String sensor_val_str;
  value_A0 = analogRead(IN_A0); 
  Serial.println(value_A0);
  sensor_value = (1024 - value_A0) * 100 / 600;
  sensor_val_str = String(sensor_value) + " %";

  server.send(200, "text/html", sensor_val_str);
}

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);   

     // Initialize the output variables as outputs
  pinMode(output5, OUTPUT);
  pinMode(IN_A0, INPUT);

  pinMode(resetwifi, INPUT);
  
  // Set outputs to LOW
  digitalWrite(output5, LOW);
  
   
    Serial.println("Open http://192.168.4.1");
    Serial.println(WiFi.localIP());
    Serial.println("/ in your browser to see it working");
    //WiFiManager
    //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;
    //reset saved settings
    if (digitalRead(resetwifi) == LOW){
        digitalWrite(output5, HIGH);
    wifiManager.resetSettings();
    }
    //set custom ip for portal
    //wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

    //fetches ssid and pass from eeprom and tries to connect
    //if it does not connect it starts an access point with the specified name
    //here  "AutoConnectAP"
    //and goes into a blocking loop awaiting configuration
    wifiManager.autoConnect("WiFi-SW-FACFA2_D13C35 ");
    //or use this for auto generated name ESP + ChipID
    //wifiManager.autoConnect();

   
    //if you get here you have connected to the WiFi
    Serial.println("HTTP SERVER CONNECTED:)");
    server.on("/", []() {
      if (!server.authenticate(www_username, www_password)) {
        return server.requestAuthentication();
      }
      handleRoot();
      handleLEDon();
      
    });
    
    server.on("/pon", []() {
      if (!server.authenticate(www_username, www_password)) {
        return server.requestAuthentication();
      }
      handleLEDon();
      
    });
    
    server.on("/psensor", []() {
       if (!server.authenticate(www_username, www_password)) {
        return server.requestAuthentication();
      }
      handleSensor();
    });

    server.on("/poff", []() {
      if (!server.authenticate(www_username, www_password)) {
        return server.requestAuthentication();
      }
      handleLEDoff();
      
    });

    server.on("/logout", []() {
      server.send(401, "text/html", "Logged out!");
    });
    
    server.begin();
    ArduinoOTA.begin();
}

void loop() {
  ArduinoOTA.handle();  
  server.handleClient();

}
