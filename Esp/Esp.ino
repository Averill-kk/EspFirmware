#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Ethernet.h>
#include <ICMPPing.h>

//WiFi Info
const char *essid="Robocenter615";
const char *key="123456789";

//WebAPi Info
IPAddress computerHost(10, 10, 143, 73);
int computerHostPort = 5050;
String url = "/api/GpsData";
String ipStr = "10.10.143.73:5050";


int status = WL_IDLE_STATUS; 
static const int RXPin = D5, TXPin = D6;
static const uint32_t GPSBaud = 9600;
SoftwareSerial ss(RXPin, TXPin);
TinyGPSPlus gps;
WiFiClient clientCommon;
bool postRequestSend = false;

IPAddress pingAddr(10,10,143,73);
SOCKET pingSocket = 0;
char buffer [256];
ICMPPing ping(pingSocket, (uint16_t)random(0, 255));

String Id;
char messageId;
void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; 
       
  }
 
  // задаем скорость передачи данных через порт SoftwareSerial:
  ss.begin(GPSBaud);
WiFi.begin(essid,key);
while(WiFi.status() != WL_CONNECTED)
{
    delay(500);
    Serial.print(".");
}
Serial.println("WiFi connected");
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
}
 
void loop() {
float flat, flon;
Serial.println("Loop started");
Serial.print("Satellites in view: ");
Serial.println(gps.satellites.value());
      if (gps.satellites.value() < 6)
      {
        Serial.print("No fix.");
      }
      else
      {
String sendDataPost = "{\"Latitude\":"+String(gps.location.lat())+",\"Lontitude\":"+String(gps.location.lng())+",\"Satellite\":"+String(gps.satellites.value())+"}";
Serial.println(messageId);
if(!postRequestSend)
{
  Id = SendPostRequest(sendDataPost)[7];
  postRequestSend = true;
}else
{
  String sendDataPut = "{\"id\":"+Id+",Latitude\":"+String(gps.location.lat())+",\"Lontitude\":"+String(gps.location.lng())+",\"Satellite\":"+String(gps.satellites.value())+"}";
  SendPutRequest(sendDataPut,Id);
}
   delay(10000); 
  }
 }

String SendPostRequest(String data) {
HTTPClient http;
http.begin(ipStr+url);  //Specify request destination
http.addHeader("Content-Type", "application/json"); 
int httpCode = http.POST(data);   //Send the request
   String payload = http.getString();//Get the response payload
   Serial.println(httpCode);   //Print HTTP return code
   Serial.println(payload);    //Print request response payload
 
   http.end();  //Close connection
   return payload;
}
String  SendPutRequest(String data,String id) {
HTTPClient http;
http.begin(ipStr+url+"/"+id);  //Specify request destination
http.addHeader("Content-Type", "application/json"); 
int httpCode = http.PUT(data);   //Send the request
String payload = http.getString();//Get the response payload
Serial.println(httpCode);   //Print HTTP return code
Serial.println(payload);    //Print request response payload
http.end();  //Close connection
return payload;
}
