#include "mbed.h"


//9Axis redacted for now
// // 9-Axis sensor (Pins 6-10)
// DigitalIn NineAxisClock(p5);
// DigitalIn NineAxisData(p6);
// DigitalIn NineAxisProtocol(p7);
// InterruptIn NineAxisInterrupt1(p8);
// InterruptIn NineAxisInterrupt2(p9);

// IR Sensors (Pins 16-19)
InterruptIn IR1(p16); // L0 IR (Maps to LED 1)
InterruptIn IR2(p17); // L1 IR (Maps to LED 2)
InterruptIn IR3(p18); // R1 IR (Maps to LED 3)
InterruptIn IR4(p19); // R0 IR (Maps to LED 4)



// Motors (Pins 21-28)
// Directions
DigitalOut RightMotorDirection(p22); // Direction for M2
DigitalOut LeftMotorDirection(p24); // Direction for M1

// PWM signals
PwmOut LeftMotorPwm(p21); // Duty cycle for M1
PwmOut RightMotorPwm(p23); // Duty cycle for M2

// Encoders
InterruptIn LeftEncoderA(p29);
InterruptIn LeftEncoderB(p30);
InterruptIn RightEncoderA(p25);
InterruptIn RightEncoderB(p26);

// Ultrasonic Sensor (Pin 5-6 )
DigitalOut UltraTrigger(p5);
InterruptIn UltraEcho(p6);


// Onboard Mbed LED's
DigitalOut Led1(LED1);
DigitalOut Led2(LED2);
DigitalOut Led3(LED3);
DigitalOut Led4(LED4);

// Global variables//

//Timing variables
Timer Startup; //Inital cooling period when turning on the mouse *
Timer UltraMapper; // Initates the the Ultrasoninc reading  *

Ticker MultiplexChecker; //Ticker  to allw Interrupt based components sample at regular intervals *
Ticker IRcheker; //Checks the states of each IR sensor *

Ticker MotorChecker;
//Movement Variables
bool MoveForward; // Forward and stopping *
bool LeftMotor = true; // Control over the left Motor *
bool RightMotor= true; // Control over the right Motor *
bool UTurn = true; // Complete 180 degree turns *
int TurnState; // Binary code for choosing turn states
bool MultiplexState = false;  // 0: Sensor multiplexing *
int LeftMotorCounter = 0;      // Encoder global count tracker *
int RightMotorCounter = 0;     // Encoder global count tracker *
volatile bool LeftMotorDirectionState = true; // Direction of Left motor *
volatile bool RightMotorDirectionState = true; // Direction of Righ Motor *

//Sensor variables 
volatile int DistanceState = 0; // Ultrasonic Reading Value *
volatile bool UltraSuccess = false;
int UltraError = 0 ; // Tracks number of times Ultrasonic sensor fails to read
volatile bool UltraProcessing = false;



int SensorState;   // IR Sensor  Reading Value


// Sensor state storage
bool IR1Value = false;
bool IR2Value = false;
bool IR3Value = false;
bool IR4Value = false;



// Set the direction and speed of motors based on the motor state
void MotorDirection(bool MoveForward) {

    if (MoveForward) { // Mouse moves forward movement if true
        LeftMotorDirection = 1;
        RightMotorDirection = 1;

        LeftMotorPwm = 0.5;
        RightMotorPwm = 0.5;

    } else { // No movement if  false
        LeftMotorDirection = 1;
        RightMotorDirection = 1;

        LeftMotorPwm = 0;
        RightMotorPwm = 0;
    }
}
void MoveLeft(bool LeftMotor){
    LeftMotorCounter = 3501;
    RightMotorCounter = -3501;
    MoveForward = false;
    while(LeftMotorCounter && RightMotorCounter != 0){
    LeftMotorDirection = 1;
    LeftMotorPwm = 0.8;

    RightMotorDirection = 1;
    RightMotorPwm = 0.4;
    }
}
void MoveRight(bool LeftMotor){
    LeftMotorDirection = 1;
    LeftMotorPwm = 0.4;

    RightMotorDirection = 1;
    RightMotorPwm = 0.8;
}
void TurnAround(bool UTurn){
    LeftMotorDirection = 0;
    LeftMotorPwm = 0.4;

    RightMotorDirection = 1;
    RightMotorPwm = 0.4;
}

// Motor Encoder functions
void LeftEncoderHandler() {//should count whilst turning 
    if (LeftEncoderB.read() != LeftEncoderA.read()) { //  If B is 0 the wheel is rotating clockwise
        LeftMotorCounter++;
      
    } else {
        LeftMotorCounter--;
    }
}
void RightEncoderHandler(){
if (RightEncoderB.read() != RightEncoderA.read()) {
        RightMotorCounter++;    
    } else {
        RightMotorCounter--;
    }
}
void MotorHandler(){

    LeftEncoderA.rise(&LeftEncoderHandler);
    LeftEncoderB.fall(&LeftEncoderHandler);

    RightEncoderA.rise(&RightEncoderHandler);
    RightEncoderA.fall(&RightEncoderHandler);
}

void UltraSense(){
    UltraMapper.start();
}
void UltraDistance(){
        UltraMapper.stop();
        DistanceState = UltraEcho.read() * ((340 / 2) - 1); // -1mm to account for loss of precision
        UltraSuccess = true;
        UltraMapper.reset();
        
    if(UltraEcho.read() != UltraEcho.read()){
        UltraError++; // Incrament failed reading counter
        }
}
 

//The handlers are functions we use if we need to target specific IR sensors
void IR1Handler() {
        IR1Value = IR1.read();
        Led1 = IR1Value; // Turn LED on/off based on sensor state  
}
void IR2Handler() {
        IR2Value = IR1.read();
        Led2 = IR2Value; 
}
void IR3Handler() {
        IR3Value = IR3.read();
        Led3 = IR3Value; 
}
void IR4Handler() {
        IR4Value = IR4.read();
        Led4 = IR4Value; 
    
}

// Multiplexing for sensors
void MultiplexHandler() {
    //State switcher
    MultiplexState = !MultiplexState;
    
    if (MultiplexState == 0){ // Reads sensors in group 0 ( IR 1 and 4)
        IR1Value = IR1.read();
        IR4Value = IR4.read();
        
        // Update LEDs to show when a sensor has been registered as on or off
        Led1 = IR1Value;
        Led4 = IR4Value;
        
         // Turn off LED's outside of group
        Led2 = 0;
        Led3 = 0;
    } else { // Reads sensors in group 1 (IR 2 and 3)
        IR2Value = IR2.read();
        IR3Value = IR3.read();
        
        // Display Active sesnors with LED's
        Led2 = IR2Value;
        Led3 = IR3Value;
        
         // Turn off LED's outside of group
        Led1 = 0;
        Led4 = 0;
    }
    
    
}


// Initialize all the interrupts and tickers
void IRHandler() {
    IR1.rise(&IR1Handler);
    IR1.fall(&IR1Handler);
    
    IR2.rise(&IR2Handler);
    IR2.fall(&IR2Handler);
    
    IR3.rise(&IR3Handler);
    IR3.fall(&IR3Handler);
    
    IR4.rise(&IR4Handler);
    IR4.fall(&IR4Handler);
    
}
// Control turning based on sensor state

void Turn() {
    // Check mapping algo 
    if (DistanceState <= 25) {
        TurnState = (IR1Value ? 0b0001 : 0) | (IR2Value ? 0b0010 : 0) | (IR3Value ? 0b0100 : 0) | (IR4Value ? 0b1000 : 0);
       
        switch (TurnState) {
            case 0b1100:  
                MoveLeft(LeftMotor);
                break;

            case 0b0011:  
                MoveRight(RightMotor);
                break;
            
            case 0b1111: 
                TurnAround(UTurn);
                break;
        }
    }
}

int main() {
//Start startup timer
Startup.start();//Startup timer to allow all comonents to start up.
    // Initialising Components and variables\\
//Sensors
    UltraEcho.rise(&UltraSense);
    UltraEcho.fall(&UltraDistance);
    MultiplexChecker.attach(&MultiplexHandler, 0.05);
    IRcheker.attach(&IR1Handler,0.05);
    MotorChecker.attach(&MotorHandler, 0.05);// maybe not needed

// Reset and end start up timer
Startup.reset();
Startup.stop();
    // Main control loop
    while (1) {
        if (DistanceState < 100 ){
            //Check mapping
            //Check sensors
            //
        }
//Flags
        UltraSuccess = false;
        MoveForward = true;
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
}// End of main function
