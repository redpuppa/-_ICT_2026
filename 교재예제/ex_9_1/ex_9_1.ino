#include "FspTimer.h"

FspTimer buzzerTimer;

int buzzerPin = 9;
int songLength = 16;

// 노래 데이터, 공백은 쉬는 구간을 나타낸다
char notes[] = "cee egg dff abb ";
// 음의 길이, 노래 데이터와 맞춰 음의 길이를 설정한다.
int beats[] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};

// 노래의 빠르기를 설정한다.
int tempo = 200;

void setup() 
{
  // 부저핀을 출력으로 설정한다
  // D9(P303) = GPT 타이머 7번, 채널 B (variant 핀맵에 고정)
  buzzerTimer.begin(TIMER_MODE_PWM, GPT_TIMER, 7, 1.0f, 10.0f);  // 50Hz, 1.5ms
  buzzerTimer.add_pwm_extended_cfg();
  buzzerTimer.enable_pwm_channel(CHANNEL_B);   // open() 전에 호출해야 적용됨
  buzzerTimer.open();
  buzzerTimer.start();

  // 9번 핀을 GPIO에서 GPT 출력 기능으로 전환
  R_IOPORT_PinCfg(&g_ioport_ctrl, BSP_IO_PORT_03_PIN_03,
                  (uint32_t)(IOPORT_CFG_PERIPHERAL_PIN | IOPORT_PERIPHERAL_GPT1));
}


void loop() 
{
  // 부저 출력 시간에 사용할 변수 설정
  int duration;
  
  // 노래 길이 데이터 갯수만큼 반복한다
  for (int i = 0; i < songLength; i++){
    // 한 음의 시간을 계산한다.
    duration = beats[i] * tempo;

    if (notes[i] == ' '){      // 공란일 경우 음을 출력하지 않는다
      delay(duration);
    }
    else{
      // tone 명령어를 통하여 부저 핀으로 사각파를 출력한다
      buzzerTimer.set_frequency(frequency(notes[i]));
      // tone(buzzerPin, frequency(notes[i]), duration);
      delay(duration);
    }
    // 음이 바뀔 때 잠시 쉬어준다
    delay(tempo / 10);
  }
}

float frequency(char note){
  // 노래 데이터를 주파수 값으로 변경하기 위한 함수

  int i;
  // 음계의 갯수 설정
  int notes = 8;

  char names[] = { 'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C' };
  float frequencies[] = {523.25, 587.33, 659.26, 698.46, 783.99, 880.0, 987.77, 1046.5};

  // 노래 데이터를 주파수 값으로 변경하기 위해 반복하여 비교한다  
  for (i = 0; i < notes; i++){
    if (names[i] == note){
      // 맞는 값을 찾았을 경우 이 값을 회신한다
      return(frequencies[i]);
    };
  };
  // 앞의 for문에서 맞는 값을 못찾았을 경우 0을 회신한다
  return (0.0);
}

