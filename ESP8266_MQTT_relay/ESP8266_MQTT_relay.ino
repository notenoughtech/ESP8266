#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "secure.h"

// Wifi Connection
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASS;

// MQTT Server address
const char* mqtt_server = MQTT_SERVER;  // ip of your server
const char* ESP_name = MQTT_CLIENT_NAME; //Unique MQTT name for your ESP
const char* topic_internet = "4relay/network";   // "online or offline" network status
const char* topic_status = "4relay/relay";       // "1 or 0" to toggle socket
const char* topic_c_state = "4relay/update";     //"offline"// current state "1 or 0" 
const char* topic_will = "4relay/network";       // last will the same as "topic_internet"
const char* ESP_will = "offline";             //last will
              
int pin = 15;                                  //ESP pin RX is the GPIO3 

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup() {
  
  pinMode(pin, OUTPUT);
  digitalWrite(pin, 0);
//disabled serial after troubleshooting
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void setup_wifi() {

  delay(2);
  Serial.println();
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if ((char)payload[0] == '1') { 
    digitalWrite(pin, 1);
    client.publish(topic_c_state, "1");    
  } 
  else {
    digitalWrite(pin, 0);
    client.publish(topic_c_state, "0");
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(ESP_name, topic_will, 1, false, ESP_will)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(topic_internet, "online");
      // ... and resubscribe
      client.subscribe(topic_status);      
    } 
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 10000) {
    lastMsg = now;
    ++value;
    snprintf (msg, 75, "online", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish(topic_internet, msg);
  }
}
