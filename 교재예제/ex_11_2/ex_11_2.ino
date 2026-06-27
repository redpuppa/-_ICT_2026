/*
 예제 11.2
 자동 조명 장치
*/

// 조명이 켜지는 기준값과 꺼지는 기준값을 설정한다
const int ledOnValue = 900;
const int ledOffValue = 850;

// 0번 아날로그핀을 CdS 셀 입력으로 설정한다.
const int CdSPin = 0;

// 조명 상태를 저장할 변수 설정
boolean ledState;

// 샘플 타임 저장용 변수 설정
int sampleTime;
// ADC의 평균값을 저장할 변수 설정
int adcAverage;

void setup()
{
  // 2~9번핀을 모두 출력으로 설정
  for(int i=2 ; i <= 9 ; i++){
    pinMode(i, OUTPUT);
  }
}

void loop()
{
  // ADC의 노이즈 방지를 위하여 
  // adcAverage변수에 조명값을 10회 저장한 후에 평균을 내어 사용한다.
  adcAverage = adcAverage + analogRead(CdSPin);
  ++sampleTime;
  if(sampleTime >= 10){
    adcAverage = adcAverage / 10;
    
    // 조명을 켜고 끄는 기준값을 달리하여 기준점에서 조명이 깜박이는 것을 방지한다
    // 조명이 꺼져있을 때 기준값과 비교하여 조명을 켠다
    if((adcAverage >= ledOnValue)&&(!ledState))	ledOn();
    // 조명이 켜져있을 때 기준값과 비교하여 조명을 끈다
    if((adcAverage <= ledOffValue)&&(ledState))	ledOff();

    adcAverage = 0;
    sampleTime = 0;    
  }
  delay(10);
}

void ledOn(){
  // 조명을 켜는 루틴
  ledState = HIGH;
  for(int i=2 ; i <= 9 ; i++){
    digitalWrite(i,HIGH);
  }	
}

void ledOff(){
  // 조명을 끄는 루틴
  ledState = LOW;
  for(int i=2 ; i <= 9 ; i++){
    digitalWrite(i,LOW);
  }	
}
	
