# ESP32 + MQTT 실습 단계별 가이드

## (초음파 / 온도 / 서보모터 + QoS 비교 실습)

> **실습 환경**: 라즈베리파이 3 (MQTT 브로커) + ESP8266 (클라이언트)
> **참고 문서**: [mqtt_reference.md](mqtt_reference.md)
> **기반 코드**: [mqtt_esp8266/mqtt_esp8266.ino](mqtt_esp8266/mqtt_esp8266.ino)

---

## 사전 준비

### 하드웨어

| 부품 | 수량 | 비고 |
|------|------|------|
| ESP8266 (NodeMCU) | 1 | ESP32도 가능 |
| HC-SR04 초음파 센서 | 1 | 1~4단계 |
| DHT11 온도/습도 센서 | 1 | 3단계 |
| SG90 서보모터 | 1 | 4단계 |
| 브레드보드 + 점퍼선 | - | |
| 저항 1kΩ, 2kΩ 각 1개 | - | HC-SR04 ECHO 전압 분배용 |

### 소프트웨어

| 항목 | 설치 방법 |
|------|----------|
| Arduino IDE | https://www.arduino.cc/en/software |
| ESP8266 보드 매니저 | 보드 매니저에서 `esp8266` 검색 후 설치 |
| PubSubClient 라이브러리 | 라이브러리 관리자에서 `PubSubClient` 검색 → Nick O'Leary 버전 설치 |
| DHT sensor library | 라이브러리 관리자에서 `DHT sensor library` 검색 → Adafruit 버전 설치 |
| Servo 라이브러리 | ESP8266: `Servo` (기본 내장) |

### 라즈베리파이 MQTT 브로커 설정

```bash
# Mosquitto 설치
sudo apt update
sudo apt install mosquitto mosquitto-clients -y

# 브로커 시작
sudo systemctl start mosquitto
sudo systemctl enable mosquitto

# 외부 접속 허용 설정
sudo nano /etc/mosquitto/mosquitto.conf
```

`mosquitto.conf`에 추가:

```
listener 1883
allow_anonymous true
```

```bash
# 설정 적용
sudo systemctl restart mosquitto

# 브로커 IP 확인
hostname -I
```

### 브로커 동작 확인 (터미널 2개 사용)

```bash
# 터미널 1: 구독 (메시지 수신 대기)
mosquitto_sub -t "test/hello"

# 터미널 2: 발행 (메시지 전송)
mosquitto_pub -t "test/hello" -m "Hello MQTT!"
```

터미널 1에 `Hello MQTT!`가 표시되면 브로커가 정상 동작하는 것이다.

---

## 기반 코드 구조 이해

모든 단계의 코드는 아래 기반 코드([mqtt_esp8266.ino](mqtt_esp8266/mqtt_esp8266.ino))에서 출발한다.

```
[기반 코드 구조]

setup_wifi()    → Wi-Fi 연결
reconnect()     → MQTT 브로커 연결/재연결
setup()         → 초기화 (시리얼, 핀, Wi-Fi, MQTT)
loop()          → 주기적으로 센서 읽기 → MQTT publish
```

### 수정이 필요한 부분 (매 단계 공통)

```cpp
// ① Wi-Fi 정보 → 본인 환경에 맞게
const char *ssid     = "YOUR_WIFI_SSID";
const char *password  = "YOUR_WIFI_PASSWORD";

// ② 디바이스 ID → 학생마다 고유하게
const char *ID = "student_01";

// ③ MQTT 브로커 IP → 라즈베리파이 IP
IPAddress broker(192, 168, 0, 81);

// ④ 토픽 → 단계별로 변경
const char *TOPIC = "device/01/ultrasonic";
```

---

## 1단계: 초음파 센서 데이터 읽기 (로컬 테스트)

### 목표

ESP8266에 초음파 센서를 연결하고 거리 값을 시리얼 모니터에서 확인한다.
이 단계에서는 **MQTT를 사용하지 않는다**. 센서가 정상 동작하는지 먼저 확인한다.

### 회로 연결

```
ESP8266 (NodeMCU)       HC-SR04
-----------------       -------
Vin (5V)           -->  VCC
GND                -->  GND
D1 (GPIO 5)        -->  TRIG
D2 (GPIO 4)        -->  ECHO (※ 전압 분배 필요)
```

**ECHO 핀 전압 분배 회로** (HC-SR04의 ECHO는 5V 출력, ESP8266은 3.3V 입력):

```
HC-SR04 ECHO ──[1kΩ]──┬── ESP8266 D2 (GPIO 4)
                       │
                     [2kΩ]
                       │
                      GND
```

> 이 분배기를 통해 5V → 약 3.3V로 낮춰서 ESP8266을 보호한다.

### 코드 작성 포인트

기반 코드에서 **MQTT 관련 코드를 모두 제거**하고, 초음파 센서 읽기에 집중한다.

**추가할 내용:**

1. TRIG, ECHO 핀 정의 및 `pinMode` 설정
2. 거리 측정 함수 작성
3. `loop()`에서 주기적으로 측정 후 시리얼 출력

**거리 측정 원리:**

```
거리(cm) = 초음파 왕복 시간(μs) × 0.034 / 2

- 음속: 약 340m/s = 0.034cm/μs
- 왕복이므로 2로 나눔
```

**핵심 함수:**

| 함수 | 역할 |
|------|------|
| `digitalWrite(TRIG, HIGH)` | 10μs HIGH 펄스로 초음파 발사 |
| `pulseIn(ECHO, HIGH, 30000)` | ECHO 핀의 HIGH 지속 시간(μs) 측정, 30ms 타임아웃 |
| `dtostrf(distance, 4, 1, buf)` | float → 문자열 변환 (예: `23.5`) |

### 확인 방법

| 확인 항목 | 예상 결과 |
|-----------|----------|
| 시리얼 모니터 (115200 baud) | `거리: 23.5 cm` 형태로 2초마다 출력 |
| 손을 가까이 가져가기 | 거리 값이 줄어듦 (약 2~400cm 범위) |
| 센서 범위 밖 물체 | `측정 실패` 또는 비정상 값 |

### 학습 포인트

- **센서 → MCU → 시리얼 모니터** 데이터 흐름 이해
- 측정 단위(cm)와 HC-SR04의 유효 범위 (2~400cm)
- `delay()` 대신 `millis()` 기반 비차단 타이밍의 이점

### 실험 제안

- 측정 주기를 `2000ms → 500ms → 100ms`로 변경하면 값이 어떻게 달라지는가?
- 부드러운 벽 vs 울퉁불퉁한 천에서 측정 정확도 차이는?

---

## 2단계: 초음파 데이터를 MQTT로 퍼블리시

### 목표

1단계에서 확인한 초음파 거리 데이터를 **MQTT를 통해 브로커(라즈베리파이)로 전송**한다.

### 토픽 설계

```
device/{student_id}/ultrasonic
```

예시: `device/01/ultrasonic`, `device/kim/ultrasonic`

### 코드 작성 포인트

1단계 코드에 **MQTT 연결 및 publish 코드를 추가**한다.
기반 코드([mqtt_esp8266.ino](mqtt_esp8266/mqtt_esp8266.ino))의 Wi-Fi 연결, MQTT 연결 부분을 그대로 활용한다.

**변경 사항:**

| 항목 | 기반 코드 | 2단계 |
|------|----------|-------|
| 센서 | 버튼 (SWITCH_PIN) | HC-SR04 초음파 |
| 토픽 | `room/light` | `device/01/ultrasonic` |
| 전송 데이터 | `"on"` / `"off"` | 거리 값 (예: `"23.5"`) |
| 전송 방식 | 버튼 누를 때 | 2초 주기 자동 전송 |

**`loop()` 핵심 흐름:**

```
loop() {
    1. MQTT 연결 유지       → reconnect() + client.loop()
    2. 2초 간격 확인        → millis() 기반
    3. 거리 측정            → measureDistance()
    4. MQTT 퍼블리시        → client.publish(TOPIC, distStr)
    5. 시리얼 모니터 출력   → 디버깅용
}
```

### 라즈베리파이에서 수신 확인

```bash
# 구독하여 ESP8266이 보내는 데이터 확인
mosquitto_sub -t "device/01/ultrasonic" -v
```

예상 출력:

```
device/01/ultrasonic 23.5
device/01/ultrasonic 22.1
device/01/ultrasonic 15.8
```

### 확인 방법

| 확인 항목 | 예상 결과 |
|-----------|----------|
| ESP8266 시리얼 모니터 | `device/01/ultrasonic => 23.5 cm` |
| 라즈베리파이 `mosquitto_sub` | 거리 값이 2초마다 수신됨 |
| 물체 접근 시 | 거리 값이 줄어드는 것을 양쪽에서 모두 확인 |

### QoS 실험

#### PubSubClient의 QoS 제한사항

> **중요**: PubSubClient 라이브러리는 **Publish는 QoS 0만 지원**하고, **Subscribe는 QoS 0, 1을 지원**한다.
> ([mqtt_reference.md](mqtt_reference.md) 6장 참고)

| 동작 | PubSubClient 지원 QoS |
|------|----------------------|
| Publish (메시지 보내기) | **QoS 0만 가능** |
| Subscribe (메시지 받기) | QoS 0, 1 |

따라서 이 단계에서는 **QoS의 개념을 이해**하는 데 초점을 맞추고,
실제 QoS 차이 실험은 **라즈베리파이의 `mosquitto_pub` 명령어**로 진행한다.

#### 실험 1: QoS 0 vs QoS 1 비교 (mosquitto 명령어 사용)

```bash
# 구독자: QoS 1로 구독
mosquitto_sub -t "test/qos" -q 1 -v

# 다른 터미널에서 QoS 0으로 발행
mosquitto_pub -t "test/qos" -m "QoS 0 message" -q 0

# QoS 1로 발행
mosquitto_pub -t "test/qos" -m "QoS 1 message" -q 1
```

정상 환경에서는 두 메시지 모두 도착한다. 차이는 **네트워크 불안정 시** 나타난다.

#### 실험 2: 네트워크 불안정 시뮬레이션

1. `mosquitto_sub -t "device/01/ultrasonic" -q 1 -v`로 구독
2. ESP8266이 데이터를 전송하는 중에 **ESP8266의 Wi-Fi를 잠깐 끊기** (리셋 또는 AP 끄기)
3. 다시 연결 후 **QoS 1 구독자가 밀린 메시지를 받는지** 확인

> **QoS 다운그레이드 규칙**: 최종 전달 QoS = MIN(발행자 QoS, 구독자 QoS)
> PubSubClient가 QoS 0으로 발행하므로, 구독자가 QoS 1로 구독해도 실제 전달은 QoS 0이 된다.

#### 실험 3: QoS 차이를 제대로 보려면

라즈베리파이끼리 실험하면 QoS 차이를 명확히 볼 수 있다:

```bash
# 터미널 1: QoS 1 구독 + 클린세션 끄기
mosquitto_sub -t "test/qos" -q 1 -c -i "sub_client_01"

# 구독자를 Ctrl+C로 종료 (오프라인 상태)

# 터미널 2: QoS 1로 메시지 발행
mosquitto_pub -t "test/qos" -m "offline message 1" -q 1
mosquitto_pub -t "test/qos" -m "offline message 2" -q 1

# 터미널 1: 같은 Client ID로 다시 구독
mosquitto_sub -t "test/qos" -q 1 -c -i "sub_client_01"
# → 오프라인 동안 쌓인 메시지가 한꺼번에 도착!
```

| 옵션 | 의미 |
|------|------|
| `-q 1` | QoS 1 |
| `-c` | CleanSession = false (영구 세션) |
| `-i "sub_client_01"` | 클라이언트 ID 지정 (세션 유지를 위해 필요) |

### 학습 포인트

- **Publish / Subscribe 구조**: 발행자는 구독자가 누구인지 모른다
- **브로커의 역할**: 메시지를 중계하고, 영구 세션 시 오프라인 메시지를 보관
- **QoS 0 vs 1 차이**: 정상 환경에서는 같아 보이지만, 네트워크 불안정 시 차이 발생
- **PubSubClient 제한**: Arduino에서 QoS를 자유롭게 쓰려면 다른 라이브러리 필요

---

## 3단계: 온도 센서 조건부 퍼블리시 (QoS 0)

### 목표

온도가 **특정 조건을 만족할 때만** MQTT 메시지를 전송한다.
불필요한 전송을 줄여 네트워크 트래픽을 절약하는 개념을 실습한다.

### 회로 연결

```
ESP8266 (NodeMCU)       DHT11
-----------------       -----
3.3V               -->  VCC
GND                -->  GND
D4 (GPIO 2)        -->  DATA
```

> DHT11의 DATA 핀에 10kΩ 풀업 저항을 VCC와 사이에 연결하는 것을 권장한다.

### 토픽 설계

```
device/{student_id}/temperature
```

### 조건부 전송 로직

두 가지 방식 중 택 1:

**방식 A: 임계값 기반**

```
온도 >= 30℃  → 전송
온도 < 30℃   → 전송하지 않음
```

**방식 B: 변화폭 기반**

```
|현재 온도 - 이전 전송 온도| >= 1.0℃  → 전송
변화폭 < 1.0℃                        → 전송하지 않음
```

> 방식 B가 실무에서 더 많이 사용된다. 불필요한 중복 전송을 줄이면서도 의미 있는 변화는 놓치지 않는다.

### 코드 작성 포인트

2단계 코드를 기반으로 **센서와 전송 조건을 변경**한다.

**변경 사항:**

| 항목 | 2단계 | 3단계 |
|------|-------|-------|
| 센서 | HC-SR04 초음파 | DHT11 온도 |
| 토픽 | `device/01/ultrasonic` | `device/01/temperature` |
| 전송 조건 | 매 주기 전송 | **조건 만족 시에만 전송** |
| 전송 데이터 | 거리 (cm) | 온도 (℃) |

**`loop()` 핵심 흐름:**

```
loop() {
    1. MQTT 연결 유지
    2. 2초 간격 확인
    3. 온도 측정              → dht.readTemperature()
    4. 조건 확인              → 임계값 or 변화폭 비교
    5. 조건 만족 시에만 전송  → client.publish(TOPIC, tempStr)
    6. 시리얼에는 항상 출력   → 전송 여부 구분을 위해
}
```

**시리얼 출력 예시:**

```
온도: 25.3 ℃ → [스킵] 변화폭 부족
온도: 25.4 ℃ → [스킵] 변화폭 부족
온도: 26.8 ℃ → [전송] device/01/temperature => 26.8
온도: 26.9 ℃ → [스킵] 변화폭 부족
온도: 28.1 ℃ → [전송] device/01/temperature => 28.1
```

### QoS 설정: QoS 0

| 이유 | 설명 |
|------|------|
| 주기적 데이터 | 잠시 후 다시 측정하므로 하나 유실돼도 괜찮음 |
| 경량성 중시 | 배터리 구동 IoT에서는 전송 오버헤드를 줄이는 것이 중요 |
| 실무 관행 | 대부분의 IoT 센서 텔레메트리는 QoS 0 사용 |

> PubSubClient는 Publish가 QoS 0만 가능하므로, 이 단계의 QoS 요구사항과 정확히 맞는다.

### 라즈베리파이에서 수신 확인

```bash
mosquitto_sub -t "device/01/temperature" -v
```

### 확인 방법

| 확인 항목 | 예상 결과 |
|-----------|----------|
| 시리얼 모니터 | 매 2초마다 온도 출력 + 전송/스킵 표시 |
| `mosquitto_sub` | 조건 만족 시에만 메시지 수신 |
| 센서에 손 대기 (체온 전달) | 온도 상승 → 전송 빈도 증가 |

### 실험 제안

#### 실험 1: 무조건 전송 vs 조건부 전송 비교

```bash
# 30초간 수신된 메시지 수 세기
timeout 30 mosquitto_sub -t "device/01/temperature" | wc -l
```

- 무조건 전송 (2초 간격): 약 15개
- 조건부 전송: 환경에 따라 0~5개

#### 실험 2: 임계값 변경

- 임계값 `30℃ → 25℃`로 변경하면 전송 빈도는?
- 변화폭 `1.0℃ → 0.5℃`로 변경하면?

### 학습 포인트

- **조건부 Publish**: 모든 데이터를 보내는 것이 항상 좋은 것은 아니다
- **네트워크 트래픽 절약**: IoT 환경에서 수백 개 디바이스가 매초 전송하면 브로커 부하 급증
- **QoS 0이 적합한 이유**: 비중요 주기적 데이터에는 "보내고 잊기"가 효율적

---

## 4단계: 서보모터 제어 - MQTT 명령 수신 (QoS 2)

### 목표

ESP8266이 MQTT **구독자(Subscriber)**가 되어, 브로커에서 명령을 받아 서보모터를 제어한다.
이전 단계(Publish)와 반대 방향의 데이터 흐름을 실습한다.

```
[라즈베리파이]              [ESP8266]
mosquitto_pub  → 브로커 →  subscribe → 서보모터 제어
   "OPEN"                   콜백 함수      → 90도 회전
```

### 회로 연결

```
ESP8266 (NodeMCU)       SG90 서보모터
-----------------       -------------
Vin (5V)           -->  빨간색 (VCC)
GND                -->  갈색 (GND)
D5 (GPIO 14)       -->  주황색 (Signal)
```

> 서보모터는 전류 소비가 크므로 외부 전원(5V) 사용을 권장한다.
> NodeMCU의 Vin으로 부족할 수 있다.

### 토픽 설계

```
device/{student_id}/servo/cmd
```

### 명령 형식

| 명령 | 동작 |
|------|------|
| `OPEN` | 서보 90도 회전 (열기) |
| `CLOSE` | 서보 0도 회전 (닫기) |
| `45` | 서보 45도로 이동 (각도 직접 지정) |

### 코드 작성 포인트

이 단계는 이전 단계와 가장 큰 차이가 있다: **Publish가 아니라 Subscribe**를 사용한다.

**새로 필요한 요소:**

| 요소 | 설명 |
|------|------|
| `client.subscribe(TOPIC, qos)` | 토픽 구독 등록 |
| `client.setCallback(callback)` | 메시지 수신 시 호출될 함수 등록 |
| `callback(topic, payload, length)` | 수신된 메시지를 처리하는 콜백 함수 |
| `Servo.h` | 서보모터 제어 라이브러리 |

**`reconnect()`에 구독 추가:**

```
reconnect() 성공 시:
    client.subscribe(TOPIC, 1);   // QoS 1로 구독
```

> Subscribe는 QoS 1까지 가능하다 (PubSubClient 제한).

**콜백 함수 핵심 흐름:**

```
callback(topic, payload, length) {
    1. payload를 문자열로 변환
    2. "OPEN"이면  → servo.write(90)
    3. "CLOSE"이면 → servo.write(0)
    4. 숫자이면    → servo.write(숫자)
    5. 시리얼에 수신 내용 출력
}
```

### 라즈베리파이에서 명령 전송

```bash
# 서보 열기
mosquitto_pub -t "device/01/servo/cmd" -m "OPEN"

# 서보 닫기
mosquitto_pub -t "device/01/servo/cmd" -m "CLOSE"

# 특정 각도로 이동
mosquitto_pub -t "device/01/servo/cmd" -m "45"

# QoS 2로 전송
mosquitto_pub -t "device/01/servo/cmd" -m "OPEN" -q 2
```

### QoS 설정: QoS 2 (개념 학습)

| QoS | 의미 | 서보 제어에서의 위험 |
|-----|------|---------------------|
| QoS 0 | 전달 안 될 수도 있음 | 문이 안 열릴 수 있음 |
| QoS 1 | 최소 1번 전달, 중복 가능 | "OPEN→CLOSE→OPEN" 중 CLOSE가 중복되면 문이 다시 닫힘 |
| **QoS 2** | **정확히 1번 전달** | **명령이 한 번만 실행됨 → 안전** |

#### PubSubClient에서의 QoS 2 한계

> **PubSubClient는 QoS 2를 지원하지 않는다.**
> - Publish: QoS 0만 가능
> - Subscribe: QoS 0, 1만 가능

따라서 QoS 2의 **개념과 필요성**을 이해하고, 실제 QoS 2 실험은 `mosquitto_pub/sub`로 진행한다:

```bash
# QoS 2 발행/구독 실험 (라즈베리파이에서)
# 터미널 1: QoS 2 구독
mosquitto_sub -t "test/servo" -q 2 -v

# 터미널 2: QoS 2 발행
mosquitto_pub -t "test/servo" -m "OPEN" -q 2
```

### 확인 방법

| 확인 항목 | 예상 결과 |
|-----------|----------|
| `mosquitto_pub -m "OPEN"` 실행 | 서보가 90도 회전 |
| `mosquitto_pub -m "CLOSE"` 실행 | 서보가 0도로 복귀 |
| `mosquitto_pub -m "45"` 실행 | 서보가 45도로 이동 |
| ESP8266 시리얼 모니터 | `수신: OPEN → 서보 90도` |

### 학습 포인트

- **Publish vs Subscribe**: 센서(Pub)와 액추에이터(Sub)의 데이터 흐름이 반대
- **Exactly Once (QoS 2)**: 제어 명령은 중복 실행 시 위험할 수 있음
- **콜백 패턴**: 메시지가 올 때까지 기다리지 않고, 도착하면 자동으로 처리됨
- **MQTT의 제어 활용**: 센서 데이터 수집뿐 아니라 원격 제어에도 활용 가능

---

## 5단계: 세 센서/제어 구조 비교 정리

### QoS 비교표

| 항목 | 방향 | QoS | 이유 |
|------|------|-----|------|
| 온도 센서 | ESP → 브로커 (Publish) | **0** | 유실 허용, 주기적 데이터 |
| 초음파 센서 | ESP → 브로커 (Publish) | **1** | 이벤트 최소 1회 보장 |
| 서보 명령 | 브로커 → ESP (Subscribe) | **2** | 정확히 1회 실행 보장 |

### 데이터 흐름 비교

```
[온도 센서]                              [구독자/대시보드]
  DHT11 → ESP8266 → Publish(QoS 0) → 브로커 → Subscribe

[초음파 센서]                            [구독자/알림 시스템]
  HC-SR04 → ESP8266 → Publish(QoS 1) → 브로커 → Subscribe

[서보 제어]                              [서보모터]
  명령 발행자 → Publish(QoS 2) → 브로커 → Subscribe → ESP8266 → SG90
```

### 토론: 왜 모든 것을 QoS 2로 하지 않는가?

| QoS | 핸드셰이크 단계 | 오버헤드 | 지연 |
|-----|----------------|----------|------|
| 0 | 1단계 | 최소 | 최소 |
| 1 | 2단계 (PUBACK) | 중간 | 중간 |
| 2 | 4단계 (PUBREC→PUBREL→PUBCOMP) | 최대 | 최대 |

> **핵심**: QoS 레벨이 높을수록 안전하지만, 네트워크 트래픽과 지연이 증가한다.
> IoT에서는 **데이터의 중요도에 따라 적절한 QoS를 선택**하는 것이 설계의 핵심이다.

### PubSubClient QoS 지원 정리

| 동작 | QoS 0 | QoS 1 | QoS 2 |
|------|-------|-------|-------|
| Publish | O | **X** | **X** |
| Subscribe | O | O | **X** |

> 실무에서 QoS 1/2 Publish가 필요하면: **AsyncMqttClient** (ESP32) 또는 **MicroPython umqtt** 고려

---

## 6단계: HTTP와 MQTT 비교 토론

### 토론 질문

#### Q1. 온도 센서를 HTTP로 구현하면 어떻게 해야 할까?

```
[HTTP 방식]
ESP8266 → HTTP POST → Flask 서버 → DB 저장 → 브라우저가 GET 요청

[MQTT 방식]
ESP8266 → MQTT Publish → 브로커 → 구독자가 자동 수신
```

| 비교 항목 | HTTP | MQTT |
|-----------|------|------|
| 전송 방식 | ESP가 서버 URL을 알아야 함 | ESP는 토픽만 알면 됨 |
| 수신 방식 | 브라우저가 주기적으로 GET (Polling) | 구독자가 자동 수신 (Push) |
| 연결 | 매번 연결/해제 | 지속 연결 (Keep-Alive) |
| 다중 수신자 | 각각 따로 GET 요청 | 브로커가 모든 구독자에게 전달 |

#### Q2. 초음파 이벤트를 HTTP Polling으로 처리하면?

- **문제 1**: Polling 간격이 1초면, 0.5초짜리 이벤트를 놓칠 수 있음
- **문제 2**: 이벤트가 없어도 매번 요청 → 서버 부하 + 네트워크 낭비
- **문제 3**: Polling 간격을 줄이면 부하 증가, 늘리면 응답성 저하 (딜레마)
- **MQTT 장점**: 이벤트가 발생할 때만 전송 → 실시간 + 효율적

#### Q3. 다수의 학생 ESP가 동시에 전송하면?

| 상황 | HTTP | MQTT |
|------|------|------|
| 30명 동시 전송 | Flask 서버에 30개 동시 요청 → 서버 병목 | 브로커가 효율적으로 처리 |
| 수신자 추가 | 새 서버 개발 필요 | 구독자만 추가하면 됨 |
| 헤더 크기 | HTTP 헤더 수백 바이트 | MQTT 고정 헤더 2바이트 |
| 연결 비용 | 매번 TCP 핸드셰이크 | 한 번 연결 후 유지 |

### 실무에서의 선택 기준

| 상황 | 권장 프로토콜 |
|------|-------------|
| 센서 데이터 실시간 수집 | **MQTT** |
| 사용자가 요청할 때 데이터 조회 | **HTTP** |
| 디바이스 원격 제어 | **MQTT** |
| 펌웨어 업데이트 (큰 파일) | **HTTP** |
| 1:N 데이터 배포 | **MQTT** |
| REST API 통합 | **HTTP** |

---

## 7단계: 확장 아이디어

### 7-1. Last Will 메시지 실습

ESP8266이 비정상 종료(전원 차단 등)되면 브로커가 자동으로 "오프라인" 메시지를 발행한다.

```
토픽: device/{student_id}/status
Will 메시지: "offline"
정상 연결 시: "online" 발행
```

**코드 힌트:**

```cpp
// connect() 시 Will 메시지 설정
client.connect(ID, willTopic, willQoS, willRetain, willMessage);
// 예: client.connect("student_01", "device/01/status", 1, true, "offline");
```

**확인 방법:**

```bash
mosquitto_sub -t "device/01/status" -v
# ESP8266 전원 끄기 → 약 1.5×KeepAlive 후 "offline" 수신
```

### 7-2. Retained 메시지 실험

새로운 구독자가 연결되면 **마지막으로 발행된 Retained 메시지**를 즉시 받는다.

```bash
# Retained 메시지 발행 (-r 옵션)
mosquitto_pub -t "device/01/temperature" -m "25.3" -r

# 나중에 구독해도 마지막 값을 바로 받음
mosquitto_sub -t "device/01/temperature" -v
# → 즉시 "25.3" 수신
```

### 7-3. 여러 학생 데이터를 하나의 대시보드로 집계

```bash
# 와일드카드로 모든 학생의 초음파 데이터 구독
mosquitto_sub -t "device/+/ultrasonic" -v

# 모든 학생의 모든 센서 데이터 구독
mosquitto_sub -t "device/#" -v
```

| 와일드카드 | 의미 | 예시 |
|-----------|------|------|
| `+` | 한 레벨 | `device/+/ultrasonic` → 모든 학생의 초음파 |
| `#` | 모든 하위 레벨 | `device/#` → 모든 학생의 모든 데이터 |

### 7-4. 네트워크 트래픽 비교 (HTTP vs MQTT)

라즈베리파이에서 `tcpdump`로 패킷 크기를 비교한다:

```bash
# MQTT 트래픽 캡처 (포트 1883)
sudo tcpdump -i wlan0 port 1883 -c 20

# HTTP 트래픽 캡처 (포트 5000)
sudo tcpdump -i wlan0 port 5000 -c 20
```

---

## 수업 핵심 메시지

> MQTT는 단순히 가벼운 프로토콜이 아니라,
> **데이터의 중요도에 따라 전달 품질(QoS)을 설계할 수 있는 프로토콜**이다.

| 배운 것 | 핵심 |
|---------|------|
| 1단계 | 센서 → MCU → 시리얼 (하드웨어 확인) |
| 2단계 | Publish + QoS 개념 (데이터 전송) |
| 3단계 | 조건부 Publish + 트래픽 최적화 |
| 4단계 | Subscribe + 제어 명령 (양방향 통신) |
| 5단계 | QoS 선택 = **설계 결정** |
| 6단계 | MQTT vs HTTP = **용도에 따른 선택** |

---

## 트러블슈팅

| 증상 | 원인 | 해결 |
|------|------|------|
| `Attempting MQTT connection...failed` | 브로커 IP 오류 또는 브로커 미실행 | `hostname -I`로 IP 확인, `sudo systemctl status mosquitto` |
| Wi-Fi 연결 안 됨 | SSID/비밀번호 오류, 5GHz AP | 2.4GHz AP 확인, 비밀번호 재확인 |
| 초음파 거리값이 0 또는 음수 | 배선 오류, 전압 분배 미적용 | TRIG/ECHO 핀 확인, 전압 분배 회로 확인 |
| DHT 센서 `nan` 반환 | 배선 오류, 핀 번호 불일치 | DATA 핀 번호 확인, 풀업 저항 확인 |
| 서보모터가 떨림 | 전류 부족 | 외부 5V 전원 사용 |
| `mosquitto_sub`에 아무것도 안 뜸 | 토픽 불일치 | ESP와 `mosquitto_sub`의 토픽이 정확히 일치하는지 확인 |
| `rc=-2` (MQTT 연결 실패) | 네트워크 문제 | Wi-Fi 연결 상태 확인, 브로커 방화벽 확인 |
