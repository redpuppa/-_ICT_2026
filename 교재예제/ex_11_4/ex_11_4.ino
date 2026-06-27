/*
예제 11.4
RFID를 이용한 도어락
*/

// SPI 라이브러리를 불러온다.
#include <SPI.h>
// MFRC522 라이브러리를 불러온다
#include <MFRC522.h>

// 서보모터 라이브러리 불러오기
#include <Servo.h>

// SS 핀을 10번 핀으로 설정한다.
#define SS_PIN 10    //Arduino Uno
// Reset 핀을 9번 핀으로 설정한다.
#define RST_PIN 9
// 부저 핀을 5번으로 설정한다.
#define buzzerPin 5
// 서보모터 핀을 9번을 설정한다.
#define servoMotorPin 3
// 스위치 핀을 2번으로 설정한다.
#define switchPin 2

// 도어 상태에 쓰일 상수를 설정한다.
#define OPEN 1
#define CLOSE 0

#define doorCloseAngle 0;
#define doorOpenAngle 90;

// mfrc522란 이름의 RFID 판독기를 설정한다.
MFRC522 mfrc522(SS_PIN, RST_PIN);

// motor1으로 서보모터 이름 설정
Servo doorMotor; 

// 도어가 열려있는 시간을 측정하는 타이머 
int doorOpenTimer = 0;

// RFID를 저장할 변수 설정
byte rfidDoorBuff[8]={0,0,0,0,0,0,0,0};
byte rfidDoor[8]={0,0,0,0,0,0,0,0};

// 도어 상태를 저장할 변수 설정
boolean doorState = OPEN;
// rfid 비교용 변수 설정
boolean rfidIsOk = 0;

void setup(){
  Serial.begin(9600);
  // RFID 모듈과의 통신을 위해 SPI 통신을 시작한다.
  SPI.begin();
  // 앞서 설정한  mrfc522란 이름의 RFID 판독기를 시작한다.
  mfrc522.PCD_Init();        // Init MFRC522 card
  // 서보모터 설정. 0.6ms 부터 2.4ms 범위로 설정
  doorMotor.attach(servoMotorPin,600,2400);	
  // 스위치 핀을 입력으로 설정한다.
  pinMode(switchPin, INPUT_PULLUP);
  // 부저핀을 출력으로 설정한다
  pinMode(buzzerPin, OUTPUT);
  Serial.println("**************************");
  Serial.println("* EX 10.4 RFID DOOR LOCK *");
  Serial.println("**************************");
  delay(1000);
  doorClose();
}

void loop() {
  // 새로운 카드를 기다린다.
  if (mfrc522.PICC_IsNewCardPresent()){
    rfidCardRead();
  };
  // 5초이상 문이 열려있으면 문을 닫는다.
  if(doorState == OPEN){
    delay(100);
    ++doorOpenTimer;
    if(doorOpenTimer >= 50)  doorClose();
  }
  else  doorOpenTimer = 0;
}

void rfidCardRead(){
  // 카드를 읽는다.
  if (!mfrc522.PICC_ReadCardSerial())  return;

  // 판독기에 입력된 UID의 바이트수 만큼 읽어낸다.
  // UID는 카드의 종류에 따라 최대 8바이트를 갖는다.
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    rfidDoorBuff[i] = mfrc522.uid.uidByte[i];
  }
  // UID가 8 바이트가 안될 경우 나머지는 0으로 채운다.
  for (byte i = mfrc522.uid.size; i < 8; i++) {
    rfidDoorBuff[i] = 0;
  }
  
  Serial.println();
  Serial.print("Card UID      : ");
  for (byte i = 0; i < 8; i++) {
    Serial.print(rfidDoorBuff[i],HEX);
  } 
  
  Serial.println();
  Serial.print("Door Lock UID : ");
  for (byte i = 0; i < 8; i++) {
    Serial.print(rfidDoor[i],HEX);
  } 
  Serial.println();  
  
  if(digitalRead(switchPin)){// 스위치를 누르지 않았을 때는 오픈/클로즈 동작
    Serial.println("SW OFF");
    for (byte i = 0; i < 8; i++) {
      // 8 바이트의 UID를 비교한다.
      if(rfidDoor[i] == rfidDoorBuff[i]){
        // 모두 일치할 경우 rfidIsOk 변수는 8이된다
        ++rfidIsOk;
      };
    }     
    // rfidIsOk 변수가 8일 경우 도어를 연다
    if(rfidIsOk == 8){
      doorOpen();
      rfidIsOk = 0;
    }
    else  rfidIsOk = 0;
  }
  else{// 스위치를 눌렀을 때는 입력된 RFID를 도어 열림키로 저장
    Serial.println("SW ON");
    for (byte i = 0; i < 8; i++) {
      rfidDoor[i] = rfidDoorBuff[i];
    } 
    rfidSaveBuzzer();
  }

  // mfrc522 판독기를 초기화 한다
  mfrc522.PICC_HaltA(); // Halt PICC
  mfrc522.PCD_StopCrypto1();  // Stop encryption on PCD
}

void doorOpen(){
  // 도어를 여는 루틴
  if(doorState == CLOSE){
    Serial.println("Door is OPENED");
    // 서보모터의 각도를 90도로 한다.
    doorMotor.write(90);
    openBuzzer();
    doorState = OPEN;
  };    
}

void doorClose(){
  // 도어를 닫는 루틴  
  if(doorState == OPEN){
    Serial.println("Door is CLOSED");
    // 서보모터의 각도를 0도로 한다.
    doorMotor.write(0);
    closeBuzzer();
    doorState = CLOSE;
  };    	
}

void openBuzzer(){
  // 도레미 부저음을 낸다  
  tone(buzzerPin, 262, 200);
  delay(200);
  tone(buzzerPin, 294, 200);
  delay(200);
  tone(buzzerPin, 330, 200);
  delay(200);
  noTone(buzzerPin);  
}

void closeBuzzer(){
  // 미레도 부저음을 낸다
  tone(buzzerPin, 330, 200);
  delay(200);
  tone(buzzerPin, 294, 200);
  delay(200);
  tone(buzzerPin, 262, 200);
  delay(200);
  noTone(buzzerPin);  
}

void rfidSaveBuzzer(){
  // 도도 부저음을 낸다
  tone(buzzerPin, 523, 200);
  delay(200);
  tone(buzzerPin, 523, 200);
  delay(200);
  noTone(buzzerPin); 	
}	



