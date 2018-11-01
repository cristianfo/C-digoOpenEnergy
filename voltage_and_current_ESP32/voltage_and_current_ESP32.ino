// EmonLibrary examples openenergymonitor.org, Licence GNU GPL V3

#include "EmonLib.h"             // Include Emon Library
EnergyMonitor emon1;             // Create an instance
#define ANALOG_PIN_0 36
#define ANALOG_PIN_3 39


void setup()
{  
  Serial.begin(9600);
  analogReadResolution(12);

  
  emon1.voltage(39, 51, 1.7);  // Voltage: input pin, calibration, phase_shift
  emon1.current(36, 5.0);       // Current: input pin, calibration.
  //calibração com 3 voltas, 47/3
}

void loop()
{
  emon1.calcVI(20,2000);         // Calculate all. No.of half wavelengths (crossings), time-out
  emon1.serialprint();           // Print out all variables (realpower, apparent power, Vrms, Irms, power factor)
  
  float realPower       = emon1.realPower;        //extract Real Power into variable
  float apparentPower   = emon1.apparentPower;    //extract Apparent Power into variable
  float powerFActor     = emon1.powerFactor;      //extract Power Factor into Variable
  float supplyVoltage   = emon1.Vrms;             //extract Vrms into Variable
  float Irms            = emon1.Irms;             //extract Irms into Variable

  Serial.print(analogRead(39));
  Serial.print(" ,");
  Serial.println(analogRead(36));  
}
