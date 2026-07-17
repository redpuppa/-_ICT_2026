String rx_str = "";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600, SERIAL_8N1);
}

void loop() {
  // put your main code here, to run repeatedly:
  // 수신 버퍼에 받은 데이터가 있는지 확인
  if(Serial.available() > 0){
    char c = Serial.read();

    if(c == '\r' || c == '\n'){
      Serial.print("받은 문자: ");
      Serial.println(rx_str);
      rx_str = "";
    }
    else{
      rx_str += (String)c;
    }    
  }
}



