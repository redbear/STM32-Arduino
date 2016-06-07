/*
 *
 * Copyright (c) 2015 Particle Industries, Inc.  All rights reserved.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * Modified by Guohui @2016/6/4
 */

/* For the Duo that is running this sketch, as long as it is connecting to the Particle Cloud, 
 * You can tinker it wherever you can access the internet.
 * 
 * Tinker your Duo using curl:
 *     curl https://api.particle.io/v1/devices/{YOUR_DEVICE_ID}/{FUNCTION_NAME} -d access_token={YOUR_ACCESS_TOKEN} -d "args={ARGUMENTS}" -k
 * E.g.(Assume that your Duo's device ID is: 112233445566778899aabbcc and your access token is 12345678901234567890):
 *     1. curl https://api.particle.io/v1/devices/112233445566778899aabbcc/digitalwrite -d access_token=12345678901234567890 -d "args=D0 HIGH" -k
 *     2. curl https://api.particle.io/v1/devices/112233445566778899aabbcc/digitalread -d access_token=12345678901234567890 -d "args=D1" -k
 *     3. curl https://api.particle.io/v1/devices/112233445566778899aabbcc/analogwrite -d access_token=12345678901234567890 -d "args=D0 127" -k
 *     4. curl https://api.particle.io/v1/devices/112233445566778899aabbcc/analogread -d access_token=12345678901234567890 -d "args=A0" -k
 *   
 * Your access token can be found on Particle Build: https://build.particle.io, After you login in, check it under the "Settings" tag.
 */

/*
 * SYSTEM_MODE:
 *     - AUTOMATIC: Automatically try to connect to Wi-Fi and the Particle Cloud and handle the cloud messages.
 *     - SEMI_AUTOMATIC: Manually connect to Wi-Fi and the Particle Cloud, but automatically handle the cloud messages.
 *     - MANUAL: Manually connect to Wi-Fi and the Particle Cloud and handle the cloud messages.
 *     
 * SYSTEM_MODE(AUTOMATIC) does not need to be called, because it is the default state. 
 * However the user can invoke this method to make the mode explicit.
 * Learn more about system modes: https://docs.particle.io/reference/firmware/photon/#system-modes .
 */
SYSTEM_MODE(AUTOMATIC); // Connect to Particle Cloud to tinker your Duo.

/* Function prototypes -------------------------------------------------------*/
int tinkerDigitalRead(String pin);
int tinkerDigitalWrite(String command);
int tinkerAnalogRead(String pin);
int tinkerAnalogWrite(String command);


/* This function is called once at start up ----------------------------------*/
void setup() {
  Serial.begin(115200);
  Serial.println("Tinker application started.");
  
  //Register all the Tinker functions
  Particle.function("digitalread", tinkerDigitalRead);
  Particle.function("digitalwrite", tinkerDigitalWrite);
  Particle.function("analogread", tinkerAnalogRead);
  Particle.function("analogwrite", tinkerAnalogWrite);
}

/* This function loops forever --------------------------------------------*/
void loop() {
  //This will run in a loop
}

/*******************************************************************************
 * Function Name  : tinkerDigitalRead
 * Description    : Reads the digital value of a given pin
 * Input          : Pin
 * Output         : None.
 * Return         : Value of the pin (0 or 1) in INT type
                    Returns a negative number on failure
 *******************************************************************************/
int tinkerDigitalRead(String pin) {
  //convert ascii to integer
  int pinNumber = pin.charAt(1) - '0';
  
  //Sanity check to see if the pin numbers are within limits
  if (pinNumber < 0 || pinNumber > 7) return -1;

  if (pin.startsWith("D")) {
    pinMode(pinNumber, INPUT_PULLDOWN);
    return digitalRead(pinNumber);
  }
  else if (pin.startsWith("A")) {
    pinMode(pinNumber+10, INPUT_PULLDOWN);
    return digitalRead(pinNumber+10);
  }
  return -2;
}

/*******************************************************************************
 * Function Name  : tinkerDigitalWrite
 * Description    : Sets the specified pin HIGH or LOW
 * Input          : Pin and value
 * Output         : None.
 * Return         : 1 on success and a negative number on failure
 *******************************************************************************/
int tinkerDigitalWrite(String command) {
  bool value = 0;
  //convert ascii to integer
  int pinNumber = command.charAt(1) - '0';
  
  //Sanity check to see if the pin numbers are within limits
  if (pinNumber < 0 || pinNumber > 7) return -1;

  if (command.substring(3,7) == "HIGH") value = 1;
  else if(command.substring(3,6) == "LOW") value = 0;
  else return -2;

  if (command.startsWith("D")) {
    pinMode(pinNumber, OUTPUT);
    digitalWrite(pinNumber, value);
    return 1;
  }
  else if (command.startsWith("A")) {
    pinMode(pinNumber+10, OUTPUT);
    digitalWrite(pinNumber+10, value);
    return 1;
  }
  else return -3;
}

/*******************************************************************************
 * Function Name  : tinkerAnalogRead
 * Description    : Reads the analog value of a pin
 * Input          : Pin
 * Output         : None.
 * Return         : Returns the analog value in INT type (0 to 4095)
                    Returns a negative number on failure
 *******************************************************************************/
int tinkerAnalogRead(String pin) {
  //convert ascii to integer
  int pinNumber = pin.charAt(1) - '0';
  
  //Sanity check to see if the pin numbers are within limits
  if (pinNumber < 0 || pinNumber > 7) return -1;

  if (pin.startsWith("D")) {
    return -3;
  }
  else if (pin.startsWith("A")) {
    return analogRead(pinNumber+10);
  }
  return -2;
}

/*******************************************************************************
 * Function Name  : tinkerAnalogWrite
 * Description    : Writes an analog value (PWM) to the specified pin
 * Input          : Pin and Value (0 to 255)
 * Output         : None.
 * Return         : 1 on success and a negative number on failure
 *******************************************************************************/
int tinkerAnalogWrite(String command) {
  String value = command.substring(3);

  if (command.substring(0,2) == "TX") {
    pinMode(TX, OUTPUT);
    analogWrite(TX, value.toInt());
    return 1;
  }
  else if (command.substring(0,2) == "RX") {
    pinMode(RX, OUTPUT);
    analogWrite(RX, value.toInt());
    return 1;
  }

  //convert ascii to integer
  int pinNumber = command.charAt(1) - '0';
  
  //Sanity check to see if the pin numbers are within limits
  if (pinNumber < 0 || pinNumber > 7) return -1;

  if (command.startsWith("D")) {
    pinMode(pinNumber, OUTPUT);
    analogWrite(pinNumber, value.toInt());
    return 1;
  }
  else if (command.startsWith("A")) {
    pinMode(pinNumber+10, OUTPUT);
    analogWrite(pinNumber+10, value.toInt());
    return 1;
  }
  else if (command.substring(0,2) == "TX") {
    pinMode(TX, OUTPUT);
    analogWrite(TX, value.toInt());
    return 1;
  }
  else if (command.substring(0,2) == "RX") {
    pinMode(RX, OUTPUT);
    analogWrite(RX, value.toInt());
    return 1;
  }
  else return -2;
}

