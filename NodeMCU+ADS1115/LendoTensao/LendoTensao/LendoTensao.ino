#include <Wire.h>
#include <Adafruit_ADS1015.h>

Adafruit_ADS1115 ads;  // Declare an instance of the ADS1115

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

void setup(void)
{
  Serial.begin(9600); 
  Serial.println("Hello!");
  Serial.println("Getting differential reading from AIN0 (P) and AIN1 (N)");
  Serial.println("ADC Range: +/- 2.048V (1 bit = 0.0625mV/ADS1115)");
  ads.setGain(GAIN_TWO); // 2x gain   +/- 2.048V  1 bit = 0.0625mV
  ads.begin();
  
}

void loop(void)
{  
  int16_t calibration = 3000/33; //Nº de voltas do secundário/resistor burden.
  rawADCvalue = ads.readADC_Differential_0_1();
  volts = (rawADCvalue * scalefactor);
  //int16_t Vrms = (calc_Vrms(50)*scalefactor);
  
 // Serial.print("Raw ADC Value = "); 
 // Serial.print(rawADCvalue); 
 // Serial.print("Voltage Measured = ");
 // Serial.println(volts,6);
  Serial.print("Vrms Measured = ");
  Serial.println(calc_Vrms(50)*scalefactor,6);
  Serial.print("Irms measured: ");
  Serial.println(calc_Vrms(50)*scalefactor*calibration,7);
  

  delay(10);
}
