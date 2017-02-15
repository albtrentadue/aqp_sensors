/*
  aqp_sensors_ada / Sensor node with Adafruit MQTT
  by Casa Corsini Team Dec.2016

  Sensor node control for the aqp_sensors.

  This file is part of aqp_sensors
  aqp_sensors is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  aqp_sensors is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with ClEnSensors.  If not, see <http://www.gnu.org/licenses/>.
*/

//TODO: Not sure these defines are working.
//< --- Serial debug setup --- >
#define DEBUG   // uncomment to enable serial debug
#ifdef DEBUG
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#endif
//-------------------------------

// <--- ESP8266 + WiFi libraries --->
#include <ESP8266WiFi.h>

// <--- Include the correct MQTT Adafruit library --->
/*  ----> MQTT Adafruit library: https://goo.gl/4ewcc2
            Adafruit tutorial: https://goo.gl/BVXdso
    ----> MQTT Broker: io.adafruit.com
            Chrome: MQTTLens
*/
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// <--- Include the correct display library --->
/*  ----> Display: SSD1306
            Library: https://goo.gl/5KHEsx
            we are using Upgrade from 2.0 to 3.0, https://goo.gl/3LljMv
*/
#define oled
#ifdef oled
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
#endif

// < --- Include DHT11 library Adafruit --->
// you must instal Adafruit Unified Sensor
#include "DHT.h"

// < --- Include OneWire library --->
#include <OneWire.h>  // https://goo.gl/Ygk5Id
#include <DallasTemperature.h> // DS18B20  https://goo.gl/xmlB9d

// < --- Include I2C library --->
// https://goo.gl/xAgrmO
#include <Wire.h>

// Node MCU 0.9 Pin mapping: https://goo.gl/bZ11wH
#define LED_ONCARD D0         // The blue led on the Node MCU
//#define EXT_LED 11
// < ------ DHT22 ------ >
#define DHTPIN D1           // real D1 = 5
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
#define ONE_WIRE_BUS D2        // real D2 = 4
//#define SWSERIAL_RX 2
//#define SWSERIAL_TX 3

// cycle counters
#define MAIN_CYCLE_DELAY 1000  // The loop main cycle delay in millis
#define CICLI_HEART 1          // Sets the loop() times per led toggle
#define MEAS_INTERVAL 6000     // The time interval to send the measurs in milliseconds
#define AVG_VALUES 1           // The number of values to be averaged per sample
// NOTE!! AVG_VALUES MUST BE LESS than the number of times
// the duration of one loop(), in milliseconds, fits into MEAS_INTERVAL

//EZO Dissolved Oxiygen Sensor from Atlas Scientific
//https://www.atlas-scientific.com/dissolved-oxygen.html
#define DO_ADDRESS 97
#define DO_CORR_OFFSET -0.5
// < --------------- ORP I2C --------------- >
//https://www.atlas-scientific.com/orp.html
#define ORP_ADDRESS 98
#define ORP_CORR_FACTOR -0.01797
#define ORP_CORR_OFFSET 7.050
// < --------------------------------------- >
#define I2C_DATA_LENGTH 20
#define SNS_CAL_READ_TIME 1800
#define SNS_OTHER_TIME 300

/*
  Not used in this version
  //EEPROM USED ADDRESSES
  #define ADDRESS_MYID 1
  #define ADDRESS_CICLI_SLEEP 2
*/
/*
  Not used in this version
  #define RESP_OK 0
  #define NOT_MINE 1
  #define UNHANDLED 2
*/

// TODO: Configurable wifi settings
// FreeLepida_Fiorano
#define WLAN_SSID       "FreeLepida_Fiorano"
#define WLAN_PASS       ""

// ---- MQTT connection ---------
// TODO: Configurable MQTT settings
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883  // use 8883 for SSL
#define AIO_USERNAME    "MarkCalaway"
#define AIO_KEY         "8533e8dbf95646858144232621bb963d"

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Setup a feed called 'dht11' for publishing.
Adafruit_MQTT_Publish dht_temperature = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/dht_temperature");

Adafruit_MQTT_Publish dht_humidity = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/dht_humidity");

Adafruit_MQTT_Publish diss_oxygen = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Ossigeno_dish");

Adafruit_MQTT_Publish or_potential = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Potenziale_OR");

Adafruit_MQTT_Publish PH = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/PH");

Adafruit_MQTT_Publish Conducibilita = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Conducibilita");

Adafruit_MQTT_Publish w_temperature = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/w_temperature");

// Setup a feed called 'onoff' for subscribing to changes.
// Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/onoff");

/************************************************************/

DHT dht(DHTPIN, DHTTYPE);

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

/*
   The following global vars accumulate the values taken
   from sensors over the program cycles. The values are taken
   from the sensors using different functions in the sketch.
   At the last cycle, the values are divided by the number
   of measurement to send the average.
*/
float DHT_temp = 0;
float DHT_hum = 0;
int W_TEMP = 0;
char ATS_data[I2C_DATA_LENGTH];  //20 byte character array to hold incoming data from the I2C sensor circuit.
bool ATS_data_valid;             //This flag indicates that the value returned by the ATS sensor is valid
float ATS_float;                 //The global variable used to return the value measured by the ATS sensor
float DO_value = 0.0;
float ORP_value = 0.0;

/*
  Not used in this version
  byte ext_led_on = 0;
*/
byte heart = 0;               //holds the bit used to blink the ON BOARD led
byte cnt_heart = CICLI_HEART; //loop() cycles for every led toggle

unsigned long prev_time = 0;  //keeps track of the time within a sampling interval
byte cnt_values = 0;          //counts the values averaged so far

#ifdef oled
// Include custom images if you want
// #include "images.h"
// Initialize the OLED display using Wire library
// D3=GPIO0 - D5=GPIO14
SSD1306 display(0x3c, D3, D5);
#endif

/*
   Not used in this version
  void callback(char* topic, byte* payload, unsigned int length) {
  DEBUG_PRINT("Message arrived [");
  DEBUG_PRINT(topic);
  DEBUG_PRINT("] ");
  for (int i = 0; i < length; i++) {
    DEBUG_PRINT((char)payload[i]);
  }
  DEBUG_PRINTLN();

  //if ((char)payload[0] == '1') {}

  }
*/

/* NODE SETUP */
void setup() {

  Serial.begin(115200);
  delay(10);

  // i2c
  Wire.begin(D3, D5); //enable I2C port with pins (sda,scl)

  // 1Wire
  sensors.begin(); // Pass our oneWire reference to Dallas Temperature.

  // DHTXX
  dht.begin();

  Serial.println(F("Adafruit MQTT Hydroponics"));

  // Connect to WiFi access point.
  Serial.println();
  Serial.print("Connecting to ");
  Serial.print(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) serial_dot();
  Serial.println();

  Serial.println("WiFi connected");
  Serial.print("Local IP address: ");
  Serial.println(WiFi.localIP());

  /*
     Setup MQTT subscription for command feed.
  */
  // mqtt.subscribe(&onoffbutton);
  /*
     In future version we can implement a startup configuration function
     pushing the configuration prameters to Arduino that are stored in
     the EEPROM
  */

#ifdef oled
  // Initialising the UI will init the display too.
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);
#endif

  // This block have a glitch
  pinMode(LED_ONCARD, OUTPUT); //Heartbeat led
  //pinMode(EXT_LED, OUTPUT);  //Auxiliary led

  //FOR NOW: disables % of saturation reading from the ATS DO sensor
  enable_DO_sat(DO_ADDRESS, '0');

  //Aligns to the start of the firts average cycle
  //Serial.println("Waiting the next time interval start...");
  //while (! check_time2send()) serial_dot();
  //Serial.println();
}

/* ---------- MAIN LOOP ---------- */
void loop() {

  MQTT_connect();

  /*
    This is our 'wait for incoming subscription packets' busy subloop
    try to spend your time here

    Adafruit_MQTT_Subscribe *subscription;
    while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &onoffbutton) {
      Serial.print(F("Got: "));
      Serial.println((char *)onoffbutton.lastread);
    }
    }
  */

  if (cnt_values < AVG_VALUES)  //Measures are collected only AVG_VALUES times
    collect_measures();
  else if (check_time2send() ) {
    send_message();
    after_message();
  }

  heartbeat();
  delay(MAIN_CYCLE_DELAY);
}

/*
  Not used in this version
  void callback(char* topic, byte* payload, unsigned int length) {
  DEBUG_PRINT("Message arrived [");
  DEBUG_PRINT(topic);
  DEBUG_PRINT("] ");
  for (int i = 0; i < length; i++) {
    DEBUG_PRINT((char)payload[i]);
  }
  DEBUG_PRINTLN();

  //if ((char)payload[0] == '1') {}
  }
*/

/**
  Load the measures as they are configured in the board.

  In the this version, the following measurements are made
  - DHT11 temperature and humidity
  - Atlas Scientific EZO Dissolved Oxygen values (ID=98)
  - Atlas Scientific EZO OxideRedux Potential values (ID=97)

*/
void collect_measures() {

  //Place here the functions that read the sensors and returns the
  //measures in dedicated global strings
  Serial.println("Collecting measures");

  // < ----- DHT22  ----- >
  // 2 seconds, its a very slow sensor
  float dht_hum_data = dht.readHumidity();
  DHT_hum += dht_hum_data;
  // Read temperature as Celsius (the default)
  float dht_temp_data = dht.readTemperature();
  DHT_temp += dht_temp_data;

  Serial.print("DHT temp:");
  Serial.println(dht_temp_data);
  Serial.print("DHT hum:");
  Serial.println(dht_hum_data);

  // < ----- DS18B20 with Dallas Temperature Control Library ----- >
  sensors.requestTemperatures(); // Send the command to get temperatures
  int w_temp_data = sensors.getTempCByIndex(0); // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire
  W_TEMP += w_temp_data;

  Serial.print("Water temp:");
  Serial.println(w_temp_data);

  // < ---------- ORP I2C ---------- >
  float corr_ats_value;
  // The ATS_float global var will hold the measure
  ATS_read(ORP_ADDRESS);
  corr_ats_value = ATS_float * ORP_CORR_FACTOR + ORP_CORR_OFFSET;
  Serial.print("pH value:");
  Serial.println(corr_ats_value); 
  if (ATS_data_valid) ORP_value += corr_ats_value;
  //ATS_query_status(ORP_ADDRESS);
  // < ---------- OXY I2C ---------- >
  // The ATS_float global var will hold the measure
  ATS_read(DO_ADDRESS);
  corr_ats_value = ATS_float + DO_CORR_OFFSET;
  Serial.print("Oxy value:");
  Serial.println(corr_ats_value); 
  if (ATS_data_valid) DO_value += corr_ats_value;
  //ATS_query_status(DO_ADDRESS);

  //Increase the values count
  cnt_values++;

}

/**
   Returns true if the current time is longer than the specified
   measurement time interval
*/
bool check_time2send() {
  unsigned long curr_time = millis() % MEAS_INTERVAL;
  bool is_time = (curr_time < prev_time);//This is true only on the modulo overflow of millis()
  //so this happens approx every MEAS_INTERVAL milliseconds.
  prev_time = curr_time;
  return is_time;
}

/**
  Send a string message on the intended communication channel
  Values are averaged on the fly
*/
void send_message() {
  //ext_led_on = 1;

  //Add here the code that sends the message via network
  //In this case will be via MQTT/WiFi
  or_potential.publish(ORP_value / AVG_VALUES);
  diss_oxygen.publish(DO_value / AVG_VALUES);
  dht_temperature.publish(DHT_temp / AVG_VALUES);
  dht_humidity.publish(DHT_hum / AVG_VALUES);
  w_temperature.publish(W_TEMP / AVG_VALUES);
}

/**
  Here any action that has to be done after sending the response
  can be specified.
  May even be empty.
*/
void after_message() {
  
  //to be used - if needed.
#ifdef oled
  drawText();
#endif

  //Resets all the measurement global vars and the avg counter
  ORP_value = 0.0;
  DO_value = 0.0;
  DHT_temp = 0;
  DHT_hum = 0;
  W_TEMP = 0;

  cnt_values = 0;
}

/**
  Heartbeat on the built_in Led + EXT_LED
*/
void heartbeat()
{
  Serial.println("heartbeat...");
  cnt_heart--;
  if (cnt_heart == 0) {
    heart++;
    digitalWrite(LED_ONCARD, bitRead(heart, 0));
    //digitalWrite(EXT_LED, bitRead(ext_led_on, 0));
    //if (ext_led_on) ext_led_on = 0;

    cnt_heart = CICLI_HEART;
  }
}

/**
   Writes dots every 1/2 sec on serial
*/
void serial_dot() {
  delay(1000);
  Serial.print(".");
}


