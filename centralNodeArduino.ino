#include <HWSerial.h>
#include <Streaming.h>
#include <GSM.h>
#include <WideTextFinder.h>
#include <call.h>
#include <inetGSM.h>
#include <gps.h>
#include <SIM900.h>
#include <LOG.h>
#include <sms.h>

#include "SIM900.h"
#include <SoftwareSerial.h>
#include "inetGSM.h"
// #include "sms.h"
// #include "call.h"

//To change pins for Software Serial, use the two lines in GSM.cpp.

//GSM Shield for Arduino
//www.open-electronics.org
//this code is based on the example of Arduino Labs.

//Simple sketch to start a connection as client.

InetGSM inet;
// CallGSM call;
// SMSGSM sms;

char msg[50];
int numdata;
char inSerial[50];
int i=0;
boolean started=false;

char* host = "gregorat.admin.idroplan.org";
// char* host = "admin.idroplan.org";
int port = 80;
// int port = 8000;
char* end_point = "/measures/create_measure";

SoftwareSerial flutter(5,4);

void setup() {
  // initialize both serial ports:
  Serial.begin(9600);
  flutter.begin(9600);
}

void loop()
{

  flutter.listen();

//  Serial.println(flutter.available());
  
  if (flutter.available()>0) 
  {
    String dataFLUTTER = flutter.readString();

    Serial.println(dataFLUTTER);
    char dataPOST[150];
    dataFLUTTER.toCharArray(dataPOST,sizeof(dataPOST));
    Serial.println(dataPOST); 
//    flutter.end();
    
    if (gsm.begin(2400)) 
    {
      Serial.println("\nstatus=READY");
      started=true;
    } 
    else
    { 
      Serial.println("\nstatus=IDLE");
    }
    
    if(started) 
    {   
          //GPRS attach, put in order APN, username and password.
          //If no needed auth let them blank.
          // if (inet.attachGPRS("ibox.tim.it", "", ""))
          if (inet.attachGPRS("mobile.vodafone.it", "", ""))
          // if (inet.attachGPRS("internet.wind", "", ""))
            Serial.println("status=ATTACHED");
          else Serial.println("status=ERROR");
          delay(1000);

          //Read IP address.
          gsm.SimpleWriteln("AT+CIFSR");
          delay(5000);
          //Read until serial buffer is empty.
          gsm.WhileSimpleRead();

          // TCP Client GET, send a GET request to the server and
          // save the reply.
          // char JsonData[] = "{\"sensor\": 1, \"f_res\": 15, \"s_res\": 35, \"f_hum\": 5, \"s_hum\": 23, \"RSSI\": 235, \"B\": 2}";
          // char JsonData[]= "{\"label\": \"teo\", \"f_res\": 9, \"f_hum\": 9, \"s_res\": 9, \"s_hum\": 9, \"t_res\": 9, \"t_hum\": 9}" ;
           numdata = inet.httpPOST("gregorat.admin.idroplan.org",80,"/measures/create_measure/",dataPOST,msg,50);
          // numdata = inet.httpPOST("admin.idroplan.org",80,"/measures/create_measure/",dataPOST,msg,50);
          // numdata = inet.httpPOST(host,port,end_point,dataPOST,msg,50);
          // numdata = inet.httpPOST(host,port,end_point,JsonData,msg,50);
          // numdata = inet.httpPOST("gregorat.admin.idroplan.org",80,"/measures/create_measure/",JsonData,msg,50);

          //Print the results.
          Serial.println("\nNumber of data received:");
          Serial.println(numdata);
          Serial.println("\nData received:");
          Serial.println(msg);
 
//          digitalWrite(GSM_ON, HIGH);
//          delay(1200);

     }
   }
}
  
void serialhwread()
{
     i=0;
     if (Serial.available() > 0) {
          while (Serial.available() > 0) {
               inSerial[i]=(Serial.read());
               delay(10);
               i++;
          }

          inSerial[i]='\0';
          if(!strcmp(inSerial,"/END")) {
               Serial.println("_");
               inSerial[0]=0x1a;
               inSerial[1]='\0';
               gsm.SimpleWriteln(inSerial);
          }
          //Send a saved AT command using serial port.
          if(!strcmp(inSerial,"TEST")) {
               Serial.println("SIGNAL QUALITY");
               gsm.SimpleWriteln("AT+CSQ");
          }
          //Read last message saved.
          if(!strcmp(inSerial,"MSG")) {
               Serial.println(msg);
          } else {
               Serial.println(inSerial);
               gsm.SimpleWriteln(inSerial);
          }
          inSerial[0]='\0';
     }
}

void serialswread()
{
     gsm.SimpleRead();
}
