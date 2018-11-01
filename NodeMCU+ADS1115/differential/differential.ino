#include <Wire.h>
#include <Adafruit_ADS1015.h>

 Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */



double calc_mVrms(unsigned int Number_of_Samples)
{
  /* Be sure to update this value based on the IC and the gain settings! */
  float multiplier = 0.0625F;    /* 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV*/

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


void setup(void)
{
  Serial.begin(9600);
  Serial.println("Hello!");
  
  Serial.println("Getting differential reading from AIN0 (P) and AIN1 (N)");
  Serial.println("ADC Range: +/- 2.048V (1 bit = 1mV/ADS1015, 0.0625mV/ADS1115)");
  
  // The ADC input range (or gain) can be changed via the following
  // functions, but be careful never to exceed VDD +0.3V max, or to
  // exceed the upper and lower limits if you adjust the input range!
  // Setting these values incorrectly may destroy your ADC!
  //                                                                ADS1015  ADS1115
  //                                                                -------  -------
  // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  // ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
  
  ads.begin();
}

void loop(void)
{
  int16_t results;
  
  /* Be sure to update this value based on the IC and the gain settings! */

  float multiplier = 0.0625F; /* ADS1115  @ +/- 6.144V gain (16-bit results) */

  results = ads.readADC_Differential_0_1();  
    
  Serial.print("Differential: "); Serial.print(results); Serial.print("("); Serial.print(results * multiplier); Serial.println("mV)");

  double voltage = calc_mVrms(50);//2048
  Serial.print("Tensão RMS: ");
  Serial.println(voltage);
   Serial.print("Corrente RMS: ");
  Serial.println(voltage/(3*33));

  delay(1);
}
