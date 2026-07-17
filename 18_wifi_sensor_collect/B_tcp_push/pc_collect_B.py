"""
[방법 B] PC 수집기 — PC가 TCP 서버가 되어 보드가 보내는 값을 받는다.
          받은 값을 CSV로 저장하고 실시간 그래프로 보여준다.

실행 전 준비:
    pip install matplotlib
    보드 스케치의 PC_IP 를 이 PC의 IP로 맞춘다.
    (내 IP 확인: Windows에서 ipconfig → IPv4 주소)

실행:
    python pc_collect_B.py     ← 먼저 실행해 두고, 그다음 보드에 전원을 준다
종료: Ctrl+C
"""
import csv
import socket
import time
from collections import deque

import matplotlib.pyplot as plt

# ── 설정 ─────────────────────────────────────────────
HOST     = "0.0.0.0"        # 모든 네트워크 인터페이스에서 수신
PORT     = 9000             # 보드 스케치의 PC_PORT와 동일
CSV_PATH = "adc_log_B.csv"
WINDOW   = 100
# ────────────────────────────────────────────────────

srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
srv.bind((HOST, PORT))
srv.listen(1)
print(f"포트 {PORT}에서 보드 연결 대기 중...  (보드에 전원을 주세요)")
conn, addr = srv.accept()
print("보드 연결됨:", addr)
reader = conn.makefile("r")     # TCP 스트림을 줄 단위로 읽기 위한 래퍼

xs, ys = deque(maxlen=WINDOW), deque(maxlen=WINDOW)
plt.ion()
fig, ax = plt.subplots()
(line,) = ax.plot([], [], marker=".")
ax.set_title("UNO R4 WiFi - A0 (Method B: TCP push)")
ax.set_xlabel("elapsed (s)")
ax.set_ylabel("ADC (0-4095)")
ax.set_ylim(0, 4095)

t0 = time.time()
with open(CSV_PATH, "w", newline="", encoding="utf-8") as f:
    w = csv.writer(f)
    w.writerow(["elapsed_s", "adc", "volt"])
    print(f"수집 시작 → {CSV_PATH}   (Ctrl+C로 종료)")
    try:
        for raw in reader:                 # 보드가 보내는 줄을 계속 읽음
            raw = raw.strip()
            if not raw:
                continue
            try:
                adc_s, volt_s = raw.split(",")
                adc, volt = int(adc_s), float(volt_s)
            except ValueError:
                continue                   # 형식이 안 맞는 줄은 무시

            t = time.time() - t0
            w.writerow([f"{t:.2f}", adc, f"{volt:.3f}"])
            f.flush()

            xs.append(t)
            ys.append(adc)
            line.set_data(xs, ys)
            ax.relim()
            ax.autoscale_view(scaley=False)
            print(f"{t:6.1f}s  ADC={adc:4d}  V={volt:.3f}")
            plt.pause(0.01)
    except KeyboardInterrupt:
        print("\n종료. 저장 파일:", CSV_PATH)
    finally:
        conn.close()
        srv.close()
