#include <SPI.h>
#include <L6470.h>

L6470 stepper(10);                  // SS Pin 10

void setup(){
  Serial.begin(9600);

  stepper.init();
  stepper.setAcc(100);              // Set acceleration
  stepper.setMaxSpeed(800);
  stepper.setMinSpeed(1);
  stepper.setMicroSteps(2);         // 1,2,4,8,16,32,64 or 128
  stepper.setThresholdSpeed(1000);
  stepper.setOverCurrent(6000);     // Set overcurrent protection
  stepper.setStallCurrent(3000);

  //stepper.run(1, 200);

  //stepper.softStop();

  stepper.goTo(200);
}

void loop(){
  while (stepper.isBusy()) delay(10);
  stepper.goTo(-200);
  while (stepper.isBusy()) delay(10);
  stepper.goTo(2000);
}
