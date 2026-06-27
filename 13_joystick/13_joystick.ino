void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  int x_val = analogRead(A0);
  int y_val = analogRead(A1);

  Serial.print("x_val = ");
  Serial.print(x_val);
  Serial.print(", y_val = ");
  Serial.println(y_val);
}
