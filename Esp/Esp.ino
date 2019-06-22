#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoOTA.h>


#define SSIDNAME "NAVI"
#define APPSWD "70007725"

#define ESSID "RT-2.4GHz_WiFi_E556"
#define EKEY "1234567811"

IPAddress default_IP(192, 168, 240, 1); //defaul IP Address

//Device ID
String id = String(WiFi.macAddress()).substring(12);
int num = 1;

//Firmware
String fw = "1.3.1";

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

float battery = 0;
String send_status = "\"Online\"";
bool firstBoot = true;


const int BUFFER_SIZE = 59;
double lat_buffer[BUFFER_SIZE];
double lng_buffer[BUFFER_SIZE];
int cur_buffer_size=0,count=0;

double last_filtered_lat = 0.0;
double last_filtered_lng = 0.0;
bool first_filter_boot = false;

//Константа для проверки точек, если больше 10 метров, откидываем
const double distance_check = 10;


void setup() {
  pinMode(D7, OUTPUT);
  pinMode(D8, OUTPUT);
  Serial.begin(115200);
  ss.begin(GPSBaud);

  setWiFiConfig();
  
  Serial.println();
  Serial.println();
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
  if (ss.available())
    gps.encode(ss.read());
  print_status(); //Print status
}

void loop() {
  //Считываем и фильтруем координаты
  digitalWrite(D7, LOW);    // выключаем светодиод

  // read the input on analog pin 0:
  int avgsensor = 0;
  int sensorValue = 0;
  for (int i = 0; i < 3; i++)
  {
    int sensorValue = analogRead(A0);
    avgsensor = avgsensor + sensorValue;
  }

  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 3.2V):
  battery = (avgsensor / 3) * (3.2 / 1023.0) * 5.465 ;
  // print out the value you read:
  Serial.print("sensorValue: ");
  Serial.println(sensorValue);
  Serial.print("voltage: ");
  Serial.println(battery);

  if (firstBoot)
  {
    String sendDataPut = "{\"id\":\"" + id + "\",\"Number\":" + String(num) + ",\"Latitude\":" + String(-1) + ",\"Lontitude\":" + String(-1) + ",\"Satellite\":" + String(gps.satellites.value(), DEC) + ",\"Battery\":" + String(battery, 2) +",\"RSSI\":" + String(WiFi.RSSI(), DEC) + ",\"Status\":" + send_status + "}";
    SendPostRequest(sendDataPut);
    firstBoot = false;
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("No wifi connect");
  }
  else
  {
    bool hasFix = false;
    //Если спутников больше 3, отправляем данные
    if (gps.satellites.value() > 3)
    {
      if (gps.location.isUpdated())
      {
        //Считаем растояние до новой точки
        double distance = TinyGPSPlus::distanceBetween(
                            gps.location.lat(),
                            gps.location.lng(),
                            last_filtered_lat,
                            last_filtered_lng);
        if (first_filter_boot) {
          if (distance < distance_check)
          {
                  hasFix = true;
                  if(count==BUFFER_SIZE+1)
                    {
                      count = 0;
                    }
                  lat_buffer[count]=gps.location.lat();
                  lng_buffer[count]=gps.location.lng();
                  count++;
                  if(cur_buffer_size < BUFFER_SIZE+1)
                    {
                      cur_buffer_size++;
                    }
            Serial.print("Add to LAT array: "); Serial.println(gps.location.lat(), 11);
            Serial.print("Add to LON array: "); Serial.println(gps.location.lng(), 11);
            Serial.print("cur_buffer_size: "); Serial.println(cur_buffer_size);
            Serial.print("count array: "); Serial.println(count);
          }
        } else {
          last_filtered_lat = gps.location.lat();
          last_filtered_lng = gps.location.lng();
          first_filter_boot = true;
        }
      }
      
 if (hasFix)
  {
    last_filtered_lat = average(lat_buffer,BUFFER_SIZE,cur_buffer_size);
    last_filtered_lng = average(lng_buffer,BUFFER_SIZE,cur_buffer_size);
    Serial.print("LAST filtered LAT= ");  Serial.println(last_filtered_lat, 11);
    Serial.print("LAST filtered LONG= "); Serial.println(last_filtered_lng, 11);
  }

      //digitalWrite(D8, HIGH);   // включаем светодиод
      String sendDataPut = "{\"id\":\"" + id + "\",\"Number\":" + String(num) + ",\"Latitude\":" + String(last_filtered_lat, 11) + ",\"Lontitude\":" + String(last_filtered_lng, 11) + ",\"Satellite\":" + String(gps.satellites.value(), DEC) + ",\"Battery\":" + String(battery, 2)+ ",\"RSSI\":" + String(WiFi.RSSI(), DEC) + ",\"Status\":" + send_status + "}";
      SendPutRequest(sendDataPut, id);
      //digitalWrite(D8, LOW);    // выключаем светодиод

    }
    else
    {
      //Тушить диод отправки данных
      Serial.print("No fix.");
      String sendDataPut = "{\"id\":\"" + id + "\",\"Number\":" + String(num) + ",\"Latitude\":" + String(0) + ",\"Lontitude\":" + String(-1) + ",\"Satellite\":" + String(gps.satellites.value(), DEC) + ",\"Battery\":" + String(battery, 2) + ",\"RSSI\":" + String(WiFi.RSSI(), DEC) +",\"Status\":" + send_status + "}";
      SendPutRequest(sendDataPut, id);
    }

    digitalWrite(D7, HIGH);    // включаем светодиод
    ArduinoOTA.handle();
    print_status(); //Print status
    for (int i = 0; i < gps.satellites.value(); i++)
    {
      digitalWrite(D8, HIGH);
      delay(50);
      digitalWrite(D8, LOW);
    }
  }
  smartDelay(1000);
}

void setWiFiConfig() {
  //WiFi mode is remembered by the esp sdk
  if (WiFi.getMode() != WIFI_STA) 
  {
    //set default AP
    String mac = WiFi.macAddress();
    String apSSID = String(SSIDNAME) + "-" + String(mac[9]) + String(mac[10]) + String(mac[12]) + String(mac[13]) + String(mac[15]) + String(mac[16]);
    char softApssid[18];
    apSSID.toCharArray(softApssid, apSSID.length() + 1);
    //delay(1000);
    WiFi.softAP(softApssid, APPSWD);
    WiFi.softAPConfig(default_IP, default_IP, IPAddress(255, 255, 255, 0));   //set default ip for AP mode
  }
  
  //set STA mode
  WiFi.begin(ESSID, EKEY); // connect to AP with credentials remembered by esp sdk
  if (WiFi.waitForConnectResult() != WL_CONNECTED && WiFi.getMode() == WIFI_STA) 
  {
    // if STA didn't connect, start AP
    WiFi.mode(WIFI_AP_STA); // STA must be active for library connects
    setWiFiConfig(); // setup AP
  }
}

String SendPostRequest(String data) {
  HTTPClient http;
  http.begin(ipStr + url); //Specify request destination
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST(data);   //Send the request
  String payload = http.getString();//Get the response payload
  Serial.println(httpCode);   //Print HTTP return code
  Serial.println(payload);    //Print request response payload

  http.end();  //Close connection
  return payload;
}

String  SendPutRequest(String data, String id) {
  HTTPClient http;
  http.begin(ipStr + url + "/" + id); //Specify request destination
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
  Serial.print("Satellite: ");  //  Satellite
  Serial.println(gps.satellites.value());
  Serial.print("IP address:");  //  "IP-адрес: "
  Serial.println(WiFi.localIP());
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());
  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
  }
  Serial.print("RSSI:");
  Serial.println(WiFi.RSSI());
}

double average(double array[], int size,int buffer_size) {
double sum = 0;
for (int i = 0; i < size; i++) {
sum += array[i];
}
return sum / buffer_size;
}
