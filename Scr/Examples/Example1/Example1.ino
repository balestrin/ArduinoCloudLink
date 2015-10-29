/*
* Copyright (c) 2015 Elipse Software
*
* Elipse Mobile http://www.elipsemobile.com/
*
*/

#include <Ethernet.h>
#include <SPI.h>
#include <ArduinoCloudLink.h>

////////////////////////////////////////////////////////////
//Configuration
#define SERVER_NAME         "meuarduino.elipsemobile.com"
#define CONNECTION_NAME     "arduino"
#define CONNECTION_PASSWORD "123"
////////////////////////////////////////////////////////////


void EthernetSetup()
{
  byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

  if (Ethernet.begin(mac) == 0)
  {
    Ethernet.begin(mac, Ethernet.localIP());
  }

  delay(1000);
}

CloudConnection cloudConnection;

void setup()
{
  Serial.begin(9600);

  pinMode(7, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(6, OUTPUT);
  
  EthernetSetup();

  CloudConnection_Init(&cloudConnection,
                       CONNECTION_NAME,
                       SERVER_NAME,
                       80,
                       CONNECTION_PASSWORD);

  Setup(ARDUINO_UNO, NULL, NULL);
  SetEventCallback(&DefaultLedEvents);
}

void loop()
{
  CloudConnection_Loop(&cloudConnection);
}



