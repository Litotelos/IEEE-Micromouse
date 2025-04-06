#include <atomic>

#include "mbed.h"
// 9 - Axis sensor (Pins 5 - 9)
DigitalIn Nine_Clock (p5);
DigitalIn Nine_Data (p6);
DigitalIn Nine_Protocol(p7);
DigitalIn  Nine_Interrupt_one(p8);
DigitalIn Nine_Interrupt_two(p9);

//IR Sensors (Pins 10 - 13)
DigitalIn IR1 (p10); // L0 IR (Maps to LED 1)
DigitalIn IR2 (p11); // L1 IR (Maps to LED 2)
DigitalIn IR3 (p12); // R1 IR (Maps to LED 3)
DigitalIn IR4 (p13); // R0 IR (Maps to LED 4)

// Ultrasonic Sensors (Pin 14)
DigitalIn Ultra (p14); // Ultrasonic sensor

// Pins 15,16,17,18 Unused

//Motors
DigitalOut Right_Motor_Direction(p19); // Direction for M2
DigitalOut Left_Motor_Direction(p20); // Direction for M1
PwmOut Left_Motor_Pwm (p21); // Duty cycle for M1
PwmOut Right_Motor_Pwm(p22); // Duty cycle for M2
InterruptIn Left_Encoder_A(p23);
InterruptIn Left_Encoder_B(p24);
InterruptIn Right_Encoder_A(p25);
InterruptIn Right_Encoder_B(p26);

//Pins 27,28,29,30 Unused

// Onboard Mbed LED's
DigitalOut Led_F1(LED1);
DigitalOut Led_F2(LED2);
DigitalOut Led_F3(LED3);
DigitalOut Led_F4(LED4);


// global variables
bool Motor_State; // Forward and stopping
int Distance_State; // Turning
int Sensor_State; // Reading sensors

int Left_Motor_Counter = 0; // Encoder global count tracker
int Right_Motor_Counter = 0; // Encoder global count tracker
volatile bool Left_Motor_Direction_State = true;
volatile bool Right_Motor_Direction_State = true;

int Angle_State; // Alignment tracking


//Motor Functions
void Left_Encoder_State()
{
    if (Left_Encoder_B.read() == 0)
        {
        Left_Motor_Counter++;
        Left_Motor_Direction_State = true;
    }
    else {
        Left_Motor_Counter--;
        Left_Motor_Direction_State = false;
    }
}// End of Encoder function

void Right_Encoder_State()
{
    if (Right_Encoder_B.read() == 0)
    {
        Right_Motor_Counter ++;
        Motor_State = true;
        Right_Motor_Direction_State = true;
    }
    else {
        Right_Motor_Counter --;
         Right_Motor_Direction_State = true;
    }
 } // End Of Encoder Function


int Direction_state(bool Motor_State) {

    if (Motor_State == 1) // Forward movement if motors are true
    {
        Left_Motor_Direction = 1;
        Right_Motor_Direction = 1;

        Left_Motor_Pwm = 0.5;
        Right_Motor_Pwm = 0.5;

    }
    else // No  movement if the motors are false
    {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  
        Right_Motor_Direction = 1;

        Left_Motor_Pwm = 0;
        Right_Motor_Pwm = 0;
    }
    return Motor_State;
}
//UPDATE 6/04/25 Im going to add the encoder counter to determine distance. I just need to know encoder res and wheel size. But im gonna go study for algo now.
void Turn_state(int Sensor_State) {
    switch (Sensor_State) // Determines the direction of each wheel dynamically
    {
        case 1100: // Turn left
        // Indicate direction of turn
        Led_F1 = 1;
        Led_F2 = 1;

        // Set Motors to turn into the direction
        Left_Motor_Direction = 1;
        Right_Motor_Direction = 1;
        Left_Motor_Pwm=0;
        Right_Motor_Pwm = 0.5;

        // Once complete turn off Led's
        Led_F1 = 0;
        Led_F2 = 0;
        break;

        case 0011: // Turn right

        // Indicate direction of turn
        Led_F3 = 1;
        Led_F4 = 1;
        //Set Motor direction
        Left_Motor_Direction = 1;
        Right_Motor_Direction = 1;
        Left_Motor_Pwm=0.5;
        Right_Motor_Pwm = 0;

        // Once complete turn off Led's
        Led_F3 = 0;
        Led_F4 = 0;
        break;

        case 1111: // U Turn
        // Indicate Direction of turn
        Led_F1 = 1;
        Led_F2 = 1;
        Led_F3 = 1;
        Led_F4 = 1;

        // Set motor direction
        Left_Motor_Direction = 0;
        Right_Motor_Direction = 1;
        Left_Motor_Pwm= 0.5;
        Right_Motor_Pwm = 0.5;

        // Once turn complete turn off Led's
        Led_F1 = 0;
        Led_F2 = 0;
        Led_F3 = 0;
        Led_F4 = 0;
        break;
    }
}

// Sensor Functions
int Sensor_Reading(int Sensor_state)
    {
    // Reset readings
    std::atomic_uint8_t S_state; // private variable used to express complete sensor states
    S_state  &= 0x0F; // Enforcing a 4 bit capacity to S_state

    // Read sensors
    bool IR1_val=IR1.read();
    bool IR2_val=IR2.read();
    bool IR3_val=IR3.read();
    bool IR4_val=IR4.read();

    //Mapping states to S_state
    S_state = ( (IR1_val ? 0b0001 : 0) | (IR2_val ? 0b0010 : 0) | (IR3_val ? 0b0100 : 0) | (IR4_val ? 0b1000 : 0)); // S_State now represents the binary state of all 4 IR sensors
    Sensor_state = S_state;
    return(Sensor_state);
   }

int Distance_readings(int Distance_state) // Distance function using the Ultrasonic sensor
{
 Distance_state = Ultra.read() - 2; // Subtracting the tolerance
    return(Distance_state);
}


int main() {
    while (1) { // Superloop Used during mouses operations
        Motor_State = 1;
        Sensor_State = Sensor_Reading(Sensor_State);
    if (Sensor_State != 0)
    {
        Turn_state(Sensor_State);
    }
    else
    {
        Direction_state(1);
    }
        Encoder_State(Motor_State,Left_Motor_Counter,Right_Motor_Counter)


    }//End of while loop
}// End of main function