#include <PubSubClient.h>
#include <WiFi.h>
#include <DHT.h> //add

#define ON 1
#define OFF 0

#define DHTTYPE DHT22

// initial WIFI 
const char* ssid = "Wokwi-GUEST";
const char* password =  "";
WiFiClient espClient;

// initial MQTT client
const char* mqttServer = "mqtt.netpie.io";
const int mqttPort = 1883;
const char* clientID = "93c35b34-a81f-460a-9f84-012b4d267221";
const char* mqttUser = "99wVJvnLUaYN6RD8goHSwrZnCY3okhsF";
const char* mqttPassword = "3bjr8T86aZSfCifqrnu2uP8EMbSF3k8Y";
const char* topic_pub = "@shadow/data/update";
const char* topic_sub = "@msg/lab_ict_kps/command";
// send buffer
String publishMessage;

PubSubClient client(espClient);

// LED initial pin
const int RED_LED_PIN = 32;
const int GREEN_LED_PIN = 33;
const int BUZZER_PIN = 25;

const int YELLOW_LED_ALERT = 27;
const int ORANGE_LED_ALERT = 14;

const int DHT_PIN = 26;
DHT dht(DHT_PIN, DHTTYPE);

const int LDR_SENSOR = 13;
int luxThreshold = 100; 
const int  TEMP_SENSOR = 12;

int redLED_status = OFF, greenLED_status = OFF, buzzer_status = OFF;

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnectMQTT() {
  // Loop until we're reconnected
  char mqttinfo[80];
  snprintf (mqttinfo, 75, "Attempting MQTT connection at %s:%d (%s/%s)...", mqttServer, mqttPort, mqttUser, mqttPassword);
  while (!client.connected()) {
    Serial.println(mqttinfo);
    String clientId = clientID;
    if (client.connect(clientId.c_str(), mqttUser, mqttPassword)) {
      Serial.println("...Connected");
      client.subscribe(topic_sub);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void messageReceivedCallback(char* topic, byte* payload, unsigned int length) { 
  char payloadMsg[80];
  
  Serial.print("Message arrived in topic: ");
  Serial.println(topic); 
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    payloadMsg[i] = (char) payload[i];
  }
  payloadMsg[length] = '\0';   // put end of string to buffer
  Serial.println();
  Serial.println("-----------------------");
  processMessage(payloadMsg);
}

void processMessage(String recvCommand) {
 
  if (recvCommand == "RED_ON") {
    digitalWrite(RED_LED_PIN, HIGH);
    redLED_status = ON;
  } else  if (recvCommand == "RED_OFF") {
    digitalWrite(RED_LED_PIN, LOW);
     redLED_status = OFF;
  } else  if (recvCommand == "GREEN_ON") {
    digitalWrite(GREEN_LED_PIN, HIGH);
    greenLED_status = ON;
  } else  if (recvCommand == "GREEN_OFF") {
    digitalWrite(GREEN_LED_PIN, LOW);
    greenLED_status = OFF;
  } else  if (recvCommand == "BUZZER_ON") {
    //digitalWrite(BUZZER_PIN, HIGH);
    buzzer_status = ON;
  } else  if (recvCommand == "BUZZER_OFF") {
    //digitalWrite(BUZZER_PIN, LOW);
    buzzer_status = OFF;
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(messageReceivedCallback); 
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  //pinMode(BUZZER_PIN, OUTPUT);
  redLED_status = OFF;
  greenLED_status = OFF;
  buzzer_status = OFF;
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW);
  tone(BUZZER_PIN,100, 1000);

  pinMode(YELLOW_LED_ALERT, OUTPUT);
  digitalWrite(YELLOW_LED_ALERT, LOW);
  pinMode(ORANGE_LED_ALERT, OUTPUT);
  digitalWrite(YELLOW_LED_ALERT, LOW);

  pinMode(LDR_SENSOR, INPUT);
  pinMode(TEMP_SENSOR, OUTPUT);
}

void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();

  dht.read(); // Read temperature and humidity from DHT sensor
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  
  
  publishMessage = "{\"data\": {\"red_led\": " + String(redLED_status) + ", \"green_led\": "  + String(greenLED_status) + " ,\"buzzer\": " + String(buzzer_status) + ",\"temperature\": " + String(t) + ", \"humidity\": " + String(h) + "}}";
  Serial.println(publishMessage);
  client.publish(topic_pub, publishMessage.c_str());
   if (buzzer_status == ON ) {
    tone(BUZZER_PIN, 100, 1000);
  } else if (buzzer_status == OFF ) {
    noTone(BUZZER_PIN);
  }

    if (t > 30.0) {
    digitalWrite(ORANGE_LED_ALERT, LOW);
    digitalWrite(YELLOW_LED_ALERT, HIGH);
  } else {
    digitalWrite(ORANGE_LED_ALERT, HIGH);
    digitalWrite(YELLOW_LED_ALERT, LOW);
  }

  int lightstate = analogRead(LDR_SENSOR);

  Serial.print("LDR Value = ");
  Serial.print(lightstate);

  delay(2000); 
}