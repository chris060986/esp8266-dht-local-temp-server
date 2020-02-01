#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>

#define DHTTYPE   DHT22
#define DHTPIN    2

#define WLAN_SSID     "WLAN_SSID"
#define WLAN_PASSWORD "WIFI_SECURE_KEY"
#define HOSTNAME      "thermometerweb"

DHT dht(DHTPIN, DHTTYPE, 22);

ESP8266WebServer server(80);



float temperature, humidity;
int soil = analogRead(A0);
float soil_percent;
unsigned long previousMillis = 0;
const long interval = 2000;

void setup() {
    Serial.begin(115200); 
    delay(10);


    dht.begin();
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();
    pinMode(soil, INPUT);
   

    Serial.println(); Serial.println();
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

    
    server.on("/temp.json", [](){
        unsigned long currentMillis = millis();

        if (currentMillis - previousMillis >= interval) {
            previousMillis = currentMillis;

            humidity = dht.readHumidity();
            temperature = dht.readTemperature();
                     

            if (isnan(humidity) || isnan(temperature)) {
                Serial.println("Failed to read from DHT sensor!");
                return;
            }

            Serial.println("Reporting " + String((int)temperature) + "C and " + String((int)humidity) + " % humidity");
        }

        StaticJsonDocument<500> doc;
        doc["hostname"] = HOSTNAME;
        doc["temperature"] = temperature;
        doc["humidity"] = humidity;
        
       
        String jsonString;
        serializeJson(doc, jsonString);

        Serial.println(jsonString);
        server.send(200, "application/json", jsonString);
    });

    server.begin();
    Serial.println("HTTP server started! Waiting for clients!");
}

void loop() {
    server.handleClient();
}
