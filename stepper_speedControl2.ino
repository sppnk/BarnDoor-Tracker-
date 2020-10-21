
#include <AccelStepper.h>

//motor data
#define FULLSTEP 4
#define HALFSTEP 8  //Stepper library doesnt have this mode, so only 2038 steps/rev. We use 4096 to extreme precision.
#define REWIND_SPEED 700

//BARN DOOR DATA
//when run it has to be mechanically at THETA0 rads
#define RADS_PER_SEC 7.292115e-05 //SIDEREAL SPEED DESIRED - 
#define LENGTH_CM 30 // fill in with precise measured value hinge to bolt axis
// For theta zero, I used relative measurement between two boards w/ level.
// Got 0.72 degrees, which is 0.012566 radians
#define THETA0  0 // fill in with angle at fully closed position (radians)
#define ROTATIONS_PER_CM 8  // M8 tread
#define STEPS_PER_ROTATION 4096.0 //half step
#define CYCLES_PER_SECOND 80000000

#define STOPPED 0
#define RUNNING 1
#define REWIND 2

//PINS conmutated switch, never rew and fw at the same time
#define REWIND_PIN 5
#define FOWARD_PIN 6


//HALFSTEP --> 4096 steps/revo
AccelStepper stepper2(HALFSTEP, 8, 10, 9, 11);

unsigned long currentTime;  // or micros()
unsigned long previousTime;  // or micros()
unsigned long timeRunningFwd = 0; // msecs running forward
unsigned long timeInterval = 5 * 1000; // secs to compute position and change speed
unsigned long stepsDone = 0; // initial position
float actualSpeed = 0 ; // speed in steps per second.
int mode = 0; // 0 = STOP, 1 = RUN, 2 = REWND
bool isDoingStep; // from Accelstepper

// Functions
//Classic barndoor. speed increases with time
//float ypt(float ts) {
//  // bolt insertion rate in cm/s: y'(t) // bolt linear speed depending on t
//  // Note, if you run this for ~359 minutes, it goes to infinity!!
//  //derivative of funtion
//  return (LENGTH_CM / (pow(cos(THETA0 + RADS_PER_SEC * ts), 2))) * RADS_PER_SEC * ROTATIONS_PER_CM * STEPS_PER_ROTATION;
//}

//two pivots barndoor. speed decreases with time
//remember counterweight to easy the motor forces

float ypt(float ts) {
  // bolt insertion rate in cm/s: y'(t) // bolt linear speed depending on t
   //derivative of funtion
  return (LENGTH_CM * (cos((THETA0 + RADS_PER_SEC * ts) / 2)) * RADS_PER_SEC * ROTATIONS_PER_CM * STEPS_PER_ROTATION);
}

// Setup
void setup() {

  pinMode (5, INPUT_PULLUP); // Rewind to initial angle
  pinMode (6, INPUT_PULLUP); // run foward to infinite
  stepper2.setMaxSpeed(1000.0); // 1000 steps/sec
  Serial.begin(9600);
}
  

//Main loop
void loop() {

//read buttons and change mode
  if (digitalRead(REWIND_PIN) == LOW )       {
    mode = REWIND;
    if (stepsDone != 0)  {
      stepper2.setSpeed(REWIND_SPEED); // steps per second REWIND
      if (stepper2.runSpeed ()) stepsDone--;
    }
    else if (stepsDone == 0) {
      mode = STOPPED;
      timeRunningFwd = 0; //reset timer
    }//stop motor at zero position
  }

  //normal foward movement
  else if (digitalRead(FOWARD_PIN) == LOW )       {
    mode = RUNNING;
    stepper2.setSpeed(-actualSpeed);// steps per second RUN FORWARD
    if (stepper2.runSpeed ())    stepsDone++;
  }


  currentTime = millis();

  if ((currentTime - previousTime > timeInterval) ) {

    actualSpeed = ypt(timeRunningFwd / 1000); //calculate new speed
    if (mode == RUNNING) timeRunningFwd += timeInterval; //count time only if RUNNING
    previousTime = currentTime;

//
//    Serial.print("currentTime/1000 ");    Serial.println(currentTime / 1000);
    Serial.print("timeRunningFwd/1000 ");    Serial.println(timeRunningFwd / 1000);
//    Serial.print("actual degrees ");    Serial.println(((THETA0 + RADS_PER_SEC * (timeRunningFwd / 1000)) * 180 / PI));
//    Serial.print("mode ");    Serial.println(mode);
//
    //Serial.print("stepsDone ");    Serial.println(stepsDone);
    Serial.print("revsDone ");    Serial.println(stepsDone/4096.0, 4);
    Serial.print("actualSpeed ");    Serial.println(actualSpeed);
    //Serial.print("sideral calculada  "); Serial.println (24 * 60 * 60((RADS_PER_SEC * (timeRunningFwd / 1000)) * (180 / PI)) / (timeRunningFwd / 1000));

  }

}
