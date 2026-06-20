
                    // a, b, c, d, e, f, g, dp
uint8_t fnd_data_pin[8] = { 4, 5, 6, 7, 8, 9, 10, 11};
uint8_t fnd_data[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 
                        0x6D, 0x7D, 0x27, 0x7F, 0x6F};
                          //D1, D2, D3, D4
uint8_t fnd_digit_pin[4] = {14, 15, 16, 17};

const int inputPin1 = 2;
const int inputPin2 = 3;

uint16_t count = 0;

unsigned long pervMillis1 = 0;

bool run_flag_count = false;  // false:stop, true:start

void fndDisplay(uint8_t num);

void sw1_Pressed(void){
  run_flag_count = !run_flag_count;
}

void sw2_Pressed(void){
  count = 0;
  run_flag_count = false;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  pinMode(inputPin1, INPUT);
  pinMode(inputPin2, INPUT);

  for(int i=0; i<8; i++){
    pinMode(fnd_data_pin[i], OUTPUT);
    digitalWrite(fnd_data_pin[i], HIGH);
  }
  for(int i=0; i<4; i++){
    pinMode(fnd_digit_pin[i], OUTPUT);
    digitalWrite(fnd_digit_pin[i], LOW);
  }

  // 인터럽트 설정
  attachInterrupt(digitalPinToInterrupt(inputPin1), sw1_Pressed, FALLING);
  attachInterrupt(digitalPinToInterrupt(inputPin2), sw2_Pressed, FALLING);

}

void loop() {
  // put your main code here, to run repeatedly:
  unsigned long now = millis();

  // count_task
  if(run_flag_count == true)
  {
    if(now - pervMillis1 >= 1000){
      pervMillis1 = now;
      count++;
    }
  }

  fndDisplayArray(count);
}

// 한 번 호출하면 1234를 한 프레임 스캔합니다.
void fndDisplayArray(int number) {
  // 각 자리 숫자 추출
  int digits[4];
  digits[0] = (number / 1000) % 10;  // 천의 자리
  digits[1] = (number / 100)  % 10;  // 백의 자리
  digits[2] = (number / 10)   % 10;  // 십의 자리
  digits[3] =  number         % 10;  // 일의 자리

  // 4자리를 순서대로 스캔
  for (int i = 0; i < 4; i++) {
    fndDisplay(digits[i]);   // 해당 자리에 숫자 세그먼트 출력

    // i번째 자리만 켜고 나머지는 끔
    for (int j = 0; j < 4; j++) {
      digitalWrite(fnd_digit_pin[j], (i == j) ? HIGH : LOW);
    }

    delay(1);
  }
}

void fndDisplay(uint8_t num){
  for(int i=0; i<8; i++){
    digitalWrite(fnd_data_pin[i], !bitRead(fnd_data[num],i));
  }
}
