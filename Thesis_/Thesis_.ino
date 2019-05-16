#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

/* 
   Connections
   ===========
   Connect SCL to analog 5
   Connect SDA to analog 4
   Connect VDD to 3.3-5V DC
   Connect GROUND to common ground

/* Set the delay between fresh samples */
#define BNO055_SAMPLERATE_DELAY_MS (20)

Adafruit_BNO055 bno = Adafruit_BNO055(55);
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

#define FACTORYRESET_ENABLE         1
#define MINIMUM_FIRMWARE_VERSION    "0.7.0"

/* ...hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST */
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

Adafruit_BLEMIDI midi(ble);
bool isConnected = false;
int cc2 = 0;
int cc3 = 0;
int cc4 = 0;

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
  if(!bno.begin())
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }
   
  delay(1000);

  /* Use external crystal for better accuracy */
  bno.setExtCrystalUse(true);
  
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
  Serial.end();
}

void loop(void)
{
  // interval for each scanning ~ 500ms (non blocking)
  ble.update(10);

  // bail if not connected
  if (! isConnected)
    return;

  /* Get a new sensor event */
  sensors_event_t event;
  bno.getEvent(&event);
/*
 Serial.print("x = ");
 Serial.println(event.orientation.x);
 Serial.print("y = ");
 Serial.println(event.orientation.y);
 Serial.print("z = ");
 Serial.println(event.orientation.z);
 
 
 //these are the ranges of the bno sensor data remapped to midi CC range 0-127
  int cc2a = map((float)event.orientation.x,0,360,0,127);  
  int cc3a = map((float)event.orientation.y,-90,90,0,127);
  int cc4a = map((float)event.orientation.z,-180,180,0,127);
 //the problem is that some of the axis don't smoothly transition as they rotate and 
 //will jump back to 0 when they have completed a rotation
 //this causes bad sounds.
*/
   
 int cc2a = (float)event.orientation.x;
 int cc3a = (float)event.orientation.y;
 int cc4a = (float)event.orientation.z;

 // make cc2a wrap smoothly  
 if (cc2a > 180){
  cc2a = 360 - cc2a;
 }
   
 // make cc4a wrap smoothly  
 if (cc4a < 0){
  cc4a = cc4a * -1;
 }

 //remap to midi CC range, effectivley 180degree of rotation in any axis is 0-127, the next 180degree is 127-0 so transitions are always smooth
 cc2a = map(cc2a,0,180,0,127);
 cc3a = map(cc3a,-90,90,0,127);
 cc4a = map(cc4a,0,180,0,127);

  Serial.print("x = ");
 Serial.println(cc2a);
 Serial.print("y = ");
 Serial.println(cc3a);
 Serial.print("z = ");
 Serial.println(cc4a);
 
  if (cc2a != cc2){
   cc2 = cc2a;
   midi.send(0xb0, 0x2, cc2);
  }
  
  if (cc3a != cc3){
   cc3 = cc3a;
   midi.send(0xb0, 0x3, cc3);
  }
  
  if (cc4a != cc4){
   cc4 = cc4a;
   midi.send(0xb0, 0x4, cc4);
  }

delay(BNO055_SAMPLERATE_DELAY_MS);
}
