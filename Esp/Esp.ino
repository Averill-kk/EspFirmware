#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Ethernet.h>
#include <ArduinoOTA.h>

//Device ID
String id = String(WiFi.macAddress()).substring(12);
int num = 1;

//Firmware
String fw = "1.1";

//WiFi Info
const char *essid="RT-2.4GHz_WiFi_E556";
const char *key="1234567811";

const char* apssid = id.c_str();    // Имя точки доступа
const char* appassword = "naviboat2019"; 
//WebAPi Info
int computerHostPort = 5050;
String url = "/api/GpsDatasApi";
String ipStr = "http://naviboat.ru:5050";

//Board Options
int status = WL_IDLE_STATUS; 
static const int RXPin = D5, TXPin = D6;
static const uint32_t GPSBaud = 9600;
SoftwareSerial ss(RXPin, TXPin);
TinyGPSPlus gps;
WiFiClient clientCommon;

int battery = 2;
String send_status = "\"Online\"";  
bool firstBoot = true;

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(apssid, appassword);
  ss.begin(GPSBaud);
WiFi.begin(essid,key);
if(WiFi.status() != WL_CONNECTED)
{
//Тушить диод подключения
    delay(500);
    Serial.print("No wifi connect");
//Зажигать диод подключения
}
Serial.println("WiFi connected");
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
  
  // строчка для номера порта по умолчанию
  // можно вписать «8266»:
  // ArduinoOTA.setPort(8266);
 
  // строчка для названия хоста по умолчанию;
  // можно вписать «esp8266-[ID чипа]»:
  // ArduinoOTA.setHostname("myesp8266");
 
  // строчка для аутентификации
  // (по умолчанию никакой аутентификации не будет):
  // ArduinoOTA.setPassword((const char *)"123");
 
  ArduinoOTA.onStart([]() {
    Serial.println("Start");  //  "Начало OTA-апдейта"
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");  //  "Завершение OTA-апдейта"
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    //  "Ошибка при аутентификации"
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    //  "Ошибка при начале OTA-апдейта"
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    //  "Ошибка при подключении"
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    //  "Ошибка при получении данных"
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
    //  "Ошибка при завершении OTA-апдейта"
  });
  ArduinoOTA.begin();
  if(ss.available())
      gps.encode(ss.read());
print_status(); //Print status
}
 
void loop() {
  ArduinoOTA.handle();
  print_status(); //Print status

if(firstBoot && WiFi.status() == WL_CONNECTED )
{
  String sendDataPut = "{\"id\":\""+id+"\",\"Number\":"+String(num)+",\"Latitude\":"+String(-1)+",\"Lontitude\":"+String(-1)+",\"Satellite\":"+String(gps.satellites.value(), DEC)+",\"Battery\":"+String(battery, DEC) +",\"Status\":"+send_status+"}";
  SendPostRequest(sendDataPut);
  firstBoot = false;
}
//Если спутников больше 3, отправляем данные
if (gps.satellites.value()>3)
      {
//Зажигать диод отправки данных
  String sendDataPut = "{\"id\":\""+id+"\",\"Number\":"+String(num)+",\"Latitude\":"+String(gps.location.lat(),11)+",\"Lontitude\":"+String(gps.location.lng(),11)+",\"Satellite\":"+String(gps.satellites.value(), DEC)+",\"Battery\":"+String(battery, DEC) +",\"Status\":"+send_status+"}";
  SendPutRequest(sendDataPut,id);
  delay(1000);
  }else
  {
//Тушить диод отправки данных
     Serial.print("No fix.");
     String sendDataPut = "{\"id\":\""+id+"\",\"Number\":"+String(num)+",\"Latitude\":"+String(0)+",\"Lontitude\":"+String(-1)+",\"Satellite\":"+String(gps.satellites.value(), DEC)+",\"Battery\":"+String(battery, DEC) +",\"Status\":"+send_status+"}";
     SendPutRequest(sendDataPut,id);
     delay(1000);
  }
   smartDelay(1000);//считываение данных с GPS устройства
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
static void print_status()
{
  Serial.print("Board ID: ");  //  Id
  Serial.println(id);
  Serial.print("Board ID: ");  //  Firmware
  Serial.println(fw);
  Serial.print("Battery: ");  //  Battery
  Serial.println(battery);
  Serial.print("GPS port avaible: ");  //  GPS
  Serial.println(ss.available());
  Serial.print("Satellite: ");  //  Satellite
  Serial.println(gps.satellites.value());
  Serial.print("IP address:");  //  "IP-адрес: "
  Serial.println(WiFi.localIP());
  Serial.print("MAC: ");
   Serial.println(WiFi.macAddress());
}
