#include <AccelStepper.h>

// LEDs
#define LED_tracking 5
#define LED_OFF 6
#define LED_slewing 7

// pins for rotary encoder
#define encoderDT 3
#define encoderCLK 2 // Interrupt
#define encoderSW 4
int previousDT_val;
int previousCLK_val;
int previousSW_val;

// pins for stepper motor
#define stepPin 11
#define dirPin 10 

// input buttons
#define slewing_SW 8
#define tracking_SW 9
int previous_slewing_SW_val;
int previous_tracking_SW_val;

float sidereal_step_per_second = -16.3;  // Initial speed for tracking (steps/sec)
float tracking_rotary_change_sps = -0.1; // Every rotary step is a change of rotary_multiplier step/sec in case of tracking mode
float slewing_step_per_second = 0;  // Initial speed for slewing (steps/sec)
float slewing_rotary_change_sps = sidereal_step_per_second; // Every rotary step is a change of rotary_multiplier step/sec in case of slewing mode  (this was -50 in the first version)

float steps_per_second = 0; // Initialization actual speed of stepper motor
float max_steps_per_second = 1000; // Maximum speed possible for stepper motor



// Define the stepper motor and the pins that is connected to
AccelStepper stepper1(1, stepPin, dirPin); // (Type of driver: with 2 pins, STEP, DIR)

void setup() {
  // Setup output status LEDs
  pinMode(LED_tracking, OUTPUT);
  pinMode(LED_OFF, OUTPUT);
  pinMode(LED_slewing, OUTPUT);

  // Setup for rotary encoder
  Serial.begin(9600);
  pinMode(encoderDT, INPUT_PULLUP);
  pinMode(encoderCLK, INPUT_PULLUP);
  pinMode(encoderSW, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(encoderCLK), calcSpeed, CHANGE);
  previousDT_val = digitalRead(encoderDT);
  previousCLK_val = digitalRead(encoderCLK);
  previousSW_val = digitalRead(encoderSW);  

  // Setup for input buttons
  pinMode(slewing_SW, INPUT);
  pinMode(tracking_SW, INPUT);
  
  previous_slewing_SW_val = digitalRead(slewing_SW);
  previous_tracking_SW_val = digitalRead(tracking_SW);

  // Setup LEDs
  int slewing_SW_val = digitalRead(slewing_SW); 
  int tracking_SW_val = digitalRead(tracking_SW); 
  initializeMode(slewing_SW_val,tracking_SW_val);

  // Set maximum speed value for the stepper
  stepper1.setMaxSpeed(max_steps_per_second);    
}

void loop() {
  int actualSW_val = digitalRead(encoderSW); 
  int slewing_SW_val = digitalRead(slewing_SW); 
  int tracking_SW_val = digitalRead(tracking_SW); 

  if (slewing_SW_val != previous_slewing_SW_val || tracking_SW_val != previous_tracking_SW_val || actualSW_val != previousSW_val) {
    initializeMode(slewing_SW_val,tracking_SW_val);
        
    previous_tracking_SW_val = tracking_SW_val;
    previous_slewing_SW_val = slewing_SW_val;
    previousSW_val = actualSW_val;
  }

  stepper1.setSpeed(steps_per_second);
  stepper1.runSpeed();

}



void calcSpeed()
{
  int actualCLK = digitalRead(encoderCLK);
  int actualEncoderDT = digitalRead(encoderDT);
  int slewing_SW_val = digitalRead(slewing_SW); 
  int tracking_SW_val = digitalRead(tracking_SW); 

  if (slewing_SW_val && !tracking_SW_val) { // TRACKING MODE
    if ((actualCLK == 1) and (previousCLK_val == 0))
    {
      if (actualEncoderDT == 1)
        steps_per_second=steps_per_second-0.5*tracking_rotary_change_sps;
      else
        steps_per_second=steps_per_second+0.5*tracking_rotary_change_sps;
    }

    if ((actualCLK == 0) and (previousCLK_val == 1))
    {
      if (actualEncoderDT == 1)
        steps_per_second=steps_per_second+0.5*tracking_rotary_change_sps;
      else
        steps_per_second=steps_per_second-0.5*tracking_rotary_change_sps;
    }
  }
  else if (!slewing_SW_val && tracking_SW_val) { // SLEWING MODE
    if ((actualCLK == 1) and (previousCLK_val == 0))
    {
      if (actualEncoderDT == 1)
        steps_per_second=steps_per_second-0.5*slewing_rotary_change_sps;
      else
        steps_per_second=steps_per_second+0.5*slewing_rotary_change_sps;
    }

    if ((actualCLK == 0) and (previousCLK_val == 1))
    {
      if (actualEncoderDT == 1)
        steps_per_second=steps_per_second+0.5*slewing_rotary_change_sps;
      else
        steps_per_second=steps_per_second-0.5*slewing_rotary_change_sps;
    }
  }    

  // Set correct direction for rotation
  if (steps_per_second < 0)
  {
    digitalWrite(dirPin,HIGH); //Changes the rotation backwards
  }
  if (steps_per_second > 0)
  {
    digitalWrite(dirPin,LOW); //Changes the rotation forward
  }  

  // Enforce maximum speed of rotation
  if (steps_per_second > max_steps_per_second)
  {
    steps_per_second = max_steps_per_second;
  } else if (steps_per_second < -max_steps_per_second) {
    steps_per_second = -max_steps_per_second;
  }

  Serial.print(steps_per_second);
  Serial.println(" steps/sec");

  previousCLK_val = actualCLK;
}

void initializeMode(int slewing_SW_val,int tracking_SW_val)
{

  if (slewing_SW_val && tracking_SW_val){ // OFF
      digitalWrite(LED_tracking, LOW);
      digitalWrite(LED_OFF, HIGH);
      digitalWrite(LED_slewing, LOW);
      Serial.println("Motor turned off!");
      steps_per_second = 0;
    } 
    else if (slewing_SW_val && !tracking_SW_val) { // TRACKING MODE
      digitalWrite(LED_tracking, HIGH);
      digitalWrite(LED_OFF, LOW);
      digitalWrite(LED_slewing, LOW);
      Serial.println("Motor in tracking mode!");
      steps_per_second = sidereal_step_per_second;
    }
    else if (!slewing_SW_val && tracking_SW_val) { // SLEWING MODE
      digitalWrite(LED_tracking, LOW);
      digitalWrite(LED_OFF, LOW);
      digitalWrite(LED_slewing, HIGH);
      Serial.println("Motor in slewing mode!");
      steps_per_second = slewing_step_per_second;
    }    

    digitalWrite(dirPin,LOW); //Changes the rotation direction forward

}