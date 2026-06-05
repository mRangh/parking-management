#include <Arduino.h>
#include <vector>
#ifndef CLASSES_HPP
#define CLASSES_HPP

// Base class for any globally tracked hardware object.
// Derived classes should implement `set_hardware()` to configure the
// hardware state for each instance.
class Global {
    
    public:

        // Default virtual destructor allows safe deletion through base pointers.
        virtual ~Global() = default;

        // Register a new object in the global object list.
        // This enables centralized initialization via `Global::set()`.
        Global(Global* new_object){
            if (new_object != nullptr){
                object_list.push_back(new_object);
            }
        }

        // Configure hardware for every registered global object.
        // Call this once after constructing all objects to initialize pins.
        static void set(){
            for (Global* obj : object_list){
                if (obj != nullptr){
                    obj->set_hardware();
                }
            }
        }

        // Derived classes must implement this to initialize their hardware.
        virtual void set_hardware() = 0;

    private:

        // Shared registry of all globally created hardware objects.
        static std::vector<Global*> object_list;

};

// Define the static object list storage for the Global class.
std::vector<Global*> Global::object_list;

// Base class for a digital input pin.
class Input: public Global {

    public:

        int pin = 0;        // ESP32 pin number.
        bool value = 0;     // Last read value.
        uint8_t kind;       // Pin mode, typically INPUT_PULLDOWN.

        virtual ~Input() = default;

        Input(int port, Global* real_object) : Global(real_object){
            pin = port;
            kind = 0x09;
            value = 0;
        }

        // Configure the pin mode on the board.
        void set_hardware() override{
            pinMode(pin, kind);
        }

        // Read the digital state from the pin.
        virtual int get_value(){
            value = digitalRead(pin);
            return value;
        }

    private:

};

// A button with toggle behavior based on rising edge detection.
class Button: public Input {

    public:

        Button(int port) : Input(port, this){}

        // Toggle the switch state when the button is pressed.
        bool switch_logic(){
            actual_state = get_value();
            if (actual_state == 1 && last_state == 0) switch_state = !switch_state;
            last_state = actual_state;
            return switch_state;
        }

    private:

        bool switch_state = 0; // Current toggled state.
        bool last_state = 0;   // Previous raw input read.
        bool actual_state = 0; // Current raw input read.

};

// A simple analogic potentiometer input.
class Potentiometer: public Input {

    public:

        Potentiometer(int port): Input(port, this){}

        int get_value() override{
            value = analogRead(pin); // Analogic reading
            return value;
        }

    private:

};

// A simple digital switch input.
class Switch: public Input {

    public:

        Switch(int port) : Input(port, this){}

    private:

};

// An analog switch that maps an analog reading to discrete switch positions.
class Multiswitch: public Input {

    public:

        int state;
        int num_of_pos;

        Multiswitch(int port, int how_much_states) : Input(port, this){
            num_of_pos = how_much_states;
        }

        int get_value() override{
            analog_step = round(1023.0 / (num_of_pos - 1));
            reading = analogRead(pin);

            for (int i = 0; i < num_of_pos; i++){
                int exp_value = i * analog_step;
                int l_bound = exp_value - (analog_step / 2);
                int u_bound = exp_value + (analog_step / 2);
                if (reading >= l_bound && reading <= u_bound) return i;
            }
            return 0;
        }

    private:

        int analog_step = 0; // Analog range per switch position.
        int reading = 0;     // Raw analog input.

};

class Ultrassonic: public Input {

    public:

        long sound_speed = 0.034; // Sound speed in cm/us

        Ultrassonic(int port) : Input(port, this){}

        int get_value(){
            duration = pulseIn(pin, 1);       // Reads the travel time of the sound wave.
            value = duration * sound_speed/2; // Calculates the distance.
            return value;
        }

    private:

        long duration = 0.0;

};

// Base class for a digital output pin.
class Output: public Global {

    public:

        int pin = 0;        // ESP32 pin number.
        bool value;         // Last written output value.
        uint8_t kind;       // Pin mode, typically OUTPUT.

        virtual ~Output() = default;

        Output(int port, Global* real_object) : Global(real_object){
            pin = port;
            kind = 0x03;
            value = 0;
        }

        // Configure the pin mode as output.
        void set_hardware() override{
            pinMode(pin, kind);
        }

        // Write a digital value to the pin.
        Output& set_value(bool src){
            value = src;
            digitalWrite(pin, value);
            return *this;
        }

    private:

};

// LED output class with optional PWM brightness control.
class Led : public Output {

    public:

        Led(int port) : Output(port, this){}

        // Set LED brightness using PWM. Expected 0-255.
        Led& glow(int power){
            if (0 <= power <= 100) analogWrite(pin, (power*255)/100);
            else Serial.println("PWM out of bounds");
            return *this;
        }

    private:

};

// Gate controller that manages LED indicators and ticket printing.
class Gate {

    public:

        Led& red_led, green_led, yellow_led; // LEDs for closed/open/printing.
        bool status = 0; // Gate status: 0 closed, 1 open.
        bool print = 0;  // Ticket print state.

        Gate(Led& led1, Led&led2, Led&led3) : red_led(led1) , green_led(led2) , yellow_led(led3) {
            // Initialize gate to closed state.
            led1.set_value(!status);
            led2.set_value(status);
            led3.set_value(status);
        }

        // Open the gate and update LEDs.
        Gate& open(){
            status = 1;
            green_led.set_value(1);
            red_led.glow(0);
            return *this;
        }

        // Close the gate with a short delay and update LEDs.
        Gate& close(){
            delay(1000);
            status = 0;
            green_led.set_value(0);
            red_led.glow(100);
            return *this;
        }

        // Simulate a ticket printing sequence.
        Gate& print_ticket(){
            print = 1;
            yellow_led.set_value(1);
            Serial.println("\nPrinting your ticket, please wait a few seconds:");
            for (int i = 3; i >= 0; i--){
                red_led.glow(50);
                Serial.print(i);
                Serial.println("s");
                delay(500);
                red_led.glow(100);
                delay(500);
            }
            Serial.println("You're free to go, have a good day :)\n");
            yellow_led.set_value(0);
            return *this;
        }

    private:

};

#endif
