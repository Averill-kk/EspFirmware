#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char *essid="Robocenter615";
const char *key="123456789";

int status = WL_IDLE_STATUS; 
static const int RXPin = D5, TXPin = D6;
static const uint32_t GPSBaud = 9600;
SoftwareSerial ss(RXPin, TXPin);
TinyGPSPlus gps;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;  //  ждем подключения последовательного порта
       // (нужно только для штатного USB-порта)
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
 

Serial.print("You're connected to the network");
}
 
void loop() {
 //printInt(gps.satellites.value(), gps.satellites.isValid(), 5);
 //printFloat(gps.hdop.hdop(), gps.hdop.isValid(), 6, 1);
 //printFloat(gps.location.lat(), gps.location.isValid(), 11, 6);
 //printFloat(gps.location.lng(), gps.location.isValid(), 12, 6);
 //printInt(gps.location.age(), gps.location.isValid(), 5);
   delay(10000);
}


static void printFloat(float val, bool valid, int len, int prec)
{
  if (!valid)
  {
    while (len-- > 1)
      Serial.print('*');
    Serial.print(' ');
  }
  else
  {
    Serial.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i=flen; i<len; ++i)
      Serial.print(' ');
  }
  smartDelay(0);
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
static void printInt(unsigned long val, bool valid, int len)
{
  char sz[32] = "*****************";
  if (valid)
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i=strlen(sz); i<len; ++i)
    sz[i] = ' ';
  if (len > 0) 
    sz[len-1] = ' ';
  Serial.print(sz);
  smartDelay(0);
}
