#include <WiFi.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>

// Emulate Serial1 on pins 6/7 if not present
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial2(6, 7); // RX, TX
#endif


#define PIN 22 // Pin for connecting LED Strip
Adafruit_NeoPixel strip = Adafruit_NeoPixel(13, PIN, NEO_GRB + NEO_KHZ800);

/* Wi-Fi info */
const char* ssid = " ";       // your network SSID (name)
const char* pass = " ";   // your network password

int status = WL_IDLE_STATUS;  // the Wifi radio's status


char server1[] = "api.openweathermap.org"; //server for fetching weather data
char server2[] = "date.jsontest.com"; //server for fetching time data

unsigned long lastConnectionTime1 = 0;         // last time you connected to the server, in milliseconds
const unsigned long postingInterval1 = 900000L; // delay between updates, in milliseconds 

unsigned long lastConnectionTime2 = 0;         // last time you connected to the server, in milliseconds
const unsigned long postingInterval2 = 10000L; // delay between updates, in milliseconds 

// Initialize the Ethernet client object
WiFiClient client1;
WiFiClient client2;

String weatherMessage = " ";  //weather data came from server
String weatherData = " ";     //filtered weather data

String timeMessage = " ";  //time data came from server
String timeData = " ";     //filtered time data
String amHours = " AM";
String pmHours = " PM";

int h = 0;  //hour before adding time zone
int m = 0;  //minutes before adding time zone
int hour = 0;  //final hour
int minute = 0;  //final minutes
int clouds = 0;  //cloud intensity in percentage
int rainy = 0;
int tstorm = 0;
int snowy = 0;
int naders = 0;


void setup() {
  Serial.begin(11520);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  
  strip.begin();
  strip.setBrightness(255);  //Brigtness from 0 to 255
  strip.show();
  
  httpRequest1();  // request weather data
  httpRequest2();
}

void loop() {
    // if there's incoming data from the net connection send it out the serial port
    // this is for debugging purposes only
    while (client1.available()) {
      char c = client1.read();
      weatherMessage = weatherMessage + c;
    }
    if(weatherMessage != " "){
      int indexS = weatherMessage.indexOf('{');
      int indexE = weatherMessage.lastIndexOf('}');
      weatherData = weatherMessage.substring(indexS, indexE+1);
      StaticJsonDocument<1024> doc1;
      DeserializationError error = deserializeJson(doc1, weatherData);
      
      JsonObject main = doc1["main"];
      float temp = main["temp"]; 
      int humidity = main["humidity"]; 
      
      if (doc1.containsKey("rain")) {
      int rain = 501;
      rainy = rain;
      }
      if (doc1.containsKey("snow")) {
      int snow = 601; 
      snowy = snow; 
      }
      if (doc1.containsKey("tornado")) {
      int nader = 781;
      naders = nader;  
      }
      if (doc1.containsKey("thunderstorm")) {
      int storm = 211; 
      tstorm = storm; 
      }
      int cloud = doc1["clouds"]["all"];
      clouds = cloud;
       
      
      Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
      Serial.println(temp);  //temperature
      Serial.println(cloud); //local variable from cloud%

      Serial.println(clouds);  //global variable from cloud%
      Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");

//------------------------Changing Base Color acording to temperature--------------------
      if(temp < 273){
        BaseChange(strip.Color(255, 255, 255), 50); //White
      }
      else if(temp >= 273 && temp < 277){
        BaseChange(strip.Color(160, 8, 255), 50); //violet
      }
      else if(temp >= 277 && temp < 283){
        BaseChange(strip.Color(0, 0, 255), 50); //blue
      }
      else if(temp >= 283 && temp < 288){
        BaseChange(strip.Color(0, 238, 255), 50); //light blue
      }
      else if(temp >= 288 && temp < 294){
        BaseChange(strip.Color(0, 255, 0), 50); //green
      }
      else if(temp >= 294 && temp < 299){
        BaseChange(strip.Color(255, 255, 0), 50); // yellow
      }
      else if(temp >= 299 && temp < 305){
        BaseChange(strip.Color(255, 170, 0), 50); //orange
      }
      else if(temp >= 305 && temp < 310){
        BaseChange(strip.Color(255, 41, 148), 50); //pink
      }
      else if(temp > 310){
        BaseChange(strip.Color(255, 0, 0), 50); //red
      }      
      else{
        BaseChange(strip.Color(255, 0, 255), 50);  // purple trouble code
      }
    }
    weatherMessage = " ";
  
    // if 10 seconds have passed since your last connection,
    // then connect again and send data
    if (millis() - lastConnectionTime1 > postingInterval1) {
      httpRequest1();
    }

    while (client2.available()) {
      char c = client2.read();
      timeMessage = timeMessage + c;
    }
    if(timeMessage != " "){
      int indexS = timeMessage.indexOf('{');
      int indexE = timeMessage.lastIndexOf('}');
      timeData = timeMessage.substring(indexS, indexE+1);
    
      StaticJsonDocument<128> doc2;
    
      DeserializationError error = deserializeJson(doc2, timeData);
    
      const char* date = doc2["date"]; // "08-22-2022"
      const char* time = doc2["time"]; // "02:33:46 PM"
      String times = String(time);
      h = ((times.substring(0,2)).toInt());
      m = ((times.substring(3, 5)).toInt());
      
      if(times.endsWith(amHours)){
        h = h % 24;
        Serial.println(h);
        Serial.println(times);
        minute = (m + 0) % 60;
      }
      else if(times.endsWith(pmHours)){
      h = (h + 12) % 24;
      Serial.println(h);
      minute = (m + 0) % 60;
      }

      hour = h;
      
      Serial.println("***************************************************************************");
      Serial.println(time);
      Serial.println(hour);
      Serial.println(minute);
      Serial.println(clouds);
      Serial.println(rainy);
      Serial.println(snowy);
      Serial.println(tstorm);
      Serial.println(naders);
      Serial.println("***************************************************************************");

          /*------------------------clouds-------------------------*/
      
      if(clouds < 20 && rainy < 20 && snowy < 20 && tstorm < 20 && naders < 20)
      {
       CloudChange(strip.Color(0, 235, 255), 50); //light blue -clear
      }
      else if(clouds < 85 && rainy < 20 && snowy < 20 && tstorm < 20 && naders < 20)
      {
       CloudChange(strip.Color(240, 240, 240), 50); //white mild cloud cover
      }
      else if(clouds >= 85 && rainy < 20 && snowy < 20 && tstorm < 20 && naders < 20)
      {
       CloudChange(strip.Color(150, 150, 150), 50); // gray cloudy
      }
      else if(rainy >= 50 && snowy < 20 && tstorm < 20 && naders < 20)
      {
       CloudChange(strip.Color(0, 255, 0), 50); // green rain
      }
      else if(snowy >= 50 && rainy < 20 && tstorm < 20 && naders < 20)
      {
       CloudChange(strip.Color(160, 8, 255), 50); //purple snow
      }
      else if(tstorm >= 20)
      {
       thunder(strip.Color(255, 255, 0), 50); //flashing yellow thunderstorm
      }
      else if(naders >= 20)
      {
       thunder(strip.Color(255, 0, 0), 50); // flashing red tornado
      }
      else
      {
       CloudChange(strip.Color(50, 50, 50), 50); // black or off error code
      }

//-----------------------Changing Sun / Moon Color according to time---------------------------------
      if(hour >= 6 && hour < 11){
        SunMoonChange(strip.Color(255, 255, 255), 50); // white moon after midnight
      }
      else if(hour >= 11 && hour < 14){
        SunMoonChange(strip.Color(245, 30, 129), 50); // orange 6-8am 
      }
      else if(hour >= 14 && hour < 18){
        SunMoonChange(strip.Color(255, 170, 0), 50); // light orange 8-12
      }
      else if(hour >= 18 && hour < 23){
        SunMoonChange(strip.Color(255, 255, 0), 50); // yellow noon-5
      }
      else if(hour >= 23){
        SunMoonChange(strip.Color(255, 0, 0), 50); // red -5-6
      }
      else if(hour = 12 && times.endsWith(amHours)){
        SunMoonChange(strip.Color(222, 0, 170), 50); //red violet 6-8
      }
      else if(hour >= 0 && hour < 2){
        SunMoonChange(strip.Color(222, 0, 170), 50); //red violet continued
      }
      else if(hour >= 2 && hour <= 6){
        SunMoonChange(strip.Color(145, 18, 219), 50); //purple blue 8- midnight
      }
      else{
        SunMoonChange(strip.Color(70, 10, 200), 50); //blue error
      }
    }
    timeMessage = " ";
    
    // if 10 seconds have passed since your last connection,
    // then connect again and send data
    if (millis() - lastConnectionTime2 > postingInterval2) {
      httpRequest2();
    }
}

// this method makes a HTTP connection to the server
void httpRequest1() {
  Serial.println();
    
  // close any connection before send a new request
  // this will free the socket on the WiFi shield
  client1.stop();

  // if there's a successful connection
  if (client1.connect(server1, 80)) {
    Serial.println("Connecting...");
    
    // send the HTTP PUT request
    client1.println(F("GET /data/2.5/weather?lat=34.757080&lon=-96.668090&appid=42d37f2a43cf2550d64789c35161f07a HTTP/1.1"));
    client1.println(F("Host: api.openweathermap.org"));
    client1.println("Connection: close");
    client1.println();

    // note the time that the connection was made
    lastConnectionTime1 = millis();
  }
  else {
    // if you couldn't make a connection
    Serial.println("Connection failed");
  }
}

void httpRequest2() {
  Serial.println();
    
  // close any connection before send a new request
  // this will free the socket on the WiFi shield
  client2.stop();

  // if there's a successful connection
  if (client2.connect(server2, 80)) {
    Serial.println("Connecting...");
    
    // send the HTTP PUT request
    client2.println(F("GET /asciilogo.txt HTTP/1.1"));
    client2.println(F("Host: date.jsontest.com"));
    client2.println("Connection: close");
    client2.println();

    // note the time that the connection was made
    lastConnectionTime2 = millis();
  }
  else {
    // if you couldn't make a connection
    Serial.println("Connection failed");
  }
}

void BaseChange(uint32_t c, uint8_t wait) {
  for(uint16_t i = 0; i < 8 ; i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void CloudChange(uint32_t c, uint8_t wait) {
  for(uint16_t i = 8; i < 12 ; i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void thunder(uint32_t c, uint8_t wait) {
  for(uint16_t i = 8; i < 12 ; i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
  delay(300);
  for(uint16_t i = 8; i < 12 ; i++) {
    strip.setPixelColor(i, strip.Color(250, 250, 250));
    strip.show();
    delay(wait);
  }
  delay(200);
  for(uint16_t i = 8; i < 12 ; i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
  delay(150);
  for(uint16_t i = 8; i < 12 ; i++) {
    strip.setPixelColor(i, strip.Color(250, 250, 250));
    strip.show();
    delay(wait);
  }
}

void SunMoonChange(uint32_t c, uint8_t wait) {
    strip.setPixelColor(12, c);
    strip.show();
    delay(wait);
}
