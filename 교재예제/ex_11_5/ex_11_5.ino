/*
 예제 11.5
 LCD 시계와 온습도계
*/

// DS1302 라이브러리를 불러온다
#include <DS1302.h>
// I2C 통신 라이브러리 설정
#include <Wire.h>
// I2C LCD 라리브러리 설정
#include <LiquidCrystal_I2C.h>
// DHT 라이브러리를 추가한다
#include <dht.h>

// DS1302 핀 설정
const int CEPin = 5;
const int IOPin = 6;
const int CLKPin = 7; 

// DHT11이 연결된 핀을 설정한다
int DHT11Pin = 2;

// LCD I2C address 설정 PCF8574:0x27, PCF8574A:0x3F
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
//	               addr, en,rw,rs,d4,d5,d6,d7,bl,blpol

// DS1302 모듈을 rtc 오브젝트로 설정
DS1302 rtc(CEPin, IOPin, CLKPin);

// DHT11 이란 이름으로 센서 이름을 설정한다
dht DHT11;

// 현재 시간과 날짜와 과거 시간과 날짜를 저장할 변수 설정
int nowYear, nowMonth, nowDay, nowHour, nowMinute, nowSecond;
int pastYear, pastMonth, pastDay, pastHour, pastMinute, pastSecond;

// 현재 요일과 과거 요일을 저장할 변수 설정
String nowWeek, pastWeek;

// 현재 온도, 습도와 과거 온도, 습도를 저장할 변수 설정 
int nowTemp, nowHumidity;
int pastTemp, pastHumidity;

// 10회 읽은 값을 평균내어 사용하기 위한 변수 설정
char sampleTime;
int tempAverage, humidityAverage;

// DS1302에서 읽어낸 데이터 중 day 값으로 요일을 설정한다
String dayAsString(const Time::Day day) {
  switch (day) {
    case Time::kSunday: return "SUN";
    case Time::kMonday: return "MON";
    case Time::kTuesday: return "TUE";
    case Time::kWednesday: return "WED";
    case Time::kThursday: return "THU";
    case Time::kFriday: return "FRI";
    case Time::kSaturday: return "SAT";
  }
  return "---";
}

void setup(){
  // 시리얼 통신 설정
  Serial.begin(9600);

  // 16X2 LCD 모듈 설정
  lcd.begin(16,2);  

  // 초기 메세지 표시
  lcd.clear();  
  lcd.setCursor(4,0);
  lcd.print("ex  10.5");
  lcd.setCursor(1,1);
  lcd.print("ARDUINO  CLOCK");
  delay(3000);

  // 현재 시간을 입력한다.
  // 시간이 맞춰져 있을 경우 주석처리 한다.
  //  setTime();

  // LCD 초기 표시값들을 표시한다
  lcdDefault();
}

void loop() {
  // 시간, 날짜 정보를 읽어온다.
  readTime();
  // 온도, 습도 정보를 읽어온다.
  tempAndHumidityRead();
  // LCD에 표시한다.  
  lcdDisplay();
  // 데이터의 변화를 감지하기 위해 변수에 저장한다.
  saveOldData();
  delay(100);
}

void setTime(){
  // rtc에 쓰기방지 OFF, 정지기능 OFF
  rtc.writeProtect(false);
  rtc.halt(false);

  // 입력하고자 하는 시간을 적는다
  // 2015년 7월 21일 18시 48분 0초 화요일 로 설정한다
  Time t(2015, 7, 21, 18, 48, 00, Time::kTuesday);

  // 현재의 시간을 DS1302에 입력한다
  rtc.time(t);  
}

void readTime(){
  // DS1302로 부터 현재 시간, 날짜 정보을 읽어온다
  Time t = rtc.time();
  // day 데이터를 요일로 변경한다
  const String day = dayAsString(t.day);
  nowWeek = day.c_str();

  // 시간, 날짜 정보를 저장한다.  
  nowYear = t.yr;
  nowMonth = t.mon;
  nowDay = t.date;
  nowHour = t.hr;
  nowMinute = t.min;
  nowSecond = t.sec;
}  



void tempAndHumidityRead(){
  // check라는 변수에 DHT11의 상태를 읽어 저장한다
  int check = DHT11.read11(DHT11Pin);
 
  if(check == DHTLIB_OK){
    // DHT11에서 정상적으로 데이터를 읽어오면
    // 온도와 습도 정보를 변수에 저장한다
    ++sampleTime;
    tempAverage = tempAverage + DHT11.temperature;
    humidityAverage = humidityAverage + DHT11.humidity;
    if(sampleTime >= 10){
      nowTemp = tempAverage / 10;
      nowHumidity = humidityAverage / 10;
      tempAverage = 0;
      humidityAverage = 0;
      sampleTime = 0;
    };
  };
  delay(50);
}  

void lcdDisplay(){
  // LCD에 표시는 값이 변화된 데이터만 갱신해 준다
  if(nowYear != pastYear){
    lcd.setCursor(1,0);
    lcd.print("    ");
    lcd.setCursor(1,0);
    lcd.print(nowYear);
  };

  if(nowMonth != pastMonth){
    lcd.setCursor(6,0);
    lcd.print("  ");
    if(nowMonth >= 10)  lcd.setCursor(6,0);
    else lcd.setCursor(7,0);
    lcd.print(nowMonth); 
  };

  if(nowDay != pastDay){
    lcd.setCursor(9,0);
    lcd.print("  ");
    if(nowDay >= 10)  lcd.setCursor(9,0);
    else  lcd.setCursor(10,0);
    lcd.print(nowDay); 
  };

  if(nowHour != pastHour){
    lcd.setCursor(8,1);
    lcd.print("  ");
    if(nowHour >= 10)  lcd.setCursor(8,1);
    else  lcd.setCursor(9,1);
    lcd.print(nowHour); 
  };

  if(nowMinute != pastMinute){
    lcd.setCursor(11,1);
    lcd.print("  ");
    if(nowMinute >=10 )  lcd.setCursor(11,1);
    else  lcd.setCursor(12,1);
    lcd.print(nowMinute); 
  };

  if(nowSecond != pastSecond){
    lcd.setCursor(14,1);
    lcd.print("  ");
    if(nowSecond >= 10)  lcd.setCursor(14,1);
    else  lcd.setCursor(15,1);
    lcd.print(nowSecond); 
  };

  if(nowTemp != pastTemp){
    lcd.setCursor(0,1);
    lcd.print("  ");
    if(nowTemp >= 10)  lcd.setCursor(0,1);
    else  lcd.setCursor(1,1);
    lcd.print(nowTemp); 
  };

  if(nowHumidity != pastHumidity){
    lcd.setCursor(4,1);
    lcd.print("  ");
    if(nowHumidity >= 10 )  lcd.setCursor(4,1);
    else  lcd.setCursor(5,1);
    lcd.print(nowHumidity); 
  };

  if(nowWeek != pastWeek){
    lcd.setCursor(12,0);
    lcd.print("   ");
    lcd.setCursor(12,0);
    lcd.print(nowWeek); 
  };
}  

void saveOldData(){
  // 다음에 읽을 데이터와 비교하기 위해 현재 데이터를 저장한다
  pastYear = nowYear;
  pastMonth = nowMonth;
  pastDay = nowDay;
  pastHour = nowHour;
  pastMinute = nowMinute;
  pastSecond = nowSecond;

  pastTemp = nowTemp;
  pastHumidity = nowHumidity;

  pastWeek = nowWeek;  
}  

void lcdDefault(){
  // LCD의 커서를 안보이게 한다.
  lcd.noCursor();
  // LCD의 백라이트를 켠다
  lcd.backlight(); 
  lcd.clear();

  // C, % : 등 변화가 없는 문자들을 미리 출력한다.
  lcd.setCursor(2,1);
  lcd.print("C");
  lcd.setCursor(6,1);
  lcd.print("%");
  lcd.setCursor(10,1);
  lcd.print(":");
  lcd.setCursor(13,1);
  lcd.print(":"); 
}  
