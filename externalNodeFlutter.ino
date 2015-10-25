// "Vinduino" portable soil moisture sensor code V3.00
// Date February 07, 2015

// calcolo del moisture (umidità) secondo il modello matematico vinduino
#include <string.h>
#include <math.h>
#include "Flutter.h"
#define TRANSMITTER
#define NUM_READS 21    // Number of sensor reads for filtering

typedef struct {        // Structure to be used in percentage and resistance values matrix to be filtered (have to be in pairs)
  int moisture;
  long resistance;
} values;

Flutter flutter;
int nodeID = 5;
int netsize = 6; //number of external nodes (excluding the central node)
long gprsDelay = 120000;
long sensorUpdateTime = 3600000;

const long knownResistor = 2200;  // Constant value of known resistor in Ohms

const long openResistor = 1000000; // Constant value of open circuit sensor value

int activeDigitalPin1 = 4;         // 4 or 5 interchangeably
int supplyVoltageAnalogPin1;       // 4-ON: A1, 5-ON: A0
int sensorVoltageAnalogPin1;       // 4-ON: A0, 5-ON: A1

int supplyVoltage1;                // Measured supply voltage
int sensorVoltage1;                // Measured sensor voltage

int activeDigitalPin2 = 6;         // 6 or 7 interchangeably
int supplyVoltageAnalogPin2;       // 6-ON: A3, 7-ON: A2
int sensorVoltageAnalogPin2;       // 6-ON: A2, 7-ON: A3

int supplyVoltage2;                // Measured supply voltage
int sensorVoltage2;                // Measured sensor voltage

//int activeDigitalPin3 = 8;         // 8 or 9 interchangeably
//int supplyVoltageAnalogPin3;       // 8-ON: A5, 9-ON: A4
//int sensorVoltageAnalogPin3;       // 8-ON: A4, 9-ON: A5
//
//int supplyVoltage3;                // Measured supply voltage
//int sensorVoltage3;                // Measured sensor voltage

values value_1_Of[NUM_READS];        // Calculated moisture percentages and resistances to be sorted and filtered
values value_2_Of[NUM_READS];        // Calculated moisture percentages and resistances to be sorted and filtered
// values value_3_Of[NUM_READS];        // Calculated moisture percentages and resistances to be sorted and filtered

int i;                            // Simple index variable

void setup() {

  //Serial.begin(115200);
  Serial.begin(9600);
  flutter.band = NORTH_AMERICA;
  flutter.setNetworkName("Range Test");
  Serial.println("Initializing...");
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

  flutter.setAddress(6);
  // flutter.setAddress(nodeID);
  flutter.connect(1);
  // initialize the digital pin as an output.
  // Pin 4 is sense resistor (1) voltage supply 1
  pinMode(4, OUTPUT);    

  // initialize the digital pin as an output.
  // Pin 5 is sense resistor (1) voltage supply 2
  pinMode(5, OUTPUT);   
  
    // initialize the digital pin as an output.
  // Pin 6 is sense resistor (2) voltage supply 1
  pinMode(6, OUTPUT);    

  // initialize the digital pin as an output.
  // Pin 7 is sense resistor (2) voltage supply 2
  pinMode(7, OUTPUT); 
  
    // initialize the digital pin as an output.
  // Pin 8 is sense resistor (3) voltage supply 1
  // pinMode(8, OUTPUT);    

  // initialize the digital pin as an output.
  // Pin 9 is sense resistor (3) voltage supply 2
  // pinMode(9, OUTPUT); 

  delay(500);   
}

void loop() {
  byte packetPayload[26];
  byte CRCHeader;
  byte packet[27];
  byte packetReceived[3];
  // read sensor, filter, and calculate resistance value
  // Noise filter: median filter
  for (i=0; i<NUM_READS; i++) {

    setupCurrentPath();      // Prepare the digital and analog pin values

    // Read 3 pairs of voltage values
    digitalWrite(activeDigitalPin1, HIGH);                 // set the voltage supply on
    digitalWrite(activeDigitalPin2, HIGH);                 // set the voltage supply on
    // digitalWrite(activeDigitalPin3, HIGH);                 // set the voltage supply on
    delay(10);
    supplyVoltage1 = analogRead(supplyVoltageAnalogPin1);   // read the supply voltage
    sensorVoltage1 = analogRead(sensorVoltageAnalogPin1);   // read the sensor voltage
    supplyVoltage2 = analogRead(supplyVoltageAnalogPin2);   // read the supply voltage
    sensorVoltage2 = analogRead(sensorVoltageAnalogPin2);   // read the sensor voltage    
    // supplyVoltage3 = analogRead(supplyVoltageAnalogPin3);   // read the supply voltage
    // sensorVoltage3 = analogRead(sensorVoltageAnalogPin3);   // read the sensor voltage    delay(10); 
    digitalWrite(activeDigitalPin1, LOW);                  // set the voltage supply off  
    digitalWrite(activeDigitalPin2, LOW);                  // set the voltage supply off  
    // digitalWrite(activeDigitalPin3, LOW);                  // set the voltage supply off  
    delay(10); // se i 2 delay sono uguali allora dutycycle è 25%, se commenti il secondo delay dutycycle è 50%

    // Calculate resistance and moisture percentage without overshooting 100
    // the 0.5 add-term is used to round to the nearest integer
    // Tip: no need to transform 0-1023 voltage value to 0-5 range, due to following fraction
    value_1_Of[i].resistance = long( float(knownResistor) * ( supplyVoltage1 - sensorVoltage1 ) / sensorVoltage1 + 0.5 );
    value_1_Of[i].moisture = min( int( pow( value_1_Of[i].resistance/31.65 , 1.0/-1.695 ) * 400 + 0.5 ) , 100 );
//  value_1_Of[i].moisture = min( int( pow( valueOf[i].resistance/331.55 , 1.0/-1.695 ) * 100 + 0.5 ) , 100 );
  
    value_2_Of[i].resistance = long( float(knownResistor) * ( supplyVoltage2 - sensorVoltage2 ) / sensorVoltage2 + 0.5 );
    value_2_Of[i].moisture = min( int( pow( value_2_Of[i].resistance/31.65 , 1.0/-1.695 ) * 400 + 0.5 ) , 100 );
//  value_2_Of[i].moisture = min( int( pow( valueOf[i].resistance/331.55 , 1.0/-1.695 ) * 100 + 0.5 ) , 100 );

//  value_3_Of[i].resistance = long( float(knownResistor) * ( supplyVoltage3 - sensorVoltage3 ) / sensorVoltage3 + 0.5 );
//  value_3_Of[i].moisture = min( int( pow( value_3_Of[i].resistance/31.65 , 1.0/-1.695 ) * 400 + 0.5 ) , 100 );    
//  value_3_Of[i].moisture = min( int( pow( valueOf[i].resistance/331.55 , 1.0/-1.695 ) * 100 + 0.5 ) , 100 );
    Serial.println(value_1_Of[i].resistance);
  }

  // end of multiple read loop

  // Sort the moisture-resistance vector according to moisture
  sortMoistures_1();
  sortMoistures_2();
  // sortMoistures_3();
  int batteryVoltage = analogRead(A5);
  Serial.println("R1");
  Serial.println(value_1_Of[NUM_READS/2].resistance);
  Serial.println("R2");
  Serial.println(value_2_Of[NUM_READS/2].resistance);
  
  if(value_1_Of[NUM_READS/2].resistance<0)
  {
    value_1_Of[NUM_READS/2].resistance = openResistor;
    value_1_Of[NUM_READS/2].moisture = 0;
  }  
  
  if(value_2_Of[NUM_READS/2].resistance<0)
  {
    value_2_Of[NUM_READS/2].resistance = openResistor;
    value_2_Of[NUM_READS/2].moisture = 0;
  }  
  
  Serial.println("R1 POST");
  Serial.println(value_1_Of[NUM_READS/2].resistance);
  Serial.println("R2 POST");
  Serial.println(value_2_Of[NUM_READS/2].resistance);
  
  payload(nodeID,value_1_Of[NUM_READS/2].resistance,int(value_1_Of[NUM_READS/2].moisture),value_2_Of[NUM_READS/2].resistance,int(value_2_Of[NUM_READS/2].moisture),batteryVoltage, &packetPayload[0]); // funzione normale
//  payload(nodeID,value_1_Of[NUM_READS/2].resistance,batteryVoltage,value_2_Of[NUM_READS/2].resistance,int(value_2_Of[NUM_READS/2].moisture),batteryVoltage, &packetPayload[0]); // H1 rappresenta il valore di tensione della batteria al litio (debug sul database)
  CRCHeader = CRC(packetPayload,26);
  Serial.write(packetPayload,26);
  Serial.println("");
  Serial.write(CRCHeader);
  Serial.println("");
  memcpy(packet,&packetPayload,26);
  memcpy(packet+26,&CRCHeader,1);
  Serial.write(packet,27);  
  Serial.println("");
  Serial.println("");
   
  flutter.sendData(packet,27,1); //length is 3, 2 is car's address
  flutter.setLED(BLUE);
  delay(2000);
  flutter.setLED(0,0,0);
  delay(2000);
  int timeStamp = millis();
  while(millis()-timeStamp < (netsize-1)*(gprsDelay)) // delay is referred to the GSM/GPRS transmission time
  {
    // flutter.setLED(YELLOW);
    {
      if (flutter.dataAvailable()>0) 
      { 
       int packetSize = flutter.nextPacketLength();
       byte array[packetSize];
       flutter.readBytes(array, packetSize);
       flutter.nextPacket();
       flutter.setLED(PURPLE);
       delay(2000);
       flutter.setLED(0,0,0);
       memcpy(packetReceived,array+5,packetSize);
       Serial.write(packetReceived,sizeof(packetReceived));
       Serial.println("");
       if(packetReceived[0]!=65)
        {
          flutter.sendData(packet,27,1); //legth is 3, 2 is car's address
          flutter.setLED(RED);
          delay(2000);
          flutter.setLED(0,0,0);        
        }
        else
        {
          flutter.setLED(GREEN);
          delay(2000);
          flutter.setLED(0,0,0);
          delay(sensorUpdateTime); // Update sensor reading
          Software_Reset();
          break;
        }
       }
     }
  }
  
//  Serial.print("Moisture 3: ");
//  Serial.print(value_3_Of[NUM_READS/2].moisture);
//  Serial.print(" %");
//
//  Serial.print("Sensor 3: " );
//  Serial.print(value_3_Of[NUM_READS/2].resistance);
//  Serial.println(" ");
//  Serial.write(1);

  // delay until next measurement (msec)
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


void setupCurrentPath() {
  if ( activeDigitalPin1 == 4 ) {
    activeDigitalPin1 = 5;
    activeDigitalPin2 = 7;
    // activeDigitalPin3 = 7;    
    supplyVoltageAnalogPin1 = A0;
    sensorVoltageAnalogPin1 = A1;
    supplyVoltageAnalogPin2 = A2;
    sensorVoltageAnalogPin2 = A3;
    // supplyVoltageAnalogPin3 = A4;
    // sensorVoltageAnalogPin3 = A5;
  }
  else {
    activeDigitalPin1 = 4;
    activeDigitalPin2 = 6;
    // activeDigitalPin3 = 6;    
    supplyVoltageAnalogPin1 = A1;
    sensorVoltageAnalogPin1 = A0;
    supplyVoltageAnalogPin2 = A3;
    sensorVoltageAnalogPin2 = A2;
    // supplyVoltageAnalogPin3 = A5;
    // sensorVoltageAnalogPin3 = A4;
  }
}

// Selection sort algorithm
void sortMoistures_1() {
  int j;
  values temp;
  for(i=0; i<NUM_READS-1; i++)
    for(j=i+1; j<NUM_READS; j++)
      if ( value_1_Of[i].moisture > value_1_Of[j].moisture ) {
        temp = value_1_Of[i];
        value_1_Of[i] = value_1_Of[j];
        value_1_Of[j] = temp;
      }
}

void sortMoistures_2() {
  int j;
  values temp;
  for(i=0; i<NUM_READS-1; i++)
    for(j=i+1; j<NUM_READS; j++)
      if ( value_2_Of[i].moisture > value_2_Of[j].moisture ) {
        temp = value_2_Of[i];
        value_2_Of[i] = value_2_Of[j];
        value_2_Of[j] = temp;
      }
}

//void sortMoistures_3() {
//  int j;
//  values temp;
//  for(i=0; i<NUM_READS-1; i++)
//    for(j=i+1; j<NUM_READS; j++)
//      if ( value_3_Of[i].moisture > value_3_Of[j].moisture ) {
//        temp = value_3_Of[i];
//        value_3_Of[i] = value_3_Of[j];
//        value_3_Of[j] = temp;
//      }
//}

void payload(int nodeID, long R1, int H1, long R2, int H2, int B, byte *packetData) {

  //int payloadLength = 20;
  //byte packetData[payloadLength];
  
  packetData[0] = 70; //F
  packetData[1] = highByte(nodeID);
  packetData[2] = lowByte(nodeID);
  packetData[3] = 82; //R
  packetData[4] = 49; //1
  packetData[5] = ((R1 >> 24) & 0xFF);
  packetData[6] = ((R1 >> 16) & 0xFF);
  packetData[7] = ((R1 >> 8) & 0xFF);
  packetData[8] = (R1 & 0xFF);
  packetData[9] = 82; //R
  packetData[10] = 50; //2
  packetData[11] = ((R2 >> 24) & 0xFF); 
  packetData[12] = ((R2 >> 16) & 0xFF);
  packetData[13] = ((R2 >> 8) & 0xFF);
  packetData[14] = (R2 & 0xFF);
  packetData[15] = 72; //H
  packetData[16] = 49; //1
  packetData[17] = highByte(H1);
  packetData[18] = lowByte(H1);
  packetData[19] = 72; //H
  packetData[20] = 50; //2
  packetData[21] = highByte(H2);
  packetData[22] = lowByte(H2);
  packetData[23] = 66;//B
  packetData[24] = highByte(B);
  packetData[25] = lowByte(B);
//  packetData[19] = 33; //! = EndOfHeader
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


