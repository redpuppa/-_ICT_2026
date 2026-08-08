/*
  [방법 A] 보드가 "웹서버"가 되고, PC가 값을 가져간다 (HTTP GET)
  - 보드: WiFi(STA)로 공유기에 접속 → 80번 포트로 웹서버 개설
  - 경로로 분기(routing):
      · http://보드IP/        → 브라우저용 대시보드(HTML, 큰 글씨 + 게이지, 자동 갱신)
      · http://보드IP/data    → "adc,volt" CSV 한 줄 (PC 수집기 pc_collect_A.py 용)
  - 센서: 가변저항(가운데 핀 → A0), 양끝 → 5V / GND

  보드: Arduino UNO R4 WiFi
  필요 라이브러리: WiFiS3 (UNO R4 보드 패키지에 기본 포함)
*/
#include "WiFiS3.h"
#include "arduino_secrets.h"       // SECRET_SSID / SECRET_PASS

#define ADC_BIT    12
#define RESOLUTION (1 << ADC_BIT)  // 4096 (12-bit)
#define VA_REF     5.0             // 기준 전압 5V

WiFiServer server(80);             // 80번 포트(HTTP) 웹서버

void setup() {
  Serial.begin(115200);
  analogReadResolution(ADC_BIT);   // ADC를 12비트로 (11_ADC_VR과 동일)

  pinMode(LED_BUILTIN, OUTPUT);    // 내장 LED 출력 설정
  digitalWrite(LED_BUILTIN, LOW);  // 시작은 꺼짐

  Serial.print("WiFi 연결 중");
  WiFi.begin(SECRET_SSID, SECRET_PASS);
  // WL_CONNECTED 이면서 실제 IP(DHCP)를 받을 때까지 대기
  // (실패 시 0.0.0.0으로 넘어가는 것을 방지)
  while (WiFi.status() != WL_CONNECTED || WiFi.localIP() == IPAddress(0, 0, 0, 0)) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  server.begin();
  Serial.print("연결 완료! PC/브라우저에서 접속할 주소 →  http://");
  Serial.println(WiFi.localIP());  // ★ 이 IP를 PC 수집기(pc_collect_A.py)에 입력
}

void loop() {
  WiFiClient client = server.available();   // 접속한 클라이언트가 있는가?
  if (!client) return;

  // 요청 라인(첫 줄)을 읽어 경로를 판별한다. 예: "GET /data HTTP/1.1"
  String reqLine = client.readStringUntil('\n');
  // 나머지 헤더는 빈 줄이 나올 때까지 읽어서 버린다.
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") break;                // 헤더의 끝(빈 줄)
  }

  if (reqLine.indexOf("/led/on") >= 0) {
    // ----- /led/on : 내장 LED 켜기 -----
    digitalWrite(LED_BUILTIN, HIGH);
    sendPlain(client, "1");                  // 새 상태(1=ON) 응답
  } else if (reqLine.indexOf("/led/off") >= 0) {
    // ----- /led/off : 내장 LED 끄기 -----
    digitalWrite(LED_BUILTIN, LOW);
    sendPlain(client, "0");                  // 새 상태(0=OFF) 응답
  } else if (reqLine.indexOf("/data") >= 0) {
    // ----- /data : CSV 응답 (PC 수집기 pc_collect_A.py 용) -----
    int   adc  = analogRead(A0);
    float volt = (VA_REF / RESOLUTION) * adc;

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/plain");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Connection: close");
    client.println();
    client.print(adc);
    client.print(",");
    client.println(volt, 3);
  } else {
    // ----- / : 브라우저용 대시보드(HTML) -----
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html; charset=utf-8");
    client.println("Connection: close");
    client.println();
    sendDashboard(client);
  }

  client.stop();                             // 연결 종료
}

// 브라우저용 대시보드 HTML을 전송한다.
// (값은 페이지의 자바스크립트가 /data 를 0.5초마다 읽어와 갱신)
void sendDashboard(WiFiClient& client) {
  client.print(F(
    "<!DOCTYPE html><html lang=\"ko\"><head><meta charset=\"utf-8\">"
    "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
    "<title>UNO R4 센서</title><style>"
    "body{margin:0;font-family:system-ui,sans-serif;background:#0f172a;color:#e2e8f0;"
    "display:flex;flex-direction:column;align-items:center;justify-content:center;min-height:100vh}"
    "h1{font-size:1.5rem;color:#38bdf8;margin:0 0 2rem}"
    ".volt{font-size:7rem;font-weight:800;line-height:1;color:#fbbf24}"
    ".unit{font-size:2.5rem;color:#94a3b8}"
    ".adc{font-size:1.6rem;color:#94a3b8;margin-top:.6rem}"
    ".bar{width:80%;max-width:420px;height:32px;background:#1e293b;border-radius:16px;"
    "margin-top:2.5rem;overflow:hidden}"
    ".fill{height:100%;width:0;background:linear-gradient(90deg,#22c55e,#fbbf24,#ef4444);transition:width .3s}"
    ".led{display:flex;align-items:center;gap:1rem;margin-top:2.5rem;font-size:1.3rem}"
    ".dot{width:22px;height:22px;border-radius:50%;background:#334155;transition:.2s}"
    ".dot.active{background:#fbbf24;box-shadow:0 0 16px #fbbf24}"
    "button{font-size:1.2rem;font-weight:700;padding:.6rem 1.6rem;border:none;border-radius:12px;"
    "color:#0f172a;cursor:pointer}"
    ".on{background:#22c55e}.off{background:#ef4444;color:#fff}"
  ));
  client.print(F(
    "</style></head><body>"
    "<h1>⚡ UNO R4 WiFi 센서 모니터</h1>"
    "<div><span class=\"volt\" id=\"v\">0.00</span><span class=\"unit\"> V</span></div>"
    "<div class=\"adc\">ADC: <span id=\"a\">0</span> / 4095</div>"
    "<div class=\"bar\"><div class=\"fill\" id=\"f\"></div></div>"
    "<div class=\"led\"><span>내장 LED</span><span class=\"dot\" id=\"d\"></span>"
    "<button class=\"on\" onclick=\"led(1)\">ON</button>"
    "<button class=\"off\" onclick=\"led(0)\">OFF</button></div>"
    "<script>"
    "async function u(){try{"
    "const r=await fetch('/data');const t=await r.text();"
    "const p=t.trim().split(',');"
    "document.getElementById('v').textContent=parseFloat(p[1]).toFixed(2);"
    "document.getElementById('a').textContent=p[0];"
    "document.getElementById('f').style.width=(parseFloat(p[1])/5*100)+'%';"
    "}catch(e){}}"
    "async function led(s){try{"
    "const r=await fetch(s?'/led/on':'/led/off');const v=(await r.text()).trim();"
    "document.getElementById('d').classList.toggle('active',v==='1');"
    "}catch(e){}}"
    "setInterval(u,500);u();"
    "</script></body></html>"
  ));
}

// 짧은 평문 응답(LED 상태 등)을 전송한다. (CORS 허용)
void sendPlain(WiFiClient& client, const char* body) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/plain");
  client.println("Access-Control-Allow-Origin: *");
  client.println("Connection: close");
  client.println();
  client.println(body);
}
