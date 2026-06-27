/*
  예제 11.3
  자동차용 초음파 후방감지기
 */

// 부저 핀, 트리거 핀, 에코 핀 번호를 설정한다.
const char buzzerPin = 9;
const char trigPin = 13;
const char echoPin = 12;

 
// 거리 측정에 필요한 펄스 폭과 거리 변수 설정  
int pulseWidth;
int distance;
int distanceOld;

// 부저 출력 주기 변수 설정
int buzzerDuration;
  
void setup() {
  // 시리얼 통신 설정
  Serial.begin (9600);
  // 트리거 핀은 출력으로, 에코핀은 입력으로 설정
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  // 부저핀을 출력으로 설정한다
  pinMode(buzzerPin, OUTPUT);
  // 트리거 핀의 초기값을 LOW로 한다
  digitalWrite(trigPin, LOW);
}

void loop(){
  // 10us의 트리거 신호를 HC-SR04로 내보낸다.
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Echo 펄스 폭을 측정하여 pulseWidth 변수에 저장한다.  
  pulseWidth = pulseIn(echoPin, HIGH);
  // 거리를 계산한다.
  distance = pulseWidth / 58;  

  // 감지거리인 2~200cm 범위의 거리값만 사용한다.
  if(distance <= 100 || distance >= 2){  
    // 각 거리 별로 부저 주기를 설정한다
    if((distance <= 100)&&(distance >= 50)){ // 1~0.5m
      buzzerDuration = 1000;    
      buzzerOn();
    }	
    else if((distance < 50)&&(distance >= 20)){ // 0.5~0.2m
      buzzerDuration = 500;    
      buzzerOn();
    }
    else if(distance < 20){ // 0.2m 이내
      buzzerDuration = 100;
      buzzerOn();      
    }
    else{
      noTone(buzzerPin);  
    }
    delay(buzzerDuration);
  };
}

void buzzerOn(){
    // 523Hz로 0.1초간 부저 동작
    tone(buzzerPin, 523);
    delay(100);
    noTone(buzzerPin);  
}
