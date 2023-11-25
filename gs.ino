#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include "DHT.h"
 
#define DHTPIN 4     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11
 
//---- WiFi settings
const char* ssid = "higor";
const char* password = "higor123";
 
//---- MQTT Broker settings
const char* mqtt_server = "951fb5b08cbe428ba6aeb35b89d6a4fd.s2.eu.hivemq.cloud"; // replace with your broker url
const char* mqtt_username = "usuario";
const char* mqtt_password = "Senha1234";
const int mqtt_port = 8883;
 
#define REPORTING_PERIOD_MS     1000
 
static const char *root_ca PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";
 
WiFiClientSecure espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
// Create a PulseOximeter object
PulseOximeter pox;
DHT dht(DHTPIN, DHTTYPE);
 
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
const char* sensor1_topic= "temperature";
const char* sensor2_topic= "humidity";
const char* sensor3_topic= "heartrate";
const char* sensor4_topic= "spo2";
 
 
// Time at which the last beat occurred
uint32_t tsLastReport = 0;
 
// Callback routine is executed when a pulse is detected
void onBeatDetected() {
    Serial.println("♥ Beat!");
}
 
 
void setup() {
    Serial.begin(115200);
 
    Serial.print("\nConnecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
     while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.println("Connecting to WiFi...");  
    } 
    randomSeed(micros());
    Serial.println("\nWiFi connected\nIP address: ");
    Serial.println(WiFi.localIP());
    while (!Serial) delay(1);
    espClient.setCACert(root_ca);
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
    Serial.println(F("DHTxx test!"));
    dht.begin();
 
    Serial.print("Initializing pulse oximeter..");
 
    // Initialize sensor
    if (!pox.begin()) {
        Serial.println("\nFAILED");
        for(;;);
    } else {
        Serial.println("\nSUCCESS");
    }
 
    // Configure sensor to use 7.6mA for LED drive
    pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
 
    // Register a callback routine
    pox.setOnBeatDetectedCallback(onBeatDetected);
    if (!client.connected()) reconnect();
}
 
void loop() {
    
    // Read from the sensor
    pox.update();

    // Grab the updated heart rate and SpO2 levels
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        delay(2000);
        tsLastReport = millis();
        pox.begin();
        float a = pox.getHeartRate();
        float b = pox.getSpO2();
        float h = dht.readHumidity();
        // Read temperature as Celsius (the default)
        float t = dht.readTemperature();

        publishMessage(sensor1_topic, String(t), true);
        publishMessage(sensor2_topic, String(h), true);
        publishMessage(sensor3_topic, String(a), true);
        publishMessage(sensor4_topic, String(b), true);
        
        Serial.print("Heart rate:");
        Serial.print(a);
        Serial.print("bpm / SpO2:");
        Serial.print(b);
        Serial.println("%");
        
        Serial.print(F("Humidity: "));
        Serial.print(h);
        Serial.print(F("%  Temperature: "));
        Serial.print(t);
        
    }
}
 
void reconnect() {
    // Loop until we’re reconnected
    Serial.print("Connected");
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection…");
        String clientId = "ESP8266Client-"; // Create a random client ID
        clientId += String(random(0xffff), HEX);
        // Attempt to connect
        if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("connected");
            //client.subscribe(command1_topic);   // subscribe the topics here
            //client.subscribe(command2_topic);   // subscribe the topics here
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");   // Wait 5 seconds before retrying
            delay(5000);
            }
    }
}
//=======================================
// This void is called every time we have a message from the broker
void callback(char* topic, byte* payload, unsigned int length) {
    String incommingMessage = "";
    for (int i = 0; i < length; i++) incommingMessage+=(char)payload[i];
    Serial.println("Message arrived ["+String(topic)+"]"+incommingMessage);
    // check for other commands
    /* else if( strcmp(topic,command2_topic) == 0){
    if (incommingMessage.equals(“1”)) { } // do something else
    }
    */
}
//======================================= publising as string
void publishMessage(const char* topic, String payload , boolean retained){
    if (client.publish(topic, payload.c_str(), true))
    Serial.println("Message publised\n");
}
