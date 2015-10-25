/*
This example code for Flutter is
Copyright 2015, Taylor Alexander and Flutter Wireless, Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "Flutter.h"
#include <string.h>

boolean _running = false;
const int netsize = 5; // number of external nodes (excluding central node)
int control[netsize];
// long timeout = 3600000;
long timeout = 120000*(netsize-1)*netsize;

int RXflag = 0;

int timeStamp;
Flutter flutter;

void setup()
{
	Serial.begin(9600);
	Serial1.begin(9600); // manda i dati a UNO attraverso A0 e A1
	flutter.band = NORTH_AMERICA;
	flutter.setNetworkName("Range Test");
	Serial.println("Initializing...");
        for (int i=0;i<netsize;i=i+1)
        {
          control[i] = 0;
        }

	if (flutter.init() == true)
	{
		Serial.println("Init success.");
		flutter.ledLightShow();
		delay(500);
		//analogWrite(LED_R, 128);
	}
	else
	{
		flutter.setLED(RED);
		Serial.println("Init failed.");

		while (true);
	}

	flutter.setAddress(1);
	flutter.connect(1); //form a network with this and one other device
}



void loop()
{     
        
        byte packetReceived[27];
        String dataPOST="";
        byte ACKresponse[3];
        byte NACKresponse[3];
        int rssi = 0;
        byte saddr[1];   
       
        Serial.print(control[0]);
        Serial.print(control[1]);
        Serial.print(control[2]);
        Serial.print(control[3]);
        Serial.println(control[4]);
        	  
        while (flutter.dataAvailable() > 0)
    	  {    
                Serial.println("while");
                int packetSize = flutter.nextPacketLength();
    		Serial.println("packetSize");
                Serial.println(packetSize);
                byte array[packetSize];
    		flutter.readBytes(array, packetSize);
                flutter.setLED(PURPLE);
                delay(2000);
                // flutter.setLED(0,0,0);
                memcpy(saddr,array+2,1);
                Serial.println("Source Address ");
                Serial.println(*saddr);
                Serial.println("array");
                Serial.write(array,packetSize);
                memcpy(packetReceived,array+5,27);
                Serial.println("");
                rssi = flutter.getRSSI();        
                        
                flutter.nextPacket(); //era stato cancellato
                
                Serial.println("millis-timestamp");
                Serial.println(millis()-timeStamp);
                if(packetReceived[26] == CRC(packetReceived,26))
		  {                   
                  if (RXflag==0)
                  {
                    timeStamp = millis();
                    Serial.println("timeStamp");
                    Serial.println(timeStamp);
                  }
                  
                  dataPOST = postConstruct(packetReceived,rssi);
                  Serial.println(dataPOST);
                  ACKresponse[0] = 65; //A
                  ACKresponse[1] = 67; //C
                  ACKresponse[2] = 75; //K
                  flutter.sendData(ACKresponse,3,*saddr);
                  flutter.setLED(LIME);
                  delay(2000);
                  // flutter.setLED(0,0,0);
                  if(control[*saddr-2] == 0)
                  { 
                    control[*saddr-2] = 1;
                    Serial1.println(dataPOST); // forse qui occorre un controlloper vedere se Arduino UNO effettivamente fa partire la POST

                    flutter.setLED(BLUE);
                    delay(2000);

                    delay(120000); // GPRS POST delay

                    // delay(5000); // GPRS POST delay (short for debug)
                    
                    flutter.setLED(0,0,0); // il LED si spegne alla fine dei due minuti d'attesa per la parte GSM
                    Serial.print("Buffer: ");
                    Serial.println(flutter.dataAvailable());
                    
                    RXflag = 0;
                    for (int i=0; i<netsize; i=i+1)
                    {
                       RXflag = RXflag + control[i]; 
                    }
                    
                    Serial.print("RXflag : ");
                    Serial.println(RXflag);  
                    delay(5000);                  
                    flutter.nextPacket();
                  }
                  }                                             
               else
               {
                  NACKresponse[0] = 78; //N
                  NACKresponse[1] = 65; //A
                  NACKresponse[2] = 67; //K
                  flutter.sendData(NACKresponse,3,*saddr);
                  flutter.setLED(RED);
                  delay(2000);
                  // flutter.setLED(0,0,0);               
                }
          }
          if(RXflag == netsize || millis()-timeStamp>timeout)
                    {
                    Serial.println("ALL EXTERNAL SENSORS RECEIVED");
                    for (int i=0;i<netsize;i=i+1)
                    {    
                      control[i] = 0;
                    }
                    RXflag = 0;
                    flutter.nextPacket(); //era stato cancellato
                    Software_Reset(); 
                  }          
  }  


void button1()
{
	interrupts();
	int val = digitalRead(BUTTON1); //top button

	if (val == HIGH)
	{
		// _button1=255;
	}
	else
	{
		//  _button1=0;
	}

// buttonsChanged=true;
}

void button2()
{
	interrupts();
	int val = digitalRead(BUTTON2);
#ifdef FLUTTER_R2

	if (val == HIGH)
#else
	if (val == LOW)
#endif
	{
		//_button2=255;
	}
	else
	{
		//_button2=0;
	}

// buttonsChanged=true;
}

void systemReset()
{
	flutter.setLED(0, 0, 255);
	delayMicroseconds(16000);
	delayMicroseconds(16000);
	flutter.setLED(0, 0, 0);
	delayMicroseconds(16000);
	delayMicroseconds(16000);
	flutter.setLED(0, 255, 0);
	delayMicroseconds(16000);
	delayMicroseconds(16000);
	flutter.setLED(0, 0, 0);
	delayMicroseconds(16000);
	delayMicroseconds(16000);
	flutter.setLED(255, 0, 0);
	delayMicroseconds(16000);
	delayMicroseconds(16000);
	flutter.setLED(0, 0, 0);
	initiateReset(1);
	tickReset();
}



void radioInterrupt()
{
	flutter.interrupt();
}

void softInt()
{
	flutter.processSoftInt();
}

extern boolean tickInterrupt()
{
	if (!flutter.initialized)
	{
		return true;
	}

	return flutter.tickInt();
}


//CRC-8 - algoritmo basato sulle formule di CRC-8 di Dallas/Maxim
//codice pubblicato sotto licenza GNU GPL 3.0
byte CRC(byte* packetPayload, int len) {
  byte crc = 0x00;
  while (len--) {
    byte extract = *packetPayload++;
    for (byte tempI = 8; tempI; tempI--) {
      byte sum = (crc ^ extract) & 0x01;
      crc >>= 1;
      if (sum) {
        crc ^= 0x8C;
      }
      extract >>= 1;
    }
  }
  return crc;
}

String postConstruct(byte *dataR,int rssi) {
  String dataP = "";
  int nodeID = 256*((int)dataR[1])+((int)dataR[2]);
  
  int R1 = 256*256*256*((int)dataR[5])+256*256*((int)dataR[6])+256*((int)dataR[7])+((int)dataR[8]);
  int R2 = 256*256*256*((int)dataR[11])+256*256*((int)dataR[12])+256*((int)dataR[13])+((int)dataR[14]);
  int H1 = 256*((int)dataR[17])+((int)dataR[18]);
  int H2 = 256*((int)dataR[21])+((int)dataR[22]);
  int BV = 256*((int)dataR[24])+((int)dataR[25]);
  dataP = String("{\"sensor\": ") + nodeID + ", \"f_res\": " + R1 + ", \"s_res\": " + R2 + ", \"f_hum\": " + H1 + ", \"s_hum\": " + H2 + ", \"RSSI\": " + rssi + ", \"B\": " + BV + "}";
  return dataP;

  //Serial.println(dataP);
}

void Software_Reset() {
//============================================================================================
//   führt ein Reset des Arduino DUE aus...
//
//   Parameter: keine
//   Rueckgabe: keine
//============================================================================================
  const int RSTC_KEY = 0xA5;
  RSTC->RSTC_CR = RSTC_CR_KEY(RSTC_KEY) | RSTC_CR_PROCRST | RSTC_CR_PERRST;
  while (true);
}
