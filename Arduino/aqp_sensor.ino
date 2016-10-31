/**
 aqp_sensors / Sensor node
 by Casa Corsini Team Oct.2016 - V0.1 
 
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

  MQTT library: https://goo.gl/cM5rK7
  MQTT Broker: cloudmqtt
    Chrome: MQTTLens
  Display: SSD1306
    Library: https://goo.gl/5KHEsx
 */

// <--------- Include the correct MQTT library --------->
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
// <--------- Include the correct display library --------->
#define oled
#ifdef oled
//#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"` 
#endif
// < --------- Include DHT11 library --------->
#include <dht11.h>
dht11 DHT;  // nel mio era così
// dht11 dht1;
#define DHT11_PIN 4

//< -------------- The Debug --------------------------- >
//#define DEBUG    // uncomment to enable serial debug
#ifdef DEBUG
#define DEBUG_PRINT(x)     Serial.print (x)
#define DEBUG_PRINTLN(x)  Serial.println (x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#endif
//--------------------------------------------------
// Own wifi
const char* ssid = "FreeLepida_Fiorano";
const char* password = "";
// MQTTcloud
const char* mqtt_server = "m20.cloudmqtt.com";
#define port 17394
// ACLs
const char* username = "ESP8266";
const char* passw = "corsini";
//---------------- MQTT ----------------------------
WiFiClient espClient;
PubSubClient client(espClient);
char msg[50];
int value = 0;

int temp = 0;
int hum = 0;

#ifdef oled
// Include custom images if you want
// #include "images.h"
// Initialize the OLED display using Wire library
SSD1306  display(0x3c, D3, D5);
#endif

#define LED_BUILTIN 13
#define EXT_LED 11
#define PIN_DHT11 4
#define SWSERIAL_RX 2
#define SWSERIAL_TX 3 
#define VALIM_ANALOG A4

/*
 * Not used in this version
//EEPROM USED ADDRESSES
#define ADDRESS_MYID 1
#define ADDRESS_CICLI_SLEEP 2
*/

/*
 * Not used in this version
#define RESP_OK 0
#define NOT_MINE 1
#define UNHANDLED 2
*/

//other fixed values
#define MAIN_CYCLE_DELAY 950
#define CICLI_HEART 1

//char msg_buffer[MAX_RXMSG_LEN];

// The global variable with the message to MQTT
//String tx_message;

byte heart = 0;
byte ext_led_on = 0;
byte cnt_heart = CICLI_HEART;

/*
 * Not used in this version
 
String msg_sender = "";
String msg_dest = "";
String msg_type = ""; 
String msg_data = "";
 */

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  DEBUG_PRINTLN();
  DEBUG_PRINT("Connecting to ");
  DEBUG_PRINTLN(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    DEBUG_PRINT(".");
  }

  DEBUG_PRINTLN("");
  DEBUG_PRINTLN("WiFi connected");
  DEBUG_PRINTLN("IP address: ");
  DEBUG_PRINTLN(WiFi.localIP());
}

  /* Not used in this version */
//void callback(char* topic, byte* payload, unsigned int length) {
//  DEBUG_PRINT("Message arrived [");
//  DEBUG_PRINT(topic);
//  DEBUG_PRINT("] ");
//  for (int i = 0; i < length; i++) {
//    DEBUG_PRINT((char)payload[i]);
//  }
//  DEBUG_PRINTLN();
//
//  //if ((char)payload[0] == '1') {}
//
//}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    DEBUG_PRINT("Attempting MQTT connection...");
    // Attempt to connect
    //boolean connect (clientID, username, password)
    if (client.connect("clientID", username, passw)) {
      DEBUG_PRINTLN("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      // NOT USED IN THIS VERSION
      // client.subscribe("inTopic");
    } else {
      DEBUG_PRINT("failed, rc=");
      DEBUG_PRINT(client.state());
      DEBUG_PRINTLN(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/* NODE SETUP */
void setup()
{ 
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
  #endif
  setup_wifi();
  client.setServer(mqtt_server, port);
  // NOT USED IN THIS VERSION
  // client.setCallback(callback);
  
  #ifdef oled
  // Initialising the UI will init the display too.
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);
  #endif
  pinMode(LED_BUILTIN, OUTPUT); //Heartbeat led pin 13
  pinMode(EXT_LED, OUTPUT);  //Auxiliary led pin 11

  //....

  /* Not used in this version
     In future version we can implement a startup configuration function
     pushing the configuration prameters to Arduino that are stored in
     the EEPROM
  
  int nr = receive_msg();
  */
  
}

/* OLED DISPLAY */
void drawText() {
  // clear the display
  display.clear();
  // create more fonts at http://oleddisplay.squix.ch/
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  String disp_temp = String("Temperatura: ") + temp;
  String disp_hum = String("Umidità: ") + hum;
  //drawString(int16_t x, int16_t y, String text);
  display.drawString(0, 0, disp_temp);
  display.drawString(0, 20, disp_hum);
  display.display();
}

/* ---------- MAIN LOOP ---------- */
void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  make_message(collect_measures());
  send_message();
  after_message();
  
  heartbeat();
  delay(MAIN_CYCLE_DELAY);
}

/**
 Builds the string 'tx_message' with the message to be
 transmitted via network
*/
void make_message (String resp_data) 
{  
    // Converts the contents of a string as a C-style, null-terminated string.
    snprintf (msg, 75, "Temperatura ed umidità:\n%s\n", resp_data.c_str());
  //tx_message = ...
}

/**
 Load the measures as they are configured in the board.
 
 In the PoC version, only temperature and humidity are read from one or two DHT22 sensors
 and returned as digital sources #4, #5 (and #6, #7 if two sensors are used).
 */
String collect_measures() 
{
  String measures = "";

  //Add here the code that read the sensor and returns the
  //Measures in a string, formatted in some useful way...
  // ----- DHT11  -----
  DHT.read(DHT11_PIN);
  measures = String(DHT.temperature)+"-"+String(DHT.humidity);
  return measures; 
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
 Send a string message on the RF interface
 */
void send_message() 
{
  ext_led_on = 1;

  //Add here the code that sends the message via network
  //In this case will be via MQTT/WiFi
    DEBUG_PRINT("Publish message: ");
    DEBUG_PRINTLN(msg);
    client.publish("outTopic", msg);
}

/**
 Heartbeat on the built_in Led + EXT_LED
 */
void heartbeat() 
{
  cnt_heart--;
  if (cnt_heart == 0) {
      heart++;
      digitalWrite(LED_BUILTIN, bitRead(heart,0));
      digitalWrite(EXT_LED, bitRead(ext_led_on,0));
      if (ext_led_on) ext_led_on = 0;
      
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
 
