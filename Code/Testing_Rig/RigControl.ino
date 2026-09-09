#include <EEPROM.h>

//X:
#define dirPin 2
#define stepPin 3
#define limitSwitchPin 10

//Z:
#define dirPinZ 9
#define stepPinZ 8
#define stepsPerRevolution 500
#define stepDelay 600
#define mmToStep 1000
#define ZmmToStep 480

// variable for data collection
int z_impulse_period = 1000; // microsec
// int z_impulse_period = 500; // microsec
// int z_impulse_period = 3000; // microsec
int t_waiting = 16000; // millisec
// int t_waiting = 20000; // millisec
// int t_waiting = 0; // millisec

int homingState = 0;
bool isReturningFromHoming = false;
bool isHomingZ = true;
unsigned long stateChangeTime = 0;
int homingPulseDelay = 50;

long stepCount = 0;
long stepTarget = 0;
long ZstepCount = 0;
long ZstepTarget = 0;
int stepMultiplier = 1; //for step to mm conversion in X
int ZstepMultiplier = 1; //for step to mm conversion in Z
bool isSetHome = true; //distinguish between going home and setting home
bool isGoingLeft = true; //for homing
bool isGoingDown = true; 
unsigned long homeStepCounter = 0;
//unsigned long zStepCounter = 0;
long absolutePos = 0;
long absolutePosZ = 0;




//******
//Driver takes pulse widths no lower than 2.5 microseconds
//dirPin HIGH -> moving away from stepper
//screw pitch 5mm

//Homing States:
//0 - idle
//1 - homing requested
//******

String in_chars = "";


void setup() {

  pinMode(stepPinZ, OUTPUT);
  pinMode(dirPinZ, OUTPUT);

  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(limitSwitchPin, INPUT);

  homingState = 0;



  Serial.begin(115200);
  delay(100);
  Serial.println("");
  Serial.println("");

  Serial.println("------Commands:----------------");
  Serial.println("st+X  (move X steps to the right)");
  Serial.println("st-X  (move X steps to the left)");
  Serial.println("mm+X  (move X millimetres to the right)");
  Serial.println("mm-X  (move X millimetres to the left)");
  Serial.println("setHome  (set current position as home)");
  Serial.println("home  (go to home)");
  Serial.println("zst+X  (move X steps down)");
  Serial.println("zst-X  (move X steps up)");
  Serial.println("zmm+X  (move X millimetres down)");
  Serial.println("zmm-X  (move X millimetres up)");
  Serial.println("zhome  (pull up Z acuator to the limit)");
  Serial.println("-------------------------------");

}

void loop() {
  readSerialCommand();

  if (stepCount < stepTarget && !homingState) {
    delayMicroseconds(50);
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(50);
    digitalWrite(stepPin, LOW);
    stepCount++;
    
    if (isGoingLeft){
      absolutePos--;
    }else{
      absolutePos++;
    }

    if (!(absolutePos % 10) && !isReturningFromHoming){
      Serial.println('x' + String(-absolutePos));
    }
    
    if (stepCount == stepTarget) {
      delay(1000);
      Serial.println("Done");
      Serial.println("-------------------------");
      isReturningFromHoming = false;
      
    }
  }

  if (ZstepCount < ZstepTarget && !homingState) {
    delayMicroseconds(z_impulse_period);
    digitalWrite(stepPinZ, HIGH);
    delayMicroseconds(z_impulse_period);
    digitalWrite(stepPinZ, LOW);
    ZstepCount++;

    if (isGoingDown){
      absolutePosZ++;
    }else{
      absolutePosZ--;
    }

    if (!(absolutePosZ % 6) && !isHomingZ){
      Serial.println(String('z') + String('p') + String(absolutePosZ));
    }
    if (ZstepCount == ZstepTarget) {
      if (isGoingDown){
        delay(t_waiting);
      }
      Serial.println("Done");
      Serial.println("-------------------------");
      if (isHomingZ){
        absolutePosZ = 0;
      }
      isHomingZ = false;
    }
  }

  if (homingState) {
    isReturningFromHoming = true;
    delayMicroseconds(homingPulseDelay);
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(homingPulseDelay);
    digitalWrite(stepPin, LOW);

    if (isSetHome) {
      if (isGoingLeft) {
        homeStepCounter++;
      } else {
        homeStepCounter--;
      }
    }

    //state changes:
    if (homingState == 1 && digitalRead(limitSwitchPin)) {
      //hit switch first time
      homingState = 2;
      //      Serial.println("state 2");
      homingPulseDelay = 300;
      digitalWrite(dirPin, HIGH);
      isGoingLeft = false;
      stateChangeTime = millis();
      //      Serial.println(stateChangeTime);
      delayMicroseconds(500);
    } else if (homingState == 2 && (millis() - stateChangeTime >= 1000)) {
      homingState = 3;
      //      Serial.println("state 3");
      //      Serial.println(millis()-stateChangeTime);
      homingPulseDelay = 1000;
      digitalWrite(dirPin, LOW);
      isGoingLeft = true;

      stateChangeTime = millis();

    } else if (homingState == 3 && digitalRead(limitSwitchPin) == 1 && millis() - stateChangeTime >= 500) {

      if (isSetHome) {
        EEPROM.put(0, homeStepCounter);
      }

      homingState = 0;
      
      //return to home
      digitalWrite(dirPin, HIGH);
      isGoingLeft = false;

      stepCount = 0;
      EEPROM.get(0, stepTarget);
      absolutePos = -stepTarget;
      //      Serial.println(homeStepCounter);
      //      Serial.println(stepTarget);

    }

  }

//  TEST:Loop Z

//  digitalWrite(dirPinZ, zDir);
//
//  digitalWrite(stepPinZ, HIGH);
//  delayMicroseconds(stepDelay);
//  digitalWrite(stepPinZ, LOW);
//  delayMicroseconds(stepDelay);
//  zStepCounter++;
//
//  if (zStepCounter >=  2000){
//    zStepCounter = 0;
//    zDir = !zDir;
//  }



}


void moveStepperFromSerial() {
  if (in_chars[2] == '+' || in_chars[2] == '-') {
    digitalWrite(dirPin, in_chars[2] == '+'); //HIGH if 3rd char is '+'
    isGoingLeft =  in_chars[2] == '-';
    char *dist = &in_chars[3]; //eliminate first two chars
    dist[strlen(dist) - 1] = '\0'; //eliminate \n
    //    Serial.println(atof(dist)*stepMultiplier);
    stepTarget = atof(dist) * stepMultiplier;
    Serial.print("moving [");
    Serial.print(in_chars[2]);
    Serial.print(stepTarget);
    Serial.println("] steps in X...");
    if (stepTarget) {
//      Serial.println("running...");
      stepCount = 0;
    } else {
      Serial.println("[invalid step count!]");
    }
  } else {
    Serial.println("[invalid command!]");
  }
}

void moveZStepperFromSerial() {
  if (in_chars[3] == '+' || in_chars[3] == '-') {
    digitalWrite(dirPinZ, in_chars[3] == '+'); //HIGH if 3rd char is '+'
    isGoingDown = in_chars[3] == '+';
    char *dist = &in_chars[4]; //eliminate first three chars
    dist[strlen(dist) - 1] = '\0'; //eliminate \n
    //    Serial.println(atof(dist)*stepMultiplier);
    ZstepTarget = atof(dist) * ZstepMultiplier;
    Serial.print("moving [");
    Serial.print(in_chars[3]);
    Serial.print(ZstepTarget);
    Serial.println("] steps in Z...");
    if (ZstepTarget) {
//      Serial.println("running...");
      ZstepCount = 0;
    } else {
      Serial.println("[invalid step count!]");
    }
  } else {
    Serial.println("[invalid command!]");
  }
}

void startHoming() {
  homingState = 1;
  stateChangeTime = millis();
  homingPulseDelay = 50;
  digitalWrite(dirPin, LOW);
  isGoingLeft = true;
  homeStepCounter = 0;
  Serial.println("running...");
}


void readSerialCommand() {
  //Read Serial ASCII input
  char in_char = ' ';
  while (Serial.available()) {
    in_char = Serial.read();
    if (int(in_char) != -1) {
      in_chars += in_char;
    }
  }
  // identify commands

  if (in_char == '\n') {
    Serial.print(in_chars); //includes newLine at the end
    if (in_chars[0] == 's' && in_chars[1] == 't') {
      stepMultiplier = 1;
      moveStepperFromSerial();
    } else if (in_chars[0] == 'm' && in_chars[1] == 'm') {
      stepMultiplier = mmToStep;
      moveStepperFromSerial();
    } else if (in_chars[0] == 'z' && in_chars[1] == 's' && in_chars[2] == 't') {
      stepMultiplier = 1;
      moveZStepperFromSerial();
    } else if (in_chars[0] == 'z' && in_chars[1] == 'm' && in_chars[2] == 'm') {
      ZstepMultiplier = ZmmToStep;
      moveZStepperFromSerial();
    } else if (in_chars[0] == 's' && in_chars[1] == 'e') { // originally in_chars == "setHome\n"
      isSetHome = true;
      startHoming();
    } else if (in_chars[0] == 'h' && in_chars[1] == 'o') {    // originally in_chars == "home\n"
      isSetHome = false;
      startHoming();
    } else if (in_chars[0] == 'z' && in_chars[1] == 'h') {   // originally in_chars == "zhome\n"
      ZstepTarget = 5000;
      ZstepCount = 0;
      digitalWrite(dirPinZ, LOW);
    } else {
      Serial.println("[invalid command!]");
    }
    in_chars = "";
  }

}
