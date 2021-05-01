

#include<Arduino.h>
#include<Wire.h>
#include<Servo.h>
#include<SpeedyStepper.h>

#define FRONTRIGHT 2
#define FRONTLEFT 3
#define BACKLEFT 4
#define BACKRIGHT 5
#define TRANSBELT 6
#define DUMPBELT 7
#define MOTOR_STEP_PIN 9
#define MOTOR_DIRECTION_PIN 10
#define PAYLOAD_SIZE 11
#define NODE_ADDRESS 0x51   // Change this unique address for each I2C slave node

// Talon front right and back right
Servo talonFR;
Servo talonBR;

// Talon front left and back left
Servo talonFL;
Servo talonBL;

// Dump belts
Servo transBelt;
Servo dumpBelt;

SpeedyStepper stepper;

int transBeltval = 90;
int dumpBeltval = 90;

int talonValueR = 90;
int talonValueL = 90;

int stepperValue;

char returnData[4];

void setup()
{
  talonFR.attach(FRONTRIGHT);
  talonFL.attach(FRONTLEFT);
  talonBL.attach(BACKLEFT);
  talonBR.attach(BACKRIGHT);

  transBelt.attach(TRANSBELT);
  dumpBelt.attach(DUMPBELT);
  stepper.connectToPins(MOTOR_STEP_PIN, MOTOR_DIRECTION_PIN);
  
  Wire.begin(NODE_ADDRESS);  // Activate I2C network
  //Wire.setSDA(18);
  //Wire.setSCL(19);
  Wire.setClock(100000);
  Serial.begin(115200);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent); // Request attention of master node
}

void loop()
{
  //3 lines take up all the time in the loop, just the nature of sending data unless I find some eeprom exploit
  stepper.setSpeedInStepsPerSecond(100);
  stepper.setAccelerationInStepsPerSecondPerSecond(100);
  stepper.moveRelativeInSteps(stepperValue);
  
  talonFR.write(talonValueR);
  talonBR.write(talonValueR);

  talonFL.write(talonValueL);
  talonBL.write(talonValueL);

  transBelt.write(transBeltval);
  dumpBelt.write(dumpBeltval);

  
  /*
  if(Wire.available() == 0){
    talonFR.write(90);
    talonBR.write(90);

    talonFL.write(90);
    talonBL.write(90);

    transBelt.write(90);
    dumpBelt.write(90);

    Serial.println("Master not detected");
  }
  */
  
  //Serial.printlnz("test");
}

void receiveEvent(int howmany) //howmany = Wire.write()executed by Master
{
  /*
   * Offest sent from jetson incomingData[0]
   * ID integer on incomingData[1]
   * [2-4] actual integer data
   */

  
  byte incomingData[5];
  int intData;
  int digitsPlace = 1;
  for (int i = 0; i < howmany; i++)
  {
    incomingData[i] = Wire.read();
    
  }
  
  for (int i = 0; i < howmany; i++) {
    returnData[i] = incomingData[i];
  }
  returnData[3] = '\0';
  
  for (int i = 3; i > 0; i--) {
    intData += (digitsPlace*incomingData[i]);
    digitsPlace *= 10;
  }
  
  //Serial.println(intData);

  //Belt translation 
  if(incomingData[0] == 3){
    transBeltval = intData; //convert readString into a number
    dumpBeltval = intData/2 + 45;
    Serial.print("trans belt: "); //so you can see the integer
    Serial.println(transBeltval); //so you can see the integer
    Serial.print("dump belt: ");
    Serial.println(dumpBeltval);
  }

  

  //Talons right side drive motors
  if(incomingData[0] == 1){
    talonValueR = intData; //convert readString into a number
    Serial.print("talonR: "); //so you can see the integer
    Serial.println(talonValueR); //so you can see the integer
  }

  //Talons left side drive motors
  if(incomingData[0] == 2){
    talonValueL = intData; //convert readString into a number
    Serial.print("talonL: "); //so you can see the integer
    Serial.println(talonValueL); //so you can see the integer
  }

  
}

void requestEvent()
{
  Wire.write(returnData, PAYLOAD_SIZE);
  Serial.println(returnData);
}
