#include <Arduino.h>
#include "classes.hpp"

// LEDs used to indicate gate status and ticket printing.
Led red_led{16};    // Red LED pin for gate closed state.
Led green_led{17};  // Green LED pin for gate open state.
Led yellow_led{18}; // Yellow LED pin for ticket printing indicator.

// Inputs
Button b1{19};      // Button used to request a ticket and open the gate.
Switch sensor{21};  // Entry sensor to detect a vehicle present at the gate.

// Gate controller that drives the LEDs.
Gate gate{red_led, green_led, yellow_led};

void setup() {
  Serial.begin(115200);

  // Initialize all GPIO pins before use.
  red_led.set();
  green_led.set();
  yellow_led.set();
  sensor.set();
  b1.set();
}

void loop() {
  // When a car is detected, a ticket has not yet been printed,
  // and the button is pressed, print a ticket and open the gate.
  if (sensor.get_value() && !gate.print && b1.get_value()){
    gate.print_ticket().open();
  }
  // When the car leaves after printing, close the gate and reset print status.
  else if (!sensor.get_value() && gate.print){
    gate.close();
    gate.print = 0;
  }

  // Small delay for polling cadence.
  delay(50);
}
