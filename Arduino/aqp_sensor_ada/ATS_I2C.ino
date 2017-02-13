/*
 * The general Atlas Scientific sensor command executing function
 * 
 * 4.7k pull up resistor on SDA and SCL must be required.
 * 
 * Commands:
 *    - Cal = calibration
 *    - Factory
 *    - I = info
 *    - L = led status
 *    - Plock 
 *    - Serial
 *    - R = read
 *    - Sleep
 *    - Status
 *    
 *    address is the coded I2C address of the sensor
 *    OxideReduxPotential sensor : 98
 *    Dissolved Oxygen sensor: 97
 */
void ATS_I2C(int address, char *command, int wait_time) {

  byte code = 0;                     //used to hold the I2C response code.
  byte in_char = 0;                  //used as a 1 byte buffer to store in bound bytes from the ORP Circuit.
  byte i = 0;                        //counter used for ORP_data array.
  
  // <----- Command ---->
  Serial.print("Command:");Serial.println(*command);      
  Serial.print("I2C address:");Serial.println(address);      

  //reset the array
  for (i = 0; i < I2C_DATA_LENGTH; i++) {
    ATS_data[i] = 0;
  }
  
  Wire.beginTransmission(address);//call the circuit by its ID number.
  Wire.write(command);            //transmit the command that was sent through the serial port.
  Wire.endTransmission();         //end the I2C data transmission.

  delay(wait_time);
  
  Wire.requestFrom(address, I2C_DATA_LENGTH, 1); //call the circuit and request 20 bytes (this may be more than we need)
  code = Wire.read();                   //the first byte is the response code, we read this separately.

  switch (code) {                       //switch case based on what the response code is.
    case 1:                             //decimal 1.
      Serial.println("ATS:Success");    //means the command was successful.
      break;                            //exits the switch case.
      
    case 2:                             //decimal 2.
      Serial.println("ATS:Failed");     //means the command has failed.
      break;                            //exits the switch case.

    case 254:                           //decimal 254.
      Serial.println("ATS:Pending");    //means the command has not yet been finished calculating.
      break;                            //exits the switch case.

    case 255:                           //decimal 255.
      Serial.println("ATS:No Data");    //means there is no further data to send.
      break;                            //exits the switch case.
  }

  i = 0;
  while (Wire.available()) {         //are there bytes to receive.
    in_char = Wire.read();           //receive a byte.
    ATS_data[i] = in_char;           //load this byte into our array.
    i += 1;                          //incur the counter for the array element.
    if (in_char == 0) {              //if we see that we have been sent a null command.
      i = 0;                         //reset the counter i to 0.
      Wire.endTransmission();        //end the I2C data transmission.
      break;                         //exit the while loop.
    }
  }

  Serial.print("ATS:I2C data:");
  Serial.println(ATS_data);          //print the data.
}

/*
 * Reads a measurements from the Atlas Scientific sensor at given address
 * 
 * Returned float is stored in the ATS_float global variable
 * Returns a boolean indicating if the data is valid or not
 */
bool ATS_read(int address) {

  ATS_data_valid = false;
  char the_command[2] = {'R',0};  
  ATS_I2C(address, the_command, SNS_CAL_READ_TIME);

  //If the first char is a number we know it is a DO reading, lets parse the DO reading
  if (isDigit(ATS_data[0]) || ((ATS_data[0] == '-') && (isDigit(ATS_data[1])))) {
    // Global vars for DO and Sat will be loaded by this function
    parse_ATS_values();
    ATS_data_valid = true;
  } else {
    ATS_float=0.0; //In this case ATS_data_valid stays FALSE
  }
}

/** 
 Parses the returned EZO Dissolved Oxygen data and returns the measured values.
 This function will break up the CSV string into its 2 individual parts: 
 * - Dissolved Oxygen
 * - % of Saturation
 */
void parse_ATS_values() {
  
  boolean comma = false; //indicates if a “,” was found in the string array                                                      
  byte idx = 0;
  char *N1;
  char *N2;

  //Step through each char until we see a ','
  for (idx = 0; idx < I2C_DATA_LENGTH; idx++) {
    if (ATS_data[idx] == ',') comma = true;
  }                                                                        

  //if we see the there is a ‘,’ in the string array
  if (!comma) {
    ATS_float = atof(ATS_data); 
    //Serial.print("ATS_float: ");Serial.println(ATS_float);   
  } else {                                                        
    //First part of the message
    N1 = strtok(ATS_data, ",");
    //Second part of the message
    N2 = strtok(NULL, ",");                                              
    //Serial.print("ATS:N1:");Serial.println(N1);
    //Serial.print("ATS:N2:");Serial.println(N2);
    //Put the values into floating point
    ATS_float=atof(N1);
    //DO_sat_float=atof(N2);
  }  
}

/*
 * Enable or disables % saturation reading from the DO sensor
 * parameter "enable": '1': enables, '0':disables
 */
void enable_DO_sat(int address, char enable) {

  char the_command[6] = {'O',',','%',',','0',0};
  the_command[4] = enable;
  ATS_I2C(address, the_command, SNS_OTHER_TIME);  
  
}

/*
 * Queries the status
 */
void ATS_query_status(int address) {

  char the_command[7] = {'S','T','A','T','U','S',0};
  ATS_I2C(address, the_command, SNS_OTHER_TIME);  
  
}

