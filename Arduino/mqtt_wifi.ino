/*
  Git: DaveCalaway 16/10/2016 V0.2

  This sketch demonstrates the capabilities of the pubsub library ( MQTT ) in combination
  with the ESP8266 board/library, with DHT11 and Oled display(I2C).

  It connects to an MQTT server then:
  - publishes the temperature to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  Moreover draw on the Oled display Temperature & Humidity delivered from DHT11 sensor.

  It will reconnect to the server if the connection is lost using a blocking
  reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
  achieve the same result without blocking the outTopic loop.

  Esempio: https://goo.gl/qAmSwi
  Comandì libreria: https://goo.gl/cM5rK7
  MQTT Broker: cloudmqtt
    Chrome: MQTTLens
  Display: SSD1306
    Library: https://goo.gl/5KHEsx
*/
// Include the correct MQTT library
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
// Include the correct display library
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
// Include DHT11 library
#include <dht11.h>
dht11 DHT;
#define DHT11_PIN D2

//--------------------------------------------------
// own wifi
const char* ssid = "FreeLepida_Fiorano";
const char* password = "";
// MQTTcloud
const char* mqtt_server = "m20.cloudmqtt.com";
#define port 17394
// ACLs
const char* username = "ESP8266";
const char* passw = "corsini";
//--------------------------------------------------
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

// Include custom images if you want
//#include "images.h"

// Initialize the OLED display using Wire library
SSD1306  display(0x3c, D3, D5);

int temp = 0;
int hum = 0;

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, port);
  client.setCallback(callback);

  // Initialising the UI will init the display too.
  display.init();

  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);
}

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

  //if ((char)payload[0] == '1') {}

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    //boolean connect (clientID, username, password)
    if (client.connect("clientID", username, passw)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void drawText() {
  // clear the display
  display.clear();
  // create more fonts at http://oleddisplay.squix.ch/
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  DHT.read(DHT11_PIN);
  temp = DHT.temperature;
  hum = DHT.humidity;
  String disp_temp = String("Temperatura: ") + temp;
  String disp_hum = String("Umidità: ") + hum;
  //drawString(int16_t x, int16_t y, String text);
  display.drawString(0, 0, disp_temp);
  display.drawString(0, 20, disp_hum);
  display.display();
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000) {
    drawText();
    lastMsg = now;
    //++value;
    // (buf, sizeof(buf), format, ap);
    //snprintf (msg, 75, "hello world #%ld", value);
    snprintf (msg, 75, "Temp: %ld", temp);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("outTopic", msg);
  }
}
