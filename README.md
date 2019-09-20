# EspFirmware
Прошивка для ESP8266.
При первом включении ESP отправит на сервер POST запрос, добавиться новая запись в БД с координатами ESP. Далее эта запись будет обновляться, с заданым интервалом.

В прошивке рализовано OTA обновление ESP8266, для удобства настройки.

# Настройки 

const char *essid="beacontest"; *//SSID точки доступа к которой подключается ESP*
const char *key="123456789"; *//точки доступа *

const char* apssid = id.c_str();    **// Имя точки доступа котораю создает ESP**
const char* appassword = ***//naviboat2019";  пароль точки доступа ***
//WebAPi Info
String url = "/api/GpsDatasApi"; ***// URL***
String ipStr = "http://naviboat.ru:5050"; ***//Адрес сервера***
# Процесс прошивки
Для прошивки ESP8266, через  Arduino IDE,  Arduino IDE нужно настроить. Подробно это описанно в статье по ссылке https://habr.com/post/371853/


