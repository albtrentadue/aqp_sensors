/*
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
 *    OxideReduxPower sensor : 98
 *    Dissolved Oxygen sensor: 97
 */
void ATS_I2C(int address) {

  //char command[I2C_DATA_LENGTH]    //command received from external controller
  //byte received_from_computer = 0; //we need to know how many characters have been received.
  byte code = 0;                     //used to hold the I2C response code.
  byte in_char = 0;                  //used as a 1 byte buffer to store in bound bytes from the ORP Circuit.
  byte i = 0;                        //counter used for ORP_data array.
  
  //if (Serial.available() > 0) {                                           //if data is holding in the serial buffer
    //received_from_computer = Serial.readBytesUntil(13, command, I2C_DATA_LENGTH); //we read the data sent from the serial monitor(pc/mac/other) until we see a <CR>. We also count how many characters have been received.
    //command[received_from_computer] = 0;                             //stop the buffer from transmitting leftovers or garbage.
    //command[0] = tolower(command[0]);                           //we make sure the first char in the string is lower case.
    //if (command[0] == 'c' || command[0] == 'r')time_ = 1800;    //if a command has been sent to calibrate or take a reading we wait 1800ms so that the circuit has time to take the reading.
    //else time_ = 300;                                                     //if any other command has been sent we wait only 300ms.

    // <----- Command ---->
    ATS_data_valid = false;
    char command = SNS_READ_COMMAND;     // hardcoded in the current version
    Serial.print("Reading sensor with address:");Serial.println(address);      
    
    Wire.beginTransmission(address); //call the circuit by its ID number.
    Wire.write(command);            //transmit the command that was sent through the serial port.
    Wire.endTransmission();              //end the I2C data transmission.

    //if (strcmp(command, "sleep") != 0) {  //if the command that has been sent is NOT the sleep command, wait the correct amount of time and request data.
      
      // Read and calibration request much time
      if (command == SNS_CALIBRATE_COMMAND || command == SNS_READ_COMMAND) delay(SNS_CAL_READ_TIME);
      else  delay(SNS_OTHER_TIME);
      
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

      //reset the array
      for (i = 0; i < I2C_DATA_LENGTH; i++) {
        ATS_data[i] = 0;
      }

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

      //If the first char is a number we know it is a DO reading, lets parse the DO reading
      if (isDigit(DO_data[0])) {
        // Global vars for DO and Sat will be loaded by this function
        parse_ATS_values();
        ATS_data_valid = true;
      } else {
        ATS_float=0.0; //In this case ATS_data_valid stays FALSE
      }
      
    //}
 // }
 // < ---------- ATS end ---------- >
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

  //if we see the there WAS a ‘,’ in the string array
  if (!comma) {
    ATS_float = atof(ATS_data);    
  } else {                                                        
    //First part of the message
    N1 = strtok(ATS_data, ",");
    //Second part of the message
    N2 = strtok(NULL, ",");                                              
    Serial.print("ATS:N1:");Serial.println(N1);
    Serial.print("ATS:N2:");Serial.println(N2);
    //Put the values into floating point
    ATS_float=atof(N1);
    //DO_sat_float=atof(N2);
  }  
}
