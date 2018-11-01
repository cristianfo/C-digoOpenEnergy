// EmonLibrary examples openenergymonitor.org, Licence GNU GPL V3

#include <driver/adc.h>
#define ANALOG_PIN_0 36
#define ANALOG_PIN_3 39


void setup()
{  
  Serial.begin(9600);

   // Current: input pin, calibration.
  //calibração com 3 voltas, 47/3
}

void loop()
{
  Serial.println(analogRead(ANALOG_PIN_3));
  Serial.println(analogRead(ANALOG_PIN_0));
  delay(100);
}
