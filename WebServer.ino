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
  return String(temperature);
}

String getHumidity(){
  return String(humidity);
}

String getHostname(){
  return HOSTNAME;
}

String getJson(){
  StaticJsonDocument<500> doc;
  doc["hostname"] = HOSTNAME;
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
        
  String jsonString;
  serializeJson(doc, jsonString);
  return jsonString;
}

String processor(const String& var){
  Serial.println(var);
  if (var == "TEMPERATURE"){
    return getTemperature();
  }
  else if (var == "HUMIDITY"){
    return getHumidity();
  }
  else if (var == "HOSTNAME"){
    return getHostname();
  }
  else {
    Serial.println("Error: Case not defined");
    return var;
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
    Serial.print("IP address: "); Serial.println(WiFi.localIP());   

    do{
      temperature = dht.readTemperature();
      Serial.println("temp: " + String(temperature));
      delay(2000);
    }while(isnan(temperature));
     
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/index.html", String(), false, processor);
    });

    server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
      Serial.println(getTemperature());
      request->send_P(200, "text/plain", getTemperature().c_str());
    });
  
    server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
      Serial.println(getHumidity());
      request->send_P(200, "text/plain", getHumidity().c_str());
    });

    server.on("/json", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/json", getJson().c_str());
    });

    server.begin();
    Serial.println("Async HTTP server started! Waiting for clients!");
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the last time you updated the DHT values
    previousMillis = currentMillis;
    // Read temperature as Celsius (the default)
    float newT = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    //float newT = dht.readTemperature(true);
    // if temperature read failed, don't change t value
    if (isnan(newT)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    else {
      temperature = newT;
      Serial.println(temperature);
    }
    // Read Humidity
    float newH = dht.readHumidity();
    // if humidity read failed, don't change h value 
    if (isnan(newH)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    else {
      humidity = newH;
      Serial.println(humidity);
    }
  }
  delay(200);
}
