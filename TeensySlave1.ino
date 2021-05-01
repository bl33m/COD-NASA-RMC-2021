/* Teensy Slave 1 which lives the the digging belt
 * Controls the speed of the dig belt,
 * Rotation of the dig belt
 * translation of the dig belt
 * Also returns rotation position
 * Belt translation start and stop
 * dig speed
 */

#include<Arduino.h>
#include<Wire.h>
#include<Servo.h>
#include<SpeedyStepper.h>

#define DIGBELT 2
#define BELTACT 3
#define BACKLEFT 4
#define BACKRIGHT 5
#define SERVOPIN 5
#define MOTOR_STEP_PIN 9
#define MOTOR_DIRECTION_PIN 10
#define PAYLOAD_SIZE 11
#define NODE_ADDRESS 0x60   // Change this unique address for each I2C slave node


Servo digBelt;

Servo beltAct;



SpeedyStepper stepper;

int stepperValue;

int actVal;
int digBeltval;

char returnData[4];

void setup()
{
  digBelt.attach(DIGBELT);
  beltAct.attach(BELTACT);
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
  
  digBelt.write(digBeltval);
  beltAct.write(actVal);

  //talonFL.write(talonValueL);
  //talonBL.write(talonValueL);
  
  //Serial.printlnz("test");
}

void receiveEvent(int howmany) //howmany = Wire.write()executed by Master
{
  /*
   * Offest sent from jetson incomingData[0]
   * ID integer on incomingData[1]
   * [2-4] actual integer data
   * 
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

  //Belt translation via stepper
  if(incomingData[0] == 2){
    stepperValue = intData; //convert readString into a number
    Serial.print("translating: "); //so you can see the integer
    Serial.println(stepperValue); //so you can see the integer
  }


  // Belt Rotation via linear actuator
  if(incomingData[0] == 1){
    actVal=intData;
    Serial.print("Actuator Rotating : "); //so you can see the integer
    Serial.println(actVal); //so you can see the integer
  }


  // Digging Belt
  if(incomingData[0] == 3){
      digBeltval = intData;
    Serial.print("Digging: "); //so you can see the integer
    Serial.println(digBeltval); //so you can see the integer
  }
  
}

void requestEvent()
{
  Wire.write(returnData, PAYLOAD_SIZE);
  Serial.println(returnData);
}
