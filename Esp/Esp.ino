#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Ethernet.h>

//Device ID
String Id = "1";


//WiFi Info
const char *essid="Robocenter";
const char *key="123456789";

//WebAPi Info
IPAddress computerHost(10, 10, 143, 73);
int computerHostPort = 5050;
String url = "/api/GpsData";
String ipStr = "http://10.10.143.73:5050";


int status = WL_IDLE_STATUS; 
static const int RXPin = D5, TXPin = D6;
static const uint32_t GPSBaud = 9600;
SoftwareSerial ss(RXPin, TXPin);
TinyGPSPlus gps;
WiFiClient clientCommon;


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
Serial.print("Satellites in view: ");
Serial.println(gps.satellites.value());
if (true)
      {
//String sendDataPost = "{\"Latitude\":"+String(gps.location.lat(),11)+",\"Lontitude\":"+String(gps.location.lng(),1)+",\"Satellite\":"+String(gps.satellites.value(), DEC)+"}";
  String sendDataPut = "{\"id\":"+Id+",\"Latitude\":"+String(gps.location.lat(),11)+",\"Lontitude\":"+String(gps.location.lng(),11)+",\"Satellite\":"+String(gps.satellites.value(), DEC)+"}";
  SendPutRequest(sendDataPut,Id);
  delay(1000);
  }else
  {
     Serial.print("No fix.");
  }
   smartDelay(1000);
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

static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}
