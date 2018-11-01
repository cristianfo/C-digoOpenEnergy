// EmonLibrary examples openenergymonitor.org, Licence GNU GPL V3

#include "EmonLib.h"                   // Include Emon Library
EnergyMonitor emon1;                   // Create an instance
int pino_sct = A7;
void setup()
{  
  Serial.begin(9600);
  
  emon1.current(pino_sct, 42.5);             // Current: input pin, calibration.
  //Pino, calibracao - Cur Const= Ratio/BurdenR. 2000/47 = 42.5
}

void loop()
{
  double Irms = emon1.calcIrms(1480);  // Calculate Irms only
  
  Serial.print("VA: ");	       // Apparent power
  Serial.println((Irms/3)*230.0);
  Serial.print("Corrente: ");		       // Irms
  Serial.println(Irms/3);
}
