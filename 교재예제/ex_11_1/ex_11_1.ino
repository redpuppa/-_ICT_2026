/*
 예제 11.1
 배터리 검사기
*/

// LED 핀 설정
const int redLed     =  2;
const int yellowLed  =  3;
const int greenLed   =  4;

// 배터리 전압 핀 설정
const int batteryInput = 0;

// 배터리 전압을 저장할 변수 설정
int batteryVoltage;

void setup()
{
  // LED 핀을 모두 출력으로 설정한다
  pinMode(redLed, OUTPUT);
  pinMode(yellowLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
}

void loop()
{
  // ADC를 이용하여 배터리의 전압 측정
  batteryVoltage = analogRead(batteryInput);

  // 배터리의 전압에 따라 LED 점등
  if(batteryVoltage<=204){
    // 1V 이하일 경우
    digitalWrite(redLed,HIGH);
    digitalWrite(yellowLed,LOW);
    digitalWrite(greenLed,LOW);
  }
  else if((batteryVoltage > 204) && (batteryVoltage <= 306)){
    // 1~1.5V 일 경우
    digitalWrite(redLed,LOW);
    digitalWrite(yellowLed,HIGH);
    digitalWrite(greenLed,LOW);
  }    
  else{
    // 1.5V 이상일 경우
    digitalWrite(redLed,LOW);
    digitalWrite(yellowLed,LOW);
    digitalWrite(greenLed,HIGH);
  }
  delay(100);
}
