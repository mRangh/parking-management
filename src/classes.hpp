#include <Arduino.h>
#ifndef CLASSES_HPP
#define CLASSES_HPP

class Input {

    public:

        int pin = 0;
        bool value = 0;
        uint8_t kind;

        Input(int port){
            pin = port;
            kind = INPUT;
            value = 0;
        }
        
        Input& set(){
            pinMode(pin, kind);
            return *this;
        }

        virtual int get_value(){
            value = digitalRead(pin);
            return value;
        }

    private:

};

class Button: public Input {

    public:
        
        Button(int port) : Input(port){}

        bool switch_logic(){
            actual_state = get_value();
            if (actual_state == 1 && last_state == 0) switch_state = !switch_state;
            last_state = actual_state;
            return switch_state;
        }

    private:

        bool switch_state = false;
        bool last_state = false;
        bool actual_state = false;

};

class Switch: public Input {

    public:

        Switch(int port) : Input(port){}

    private:
};

class Multiswitch: public Input {

    public:

        int state;
        int num_of_pos;
        
        Multiswitch(int port, int how_much_states) : Input(port){
            num_of_pos = how_much_states;
        }

        int get_value(){
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

        int analog_step = 0;
        int reading = 0;
};

class Output {
    
    public:

        int pin = 0;
        bool value;
        uint8_t kind;

        Output(int port){
            pin = port;
            kind = OUTPUT;
            value = 0;
        }

        Output& set(){
            pinMode(pin, kind);
            return *this;
        }

        Output& set_value(bool src){
            value = src;
            digitalWrite(pin, value);
            return *this;
        }
    
    private:

};

class Led : public Output {

    public:

        Led(int port) : Output(port){}

        Led& glow(int power){
            if (0 <= power <= 255) analogWrite(pin, power);
            else Serial.println("PWM out of bounds");
            return *this;
        }
    
    private:

};

class Gate {
    
    public:

        Led& red_led, green_led, yellow_led;
        bool status = 0;
        bool print = 0;

        Gate(Led& led1, Led&led2, Led&led3) : red_led(led1) , green_led(led2) , yellow_led(led3) {
            led1.set_value(!status);
            led2.set_value(status);
            led3.set_value(status);
        }

        Gate& open(){
            status = 1;
            green_led.set_value(1);
            red_led.set_value(0);
            return *this;
        }

        Gate& close(){
            delay(1000);
            status = 0;
            green_led.set_value(0);
            red_led.set_value(1);
            return *this;
        }

        Gate& print_ticket(){
            print = 1;
            yellow_led.set_value(1);
            Serial.println("Printing your ticket, please wait a few seconds:");
            for (int i = 3; i >= 0; i--){
                Serial.println(i);
                Serial.print("s");
                delay(1000);
            }
            yellow_led.set_value(0);
            return *this;
        }
};

#endif