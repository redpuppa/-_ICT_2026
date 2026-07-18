const int motorPin[4] = {8,9,10,11};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  for(int i=0; i<4; i++){
    pinMode(motorPin[i], OUTPUT);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  int adcValue = analogRead(A0); //어떤 의미
  int delay_val = map(adcValue, 0, 1023, 1000, 2000);
  Serial.println(delay_val);

  digitalWrite(motorPin[0], HIGH);
  digitalWrite(motorPin[1], LOW);
  digitalWrite(motorPin[2], LOW);
  digitalWrite(motorPin[3], LOW);
  delayMicroseconds(delay_val);
  digitalWrite(motorPin[0], LOW);
  digitalWrite(motorPin[1], HIGH);
  digitalWrite(motorPin[2], LOW);
  digitalWrite(motorPin[3], LOW);
  delayMicroseconds(delay_val);
  digitalWrite(motorPin[0], LOW);
  digitalWrite(motorPin[1], LOW);
  digitalWrite(motorPin[2], HIGH);
  digitalWrite(motorPin[3], LOW);
  delayMicroseconds(delay_val);
  digitalWrite(motorPin[0], LOW);
  digitalWrite(motorPin[1], LOW);
  digitalWrite(motorPin[2], LOW);
  digitalWrite(motorPin[3], HIGH);
  delayMicroseconds(delay_val);
}
