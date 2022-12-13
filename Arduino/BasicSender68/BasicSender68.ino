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
 * @file BasicSender.ino
 * Use this to test simple sender/receiver functionality with two
 * DW1000. Complements the "BasicReceiver" example sketch. 
 * 
 * @todo
 *  - move strings to flash (less RAM consumption)
 *  
 */
#include <SPI.h>
#include <SD.h>
#include "DW1000.h"

// For MKRZERO
const uint8_t PIN_RST = 6; // reset pin
const uint8_t PIN_IRQ = 7; // irq pin
const uint8_t PIN_SS = 1; // spi select pin

// TRANSFER FINISHED LED
const uint8_t LED_PIN = A0;
const uint8_t BUTTON_PIN = 0;

// DEBUG packet sent status and count
boolean sent = false;
volatile boolean sentAck = false;
volatile unsigned long delaySent = 0;
int32_t sentNum = 0; // todo check int type
DW1000Time sentTime;

void setup() {
  // DEBUG monitoring
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  delay(3000);
  Serial.println(F("### DW1000-arduino-sender-test ###"));
  // initialize the driver
  DW1000.begin(PIN_IRQ, PIN_RST);
  DW1000.select(PIN_SS);
  Serial.println(F("DW1000 initialized ..."));
  // general configuration
  DW1000.newConfiguration();
  DW1000.setDefaults();
  DW1000.setDeviceAddress(5);
  DW1000.setNetworkId(10);

//    DW1000.enableMode(DW1000.MODE_SHORTDATA_FAST_LOWPOWER); // {TRX_RATE_6800KBPS, TX_PULSE_FREQ_16MHZ, TX_PREAMBLE_LEN_128}
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
  // attach callback for (successfully) sent messages
  DW1000.attachSentHandler(handleSent);
  Serial.print("Handle attached");
  // start a transmission
  transmitter();
}

void handleSent() {
  // status change on sent success
  sentAck = true;
}

void transmitter() {
  digitalWrite(LED_BUILTIN, HIGH);// DEBUG LED ON
  // transmit some data
  Serial.print("Transmitting packet ... #"); Serial.println(sentNum);
  DW1000.newTransmit();
  DW1000.setDefaults();
  //String msg = "Hello DW1000, it's #"; msg += sentNum;
  String msg = ""; msg += sentNum;
  DW1000.setData(msg);
  // delay sending the message for the given amount
  DW1000Time deltaTime = DW1000Time(10, DW1000Time::MILLISECONDS);
  DW1000.setDelay(deltaTime);
  DW1000.startTransmit();
  delaySent = millis();
  digitalWrite(LED_BUILTIN, LOW);// DEBUG LED ON
}

void loop() {
  
  while(digitalRead(BUTTON_PIN) == HIGH){
    sentNum = 0;
    digitalWrite(LED_PIN, LOW);
    delay(3000);
    //while(digitalRead(BUTTON_PIN) == HIGH){}
  }
  
  if(sentNum <= 999){
    if (!sentAck) {
    return;
    }
    // continue on success confirmation
    // (we are here after the given amount of send delay time has passed)
    sentAck = false;
    // update and print some information about the sent message
    Serial.print("ARDUINO delay sent [ms] ... "); Serial.println(millis() - delaySent);
    DW1000Time newSentTime;
    DW1000.getTransmitTimestamp(newSentTime);
    Serial.print("Processed packet ... #"); Serial.println(sentNum);
    Serial.print("Sent timestamp ... "); Serial.println(newSentTime.getAsMicroSeconds());
    // note: delta is just for simple demo as not correct on system time counter wrap-around
    Serial.print("DW1000 delta send time [ms] ... "); Serial.println((newSentTime.getAsMicroSeconds() - sentTime.getAsMicroSeconds()) * 1.0e-3);
    sentTime = newSentTime;
    sentNum++;
    // again, transmit some data
    transmitter();
  }
  else{
    digitalWrite(LED_PIN, HIGH);
  }
}