#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
//#include <ESP8266WebServer.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>

#define DHTTYPE   DHT22
#define DHTPIN    2

#define WLAN_SSID     "WLAN_SSID"
#define WLAN_PASSWORD "WIFI_SECURE_KEY"
#define HOSTNAME      "thermometerweb"

DHT dht(DHTPIN, DHTTYPE, 22);

AsyncWebServer server(80);

float temperature, humidity;
int soil = analogRead(A0);
float soil_percent;
unsigned long previousMillis = 0;
const long interval = 2000;

String getTemperature(){
  float temp = dht.readTemperature();
  if(isnan(temp)){
    return "error reading temp";
  }
  Serial.println("Temperature read: " + String(temp) + "°C");
  return String(temp);
}

String getHumidity(){
  float humidity = dht.readHumidity();
  if(isnan(humidity)){
    return "error reading humidity";
  }
  Serial.println("Humidity read: " + String(humidity) + "%");
  return String(humidity);
}

String processor(const String& var){
  Serial.println(var);
  if (var == "TEMPERATURE"){
    return getTemperature();
  }
  else if (var == "HUMIDITY"){
    return getHumidity();
  }
  else {
    Serial.println("Error: Case not defined");
  }  
}

void setup() {
    Serial.begin(115200); 
    delay(10);

    if(!SPIFFS.begin()){
      Serial.println("An Error has occurred while mounting SPIFFS");
      return;
    }

    dht.begin();
    String temp = String(dht.readTemperature());
    String humidity = String(dht.readHumidity());
    Serial.println("Temp: " + temp + "°C" + " Humidity: " + humidity + "%");
    pinMode(soil, INPUT);
    delay(10);

    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(WLAN_SSID);

    WiFi.mode(WIFI_STA);
    WiFi.hostname(HOSTNAME);
    WiFi.begin(WLAN_SSID, WLAN_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println();
    Serial.println("WiFi connected");
    Serial.println("IP address: "); Serial.println(WiFi.localIP());        
     
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/index.html", String(), false, processor);
    });

    server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/plain", getTemperature().c_str());
    });
  
    server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/plain", getHumidity().c_str());
    });

    server.begin();
    Serial.println("Async HTTP server started! Waiting for clients!");
}

void loop() {
}
