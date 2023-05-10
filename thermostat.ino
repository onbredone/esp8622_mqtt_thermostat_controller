#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Bounce2.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>    

const char* mqtt_server = "{IP_ADRESS}";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

const char* clientID = "ESP 2";
//const char* outTopic = "ESPout1";
const char* inTopic1 = "ESPin2.1";
const char* inTopic2 = "ESPin2.2";

#define RelaisPin 2  // Relais
int active_low = 1;
int relay_pin1 = 2;
int relay_pin2 = 0;
bool relayState1 = LOW;
bool relayState2 = LOW;

ESP8266WebServer server(80);        // Serverport  hier einstellen
int val = 0; 

// Instantiate a Bounce object :
Bounce debouncer = Bounce(); 


void setup_wifi() {
  
  WiFiManager wifiManager;
  wifiManager.autoConnect("AutoConnectAP");
  Serial.println("Connected.");
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    char c = (char)payload[i];
    message += c;
  }
  Serial.println();

  if ((char)payload[0] == '0' || message == "false") {
    digitalWrite(relay_pin1, LOW);
    Serial.println("relay_pin1 -> LOW");
    relayState1 = LOW;
    EEPROM.write(0, relayState1);
    EEPROM.commit();
  } else if ((char)payload[0] == '1' || message == "true") {
    digitalWrite(relay_pin1, HIGH);
    Serial.println("relay_pin1 -> HIGH");
    relayState1 = LOW;
    EEPROM.write(0, relayState1);
    EEPROM.commit();
  } else if ((char)payload[0] == '2') {
    digitalWrite(relay_pin2, LOW);
    Serial.println("relay_pin2 -> LOW");
    relayState2 = LOW;
    EEPROM.write(1, relayState2);
    EEPROM.commit();
  } else if ((char)payload[0] == '3') {
    digitalWrite(relay_pin2, HIGH);
    Serial.println("relay_pin2 -> HIGH");
    relayState2 = HIGH;
    EEPROM.write(1, relayState2);
    EEPROM.commit(); 
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(clientID)) {
      Serial.println("connected");
      client.subscribe(inTopic1);
      client.subscribe(inTopic2);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      for(int i = 0; i<5000; i++){
//        extButton();
        delay(1);
      }
    }
  }
}

void setup() {
  EEPROM.begin(512);              // Begin eeprom to store on/off state
  pinMode(relay_pin1, OUTPUT);     // Initialize the relay pin as an output
  pinMode(relay_pin2, OUTPUT);     // Initialize the relay pin as an output 
  relayState1 = EEPROM.read(0);
  digitalWrite(relay_pin1,relayState1);
  Serial.println("Relay 1 state:" + relayState1);
  relayState2 = EEPROM.read(1);
  digitalWrite(relay_pin2,relayState2);
  Serial.println("Relay 2 state:" + relayState2);
  
  debouncer.interval(50);         // Input must be low for 50 ms
  
  Serial.begin(115200);
  setup_wifi();                   // Connect to wifi 
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
