#include <Arduino.h>
#include "ESP8266WiFi.h"
#include "FS.h"
#include "ESPAsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include "U8g2lib.h"
#include "DHT.h"

#define DHTdata 0
#define HyLed 12
#define TempLed 14
#define RangeLed 13
#define attBtn 2

#define staSSID "Nokia 6.1 Plus"
#define staPSK "20180121"

AsyncWebServer server(80);
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0,SCL,SDA,U8X8_PIN_NONE); 
DHT dht(DHTdata,DHT22);

const char* ssid=staSSID;
const char* password=staPSK;
/*
IPAddress ip(192,168,43,76);
IPAddress gateway(192,168,43,1);
IPAddress subnet(255,255,255,0);
*/
bool attState=false;
int humValue=0;
int tempValue=0;
int ledValue =0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  //pinMode
  pinMode(attBtn,INPUT);
  pinMode(DHTdata,INPUT);
  pinMode(HyLed,OUTPUT);
  pinMode(TempLed,OUTPUT);
  pinMode(RangeLed,OUTPUT);

  Serial.println();

  dht.begin();
  u8g2.begin();  
  SPIFFS.begin();

  //WiFi.config(ip,gateway,subnet);

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  server.begin();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  //button
  attachInterrupt(digitalPinToInterrupt(attBtn),attOn,RISING);



  //server
  server.on("/", HTTP_GET,[](AsyncWebServerRequest *request){
    Serial.println("index.html response");
    request->send(SPIFFS, "/index.html", String(),false);
  });
  server.on("/style.css", HTTP_GET,[](AsyncWebServerRequest *request){
    Serial.println("style.css response");
    request->send(SPIFFS, "/style.css", "text/css");
  }); 
  server.on("/sechtml.html", HTTP_GET,[](AsyncWebServerRequest *request){
    Serial.println("sechtml.html response");
    request->send(SPIFFS, "/sechtml.html", "text/html");
  });  
  server.on("/sechtml.html", HTTP_POST,[](AsyncWebServerRequest *request){
    Serial.println("response");
    int params = request->params();
    for(int i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isFile()){ //p->isPost() is also true
        Serial.printf("FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
      } else if(p->isPost()){
        Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      } else {
        Serial.printf("GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
    }
    
    if (request->hasParam("ledon")) {
      ledValue = (request->arg("ledon")).toInt();
      Serial.print("LED ON value=");
      Serial.println(ledValue);
      analogWrite(RangeLed,ledValue);
    }
    if (request->hasParam("h")) {
      Serial.print("Hum=");
      Serial.println(humValue);
      request->send(200, "text/plain",String(humValue));
    }
    if (request->hasParam("y")) {
      Serial.print("Temp=");
      Serial.println(tempValue);
      request->send(200, "text/plain",String(tempValue));
    }
    if (request->hasParam("ledoff")) {
      Serial.println("led off");
      analogWrite(RangeLed,0);
    }
  }); 




  server.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  if(attState){
    //DHT get data
    int h;
    int t;
    h=dht.readHumidity(); 
    t=dht.readTemperature();
    String Hum="Humidity:"+String(h)+"%";
    String Tem="Temperature:"+String(t)+"°C";
    Serial.println(Hum);
    Serial.println(Tem);
    u8g2.firstPage();
    do {
      u8g2.setFont(u8g2_font_5x7_tr);
      u8g2.drawStr(2,24,Hum.c_str());
      u8g2.drawStr(2,48,Tem.c_str());
    } while ( u8g2.nextPage() );

    attState=false;
    delay(5000);
    u8g2.clearDisplay(); 
  }
  getHum();
  getTemp();
  delay(500);
}

void attOn(){
  attState=true;
}

void getHum(){
  humValue=dht.readHumidity();
  if(humValue>=80){digitalWrite(HyLed,HIGH);}
  else{digitalWrite(HyLed,LOW);}
}

void getTemp(){
  tempValue=dht.readTemperature();
  if(humValue>=25){digitalWrite(TempLed,HIGH);}
  else{digitalWrite(TempLed,LOW);}
}
