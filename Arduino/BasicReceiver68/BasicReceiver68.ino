/*
 * Copyright (c) 2015 by Thomas Trojer <thomas@trojer.net>
 * Decawave DW1000 library for arduino.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @file BasicReceiver.ino
 * Use this to test simple sender/receiver functionality with two
 * DW1000. Complements the "BasicSender" example sketch.
 * 
 * @todo
 *  - move strings to flash (less RAM consumption)
 *  
 */

#include <SPI.h>
#include <SD.h>
#include "DW1000.h"

/* channel of operation. */
static constexpr byte CHANNEL_1 = 1;
static constexpr byte CHANNEL_2 = 2;
static constexpr byte CHANNEL_3 = 3;
static constexpr byte CHANNEL_4 = 4;
static constexpr byte CHANNEL_5 = 5;
static constexpr byte CHANNEL_7 = 7;

//  For MKRZERO
const uint8_t PIN_RST = 6; // reset pin
const uint8_t PIN_IRQ = 7; // irq pin
const uint8_t PIN_SS = 1; // spi select pin

// SD card chip select pin
const uint8_t chipSelect = 4;

// Filenumber for datalog
volatile int32_t fileNUM = 1;
char fileName[15] = "datalog1.txt";

// DEBUG packet sent status and count
volatile boolean received = false;
volatile boolean error = false;
volatile int32_t numReceived = 0; // todo check int type
String message;

void setup() {
  // DEBUG monitoring
  Serial.begin(115200);

  // Enable debug LED
  pinMode(LED_BUILTIN, OUTPUT);

  delay(1500);
  // Init SD-card
  //Serial.print("Initializing SD card...");

  //if(!SD.begin(chipSelect)){
  //  Serial.println("Card failed, or not present");
  //  return;
  //}
  //Serial.println("SD card initialized successfully");
  delay(1500);
  Serial.println(F("### DW1000-arduino-receiver-test ###"));
  // initialize the driver
  DW1000.begin(PIN_IRQ, PIN_RST);
  DW1000.select(PIN_SS);
  Serial.println(F("DW1000 initialized ..."));
  // general configuration
  DW1000.newConfiguration();
  DW1000.setDefaults();
  DW1000.setDeviceAddress(6);
  DW1000.setNetworkId(10);


//  DW1000.enableMode(DW1000.MODE_SHORTDATA_FAST_LOWPOWER); // {TRX_RATE_6800KBPS, TX_PULSE_FREQ_16MHZ, TX_PREAMBLE_LEN_128}
//    DW1000.enableMode(DW1000.MODE_SHORTDATA_FAST_ACCURACY); // {TRX_RATE_6800KBPS, TX_PULSE_FREQ_64MHZ, TX_PREAMBLE_LEN_128};
//    DW1000.enableMode(DW1000.MODE_LONGDATA_FAST_LOWPOWER);  // {TRX_RATE_6800KBPS, TX_PULSE_FREQ_16MHZ, TX_PREAMBLE_LEN_1024};
    DW1000.enableMode(DW1000.MODE_LONGDATA_FAST_ACCURACY);  // {TRX_RATE_6800KBPS, TX_PULSE_FREQ_64MHZ, TX_PREAMBLE_LEN_1024};

  DW1000.commitConfiguration();
  Serial.println(F("Committed configuration ..."));
  // DEBUG chip info and registers pretty printed
  char msg[128];
  DW1000.getPrintableDeviceIdentifier(msg);
  Serial.print("Device ID: "); Serial.println(msg);
  DW1000.getPrintableExtendedUniqueIdentifier(msg);
  Serial.print("Unique ID: "); Serial.println(msg);
  DW1000.getPrintableNetworkIdAndShortAddress(msg);
  Serial.print("Network ID & Device Address: "); Serial.println(msg);
  DW1000.getPrintableDeviceMode(msg);
  Serial.print("Device mode: "); Serial.println(msg);
  // attach callback for (successfully) received messages
  DW1000.attachReceivedHandler(handleReceived);
  DW1000.attachReceiveFailedHandler(handleError);
  DW1000.attachErrorHandler(handleError);
  // start reception
  receiver();
}

void handleReceived() {
  // status change on reception success
  received = true;
}

void handleError() {
  error = true;
}

void receiver() {
  DW1000.newReceive();
  DW1000.setDefaults();
  // so we don't need to restart the receiver manually
  DW1000.receivePermanently(true);
  DW1000.startReceive();
}

void loop() {
  //Serial.println("loooped");
  // enter on confirmation of ISR status change (successfully received)

  //String dataString = "";

  if (numReceived == 1000){
      //reset num received
      //fileNUM++; // Increase filenum
      numReceived = 0; // reset number of packets received
      //sprintf(fileName, "datalog%d.txt", fileNUM); // create new filename
      
  }
  else{
    if (received) {
      digitalWrite(LED_BUILTIN, HIGH); // DEBUG LED ON
      numReceived++;
      // get data as string
      DW1000.getData(message);
      //Serial.print("Received message ... #"); Serial.println(numReceived);
      //Serial.print("Data is ... "); Serial.println(message);
      //Serial.print("FP power is [dBm] ... "); Serial.println(DW1000.getFirstPathPower());
      //Serial.print("RX power is [dBm] ... "); Serial.println(DW1000.getReceivePower());
      //Serial.print("Signal quality is ... "); Serial.println(DW1000.getReceiveQuality());
      
      Serial.print(numReceived); Serial.print(" "); Serial.print(message); Serial.print(" "); Serial.print(DW1000.getFirstPathPower()); Serial.print(" "); Serial.print(DW1000.getReceivePower()); Serial.print(" "); Serial.println(DW1000.getReceiveQuality());
      
      // Write to file
      //File dataFile = SD.open(fileName, FILE_WRITE);

      //if(dataFile){
        //dataFile.println(dataString);
      //  dataFile.print(numReceived); dataFile.print(","); dataFile.print(message); dataFile.print(","); dataFile.print(DW1000.getFirstPathPower()); dataFile.print(","); dataFile.print(DW1000.getReceivePower()); dataFile.print(","); dataFile.println(DW1000.getReceiveQuality());
      //  dataFile.close();
      //}

      received = false; // Set received to false

      digitalWrite(LED_BUILTIN, LOW); //  DEBUG LED OFF
    }
    if (error) {
      //Serial.println("Error receiving a message");
      error = false;
      DW1000.getData(message);
      //Serial.print("Error data is ... "); Serial.println(message);
      Serial.print("ERROR "); Serial.println(message);
    }
  }
}
