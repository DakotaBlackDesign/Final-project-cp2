
#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"
#include "Adafruit_BLEMIDI.h"
#if SOFTWARE_SERIAL_AVAILABLE
  #include <SoftwareSerial.h>
#endif

#include "BluefruitConfig.h"

#define FACTORYRESET_ENABLE         0
#define MINIMUM_FIRMWARE_VERSION    "0.7.0"

/* ...hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST */
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

Adafruit_BLEMIDI midi(ble);
bool breathingOut = false;
bool breathingIn = false;
bool holding = false;
bool flip = true;
bool ok = false;
bool isConnected = false;
float slope = 0;
float slope1 = 0;
int holdcount = 0;
int threshold = 200;
float slopeMin = 100;
float slopeMax = -100;

//smoothing breath sensor
const int numReadings = 100;
const int numSamples = 10;
const int numSlopes =10;
float readings[numReadings];// the readings from the analog input
int readIndex = 0;              // the index of the current reading
float total = 0;                  // the running total
float average = 0;                // the average
int inputPin = A0;
float averages[numSamples];
float slopes[numSlopes];
float totalSlopes= 0;
int slopeIndex = 0;
int averageIndex = 0;
int averageIndex1 = 1;
unsigned long currentTime= 0;
unsigned long previousTime= 0;
int invervalDelay = 1000;
float reading = 0;
int cc5 = 0;
// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

// callback
void connected(void)
{
  isConnected = true;

  Serial.println(F(" CONNECTED!"));
  

}

void disconnected(void)
{
  Serial.println("disconnected");
  isConnected = false;
}

void setup(void)
{

  Serial.begin(115200);
   
  delay(1000);
  
  Serial.println(F("Adafruit Bluefruit MIDI Example"));
  Serial.println(F("---------------------------------------"));

  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ) {
      error(F("Couldn't factory reset"));
    }
  }

  //ble.sendCommandCheckOK(F("AT+uartflow=off"));
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  /* Set BLE callbacks */
  ble.setConnectCallback(connected);
  ble.setDisconnectCallback(disconnected);

  Serial.println(F("Enable MIDI: "));
  if ( ! midi.begin(true) )
  {
    error(F("Could not enable MIDI"));
  }

  ble.verbose(false);
  Serial.print(F("Waiting for a connection..."));
  
// initialize all the readings to 0:
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }
}

void loop(void)
{
  // interval for each scanning ~ 500ms (non blocking)
  ble.update();

  // bail if not connected
  if (! isConnected)
    return;

 
 reading = .9 * reading + .1 * float(analogRead(inputPin));
//Serial.println(reading);



/*
  //smooth the input values 
  total = total - readings[readIndex]; // subtract the last reading
  readings[readIndex] = analogRead(inputPin); // read from the sensor
  total = total + readings[readIndex]; // add the reading to the total
  readIndex ++; // advance to the next position in the array
  if (readIndex >= numReadings) {// if we're at the end of the array...
    readIndex = 0;// ...wrap around to the beginning
  }
  average = total / numReadings; //find the average value 
*/
average = reading; 
  // find the previous reading to compare with the current reading to get the slope
  // is the sensor value going up or down
  averages[averageIndex] = average;// add value to array
  
  averageIndex1 = averageIndex + 1; //find the index of the oldest value in the array...one index ahead unless...
  if (averageIndex1 >= numSamples) averageIndex1 = 0; //we are at the end... oldest index is the first entry
  slope = averages[averageIndex] - averages[averageIndex1];// find the diffence/change between the newest and oldest values
  averageIndex ++; // move to the next position
  if ( averageIndex >= numSamples){ //if we're at the end of the array...
    averageIndex = 0;// ...wrap around to the beginning
  }

  //Serial.println(slope);

 /*
  //slope smoothing
  totalSlopes = totalSlopes - slopes[slopeIndex];
  slopes[slopeIndex] = slope;
  totalSlopes = totalSlopes + slopes[slopeIndex];
  slopeIndex ++;
  if (slopeIndex >= numSlopes) {
    slopeIndex = 0;
  }
  slope1 = totalSlopes / numSlopes; //find the average slope
*/
//Serial.println(slope1);
  currentTime = millis();
  if (currentTime - previousTime > invervalDelay){
    ok = true;
  }
 
  if (slope < -3){
    breathingIn = true;
    breathingOut = false;
    holding = false;
    holdcount= 0;
    cc5 ++;
    midi.send(0xb0, 0x5, cc5); 
    //Serial.println(slope1);
  }
  
  if (slope > 3){
    breathingIn = false;
    breathingOut = true;
    holding = false;
    holdcount= 0;
    cc5 = cc5 - 1;
    midi.send(0xb0, 0x5, cc5);
    //Serial.println(slope1);
  }

  if (slope < 3 && slope > -3){
    if (holdcount > threshold){
      holding = true;
    }
    holdcount ++;  
  }

  if (breathingIn){
    //Serial.println("in");
    BreathInSound(); 
  }
  
  if (breathingOut){
    //Serial.println("out");
    BreathOutSound();
    //Serial.println(cc5);
  }
  
  if (holding){
    midi.send(0x90, 48, 0x00); // send note off
    holdcount = 0;
    //Serial.println("hold");
  }

 if (holding && flip){
    cc5 = 0;
    midi.send(0xb0, 0x5, cc5);
 }
  delay(10);
}


void BreathInSound(void){
  if (flip && ok){
    
    midi.send(0x90, 48, 0x00); // send note off
    midi.send(0xC0,0x00,0x01); // send program sellect
    midi.send(0x90, 48, 0x7f); // send note 
    flip = !flip; 
    previousTime = millis();
    ok = false;
    Serial.println("in");
   }
  
  }
 
void BreathOutSound(void){
  if(!flip && ok){
    midi.send(0x90, 48, 0x00); // send note off
    midi.send(0xC0,0x00,0x00); // send program sellect
    midi.send(0x90, 48, 0x7f); // send note
    flip = !flip;
    previousTime = millis();
    ok = false;
    Serial.println("out");
    }
    
  }

