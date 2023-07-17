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
<!-- for mobile detection --> \
<meta name='viewport' content='width=device-width, initial-scale=1'>\
<link rel='stylesheet' href='http://code.jquery.com/mobile/1.4.5/jquery.mobile-1.4.5.min.css' >\
<script src='http://code.jquery.com/jquery-1.11.3.min.js'></script>\
<script src='http://code.jquery.com/mobile/1.4.5/jquery.mobile-1.4.5.min.js'></script>\
<style>\
html{font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\
.button_round {\
border-radius: 12px;\
background-color: #195B6A;\
color: white; padding: 16px 40px;\
text-decoration: none; font-size: 20px; margin: 2px; cursor: pointer;\
}\
.button_all{\
border: 0;\
color: white;\
cursor: pointer;\
display: inline-block;\
font-size: 18px;\
font-weight: 600;\
outline: 0;\
padding: 16px 16px;\
position: relative;\
text-align: center;\
text-decoration: none;\
transition: all .3s;\
user-select: none;\
-webkit-user-select: none;\
height: 100px;\
width: 100px;\
border-radius: 50%%;\
display: flex;\
justify-content: center;\
align-items: center;\
}\
.button_all:visited{color: #fff;}\
.button_all:before {\
background-color: initial;\
background-image: linear-gradient(#fff 0, rgba(255, 255, 255, 0) 100%%);\
border-radius: 50%%;\
content: '';\
height: 80%%;\
left: 0;\
opacity: .5;\
position: absolute;\
top: 0;\
transition: all .3s;\
width: 90%%;\
}\
@media (min-width: 100px) {\
.button_all {padding: 16px 16px;}\
}\
.button_on {background-color: darkgreen;}\
.button_off {background-color: red;}\
.button_sensor { background-color: darkorange;}\
</style>\
</head>\
<body>\
<center>\
<h1>WiFi Switch</h1>\
<iframe frameBorder='0' height='50' width='50%%' srcdoc='<h2 align=right>State:</h2>'>\</iframe><iframe name='myIframe' height='50' width='50%%' frameBorder='0' srcdoc='<h2>%s</h2>'></iframe>\
<br><iframe frameBorder='0' height='50' width='50%%' srcdoc='<h2 align=right>Sensor Value:</h2>'></iframe><iframe name='sensor' width='50%%' height='50' frameborder='0' srcdoc='<h2>%s</h2>'></iframe>\
<br><a href='pon' target='myIframe' class='button_on button_all'><font color=white>TURN ON</font></a><br>\
<a href='poff' target='myIframe' class='button_off button_all'><font color=white>TURN OFF</font></a><br>\
<a href ='psensor' target='sensor' class='button_all button_sensor'><font color=white>Read Sensor</font></a><br>\
<br>\
<a href='logout' class='button_round'><font color=white>Logout</font></a>\
</body>\
</html>\
", status, sensor_value);


 server.send(200, "text/html", conta.c_str());

}

void handleLEDon() { 
 Serial.println("LED on page");
  digitalWrite(output5, HIGH);
   //String s = MAIN_pumpoff; //Read HTML contents
 server.send(200, "text/html", "<h2>ON</h2>"); //Send ADC value only to client ajax request
}
 
void handleLEDoff() { 
 Serial.println("LED off page");
 digitalWrite(output5, LOW);
 //String s = MAIN_pumpon; //Read HTML contents
 server.send(200, "text/html", "<h2>OFF</h2>"); //Send ADC value only to client ajax request
}

void handleSensor() {
  int sensor_value = 0;
  String sensor_val_str;
  value_A0 = analogRead(IN_A0);
  delay(10);
  value_A0 += analogRead(IN_A0);
  delay(10);
  value_A0 += analogRead(IN_A0);
  value_A0 = value_A0 / 3;
  Serial.println(value_A0);
  sensor_value = (1024 - value_A0) * 100 / 800;
  sensor_val_str = "<h2>"+String(sensor_value) + " %</h2>";

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
    Serial.println("in your browser to see it working");
    String MAC = WiFi.macAddress();
    Serial.println("MAC Address: " + MAC);
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
    
    String SSID_STR = "WiFi-SW-"+MAC;
    //char arr[SSID_STR.length() + 1];

    //strcpy(arr, SSID_STR.c_str());

    // char* SSID = "WiFi-SW-" + arr;
    wifiManager.autoConnect(SSID_STR.c_str());
    //or use this for auto generated name ESP + ChipID
    //wifiManager.autoConnect();

   
    //if you get here you have connected to the WiFi
    Serial.println("HTTP SERVER CONNECTED:)");
    server.on("/", []() {
      if (!server.authenticate(www_username, www_password)) {
        return server.requestAuthentication();
      }
      handleRoot();
      
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
