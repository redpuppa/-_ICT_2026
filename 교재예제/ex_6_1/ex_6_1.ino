#define ADC_BIT 12
#define RESOLUTION (1<<ADC_BIT)
#define VA_REF  5.0

const int potentioMeterPin = 0;
const int ledPin = 3;

void setup() {
// 시리얼 통신을 설정한다.  
  Serial.begin(9600);
  analogReadResolution(ADC_BIT);
}

void loop(){
  int adcValue; // 실제 센서로부터 읽은 값 (0~1023)
  int duty;     // LED 점멸 주기 (0~100%)
  
// 포텐쇼미터 값을 읽는다.(0~4095)
  adcValue = analogRead(potentioMeterPin);
  float volt = (VA_REF/RESOLUTION) * (float)adcValue;
// 포텐쇼미터 값을 0~255의 범위로 변경한다.
  duty = map(adcValue, 1900, 3400, 0, 100);
  analogWrite(ledPin, duty);
  
// 시리얼 통신으로 ADC 값과 Duty를 출력한다.
  Serial.print("ADC Value is ");
  Serial.print(adcValue);
  Serial.print(". Duty cycle is ");
  Serial.print(duty); 
  Serial.print(". V = ");
  Serial.print(volt);
  Serial.println(" V");
  //delay(1000);
}

