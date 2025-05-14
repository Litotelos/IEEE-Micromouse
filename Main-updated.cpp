#include "mbed.h"

// IR Sensors (Pins 16-19)
DigitalIn IR1(p16); // L0 IR (Maps to LED 1)
DigitalIn IR2(p17); // L1 IR (Maps to LED 2)
DigitalIn IR3(p18); // R1 IR (Maps to LED 3)
DigitalIn IR4(p19); // R0 IR (Maps to LED 4)

// // Motors (Pins 21-28)
// Directions
DigitalOut RightMotorDirection(p22); // Direction for M2
DigitalOut LeftMotorDirection(p24); // Direction for M1

// // PWM signals
PwmOut LeftMotorPwm(p21); // Duty cycle for M1
PwmOut RightMotorPwm(p23); // Duty cycle for M2

// // Encoders
InterruptIn LeftEncoderA(p27);
InterruptIn LeftEncoderB(p28);
InterruptIn RightEncoderA(p25);
InterruptIn RightEncoderB(p26);

// // Ultrasonic Sensor (Pin 5-6 )
DigitalOut UltraTrigger(p6);
InterruptIn UltraEcho(p5);


// // Onboard Mbed LED's
DigitalOut Led1(LED1);
DigitalOut Led2(LED2);
DigitalOut Led3(LED3);
DigitalOut Led4(LED4);

// Global variables//
//Timing variables
// ultra
Timer Startup; //Inital cooling period when turning on the mouse *
Timer UltraMapper; // Initates the the Ultrasoninc reading  *
Ticker UltraCheker; // Checks the Distance of the Ultrasonic
Ticker UltraTimeOut;

Ticker MultiplexChecker; //Ticker  to allw Interrupt based components sample at regular intervals *

// //Movement Variables
bool MoveForward = false; // Forward and stopping *
bool LeftMotor = true; // Control over the left Motor *
bool RightMotor= true; // Control over the right Motor *
bool UTurn = true; // Complete 180 degree turns *
int TurnState; // Binary code for choosing turn states
bool MultiplexState = false;  // 0: Sensor multiplexing *


volatile int LeftMotorCounter = 0;      // Encoder global count tracker *
volatile int RightMotorCounter = 0;     // Encoder global count tracker *
volatile bool TurnComplete = false; // Turning flag
const float WheelRadius = 3.0f;
const float WheelBase = 14.0f;
const int WheelResolution = 360;
const float PI = 3.14159265f;


// Encoder counters
volatile int LeftTicks = 0;
volatile int RightTicks = 0;

volatile bool LeftMotorDirectionState = false; // Direction of Left motor *
volatile bool RightMotorDirectionState = false; // Direction of Righ Motor *

// //Sensor variables 
//ultra
volatile int DistanceState = 100; // Ultrasonic Reading Value *
volatile bool UltraComplete = false;
volatile bool UltraReading = false;
volatile int UltraError = 0 ; // Tracks number of times Ultrasonic sensor fails to read
volatile int UltraTime = 0;

//int SensorState;   IR Sensor  Reading Value


// // Sensor state storage
bool IR1Value = false;
bool IR2Value = false;
bool IR3Value = false;
bool IR4Value = false;



// Set the direction and speed of motors based on the motor state
void MotorDirection(bool MoveForward) {

    if (MoveForward) { // Mouse moves forward movement if true
        LeftMotorDirection = 0;
        RightMotorDirection = 0;

        LeftMotorPwm = 0.4;
        RightMotorPwm = 0.4;

    } else { // No movement if  false
        LeftMotorDirection = 1;
        RightMotorDirection = 1;

        LeftMotorPwm = 0;
        RightMotorPwm = 0;
    }
}

// Calculate ticks needed for a turn
int calculateTurnTicks(float degrees) {
    // Calculate arc length for the turn
    float arcLength = (PI * WheelBase * degrees) / 360.0f;
    
    // Calculate wheel revolutions
    float wheelRevs = arcLength / (2.0f * PI * WheelRadius);
    
    // Convert to encoder ticks
    return (int)(wheelRevs * WheelResolution);
}

void stopMotors() {
    LeftMotorPwm = 0;
    RightMotorPwm = 0;
}

void TurnLeft(){
    LeftMotorDirectionState = true;
    RightMotorDirectionState = false;

    LeftMotorDirection = 0;
    LeftMotorPwm = 0.3;
    RightMotorDirection = 1;
    RightMotorPwm = 0.3;

    // Set target rotation (297 degrees)
    float targetDegrees = 285.0f;
    int targetTicks = calculateTurnTicks(targetDegrees);
    // Wait until we've turned enough
    while (abs(LeftTicks) < targetTicks && abs(RightTicks) < targetTicks) {
        // Could add speed balancing here if needed
    }
    
    // Stop when done
    stopMotors();
    // Reset encoder counters
    LeftTicks = 0;
    RightTicks = 0;
}

void LeftEncodeRise() {
    LeftTicks++;
}

void RightEncodeRise() {
    RightTicks++;
}

// void EncoderReset(){
//     LeftMotorCounter = 0;
//     RightMotorCounter = 0;
// }


void TurnAround(bool UTurn){
    LeftMotorDirection = 0;
    LeftMotorPwm = 0.1;

    RightMotorDirection = 1;
    RightMotorPwm = 0.1;
}

// // // Motor Encoder functions
// void LeftEncoderHandler() {//should count whilst turning 
//     if (LeftEncoderB.read() != LeftEncoderA.read()) { //  If B is 0 the wheel is rotating clockwise
//         LeftMotorCounter++;
      
//     } else {
//         LeftMotorCounter--;
//     }
// }
// void RightEncoderHandler(){
// if (RightEncoderB.read() != RightEncoderA.read()) {
//         RightMotorCounter++;    
//     } else {
//         RightMotorCounter--;
//     }
// }

// void MotorHandler(){

//     LeftEncoderA.rise(&LeftEncoderHandler);
//     // LeftEncoderB.fall(&LeftEncoderHandler);

//     RightEncoderA.rise(&RightEncoderHandler);
//     // RightEncoderB.fall(&RightEncoderHandler);
// }
 
void UltraStart(){
    UltraMapper.reset();
    UltraMapper.start();

}

void UltraStop(){
    UltraMapper.stop();
    float Time = UltraMapper.read_us();
    DistanceState = ( Time / 58) ;

    UltraComplete = true;
    UltraReading = false;

}

void UltraHandler(){
    UltraEcho.rise(&UltraStart);
    UltraEcho.fall(&UltraStop);

}
 
void UltraTriggerManager (){

if (UltraReading) return;

UltraComplete = false;
UltraReading = true;
UltraTime = 0;

Led1 = 0;
Led2 = 0;
Led3 = 0;
Led4 = 0;

UltraTrigger = 1;
for (volatile int i = 0; i < 300; i++ ){};
     UltraTrigger = 0;
}

void UltraLeds (){

if (UltraComplete && !UltraReading){

if(DistanceState <= 5 ){
    Led1 = 1;
    Led2 = 0;
    Led3 = 0;
    Led4 = 0;

}
else if (DistanceState < 20) {

    Led1 = 0;
    Led2 = 1;
    Led3 = 0;
    Led4 = 0;
} 
else if (DistanceState < 50) {

    Led1 = 0;
    Led2 = 0;
    Led3 = 1;
    Led4 = 0;
}
else if (DistanceState < 100) {

    Led1 = 0;
    Led2 = 0;
    Led3 = 0;
    Led4 = 1;
}

}

}


//Control turning based on sensor state

// void Turn() {
//     //Check mapping algo 
//     if (DistanceState <= 5) {
//         TurnState = (IR1Value ? 0b0001 : 0) | (IR2Value ? 0b0010 : 0) | (IR3Value ? 0b0100 : 0) | (IR4Value ? 0b1000 : 0);
       
//         switch (TurnState) { 
//             case 0b1100:  
//                 MoveLeft(LeftMotor);
//                 break;

//             case 0b0011:  
//                 MoveRight(RightMotor);
//                 break;
            
//             case 0b1111: 
//                 TurnAround(UTurn);
//                 break;
//         }
//     }
// }


int main() {
      // Initialize motors
    LeftMotorPwm.period(0.0001);
    RightMotorPwm.period(0.0001);
    UltraHandler();
    UltraCheker.attach(&UltraTriggerManager,0.25);
    
    // Set up encoder interrupts
    LeftEncoderA.rise(&LeftEncodeRise);
    RightEncoderA.rise(&RightEncodeRise);
    
    // Reset and end start up timer
    Startup.reset();
    Startup.stop();


    // Main control loop
    while (1) {
    // UltraLeds();
    MoveForward = true;
    MotorDirection(MoveForward);
    // Led1 = 1;
    if (DistanceState <= 4) {
    TurnLeft();
    }

//Error handling 
    if(UltraError >= 3){ // If Ultrasoinc fails to read 3 times flash all led and terminate program.
        Led1 = 1;
        Led2 = 1;
        Led3 = 1;
        Led4 = 1;
        wait_ms(100);
        Led1 = 1;
        Led2 = 1;
        Led3 = 1;
        Led4 = 1;
        wait_ms(100);
        Led1 = 1;
        Led2 = 1;
        Led3 = 1;
        Led4 = 1;
        wait_ms(100);
        return 1;
    }


    }// End of main control loop
} // End of main function
