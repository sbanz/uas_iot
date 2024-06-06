#include <ESP8266WiFi.h> 
#include <PubSubClient.h> 
#include "DHT.h"

#define DHTPIN D4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
#define LEDPIN D7

const char* ssid = "sekut cuyy";
const char* password = "gansbingit";

WiFiClient espClient;
PubSubClient client(espClient);

const char *mqtt_broker = "broker.emqx.io";
String topic = "saban/iot/";
const char *mqtt_username = "";
const char *mqtt_password = "";
const int mqtt_port = 1883;
String tempData = "";
String humData = "";


void loop() {
  client.loop();
  readTemp();
  readHum();
  sendData();
  delay(1000); 
}

void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print(message);

  if (strcmp(topic, "saban/iot/led") == 0) {
     if (message == "1") {
      digitalWrite(LEDPIN, HIGH);
      Serial.println("Led nyala");
    } else if (message == "0") {
      digitalWrite(LEDPIN, LOW);
      Serial.println("Led mati");
    }
  }
  Serial.println();
  Serial.println("-----------------------");
}

void readTemp() {
  float temp = dht.readTemperature();
  if (isnan(temp)) {
    Serial.println("Error: DHT temperature reading is NaN!");
    String error = "Error";
    String topik = topic + "data";
    client.publish(topik.c_str(), error.c_str());
  } else {
    tempData = String(temp);
  }
}

void readHum() {
  float hum = dht.readHumidity();
  if (isnan(hum)) {
    Serial.println("Error: DHT humidity reading is NaN!");
    String error = "Error";
    String topik = topic + "data";
    client.publish(topik.c_str(), error.c_str());
  } else {
    humData = String(hum);
  }
}

void sendData() {
  String Data = tempData + " - " + humData;
  String topik = topic + "data";
  client.publish(topik.c_str(), Data.c_str());
}

void setup() {
  Serial.begin(115200);

  // Connecting to a WiFi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to the WiFi network");

  dht.begin();
  pinMode(DHTPIN, INPUT);
  pinMode(LEDPIN, OUTPUT);
  digitalWrite(LEDPIN, LOW);

  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  while (!client.connected()) {
    String client_id = "mqttx_37aad4abxaxasd";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Public EMQX MQTT broker connected");
    } else {
      Serial.print("Failed with state ");
      Serial.print(client.state());
      delay(2000);
    }

  client.publish(topic.c_str(), "NodeMCU berhasil terkoneksi");
  client.subscribe(topic.c_str());
  client.subscribe("saban/iot/led");
  readTemp();
  readHum();
  sendData();
  }
}

