/*
 * 4.7k pull up resistor on SDA and SCL must be required.
 * The ORP circuit will output readings from -1019.9mV to +1019.9mV.
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
 */
void ORP_I2C() {
  
  //char computerdata[20];         //we make a 20 byte character array to hold incoming data from a pc/mac/other.
  byte received_from_computer = 0; //we need to know how many characters have been received.
  byte code = 0;                   //used to hold the I2C response code.
  char ORP_data[20];               //we make a 20 byte character array to hold incoming data from the ORP circuit.
  byte in_char = 0;                //used as a 1 byte buffer to store in bound bytes from the ORP Circuit.
  byte i = 0;                      //counter used for ORP_data array.
  
  //if (Serial.available() > 0) {                                           //if data is holding in the serial buffer
    //received_from_computer = Serial.readBytesUntil(13, computerdata, 20); //we read the data sent from the serial monitor(pc/mac/other) until we see a <CR>. We also count how many characters have been received.
    //computerdata[received_from_computer] = 0;                             //stop the buffer from transmitting leftovers or garbage.
    //computerdata[0] = tolower(computerdata[0]);                           //we make sure the first char in the string is lower case.
    //if (computerdata[0] == 'c' || computerdata[0] == 'r')time_ = 1800;    //if a command has been sent to calibrate or take a reading we wait 1800ms so that the circuit has time to take the reading.
    //else time_ = 300;                                                     //if any other command has been sent we wait only 300ms.

    // <----- Command ---->
    char computerdata = SNS_READ_COMMAND;
    
    Wire.beginTransmission(ORP_ADDRESS); //call the circuit by its ID number.
    Wire.write(computerdata);            //transmit the command that was sent through the serial port.
    Wire.endTransmission();              //end the I2C data transmission.

    //if (strcmp(computerdata, "sleep") != 0) {  //if the command that has been sent is NOT the sleep command, wait the correct amount of time and request data.
      
      // Read and calibration request much time
      if (computerdata == SNS_CALIBRATE_COMMAND || computerdata == SNS_READ_COMMAND) delay(SNS_CAL_READ_TIME);
      else  delay(SNS_OTHER_TIME);
      
      Wire.requestFrom(ORP_ADDRESS, 20, 1); //call the circuit and request 20 bytes (this may be more than we need)
      code = Wire.read();                   //the first byte is the response code, we read this separately.

      switch (code) {                       //switch case based on what the response code is.
        case 1:                             //decimal 1.
          Serial.println("ORP Success");    //means the command was successful.
          break;                            //exits the switch case.
          
        case 2:                             //decimal 2.
          Serial.println("ORP Failed");     //means the command has failed.
          break;                            //exits the switch case.

        case 254:                           //decimal 254.
          Serial.println("ORP Pending");    //means the command has not yet been finished calculating.
          break;                            //exits the switch case.

        case 255:                           //decimal 255.
          Serial.println("ORP No Data");    //means there is no further data to send.
          break;                            //exits the switch case.
      }

      //reset the array
      for (i = 0; i < 20; i++) {
        ORP_data[i] = 0;
      }

      while (Wire.available()) {         //are there bytes to receive.
        in_char = Wire.read();           //receive a byte.
        ORP_data[i] = in_char;           //load this byte into our array.
        i += 1;                          //incur the counter for the array element.
        if (in_char == 0) {              //if we see that we have been sent a null command.
          i = 0;                         //reset the counter i to 0.
          Wire.endTransmission();        //end the I2C data transmission.
          break;                         //exit the while loop.
        }
      }

      Serial.print("ORP data:");
      Serial.println(ORP_data);          //print the data.
    //}
    ORP_float = atof(ORP_data);
 // }
  // < ---------- ORP end ---------- >
}
