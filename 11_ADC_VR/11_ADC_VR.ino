#define ADC_BIT 12
#define RESOLUTION (1<<ADC_BIT)
#define VA_REF  5.0

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  analogReadResolution(ADC_BIT);
}

void loop() {
  // put your main code here, to run repeatedly:
  int adcValue = analogRead(A0);
  float volt = (VA_REF/RESOLUTION) * (float)adcValue;

  Serial.print("ADC Value is ");
  Serial.println(adcValue);
  Serial.print("V = ");
  Serial.print(volt);
  Serial.println(" V");

  delay(1000);
}
