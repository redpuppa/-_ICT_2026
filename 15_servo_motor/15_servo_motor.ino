#include <Servo.h>

Servo motor1;

#define servoMorotPin 9

void setup() {
  // put your setup code here, to run once:
  motor1.attach(servoMorotPin,570,2400);
}

void loop() {
  // put your main code here, to run repeatedly:
  for(int i=0; i<180; i++){
    motor1.write(i);
    delay(10);
  }
}
