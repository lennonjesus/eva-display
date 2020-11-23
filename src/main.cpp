#include <Arduino.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "ESP8266WiFi.h"
#include "SSD1306Wire.h"
#include "Wire.h"

#include "credentials.h" // ATTENTION: look at credentials.sample.h file
 
#define I2C_DISPLAY_ADDRESS   0x3c
#define SDA_PIN               D2
#define SCL_PIN               D1
SSD1306Wire display(I2C_DISPLAY_ADDRESS, SDA_PIN, SCL_PIN);
 
WiFiClient client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

Adafruit_MQTT_Subscribe temperatureSub = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/temperature", MQTT_QOS_1);
Adafruit_MQTT_Subscribe humiditySub = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/humidity", MQTT_QOS_1);
Adafruit_MQTT_Subscribe rainPercentSub = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/rainPercent", MQTT_QOS_1);

long previousMillis = 0;

String rainPercent = "";
String humidity = "";
String temperature = "";

void checkWifi();
void connectWiFi();
void initMQTT();
void connectMqtt();
void temperatureCallback(char *data, uint16_t len);
void humidityCallback(char *data, uint16_t len);
void rainPercentCallback(char *data, uint16_t len);
void displaySetup();
void displayWeather(int delayMills);
 
void setup() {
  Serial.begin(115200);
  delay(10);

  displaySetup();

  connectWiFi();
  initMQTT();

}
 
void loop() {
  checkWifi();
  connectMqtt();
  mqtt.processPackets(5000);

  displayWeather(5000);

  delay(1000);
}

void initMQTT() {
  temperatureSub.setCallback(temperatureCallback);
  humiditySub.setCallback(humidityCallback);
  rainPercentSub.setCallback(rainPercentCallback);
  
  mqtt.subscribe(&temperatureSub);
  mqtt.subscribe(&humiditySub);
  mqtt.subscribe(&rainPercentSub);
}

void temperatureCallback(char *data, uint16_t len) {
  temperature = data;
  Serial.print("Temperatura: "); 
  Serial.println(temperature);
}

void humidityCallback(char *data, uint16_t len) {
  humidity = data;
  Serial.print("Humidade: "); 
  Serial.print(humidity);
  Serial.println("%");
}

void rainPercentCallback(char *data, uint16_t len) {
  rainPercent = data;
  Serial.print("Chuva: "); 
  Serial.print(rainPercent);
  Serial.println("%");
}
 
void connectMqtt() {
  int8_t ret;
 
  if (mqtt.connected()) {
    return;
  }
 
  Serial.println("Conectando-se ao broker mqtt...");
 
  uint8_t num_tentativas = 5;
  
  while ((ret = mqtt.connect()) != 0) {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Falha ao se conectar. Tentando se reconectar em 5 segundos.");
    mqtt.disconnect();
    delay(5000);
    num_tentativas--;
    if (num_tentativas == 0) {
      Serial.println("Seu ESP será resetado.");
      ESP.restart();
    }
  }
 
  Serial.println("Conectado ao broker com sucesso.");
}

void checkWifi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(" Conexão não estabelecida. Reiniciando...");
    ESP.restart();
  } 
}

void connectWiFi() {

  delay(100);
  
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.print("Conectando em ");
  Serial.print(WIFI_SSID);

  int timeout = 0;

  while (WiFi.status() != WL_CONNECTED && ++timeout <= 10) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    Serial.print("."); 
  }

  checkWifi();

  Serial.println("");
  Serial.print("Conectado em ");
  Serial.print(WIFI_SSID);
  Serial.print(" com o IP ");
  Serial.println(WiFi.localIP());
}

void displaySetup() {
  display.init();
  display.clear();
  display.display();
  
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(10, 0, "EVA Display");
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 52, "SW Ver.:");
  display.drawString(45, 52, "0.0.2");
  display.display();

  delay(1000);
  
  for (int value = 0; value <= 100; value+=5) {
    display.clear();
    display.display();
    
    // x, y, width, height, value
    display.drawProgressBar(10, 32, 100, 10, value);
    display.setFont(ArialMT_Plain_16);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 15, String(value) + "%");
    display.display();
    delay(50);
  }

  delay(500);
}

void displayWeather(int delayMills) {
  display.clear();
  display.display();

  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(10, 0, "Temp.:");
  display.drawString(70, 0, (String) temperature);
  display.drawString(110, 0, "º");
  display.drawString(10, 20, "Umid.:");
  display.drawString(70, 20, (String) humidity);
  display.drawString(110, 20, "%");
  display.drawString(10, 40, "Chuva:");
  display.drawString(70, 40, (String) rainPercent);
  display.drawString(110, 40, "%");
  display.display();
  
  delay(delayMills);
  display.clear();
  display.display();
}