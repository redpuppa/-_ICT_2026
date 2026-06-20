void setup() {
  // put your setup code here, to run once:
  pinMode(3, OUTPUT);
  digitalWrite(3, HIGH);

  analogWrite(5, 0);
}

void loop() {
  // put your main code here, to run repeatedly:
  for(int i=0; i<150; i++){
    analogWrite(5, i);
    delay(1);
  }
}
