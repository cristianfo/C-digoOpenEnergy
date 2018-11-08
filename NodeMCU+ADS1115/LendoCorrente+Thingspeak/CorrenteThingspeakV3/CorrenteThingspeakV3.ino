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
#define ADS1015_REG_CONFIG_DR_1600SPS (0xC0);

#define ADC_COUNTS 32768
#define PHASECAL 1.0
//#define VCAL 168.7
#define VCAL 365.3
#define ICAL 3000/33

const char* server = "api.thingspeak.com";
WiFiClient client;

int16_t tensao = 220;
int16_t rawADCvalue;  // The is where we store the value we receive from the ADS1115
float scalefactor = 0.0000625F; // 2x gain   +/- 2.048V  1 bit=0.0625mV
float volts = 0.0; // The result of applying the scale factor to the raw value

int sampleV;                 //sample_ holds the raw analog read value
int sampleI; 
int startV; //Instantaneous voltage at start of sample window.
double filteredI;
double lastFilteredV,filteredV; //Filtered_ is the raw analog value minus the DC offset
double phaseShiftedV; //Holds the calibrated phase shifted voltage.

double offsetV;                          //Low-pass filter output
double offsetI;                          //Low-pass filter output

double realPower,
       apparentPower,
       powerFactor,
       Vrms,
       Irms;
double sqV,sumV,sqI,sumI,instP,sumP; //sq = squared, sum = Sum, inst = instantaneous
boolean lastVCross, checkVCross; //Used to measure number of times threshold is crossed.

double squareRoot(double fg)  
{
  double n = fg / 2.0;
  double lstX = 0.0;
  while (n != lstX)
  {
    lstX = n;
    n = (n + fg / n) / 2.0;
  }
  return n;
}

void calcVI(unsigned int crossings, unsigned int timeout)
{

  unsigned int crossCount = 0;                             //Used to measure number of times threshold is crossed.
  unsigned int numberOfSamples = 0;                        //This is now incremented  

  //-------------------------------------------------------------------------------------------------------------------------
  // 1) Waits for the waveform to be close to 'zero' (mid-scale adc) part in sin curve.
  //-------------------------------------------------------------------------------------------------------------------------
  boolean st=false;                                  //an indicator to exit the while loop

  unsigned long start = millis();    //millis()-start makes sure it doesnt get stuck in the loop if there is an error.

  while(st==false)                                   //the while loop...
  {
     startV = ads.readADC_Differential_2_3();                    //using the voltage waveform
     if ((abs(startV) < (ADC_COUNTS*0.55)) && (abs(startV) > (ADC_COUNTS*0.45))) st=true;  //check its within range
     if ((millis()-start)>timeout) st = true;
  }
  
  //-------------------------------------------------------------------------------------------------------------------------
  // 2) Main measurement loop
  //------------------------------------------------------------------------------------------------------------------------- 
  start = millis(); 

  while ((crossCount < crossings) && ((millis()-start)<timeout)) 
  {
    numberOfSamples++;                       //Count number of times looped.
    lastFilteredV = filteredV;               //Used for delay/phase compensation
    
    //-----------------------------------------------------------------------------
    // A) Read in raw voltage and current samples
    //-----------------------------------------------------------------------------
    sampleV = ads.readADC_Differential_2_3();                 //Read in raw voltage signal
    sampleI = ads.readADC_Differential_0_1();                 //Read in raw current signal

    //-----------------------------------------------------------------------------
    // B) Apply digital low pass filters to extract the 2.5 V or 1.65 V dc offset,
    //     then subtract this - signal is now centred on 0 counts.
    //-----------------------------------------------------------------------------
     offsetV = offsetV + ((sampleV-offsetV)/1024);
  filteredV = sampleV - offsetV;
    offsetI = offsetI + ((sampleI-offsetI)/1024);
  filteredI = sampleI - offsetI;
   
    //-----------------------------------------------------------------------------
    // C) Root-mean-square method voltage
    //-----------------------------------------------------------------------------  
    sqV= filteredV * filteredV;                 //1) square voltage values
    sumV += sqV;                                //2) sum
    
    //-----------------------------------------------------------------------------
    // D) Root-mean-square method current
    //-----------------------------------------------------------------------------   
    sqI = filteredI * filteredI;                //1) square current values
    sumI += sqI;                                //2) sum 
    
    //-----------------------------------------------------------------------------
    // E) Phase calibration
    //-----------------------------------------------------------------------------
    phaseShiftedV = lastFilteredV + PHASECAL * (filteredV - lastFilteredV); 
    
    //-----------------------------------------------------------------------------
    // F) Instantaneous power calc
    //-----------------------------------------------------------------------------   
    instP = phaseShiftedV * filteredI;          //Instantaneous Power
    sumP +=instP;                               //Sum  
    
    //-----------------------------------------------------------------------------
    // G) Find the number of times the voltage has crossed the initial voltage
    //    - every 2 crosses we will have sampled 1 wavelength 
    //    - so this method allows us to sample an integer number of half wavelengths which increases accuracy
    //-----------------------------------------------------------------------------       
    lastVCross = checkVCross;                     
    if (sampleV > startV) checkVCross = true; 
                     else checkVCross = false;
    if (numberOfSamples==1) lastVCross = checkVCross;                  
                     
    if (lastVCross != checkVCross) crossCount++;
  }
 
  //-------------------------------------------------------------------------------------------------------------------------
  // 3) Post loop calculations
  //------------------------------------------------------------------------------------------------------------------------- 
  //Calculation of the root of the mean of the voltage and current squared (rms)
  //Calibration coefficients applied. 
  float multiplier = 0.0000625F;; /* ADS1115 @ +/- 4.096V gain (16-bit results) */
  double V_RATIO = VCAL * multiplier;
  Vrms = V_RATIO * sqrt(sumV / numberOfSamples); 
  
  double I_RATIO = ICAL * multiplier;
  Irms = I_RATIO * sqrt(sumI / numberOfSamples); 

  //Calculation power values
  realPower = V_RATIO * I_RATIO * sumP / numberOfSamples;
  apparentPower = Vrms * Irms;
  powerFactor=realPower / apparentPower;

  //Reset accumulators
  sumV = 0;
  sumI = 0;
  sumP = 0;
//--------------------------------------------------------------------------------------       
}

double calc_Vrms(unsigned int Number_of_Samples)
{
  /* Be sure to update this value based on the IC and the gain settings! */

  double sqV,sumV;
  int16_t sampleV;
  double Vrms;
  
  for (unsigned int n = 0; n < Number_of_Samples; n++)
  {
    sampleV = ads.readADC_Differential_0_1();


    // Root-mean-square method
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
  
  //Calculo da corrente consumida 
  
  int16_t calibration = 3000/33; //Nº de voltas do secundário/resistor burden.
  unsigned long inicio = millis();
  calcVI(20,2000);
  double current =  calc_Vrms(50)*scalefactor*calibration;
  unsigned long fim = millis();  
  unsigned long tempoCalculo=fim-inicio;

  //end Calculo da corrente consumida

  //Calculo da tensão
  
  //end Calculo da tensão
  
 //Envia dados MQTT ao Thingspeak
  
  if (client.connect(server,80)) {  //   "184.106.153.149" or api.thingspeak.com
    String postStr = apiKey;
           postStr +="&field1=";
           postStr += String(Irms);
           postStr +="&field2=";
           postStr += String(Vrms);
           postStr +="&field3=";
           postStr += String(realPower);
           postStr +="&field4=";
           postStr += String(apparentPower);
           postStr +="&field5=";
           postStr += String(powerFactor);
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

  //end Envia dados MQTT ao Thingspeak

  Serial.print("IrmsAntiga measured: ");
  Serial.println(current,7);
  Serial.print("IrmsNova measured: ");
  Serial.println(Irms,7);
  Serial.print("Vrms measured: ");
  Serial.println(Vrms,7);
  Serial.print("Real power measured: ");
  Serial.println(realPower,7);
  Serial.print("Apparent power measured: ");
  Serial.println(apparentPower,7);
  Serial.print("Power factor measured: ");
  Serial.println(powerFactor,7);

  Serial.println("Waiting...");    
  // thingspeak needs minimum 15 sec delay between updates
  delay(16000);  
}
