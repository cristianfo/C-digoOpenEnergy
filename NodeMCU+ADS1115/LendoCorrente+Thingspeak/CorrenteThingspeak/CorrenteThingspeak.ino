/*
 *  This sketch sends ads1115 current sensor data via HTTP POST request to thingspeak server.
 *  It needs the following libraries to work (besides the esp8266 standard libraries supplied with the IDE):
 *
 *  - https://github.com/adafruit/Adafruit_ADS1X15
 *
 *  designed to run directly on esp8266-01 module, to where it can be uploaded using this marvelous piece of software:
 *
 *  https://github.com/esp8266/Arduino
 *
 *  2015 Tisham Dhar
 *  licensed under GNU GPL
 */

#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_ADS1015.h>

// replace with your channel's thingspeak API key, 
String apiKey = "SXL1QNUHNGDIPRT1";
//WIFI credentials go here
const char* ssid     = "LABIND";
const char* password = "LABINDECA2017";
Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */

const char* server = "api.thingspeak.com";
WiFiClient client;

int16_t tensao = 220;
int16_t rawADCvalue;  // The is where we store the value we receive from the ADS1115
float scalefactor = 0.0000625F; // 2x gain   +/- 2.048V  1 bit=0.0625mV
float volts = 0.0; // The result of applying the scale factor to the raw value

double calc_Vrms(unsigned int Number_of_Samples)
{
  /* Be sure to update this value based on the IC and the gain settings! */

  double sqV,sumV;
  int16_t sampleV;
  double Vrms;
  
  for (unsigned int n = 0; n < Number_of_Samples; n++)
  {
    sampleV = ads.readADC_Differential_0_1();


    // Root-mean-square method current
    // 1) square current values
    sqV = sampleV * sampleV;
    // 2) sum 
    sumV += sqV;
  }
  
  Vrms = sqrt(sumV / Number_of_Samples);

  //Reset accumulators
  sumV = 0;
//--------------------------------------------------------------------------------------       
 
  return Vrms;
}


void setup() {
  Serial.begin(9600);
  delay(10);
 
  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Hello!");
  Serial.println("Getting differential reading from AIN0 (P) and AIN1 (N)");
  Serial.println("ADC Range: +/- 2.048V (1 bit = 0.0625mV/ADS1115)");
  ads.setGain(GAIN_TWO); // 2x gain   +/- 2.048V  1 bit = 0.0625mV
  ads.begin();
}

void loop() {
  
   
  //Serial.print("Differential: "); Serial.print(results); Serial.print("("); Serial.print(trans_volt); Serial.println("mV)");
  int16_t calibration = 3000/33; //Nº de voltas do secundário/resistor burden.
  double current =  calc_Vrms(50)*scalefactor*calibration;

 
  
  if (client.connect(server,80)) {  //   "184.106.153.149" or api.thingspeak.com
    String postStr = apiKey;
           postStr +="&field1=";
           postStr += String(current);
           postStr +="&field2=";
           postStr += String(current*tensao);
           postStr += "\r\n\r\n";
 
     client.print("POST /update HTTP/1.1\n"); 
     client.print("Host: api.thingspeak.com\n"); 
     client.print("Connection: close\n"); 
     client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n"); 
     client.print("Content-Type: application/x-www-form-urlencoded\n"); 
     client.print("Content-Length: "); 
     client.print(postStr.length()); 
     client.print("\n\n"); 
     client.print(postStr);  
  }
  client.stop();

  Serial.print("Irms measured: ");
  Serial.println(calc_Vrms(50)*scalefactor*calibration,7);
  Serial.println("Waiting...");    
  // thingspeak needs minimum 15 sec delay between updates
  delay(16000);  
}
