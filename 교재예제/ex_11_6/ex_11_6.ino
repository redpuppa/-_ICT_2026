/*
 예제 11.6
 제품 신뢰성 시험기
*/

// 내부 EEPROM 사용을 위하여 라이브러리 추가
// EEPROM은 Arduino에 내장되어 있으며
// 전원을 OFF해도 지워지지 않는다
#include <EEPROM.h>
// I2C 통신 라이브러리 설정
#include <Wire.h>
// I2C LCD 라리브러리 설정
#include <LiquidCrystal_I2C.h>

// LCD I2C address 설정
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
//	                    addr, en,rw,rs,d4,d5,d6,d7,bl,blpol

// 키 입력핀 설정 
const int runKey  =           1; 
const int stopKey  =          2;
const int timeUpKey  =        3;
const int timeDownKey  =      4;
const int counterResetKey  =  5;

// 릴레이 출력핀 설정
const int relay1  =  8;
const int relay2  =  9;

// 동작 상태를 저장할 변수 설정
boolean state  =  0;  // 0 is stop, 1 is run

// 키 동작 관련 변수 설정
int keyInput = 0;
int oldKey, newKey;
unsigned char keyOn;
unsigned char keyBouncingTimer;

// 동작 관련 변수 설정
unsigned char timeSec;  //  5~255 sec 
unsigned int timer;  //  5~255 sec 
unsigned int counter;  //  0~65535 sec 
unsigned long samplingTime;

void setup()
{
  // 스위치핀과 릴레이핀 입출력 설정
  pinMode(runKey,INPUT_PULLUP);
  pinMode(stopKey,INPUT_PULLUP);
  pinMode(timeUpKey,INPUT_PULLUP);
  pinMode(timeDownKey,INPUT_PULLUP);
  pinMode(counterResetKey,INPUT_PULLUP);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);  

  // 릴레이 초기 상태는 OFF로 설정 
  relayOff();
  
  // 16X2 LCD 모듈 설정
  lcd.begin(16,2);

  // 초기 메세지 표시
  lcd.clear();  
  lcd.setCursor(4,0);
  lcd.print("ex  10.6");
  lcd.setCursor(3,1);
  lcd.print("AGING  JIG");
  delay(3000);
  lcd.clear();

  // EEPROM에 저장된 값을 읽는다    
  readTimeEEPROM();
  readCounterEEPROM();  
  
  // 초기에 LCD에 표시해야 할 것들을 표시한다.
  lcd.setCursor(0,0);
  lcd.print("Cnt.:");
  lcd.setCursor(15,1);
  lcd.print("s");
  lcd.setCursor(0,1);
  lcd.print("STOP");   
  lcd.setCursor(7,0);
  lcd.print("      ");      
  lcd.setCursor(7,0);
  lcd.print(counter);
  lcd.setCursor(11,1);
  lcd.print("   ");
  lcd.setCursor(11,1);
  lcd.print(timeSec);

}

void loop()
{
  unsigned int i;
  // 1초에 한번씩 loop()가 실행하도록 현재 시간을 저장한다.
  samplingTime = millis();

  // 스위치 입력을 받는다
  keyRead();  
  // 스위치 입력이 있을 경우 keyProcess()를 실행한다
  if(newKey > 0){
    keyProcess();
  };

  // 릴레이를 동작시킨다.    
  relayOperation();
  
  // loop()를 1초에 한번 실행하기 위해서 1초가 될 때 까지 대기한다. 
  while((millis()-samplingTime)<= 1000);  // 5ms sampling time
}

void relayOff(){
  // 릴레이를 OFF 시키는 루틴
  digitalWrite(relay1 , HIGH);
  digitalWrite(relay2 , HIGH);
  // LCD에 릴레이의 동작 상태를 표시한다
  lcd.setCursor(15,0);
  lcd.print("X");
}

void relayOn(){
  // 릴레이를 ON 시키는 루틴
  digitalWrite(relay1 , LOW);
  digitalWrite(relay2 , LOW);
  // LCD에 릴레이의 동작 상태를 표시한다
  lcd.setCursor(15,0);
  lcd.print("R");
}

void keyRead(){
  // 디지털 핀으로 부터 스위치 입력 여부를 확인한다.
  int stopKeyValue = digitalRead(stopKey);
  int runKeyValue = digitalRead(runKey);
  int timeUpKeyValue = digitalRead(timeUpKey);
  int timeDownKeyValue = digitalRead(timeDownKey);
  int counterResetKeyValue = digitalRead(counterResetKey);

  // 입력된 스위치가 있을 경우 newKey 변수에 값을 저장한다  
  if(stopKeyValue == 0){
    newKey = 1;
  }
  else if(runKeyValue == 0){
    newKey = 2;
  }  
  else if(timeUpKeyValue == 0){
    newKey = 3;
  }  
  else if(timeDownKeyValue == 0){
    newKey = 4;
  }  
  else if(counterResetKeyValue == 0){
    newKey = 5;
  }  
  else{
    newKey = 0;
  }  
}

void keyProcess(){
  // 스위치 입력이 있을 경우 실행하는 루틴
  switch  (newKey){
      case 1:  // 동작을 멈춘다
            state = 0;
            lcd.setCursor(0,1);
            lcd.print("STOP"); 
            timer = 0;
            break;    

      case 2:  // 동작을 재개한다.
            state = 1;
            break;    

      case 3:// 한 사이클 주기를 증가시킨다
            if(timeSec < 255){
              ++timeSec;
            }
            else{
              timeSec = 5; 
            }
            lcd.setCursor(11,1);
            lcd.print("   ");
            lcd.setCursor(11,1);
            lcd.print(timeSec);
            saveTimeEEPROM();
            break;    

      case 4://  한 사이클 주기를 감소시킨다
            if(timeSec >5){
              --timeSec;
            }
            else{
              timeSec = 255; 
            }
            lcd.setCursor(11,1);
            lcd.print("   ");
            lcd.setCursor(11,1);
            lcd.print(timeSec);
            saveTimeEEPROM();
            break;    

      case 5://  카운터를 리셋한다
            counter = 0;
            lcd.setCursor(7,0);
            lcd.print("      ");      
            lcd.setCursor(7,0);
            lcd.print(counter);
            saveCounterEEPROM();
            break;    

      default:
            break;    
    }
  // 스위치 입력에 대한 처리가 끝났으면 newKey 변수를 초기화 한다.
  newKey=0;  
}

void relayOperation(){
  // 릴레이 동작 루틴
  if(state == 1){
    // RUN 상태일 때
    lcd.setCursor(0,1);
    lcd.print("    ");
    lcd.setCursor(0,1);
    lcd.print(timer);

      if(timer <= timeSec){
        lcd.setCursor(0,1);
        lcd.print("    ");
        lcd.setCursor(0,1);
        lcd.print(timer);
        // 릴레이는 3초동안 ON 된다.
        if(timer <= 3){
          relayOn();
        }
        else{
          relayOff();
        }    
      ++timer;
      }
      else{
        // 한 사이클이 끝났을 때
        lcd.setCursor(0,1);
        lcd.print("    ");
        lcd.setCursor(0,1);
        lcd.print("UP");    

        timer = 0;
        ++counter;
        saveCounterEEPROM();
      
        lcd.setCursor(7,0);
        lcd.print("      ");      
        lcd.setCursor(7,0);
        lcd.print(counter);
      }  
    };
}

void saveTimeEEPROM(){
  // 시간은 0번 주소에 저장한다 
  EEPROM.write(0,timeSec);  
}  

void readTimeEEPROM(){
  // 0번 주소에서 저장되어 있는 시간을 읽는다
  timeSec = EEPROM.read(0);  
}  

void saveCounterEEPROM(){
  // 2바이트이므로 5 6번 주소에 카운터 값을 저장한다.
  EEPROM.write(5,counter>>8);
  EEPROM.write(6,counter);  
}

void readCounterEEPROM(){
  // 5 6번 주소에서 카운터 값을 읽어낸다
  counter = EEPROM.read(5);
  counter = (counter<<8) + EEPROM.read(6);
} 
