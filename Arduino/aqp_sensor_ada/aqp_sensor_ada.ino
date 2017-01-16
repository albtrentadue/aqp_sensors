/*
  aqp_sensors_ada_ORT_i2c / Sensor node with Adafruit MQTT
  by Casa Corsini Team Dec.2016

  Sensor node control for the aqp_sensors.

  This file is part of aqp_sensors
  aqp_sensors is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  ClEnSensors is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with ClEnSensors.  If not, see <http://www.gnu.org/licenses/>.
*/

//< --- Serial debug setup --- >
#define DEBUG   // uncomment to enable serial debug
#ifdef DEBUG
#define DEBUG_PRINT(x) Serial.print (x)
#define DEBUG_PRINTLN(x) Serial.println (x)
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
//#define oled
#ifdef oled
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
#endif

// < --- Include DHT11 library --->
#include <dht11.h>

// < --- Include I2C library --->
// https://goo.gl/xAgrmO
#include <Wire.h>


// Node MCU 0.9 Pin mapping: https://goo.gl/bZ11wH
#define LED_ONCARD D0
//#define EXT_LED 11
#define PIN_DHT11 4  // real D2
//#define SWSERIAL_RX 2
//#define SWSERIAL_TX 3
//
// cycle counters 
#define MAIN_CYCLE_DELAY 10
#define CICLI_HEART 1

//EZO Dissolved Oxiygen Sensor from Atlas Scientific 
//https://www.atlas-scientific.com/dissolved-oxygen.html
#define DO_ADDRESS 97
// < --------------- ORT I2C --------------- >
#define ORP_ADDRESS 98               //default I2C ID number for EZO ORP Circuit.
// < --------------------------------------- >
#define SNS_READ_COMMAND 'R'
#define SNS_CALIBRATE_COMMAND 'C'
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

// Setup a feed called 'onoff' for subscribing to changes.
// Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/onoff");

/************************************************************/

dht11 DHT;

//The array holding the Dissolved Oxygen sensor data
char DO_data[20]; 

//char msg[50];
// The global variable with the message to MQTT
//String tx_message = "";

// These global vars are used to communicate
// values from the functions to the send_message() function.
int DHT_temp = 0;
int DHT_hum = 0;
float DO_float;
float DO_sat_float;
float ORP_float;

/* 
   Not used in this session 
byte ext_led_on = 0;
 */
byte heart = 0;
byte cnt_heart = CICLI_HEART;

#ifdef oled
// Include custom images if you want
// #include "images.h"
// Initialize the OLED display using Wire library
// D3=GPIO0 - D5=GPIO14
SSD1306 display(0x3c, D3, D5);
#endif

/*
  Not used in this version

  String msg_sender = "";
  String msg_dest = "";
  String msg_type = "";
  String msg_data = "";
*/

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
  /* Not used in this version

    //NOTE! The ID of this card must be PRE-programmed by writing
    //in the ADDRESS_MYID EEPROM location
    node_ID = EEPROM.read(ADDRESS_MYID);
    myID = format_byte(node_ID, 3);
  */
  /* Not used in this version
    //NOTE! The default value of cicli_sleep (228,0) must be PRE-programmed by
    //writing in the ADDRESS_CICLI_SLEEP EEPROM location
    byte bl = EEPROM.read(ADDRESS_CICLI_SLEEP);
    byte bh = EEPROM.read(ADDRESS_CICLI_SLEEP+1);//2 bytes are read!
    cicli_sleep = bl + 0xFF * bh;
  */
#ifdef DEBUG
  Serial.begin(115200);
  delay(10);
#endif

  Wire.begin(D3,D5);                 //enable I2C port with pins (sda,scl)

  Serial.println(F("Adafruit MQTT Hydroponics"));

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.print("IP address: "); Serial.println(WiFi.localIP());

  // Setup MQTT subscription for onoff feed.
  // mqtt.subscribe(&onoffbutton);

#ifdef oled
  // Initialising the UI will init the display too.
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);
#endif

  // This block have a glitch
  //pinMode(LED_BUILTIN, OUTPUT); //Heartbeat led pin 13
  //pinMode(EXT_LED, OUTPUT);  //Auxiliary led pin 11
  //....

  /* Not used in this version
     In future version we can implement a startup configuration function
     pushing the configuration prameters to Arduino that are stored in
     the EEPROM

    int nr = receive_msg();
  */
}

/* ---------- MAIN LOOP ---------- */
void loop() {
  //Serial.println("lol");
  MQTT_connect();

  /* 
   this is our 'wait for incoming subscription packets' busy subloop
   try to spend your time here

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &onoffbutton) {
      Serial.print(F("Got: "));
      Serial.println((char *)onoffbutton.lastread);
    } 
  }
  */

  collect_measures();
  make_message();
  send_message();
  after_message();

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
  - Atlas Scientifi EZO Dissolved Oxygen values 
  
*/
void collect_measures(){
  String measures = "";

  //Add here the code that read the sensors and returns the
  //measures in dedicated global strings
  
  // ----- DHT11  -----
  DHT.read(PIN_DHT11);

  // < ---------- ORP I2C ---------- >
  // The ORP_float global var will hold the measure
  ORP_I2C(); 
  // < ---------- OXY I2C ---------- >
  // The DO_float global var will hold the measure
  OXY_I2C();
  DHT_temp = DHT.temperature;
  DHT_hum = DHT.humidity;
}
// < ---------- collect_measures end ---------- >

/**
  Builds the string '' with the message to be
  transmitted via a centralized communication channel.
*/
void make_message ()
{
  // To be used in case of communication over a single channel 
}


/**
  Send a string message on the RF interface
*/
void send_message() {
  //ext_led_on = 1;

  //Add here the code that sends the message via network
  //In this case will be via MQTT/WiFi
  or_potential.publish(ORP_float);
  diss_oxygen.publish(DO_float);
  dht_temperature.publish(DHT_temp);
  dht_humidity.publish(DHT_hum);
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
}

/**
  Heartbeat on the built_in Led + EXT_LED
*/
void heartbeat()
{
  cnt_heart--;
  if (cnt_heart == 0) {
    heart++;
    digitalWrite(LED_ONCARD, bitRead(heart, 0));
    //digitalWrite(EXT_LED, bitRead(ext_led_on, 0));
    //if (ext_led_on) ext_led_on = 0;

    cnt_heart = CICLI_HEART;
  }
}

//*** Below code is for possible future use - please keep it. ***//

/**
  Formats a byte into a fixed length string padded with zeros

  Not used in this version

  String format_byte(byte b, byte len)
  {
  String r = String(b);
  while (r.length() < len) r = '0' + r;
  return r;
  }
*/

/**
  Retrieves a message string from the bus.
  Returns the received string into message buffer,
  or "?" in case of any problem.

  Not used in this version.
  May be adapted to receive commands via MQTT
  from a remote controller

  int receive_msg()
  {
  int num_read = 0;
  //Init the message buffer to empty
  msg_buffer[0] = MSG_TERM;

  if (!xbee_sleeping) {
    // see if there's incoming serial data:
    if (XBee.available() > 0) {
      //Search the message starter
      char incomingByte = XBee.read();
      //Skip any pre-existing byte from the serial which is not a 127
      if (incomingByte == MSG_TERM) {
        //Build up the message string
        num_read = XBee.readBytesUntil(MSG_TERM, msg_buffer, MAX_RXMSG_LEN);
        //If MAX_RXMSG_LEN bytes were read, means that # was not received!
        if (num_read == MAX_RXMSG_LEN) msg_buffer[0] = '?';
        else
          //readd the terminator stripped by readBytesUntil()
          msg_buffer[num_read] = MSG_TERM;
      }
    }
  }

  return num_read;
  }

*/

/**
  Serve the command requested by the incoming message
  and load response with the reply
  Returns 0 if OK, or non zero if any error

  Not used in this version.
  May be adapted to serve the commands received from
  a remote controller

  int serve()
  {
  //Message is parsed
  parse_message();
  if ((msg_dest != BROADCAST) && (msg_dest != myID)) return NOT_MINE;
  //DEBUG_PRINT("MSGTYPE:"+msg_type);
  //Handles the IDNREQ Command
  if (msg_type.equals("IDNREQ")) {
    //Waits a time in seconds equal to myID - to avoid collision
    delay(2000L * node_ID);
    make_response(msg_sender, "IDRESP", "");
    return RESP_OK;
  }

  if (msg_type.equals("CONFIG")) {
    //Interprets and applies the configuration setting
    apply_config(msg_data);
    make_response(msg_sender, "CFGACK", "");
    return RESP_OK;
  }

  if (msg_type.equals("QRYMSR")) {
    //Read values from the available sensors

    return RESP_OK;
  }

  //Or signal an Unhandled command
  return UNHANDLED;

  }
*/

/**
  Parses the command message stored in the message buffer.
  The command messages from the controller have the following format:
  "<MITT><DEST><TIPO_MESSAGGIO>#"
  Note: the initial '#' in the message has been stripped...
  Parsed values are stored into global strings:
  msg_type, msg_sender, msg_dest, msg_data until the next command arrives.

  Not used in this version.
  May be adapted to parse the commands received from
  a remote controller

  void parse_message()
  {
  String buf = String(msg_buffer);
  msg_sender = buf.substring(0, 3);
  msg_dest = buf.substring(3, 6);
  msg_type = buf.substring(6,12);
  msg_data = buf.substring(12);
  }
*/

/**
  Parses the configuration string and applies the setting passed

  Not used in this version

  void apply_config(String cfg)
  {
  int idx=0;
  while (idx < cfg.length()-1) {
    if (cfg.charAt(idx) == MSG_TERM) break;
    if (cfg.charAt(idx) == DATA_SEP) idx++;
    idx += apply_setting(cfg.substring(idx));
  }
  }
*/

/**
  Scans a string, extract the first configuration setting and applies it
  Returns the amount of characters scanned in the given string.
  Each setting always starts with a 2-char setting ID and then a value up to
  the separator ':' or terminator '#'

  Not used in this version

  int apply_setting(String s) {
  if (s.length() > 2) {
    String cfg_setting = s.substring(0,2);
    int cnt=2;
    while (s.charAt(cnt) != DATA_SEP && s.charAt(cnt) != MSG_TERM && cnt < s.length()-1) cnt++;
    String cfg_value = s.substring(2,cnt);

    //Start the config cases
    if (cfg_setting.equals("TG")) {
      //****Time Granularity setting:
      //Dimension the sleep duration
      int granularity = cfg_value.toInt();
      if (granularity) {
        //Sleep time = granularity minus 3sec
        long gm = granularity*60000L-3000L;
        unsigned int new_cicli_sleep = gm/MAIN_CYCLE_DELAY;
        if (new_cicli_sleep != cicli_sleep) {
          cicli_sleep = new_cicli_sleep;
          //Stores the last configuration in EEPROM
          byte b = byte(cicli_sleep % 0xFF);
          EEPROM.write(ADDRESS_CICLI_SLEEP, b);
          b = byte(cicli_sleep / 0xFF);
          EEPROM.write(ADDRESS_CICLI_SLEEP+1, b);
        }
      }
    }
    return cnt;
  }
  //If the string is incomplete, just skip it
  else return s.length();

  }
*/

/**
  Sends an ERROR indication

  Not used in this version

  void send_error(byte err_code)
  {
  make_response(msg_sender, "SERROR", String(err_code));
  send_message();
  }
*/

