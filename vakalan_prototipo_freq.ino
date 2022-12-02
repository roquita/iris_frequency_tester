#include <WiFi.h>
#include <PubSubClient.h>

#define pinBuzzer 25

// setting PWM properties
const int freq = 1000;// [1-1000]Hz
const int ledChannel = 0;
const int resolution = 8;

const char* ssid = "TE_HACKEL_EL_INTER";
const char* password = "QWE&ASD%ZXC#!!";
const char* mqtt_server = "wss://broker.vakalan.com:8084/mqtt";
const char* topic_publish = "564963785";
const char* topic_suscribe = "";

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);

  /*
    // configure LED PWM functionalitites
    ledcSetup(ledChannel, freq, resolution);

    // attach the channel to the GPIO to be controlled
    ledcAttachPin(pinBuzzer, ledChannel);

    // duty cycle to 50%
    ledcWrite(ledChannel, 125);
  */
  if (connect_to_wifi() == false ) {
    esp_restart();
  }

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  static long lastMsg = 0;
  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    client.publish(topic_publish,
                   "{"
                   "\"gps\":{\"lat\":\"-12.09\",\"lng\":\"-77.05\"},"
                   "\"acc\":{\"x\":\"-1.59\",\"y\":\"0.08\",\"z\":\"10.22\"},"
                   "\"gyro\":{\"x\":\"-5.05\",\"y\":\"0.75\",\"z\":\"0.26\"}"
                   "}"
                  );
  }
}

bool connect_to_wifi() {
  WiFi.begin(ssid, password);
  Serial.println(F("Connecting"));
  int counter = 0;
  while ( (WiFi.status() != WL_CONNECTED) && (counter <= 10)) {
    delay(500);
    counter++;
    Serial.print(".");
  }
  Serial.println(F(""));
  Serial.print(F("Connected to WiFi network with IP Address: "));
  Serial.println(WiFi.localIP());

  bool is_connected = WiFi.status() == WL_CONNECTED;
  return is_connected;
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/output");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off".
  // Changes the output state according to the message
  /*
    if (String(topic) == "esp32/output") {
    Serial.print("Changing output to ");
    if (messageTemp == "on") {
      Serial.println("on");
      digitalWrite(ledPin, HIGH);
    }
    else if (messageTemp == "off") {
      Serial.println("off");
      digitalWrite(ledPin, LOW);
    }
    }
  */
}
