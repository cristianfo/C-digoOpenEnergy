// EmonLibrary examples openenergymonitor.org, Licence GNU GPL V3

#include "EmonLib.h"                   // Include Emon Library
EnergyMonitor emon1;                   // Create an instance
int pino_sct = A0;
void setup()
{  
  Serial.begin(9600);
  
  emon1.current(pino_sct, 42.5);             // Current: input pin, calibration.
}

void loop()
{
  double Irms = emon1.calcIrms(1480);  // Calculate Irms only
  
  Serial.print((Irms/3)*230.0);	       // Apparent power
  Serial.print(" ");
  Serial.println(Irms/3);		       // Irms
}
