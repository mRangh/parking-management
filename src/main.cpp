#include <Arduino.h>
#include "classes.hpp"

Led red_led{16};
Led green_led{17};
Led yellow_led{18};
Button b1{19};
Switch sensor{21};
Gate gate{red_led, green_led, yellow_led};

void setup() {
  Serial.begin(115200);
  red_led.set();
  green_led.set();
  yellow_led.set();
  sensor.set();
  b1.set();
}

void loop() {
  if (sensor.get_value() && !gate.print && b1.get_value()){
    gate.print_ticket();
    gate.open();
  } else if (!sensor.get_value() && gate.print){
    gate.close();
    gate.print = 0;
  }
  delay(50);
}