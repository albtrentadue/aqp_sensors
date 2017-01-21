/*
 * Reads the value of the Dissolved Oxygen sensor
 * and returns it in the global variable
 * 
 * 
 */

void OXY_I2C() {

  byte code = 0;                   //used to hold the I2C response code.
  byte idx = 0;
  byte in_char = 0;
  
  // ----- EZO Dissolved Oxygen ----
  char computerdata = SNS_READ_COMMAND;
  //
  Wire.beginTransmission(DO_ADDRESS);                                      
  Wire.write(computerdata);                                             
  Wire.endTransmission();

  if (computerdata == SNS_CALIBRATE_COMMAND || computerdata == SNS_READ_COMMAND) delay(SNS_CAL_READ_TIME);
  else  delay(SNS_OTHER_TIME);

  Wire.requestFrom(DO_ADDRESS, 20, 1);  //call the circuit and request 20 bytes (this may be more than we need)
  code = Wire.read();            //the first byte is the response code, we read this separately.                                   

  switch (code) {                //switch case based on what the response code is.                                       
    case 1:
      Serial.println("DO Success");  //means the command was successful.
      break;
    case 2:
      Serial.println("DO Failed");   //means the command has failed.
      break;
    case 254:
      Serial.println("DO Pending");  //means the command has not yet been finished calculating.
      break;
    case 255:
      Serial.println("DO No Data");  //means there is no further data to send.
      break;
  }

  //reset the array
  for (idx = 0; idx < 20; idx++) {
    DO_data[idx] = 0;
  }

  while (Wire.available()) {     //are there bytes to receive.
    in_char = Wire.read();
    DO_data[idx] = in_char;
    idx += 1;
    if (in_char == 0) {          //if we see that we have been sent a null command.
      idx = 0;
      Wire.endTransmission();    //end the I2C data transmission.
      break;
    }
  }

  //If the first char is a number we know it is a DO reading, lets parse the DO reading
  if (isDigit(DO_data[0])) {
      // Global vars for DO and Sat will be loaded by this function
      parse_oxygen_values();
  }
  else { //if it’s not a number
    //print the data as debug
    Serial.print("DO data:");
    Serial.println(DO_data);
  } 
}


/** 
 Parses the returned EZO Dissolved Oxygen data and returns the measured values.
 This function will break up the CSV string into its 2 individual parts: 
 * - Dissolved Oxygen
 * - % of Saturation
 */
void parse_oxygen_values() {
  //used to indicate is a “,” was found in the string array                                                      
  boolean comma = false;
  byte idx = 0;
  char *DO;
  char *sat;

  //Step through each char until we see a ','
  for (idx = 0; idx < 20; idx++) {
    if (DO_data[idx] == ',') comma = true;
  }                                                                        

  //if we see the there WAS NOT a ‘,’ in the string array
  if (! comma) {                                                        
    Serial.print("DO:");
    Serial.println(DO_data);
  } else {
    //First part of the message
    DO = strtok(DO_data, ",");
    //Second part of the message
    sat = strtok(NULL, ",");                                              
    Serial.print("OXY:");
    Serial.println(DO);
    Serial.print("OXY Sat:");
    Serial.println(sat);
    //Put the values into floating point
    DO_float=atof(DO);
    //DO_sat_float=atof(sat);

    comma = false;
  }
}

