/****************************
 * SmartOpenPark 🐦
 * (github.com/alvarogd16/SmartOpenPark)
 * 
 * Author: Álvaro García - @alvarogd16
 * Mail: alvarogarcia16102000@gmail.com
 * Licence: TODO
 ****************************/
 
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "DHT.h"

/*
 * You need a external config.h file with Wifi and API info.
 * 
 * This file is normally provided by me (Álvaro)
 * If you need to modify or create a new one the fields needed are:
 * - WIFI_SSID (your wifi name)
 * - WIFI_PASSWORD (the password)
 * - API_IP (by now we use the SmartPoliTech API)
 * - API_KEY
 * All the above are const char* to simplicity
 */
#include "./config.h"

String apiUrl = "http://__API_IP__/api/v1/output/json_write";
String data = "json={\"info\": {\"device\": \"SMARTOPENPARK_PRUEBAS_1\", \"api_key\": \"__API_KEY__\"}, \"data\": \"{\\\"temperature\\\": __TEMP__, \\\"humidity\\\": __HUMIDITY__}\", \"time\": \"__TIME__\"}";

// Get time from NTP (Network Time Protocol) server
int utcOffsetInSeconds = 7200;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", utcOffsetInSeconds);

// DHT sensor config 
// This is the temperature and humidity sensor
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

struct sensorData_t {
  float temperature;
  float humidity;
} sensorData;

const int RETRY_TIME = 2000;
const int DELAY_TIME = 10000;

/*
 * In this proyect we use a DHT22 sensor
 */
bool readSensor(sensorData_t &sensorData) {
  // Read the data and check it
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  bool isDataOK = true;

  if(isnan(t)) {
    Serial.println(F("Error reading DHT22 temp. NaN values"));
    t = 0.0;
    isDataOK = false;
  }
  if(isnan(h)) {
    Serial.println(F("Error reading DHT22 humidity. NaN values"));
    h = 0;
    isDataOK = false;
  }

  sensorData.temperature = t;
  sensorData.humidity = h;
  return isDataOK;
}

/*
 * Add current time and sensor data to the json payload
 */
String parseData(String &jsonData, sensorData_t &sensorData) {
  timeClient.update();
  String currentTime = timeClient.getFormattedDate();
  
  String jsonPayload = String(jsonData);
  jsonPayload.replace("__TEMP__", (String)sensorData.temperature);
  jsonPayload.replace("__HUMIDITY__", (String)sensorData.humidity);
  jsonPayload.replace("__TIME__", currentTime);
  Serial.println(jsonPayload);

  return jsonPayload;
}

/*
 * Send the data to the DB using the API
 * Make a POST HTTP request
 */
void sendDataRequest(String &dataToSend) {
  if(WiFi.status() != WL_CONNECTED) return;

  WiFiClient wifiClient;
  HTTPClient http;
  
  http.begin(wifiClient, apiUrl);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpResponseCode = http.POST(dataToSend);

  /*
   * To know the error corresponding to tht number
   * check the ESP8266HTTPClient documentation
   */
  if(httpResponseCode < 0) {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
    http.end();
    return;
  }

  String payload = http.getString();
  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);
  Serial.println(payload);

  // Free resources
  http.end();
}

void setup() {
  // TODO Made Serial optional with a define
  Serial.begin(115200);
  delay(10);

  // Connect to the wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("\nConectandonos a:\t");
  Serial.println(WIFI_SSID);

  while(WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print('.');
  }

  Serial.print("\nConectado a:\t");
  Serial.println(WIFI_SSID);
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());

  timeClient.begin(123);

  apiUrl.replace("__API_IP__", API_IP);
  data.replace("__API_KEY__", API_KEY);

  Serial.println(apiUrl);

  // Start DHT22 sensor
  dht.begin();
  // The sensor need two seconds to load correctly
  delay(2000);
}

void loop () {
  bool isDataOK = readSensor(sensorData);
  if(!isDataOK) {
    Serial.print("Retry to get sensor data in ");
    Serial.print(RETRY_TIME);
    Serial.println(" milliseconds");
    delay(RETRY_TIME);
    return;
  }
  
  String jsonPayload = parseData(data, sensorData);
  sendDataRequest(jsonPayload);
  delay(DELAY_TIME);
}
