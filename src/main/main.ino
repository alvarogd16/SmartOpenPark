/****************************
    SmartOpenPark proyect
*****************************/

// Utils libraries
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "DHT.h"

// Need a config file to Wifi and API config
#include "./config.h"

// API url
String url = "http://__API_IP__/api/v1/output/json_write";
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

  url.replace("__API_IP__", API_IP);
  data.replace("__API_KEY__", API_KEY);

  Serial.println(url);

  // Start DHT22 sensor
  dht.begin();
  // The sensor need two seconds to load correctly
  delay(2000);
}

void loop () {
    // Read the data and check it
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if(isnan(t)) {
    Serial.println(F("Error reading DHT22 temp. NaN values"));
    t = 0.0;
  }

  if(isnan(h)) {
    Serial.println(F("Error reading DHT22 humidity. NaN values"));
    h = 0;
  }

  timeClient.update();
  String currentTime = timeClient.getFormattedDate();


  // Create the payload with the data
  String fullData = String(data);
  fullData.replace("__TEMP__", (String)t);
  fullData.replace("__HUMIDITY__", (String)h);
  fullData.replace("__TIME__", currentTime);
  Serial.println(fullData);


  // Send the data to the DB using the API
  if(WiFi.status()== WL_CONNECTED){
    WiFiClient client;
    HTTPClient http;
    
    // Your Domain name with URL path or IP address with path
    http.begin(client, url);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpResponseCode = http.POST(fullData);
      
    if (httpResponseCode>0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();
  }

  delay(10000);
}
