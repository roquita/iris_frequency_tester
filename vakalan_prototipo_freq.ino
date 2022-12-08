#include "Arduino.h"
#include <WiFi.h>
#include "esp_log.h"
#include "esp_system.h"
#include "esp_event.h"
#include "mqtt_client.h"

//#define SECURE_MQTT // Comment this line if you are not using MQTT over SSL

#ifdef SECURE_MQTT
#include "esp_tls.h"

// Let's Encrypt CA certificate. Change with the one you need
static const unsigned char DSTroot_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDSjCCAjKgAwIBAgIQRK+wgNajJ7qJMDmGLvhAazANBgkqhkiG9w0BAQUFADA/
MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT
DkRTVCBSb290IENBIFgzMB4XDTAwMDkzMDIxMTIxOVoXDTIxMDkzMDE0MDExNVow
PzEkMCIGA1UEChMbRGlnaXRhbCBTaWduYXR1cmUgVHJ1c3QgQ28uMRcwFQYDVQQD
Ew5EU1QgUm9vdCBDQSBYMzCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB
AN+v6ZdQCINXtMxiZfaQguzH0yxrMMpb7NnDfcdAwRgUi+DoM3ZJKuM/IUmTrE4O
rz5Iy2Xu/NMhD2XSKtkyj4zl93ewEnu1lcCJo6m67XMuegwGMoOifooUMM0RoOEq
OLl5CjH9UL2AZd+3UWODyOKIYepLYYHsUmu5ouJLGiifSKOeDNoJjj4XLh7dIN9b
xiqKqy69cK3FCxolkHRyxXtqqzTWMIn/5WgTe1QLyNau7Fqckh49ZLOMxt+/yUFw
7BZy1SbsOFU5Q9D8/RhcQPGX69Wam40dutolucbY38EVAjqr2m7xPi71XAicPNaD
aeQQmxkqtilX4+U9m5/wAl0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNV
HQ8BAf8EBAMCAQYwHQYDVR0OBBYEFMSnsaR7LHH62+FLkHX/xBVghYkQMA0GCSqG
SIb3DQEBBQUAA4IBAQCjGiybFwBcqR7uKGY3Or+Dxz9LwwmglSBd49lZRNI+DT69
ikugdB/OEIKcdBodfpga3csTS7MgROSR6cz8faXbauX+5v3gTt23ADq1cEmv8uXr
AvHRAosZy5Q6XkjEGB5YGV8eAlrwDPGxrancWYaLbumR9YbK+rlmM6pZW87ipxZz
R8srzJmwN0jP41ZL9c8PDHIyh8bwRLtTcm1D9SZImlJnt1ir/md2cXjbDaJWFBM5
JDGFoqgCWjBH4d1QB7wCCZAA62RjYJsWvIjJEubSfZGL+T0yjWW06XyxV3bqxbYo
Ob8VZRzI9neWagqNdwvYkQsEjgfbKbYK7p2CNTUQ
-----END CERTIFICATE-----
)EOF";
#endif // SECURE_MQTT

esp_mqtt_client_config_t mqtt_cfg;
esp_mqtt_client_handle_t client;

const char* WIFI_SSID = "TE_HACKEL_EL_INTER";
const char* WIFI_PASSWD = "QWE&ASD%ZXC#!!";

const char* MQTT_HOST = "broker.vakalan.com";
#ifdef SECURE_MQTT
const uint32_t MQTT_PORT = 8084;
#else
const uint32_t MQTT_PORT = 1883;
#endif // SECURE_MQTT
const char* MQTT_USER = "";
const char* MQTT_PASSWD = "";

static esp_err_t mqtt_event_handler (esp_mqtt_event_handle_t event) {
  if (event->event_id == MQTT_EVENT_CONNECTED) {
    ESP_LOGI ("TEST", "MQTT msgid= %d event: %d. MQTT_EVENT_CONNECTED", event->msg_id, event->event_id);
    esp_mqtt_client_subscribe (client, "v1/iris/client1/deviceId/res", 0);  
  } 
  else if (event->event_id == MQTT_EVENT_DISCONNECTED) {
    ESP_LOGI ("TEST", "MQTT event: %d. MQTT_EVENT_DISCONNECTED", event->event_id);
    esp_mqtt_client_reconnect (event->client); //not needed if autoconnect is enabled
  } else  if (event->event_id == MQTT_EVENT_SUBSCRIBED) {
    ESP_LOGI ("TEST", "MQTT msgid= %d event: %d. MQTT_EVENT_SUBSCRIBED", event->msg_id, event->event_id);
  } else  if (event->event_id == MQTT_EVENT_UNSUBSCRIBED) {
    ESP_LOGI ("TEST", "MQTT msgid= %d event: %d. MQTT_EVENT_UNSUBSCRIBED", event->msg_id, event->event_id);
  } else  if (event->event_id == MQTT_EVENT_PUBLISHED) {
    ESP_LOGI ("TEST", "MQTT event: %d. MQTT_EVENT_PUBLISHED", event->event_id);
  } else  if (event->event_id == MQTT_EVENT_DATA) {
    ESP_LOGI ("TEST", "MQTT msgid= %d event: %d. MQTT_EVENT_DATA", event->msg_id, event->event_id);
    ESP_LOGI ("TEST", "Topic length %d. Data length %d", event->topic_len, event->data_len);
    ESP_LOGI ("TEST","Incoming data: %.*s %.*s\n", event->topic_len, event->topic, event->data_len, event->data);

  } else  if (event->event_id == MQTT_EVENT_BEFORE_CONNECT) {
    ESP_LOGI ("TEST", "MQTT event: %d. MQTT_EVENT_BEFORE_CONNECT", event->event_id);
  }
  return ESP_OK;
}

void setup () {
  mqtt_cfg.host = MQTT_HOST;
  mqtt_cfg.port = MQTT_PORT;
  mqtt_cfg.username = MQTT_USER;
  mqtt_cfg.password = MQTT_PASSWD;
  mqtt_cfg.keepalive = 15;
#ifdef SECURE_MQTT
  mqtt_cfg.transport = MQTT_TRANSPORT_OVER_SSL;
#else
  mqtt_cfg.transport = MQTT_TRANSPORT_OVER_TCP;
#endif // SECURE_MQTT
  mqtt_cfg.event_handle = mqtt_event_handler;

  mqtt_cfg.path = "mqtt";


  Serial.begin (115200);

  WiFi.mode (WIFI_MODE_STA);
  WiFi.begin (WIFI_SSID, WIFI_PASSWD);
  int counter = 0;
  while (!WiFi.isConnected () && (counter <= 50)) {
    Serial.print ('.');
    counter++;
    delay (100);
  }

  if( WiFi.status() != WL_CONNECTED){
    Serial.println ("");
    Serial.print ("Restarting ...");
    esp_restart();
  }
  
  Serial.println ();
  esp_err_t err;
#ifdef SECURE_MQTT
   err = esp_tls_set_global_ca_store (DSTroot_CA, sizeof (DSTroot_CA));
  ESP_LOGI ("TEST","CA store set. Error = %d %s", err, esp_err_to_name(err));
#endif // SECURE_MQTT
  client = esp_mqtt_client_init (&mqtt_cfg);
  //esp_mqtt_client_register_event (client, ESP_EVENT_ANY_ID, mqtt_event_handler, client); // not implemented in current Arduino core
  err = esp_mqtt_client_start (client);
  ESP_LOGI ("TEST", "Client connect. Error = %d %s", err, esp_err_to_name (err));
}

const char* data_to_publish = "{"
                            "\"gps\":{\"lat\":\"-12.09\",\"lng\":\"-77.05\"},"
                            "\"acc\":{\"x\":\"-1.59\",\"y\":\"0.08\",\"z\":\"10.22\"},"
                            "\"gyro\":{\"x\":\"-5.05\",\"y\":\"0.75\",\"z\":\"0.26\"}"
                              "}";
void loop () {  
  esp_mqtt_client_publish (client, "564963785", data_to_publish, strlen(data_to_publish), 0, false);
  delay (5000);
}
