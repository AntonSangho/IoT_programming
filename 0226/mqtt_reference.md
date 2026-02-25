# MQTT 프로토콜 참고문서

> **참고 원문**: [HiveMQ MQTT Essentials](https://www.hivemq.com/mqtt-essentials/) (Part 1~10)
> **실습 환경**: 라즈베리파이 3 (브로커) + ESP32 / ESP8266 (클라이언트)

---

## 목차

1. [MQTT란 무엇인가](#1-mqtt란-무엇인가)
2. [Publish/Subscribe 패턴](#2-publishsubscribe-패턴)
3. [클라이언트, 브로커, 연결 수립](#3-클라이언트-브로커-연결-수립)
4. [Publish / Subscribe / Unsubscribe](#4-publish--subscribe--unsubscribe)
5. [토픽과 설계 규칙](#5-토픽과-설계-규칙)
6. [QoS (서비스 품질 수준)](#6-qos-서비스-품질-수준)
7. [영구 세션과 메시지 큐잉](#7-영구-세션과-메시지-큐잉)
8. [Retained 메시지](#8-retained-메시지)
9. [Last Will and Testament (LWT)](#9-last-will-and-testament-lwt)
10. [Keep Alive와 Client Take-Over](#10-keep-alive와-client-take-over)
11. [제어 패킷 전체 목록](#11-제어-패킷-전체-목록)

---

## 1. MQTT란 무엇인가

> **HiveMQ Essentials Part 1** | https://www.hivemq.com/blog/mqtt-essentials-part-1-introducing-mqtt/

### 정의

> "MQTT는 클라이언트-서버 기반의 **발행/구독 메시지 전송 프로토콜**이다.
> 경량이고, 개방적이며, 간단하고, 구현하기 쉽도록 설계되어 있다."

- 원래 약어: **MQ Telemetry Transport** → 현재는 고유 이름으로 사용
- 기반 프로토콜: **TCP/IP** 위에서 동작

### 탄생 배경

| 연도 | 사건 |
|------|------|
| 1999 | Andy Stanford-Clark(IBM)과 Arlen Nipper(Arcom)이 위성을 통한 **석유 파이프라인 모니터링**을 위해 창안 |
| 2010 | IBM이 MQTT 3.1을 **로열티 무료**로 공개 |
| 2014 | OASIS가 MQTT를 **공개 표준**으로 표준화 |
| 2019 | OASIS가 **MQTT 5.0** 비준 |

### HTTP와의 비교

| 항목 | MQTT | HTTP |
|------|------|------|
| 메시지 형식 | **이진(Binary)** | 텍스트 |
| 연결 방식 | **지속 연결** (Persistent) | 요청마다 연결 |
| 페이로드 크기 | 최소 2바이트 헤더 | 수백 바이트 헤더 |
| 통신 방향 | **양방향 Push** | 단방향 Pull (요청-응답) |
| 적합 환경 | 저전력, 저대역폭, IoT | 웹 브라우저, REST API |

### 주요 특징

- **이진 메시지 형식**: 최소 패킷 오버헤드 → 배터리, 대역폭 절약
- **데이터 불가지론적(Data-agnostic)**: JSON, XML, 바이너리, 이미지 등 모든 형식의 페이로드 전송 가능
- **TLS 암호화** 지원
- **QoS 3단계**: 전달 신뢰도를 상황에 맞게 선택

---

## 2. Publish/Subscribe 패턴

> **HiveMQ Essentials Part 2** | https://www.hivemq.com/blog/mqtt-essentials-part2-publish-subscribe/

### 구조

```
[발행자(Publisher)]          [구독자(Subscriber)]
    ESP32-01                     ESP32-02
    ESP32-03      →  [브로커]  →  PC (MQTTX)
    센서 장치         (라즈베리파이)  대시보드
```

- **발행자**: 메시지를 생성해 브로커에 전송
- **구독자**: 브로커로부터 메시지를 수신
- 발행자와 구독자는 **서로의 존재를 모름** → 브로커가 중간에서 라우팅

### 3가지 분리(Decoupling) 차원

#### 1. 공간적 분리 (Space Decoupling)
- 발행자와 구독자는 상대방의 **IP 주소나 포트를 교환하지 않음**
- 브로커 주소만 알면 됨

#### 2. 시간적 분리 (Time Decoupling)
- 발행자와 구독자가 **동시에 실행 중일 필요 없음**
- 구독자가 오프라인 상태여도 브로커가 메시지를 저장했다가 재연결 시 전달
- **조건**: 영구 세션(Persistent Session) + QoS 1 이상

#### 3. 동기화 분리 (Synchronization Decoupling)
- 메시지 발행/수신 중에도 다른 작업이 **중단 없이 계속됨**
- 비동기 방식

### MQTT vs 메시지 큐 비교

| 항목 | 전통적 메시지 큐 (RabbitMQ 등) | MQTT |
|------|-------------------------------|------|
| 수신자 | 단일 소비자 | **모든 구독자에게 브로드캐스트** |
| 큐 생성 | 명시적 생성 필요 | **즉석에서(on-the-fly) 생성** |
| 미구독 시 | 메시지 보관 | 구독자 없으면 소비되지 않을 수 있음 |

### 메시지 필터링 방법

| 방식 | 설명 | MQTT 기본 지원 |
|------|------|---------------|
| **토픽 기반** | 계층적 토픽으로 라우팅 | ✅ |
| 내용 기반 | 페이로드 내용으로 필터링 | 브로커 확장 필요 |
| 유형 기반 | 메시지 클래스로 필터링 | 브로커 확장 필요 |

---

## 3. 클라이언트, 브로커, 연결 수립

> **HiveMQ Essentials Part 3** | https://www.hivemq.com/blog/mqtt-essentials-part-3-client-broker-connection-establishment/

### MQTT 클라이언트

- MQTT 라이브러리를 실행하고 브로커에 **네트워크로 연결되는 모든 장치**
- 발행자(메시지 전송) 또는 구독자(메시지 수신) 역할
- **하나의 클라이언트가 두 역할 동시 수행 가능**
- 예: ESP32, ESP8266, 라즈베리파이, PC, 스마트폰 등

### MQTT 브로커

발행/구독 아키텍처의 **중앙 허브**. 주요 역할:

1. **메시지 라우팅**: 구독 토픽 기반으로 메시지 필터링 및 전달
2. **동시 연결 관리**: 수백만 개의 동시 연결 처리
3. **세션 관리**: 클라이언트 구독 유지, 오프라인 중 메시지 저장
4. **인증 및 권한 부여**: 클라이언트 자격 증명 검증

### 연결 수립 흐름

```
클라이언트                    브로커
    │                           │
    │──── CONNECT 패킷 전송 ───→│
    │                           │ (자격 증명 확인)
    │←─── CONNACK 패킷 응답 ───│
    │                           │
    │       (연결 유지)          │
    │                           │
    │──── DISCONNECT 전송 ────→│
```

### CONNECT 패킷 필드

| 필드 | 설명 | 비고 |
|------|------|------|
| **ClientId** | 클라이언트 고유 식별자 | 브로커 내에서 유일해야 함 |
| **CleanSession** | 세션 지속성 결정 플래그 | true = 새 시작, false = 이전 세션 유지 |
| **Username / Password** | 인증 자격 증명 | TLS 암호화 함께 사용 권장 |
| **Will Message** | 비정상 종료 시 자동 발행될 메시지 | LWT (Part 9 참조) |
| **KeepAlive** | 하트비트 간격 (초 단위) | 0 = 비활성화 |

### CONNACK 패킷 — 반환 코드

| 코드 | 의미 |
|------|------|
| **0** | 연결 승인 ✅ |
| 1 | 프로토콜 버전 허용 불가 |
| 2 | 클라이언트 식별자 거부 |
| 3 | 서버 사용 불가 |
| 4 | 잘못된 자격 증명 |
| 5 | 인증 실패 |

### CleanSession 플래그 동작 정리

| 설정값 | 동작 |
|--------|------|
| `true` | 이전 세션 데이터 폐기. 재연결 시 항상 새로 시작 |
| `false` | 이전 구독 및 미전달 메시지(QoS 1/2) 유지. 재연결 시 자동 복원 |

> **주의**: 직접 클라이언트-클라이언트 연결은 불가. 항상 브로커를 통해 통신.

---

## 4. Publish / Subscribe / Unsubscribe

> **HiveMQ Essentials Part 4** | https://www.hivemq.com/blog/mqtt-essentials-part-4-mqtt-publish-subscribe-unsubscribe/

### PUBLISH 패킷 구조

| 필드 | 설명 |
|------|------|
| **Topic Name** | 메시지 목적지 (예: `home/esp32/temperature`) |
| **QoS** | 전달 품질 수준 (0, 1, 2) |
| **Retain Flag** | true 시 브로커가 해당 토픽 마지막 메시지 보관 |
| **DUP Flag** | 재전송 메시지 표시 (QoS 1/2에서만 사용, 라이브러리가 자동 처리) |
| **Packet Identifier** | QoS 1/2에서 응답 매칭용 고유 ID (최대 65,535) |
| **Payload** | 실제 데이터 (JSON, 바이너리 등 모든 형식) |

> **중요**: 발행자는 구독자 수나 수신 여부에 대한 **피드백을 받지 못함**

### SUBSCRIBE 패킷 구조

| 필드 | 설명 |
|------|------|
| **Packet Identifier** | SUBACK 응답 매칭용 고유 ID |
| **Subscription List** | (토픽 필터, QoS) 쌍 목록. 하나의 패킷에 여러 개 포함 가능 |

> **규칙**: 겹치는 구독이 있으면 브로커는 **가장 높은 QoS**로 전달

### SUBACK 반환 코드

| 코드 | 의미 |
|------|------|
| **0** | QoS 0으로 구독 성공 |
| **1** | QoS 1으로 구독 성공 |
| **2** | QoS 2로 구독 성공 |
| **128 (0x80)** | 구독 실패 |

### UNSUBSCRIBE / UNSUBACK

- **UNSUBSCRIBE**: Packet Identifier + 해제할 토픽 목록 (QoS 지정 불필요)
- **UNSUBACK**: `0` = 성공, `17` = 유효하지 않은 토픽으로 실패

---

## 5. 토픽과 설계 규칙

> **HiveMQ Essentials Part 5** | https://www.hivemq.com/blog/mqtt-essentials-part-5-mqtt-topics-best-practices/

### 토픽 정의

> "연결된 클라이언트의 메시지를 필터링하는 **UTF-8 문자열**"

- `/` 를 계층 구분자로 사용
- **대소문자 구분** (`home/Temperature` ≠ `home/temperature`)
- 최대 **65,535 바이트**
- 최소 **1글자** 이상 필요

### 토픽 구조 예시

```
home/livingroom/temperature
sensor/esp32-01/humidity
manufacturing/lineA/motor/status
```

### 와일드카드 (구독 시에만 사용, 발행 시 사용 불가)

#### `+` : 단일 레벨 와일드카드

정확히 **하나의 토픽 레벨**을 대체

```
home/+/temperature
  → home/livingroom/temperature  ✅
  → home/kitchen/temperature     ✅
  → home/livingroom/bedroom/temperature  ❌ (레벨 2개)
```

#### `#` : 다중 레벨 와일드카드

**나머지 모든 레벨** 포함. 반드시 **마지막 위치**에 있어야 함

```
home/groundfloor/#
  → home/groundfloor/livingroom/temperature  ✅
  → home/groundfloor/kitchen/humidity        ✅
  → home/firstfloor/bedroom                  ❌
```

> **안티패턴**: `#` 단독 구독 → 브로커 모든 메시지 수신. 성능 문제 유발.

### `$SYS` 예약 토픽

- `$`로 시작하는 토픽은 **브로커 내부 통계용**으로 예약
- `#` 구독으로 매칭 **안 됨**
- `$SYS` 토픽에 **발행 금지**

```
$SYS/broker/clients/connected   → 현재 연결된 클라이언트 수
$SYS/broker/messages/sent       → 전송된 메시지 수
$SYS/broker/uptime              → 브로커 실행 시간
```

### 토픽 설계 모범 사례

| 규칙 | 설명 |
|------|------|
| 계층적으로 설계 | `company/site/area/device/measurement` |
| 구체적으로 | 넓은 와일드카드보다 좁은 토픽 선호 |
| 고유 ID 포함 | `client-01/status`, `esp32-a1b2/temp` |
| 슬래시로 시작 금지 | `/home/temp` ❌ → `home/temp` ✅ |
| 공백 금지 | `home/living room/temp` ❌ |
| 간결하게 | 토픽이 짧을수록 네트워크 트래픽 절약 |

---

## 6. QoS (서비스 품질 수준)

> **HiveMQ Essentials Part 6** | https://www.hivemq.com/blog/mqtt-essentials-part-6-mqtt-quality-of-service-levels/

### QoS 개요

| 레벨 | 보장 수준 | 핸드셰이크 | 특징 |
|------|----------|-----------|------|
| **QoS 0** | 최대 1회 (At-most-once) | 없음 (1단계) | 빠름, 손실 가능 |
| **QoS 1** | 최소 1회 (At-least-once) | PUBACK (2단계) | 전달 보장, 중복 가능 |
| **QoS 2** | 정확히 1회 (Exactly-once) | 4단계 핸드셰이크 | 가장 신뢰성 높음, 오버헤드 큼 |

---

### QoS 0 — "발사 후 망각 (Fire and Forget)"

```
발행자  →  [PUBLISH]  →  브로커  →  [PUBLISH]  →  구독자
         (확인 응답 없음, 재전송 없음)
```

- 메시지가 전달되지 않아도 **재전송하지 않음**
- 가장 빠르고 오버헤드 없음
- **사용 사례**: 비중요 센서 원격 측정, 고빈도 업데이트 (개별 값 손실 허용)

---

### QoS 1 — 전달 보장, 중복 가능

```
발행자  →  [PUBLISH]  →  브로커  →  [PUBACK]  →  발행자
브로커  →  [PUBLISH]  →  구독자  →  [PUBACK]  →  브로커
```

- 전달을 보장하지만 **중복 수신 가능** (DUP 플래그로 표시)
- **사용 사례**: 중요 데이터 전송, 중복 처리 가능한 IoT 명령

---

### QoS 2 — 중복 없는 정확한 1회 전달

```
발행자  →  [PUBLISH]  →  브로커/구독자  ← 1단계
발행자  ←  [PUBREC]   ←  브로커/구독자  ← 2단계
발행자  →  [PUBREL]   →  브로커/구독자  ← 3단계
발행자  ←  [PUBCOMP]  ←  브로커/구독자  ← 4단계
```

| 패킷 | 역할 |
|------|------|
| **PUBACK** | QoS 1 수신 확인 |
| **PUBREC** | QoS 2 — 1단계 수신 확인 |
| **PUBREL** | QoS 2 — 2단계 메시지 릴리즈 |
| **PUBCOMP** | QoS 2 — 4단계 완료 확인 |

- **사용 사례**: 긴급 종료 명령, 금융 거래, 중복 실행 시 위험한 명령

---

### QoS 다운그레이드 규칙 (중요!)

> **최종 전달 QoS = MIN(발행자 QoS, 구독자 QoS)**

```
발행자 QoS 2  +  구독자 QoS 1  →  브로커가 QoS 1로 전달 (중복 가능)
발행자 QoS 1  +  구독자 QoS 0  →  브로커가 QoS 0으로 전달 (손실 가능)
```

각 구독자는 자신이 요청한 QoS로 **개별 협상**한다.

---

### QoS 선택 가이드

| 상황 | 권장 QoS | 이유 |
|------|----------|------|
| 온습도 센서 주기적 업데이트 | **0** | 비중요, 다음 값이 곧 옴 |
| 전력 소비 모니터링 | **1** | 중요하지만 중복 처리 가능 |
| 모터 ON/OFF 명령 | **2** | 중복 실행 시 위험 |
| GPS 위치 실시간 업데이트 | **0** | 실시간성이 정확도보다 중요 |
| DB에 저장하는 센서 기록 | **1 or 2** | 데이터 유실 금지 |

---

### 라이브러리별 QoS 지원 현황

| 라이브러리 | Publish QoS | Subscribe QoS |
|-----------|-------------|---------------|
| **PubSubClient** (Arduino) | **0만 가능** | 0, 1 |
| **umqtt** (MicroPython) | 0, 1 | 0, 1 (**QoS 2 미지원**) |

---

## 7. 영구 세션과 메시지 큐잉

> **HiveMQ Essentials Part 7** | https://www.hivemq.com/blog/mqtt-essentials-part-7-persistent-session-queuing-messages/

### 핵심 개념

**영구 세션(Persistent Session)**: 연결이 끊겨도 클라이언트의 구독 및 메시지 상태를 브로커가 유지하는 기능.

```
CleanSession = false  →  영구 세션
CleanSession = true   →  클린 세션 (매번 새로 시작)
```

### 영구 세션(CleanSession=false)에서 브로커가 저장하는 것

1. **세션 존재 자체** — 구독 없어도 유지
2. **모든 구독 목록** — 재연결 시 재구독 불필요
3. **미확인 QoS 1/2 아웃바운드 메시지**
4. **오프라인 중 발행된 QoS 1/2 메시지** (재연결 시 전달)
5. **완료 대기 중인 QoS 2 핸드셰이크 상태**

> QoS 0 메시지는 **절대 큐에 저장되지 않음**

### 세션 복원 흐름

```
클라이언트 재연결
  → CleanSession=false 로 CONNECT 전송
  → 브로커가 CONNACK에 sessionPresent=true 포함
  → 이전 구독 자동 복원 + 오프라인 중 쌓인 메시지 전달
```

### 언제 영구 세션을 써야 하나?

| 영구 세션 (CleanSession=false) | 클린 세션 (CleanSession=true) |
|-------------------------------|------------------------------|
| 모든 메시지 수신이 필요한 장치 | 발행 전용 클라이언트 |
| 자원이 제한된 임베디드 장치 | 오프라인 메시지 불필요 |
| QoS 1/2 발행자 | 매 연결마다 새로 구독하는 장치 |
| ESP32 deep sleep 후 재연결 | MQTTX 같은 테스트 클라이언트 |

---

## 8. Retained 메시지

> **HiveMQ Essentials Part 8** | https://www.hivemq.com/blog/mqtt-essentials-part-8-retained-messages/

### 정의

> "브로커가 각 토픽에 대해 **가장 최근의 retained 메시지 하나**를 보존한다."

- 발행 시 `retain = true` 플래그를 설정
- 새로운 retained 메시지가 오면 **이전 것을 덮어씀**

### 동작 방식

```
ESP32  →  [PUBLISH retain=true]  →  브로커 (메시지 보관)

나중에 새 구독자 MQTTX 연결
  →  구독 즉시 retained 메시지 수신
  →  최근 값을 별도 요청 없이 바로 알 수 있음
```

- **와일드카드 구독자**도 매칭되는 토픽의 retained 메시지 수신
- 영구 세션과 **독립적**으로 동작 (클라이언트 단위가 아닌 **토픽 단위**)

### Retained 메시지 삭제 방법

```
해당 토픽에 빈 페이로드(0 바이트)의 retained 메시지 발행
→ 브로커가 삭제 요청으로 인식하고 제거
```

### 주요 사용 사례

- 장치 온라인/오프라인 상태 표시
- 주기적 센서 값 (온도, 습도, GPS)
- 새로 연결된 장치가 현재 상태를 **즉시** 파악해야 할 때

---

## 9. Last Will and Testament (LWT)

> **HiveMQ Essentials Part 9** | https://www.hivemq.com/blog/mqtt-essentials-part-9-last-will-and-testament/

### 정의

> "클라이언트가 **예상치 못하게 연결이 끊겼을 때** 브로커가 자동으로 발행하는 메시지"

- CONNECT 패킷에 미리 등록 (별도 패킷 없음)
- 정상 종료(DISCONNECT)에서는 발행되지 않음

### Will 메시지 구성 요소

| 필드 | 설명 |
|------|------|
| **Topic** | LWT가 발행될 토픽 |
| **Payload** | 메시지 내용 |
| **QoS** | 전달 품질 (0, 1, 2) |
| **Retain Flag** | retained 메시지로 저장 여부 |

### LWT가 전송되는 조건

| 상황 | LWT 발행 여부 |
|------|--------------|
| I/O 오류 / 네트워크 단절 | ✅ 발행 |
| Keep Alive 타임아웃 초과 | ✅ 발행 |
| DISCONNECT 없이 연결 종료 | ✅ 발행 |
| 프로토콜 오류로 브로커가 강제 종료 | ✅ 발행 |
| 정상적인 DISCONNECT 전송 후 종료 | ❌ 발행 안 함 |
| 동일 ClientId로 새 클라이언트 재연결 | ❌ 발행 안 함 |

### Retained 메시지와 결합한 온라인/오프라인 감지 패턴

```
[연결 시]
ESP32 → CONNECT (Will: topic="device/status", payload="offline", retain=true)
ESP32 → PUBLISH (topic="device/status", payload="online", retain=true)

[정상 운작 중]
구독자는 "online" retained 메시지 수신

[비정상 연결 끊김]
브로커 → PUBLISH LWT (topic="device/status", payload="offline", retain=true)
구독자는 "offline" 수신
```

---

## 10. Keep Alive와 Client Take-Over

> **HiveMQ Essentials Part 10** | https://www.hivemq.com/blog/mqtt-essentials-part-10-alive-client-take-over/

### Keep Alive 메커니즘

**목적**: 클라이언트와 브로커 간 연결이 살아 있는지 주기적으로 확인

- 연결 시 Keep Alive 간격(초) 협상
- 해당 시간 내에 **아무 패킷도 없으면** PINGREQ 전송

#### PINGREQ / PINGRESP

| 패킷 | 방향 | 내용 |
|------|------|------|
| **PINGREQ** | 클라이언트 → 브로커 | 하트비트 신호 (페이로드 없음) |
| **PINGRESP** | 브로커 → 클라이언트 | 응답 (페이로드 없음) |

#### 1.5배 규칙

> 브로커가 **Keep Alive × 1.5** 시간 내에 아무것도 수신하지 못하면
> → 클라이언트 **강제 연결 해제** + LWT 발행

```
Keep Alive = 60초
→ 90초 무응답 시 브로커가 연결 종료
```

- 최대값: 18시간 12분 15초
- `0`으로 설정 시 Keep Alive **완전 비활성화**

### Half-Open 연결 문제

> 모바일/위성 네트워크에서 TCP 연결이 열린 것처럼 보이지만
> 실제로는 데이터를 **버리는("블랙홀") 상태**

Keep Alive 메커니즘이 이를 감지하여 불필요한 리소스 낭비 방지.

### Client Take-Over

**발생 조건**: 동일한 ClientId로 클라이언트가 재연결 시도

```
1. 이전 연결이 half-open 상태로 남아 있음
2. 같은 ClientId로 CONNECT 전송
3. 브로커가 기존 half-open 연결 감지
4. 브로커가 이전 연결 강제 종료
5. 새 연결 수립
6. CleanSession=false이면 이전 세션 자동 재개
```

> **다른 ClientId**로 재연결 시: 완전히 새 세션 생성, 이전 구독/메시지 이전 불가

### 주의사항

- 장치별 **고유 ClientId** 사용 필수
- 강력한 인증(Username/Password 또는 TLS) 적용

---

## 11. 제어 패킷 전체 목록

> *(C = 클라이언트, B = 브로커)*

| 패킷 | 방향 | QoS | 설명 |
|------|------|-----|------|
| **CONNECT** | C → B | - | 연결 요청 |
| **CONNACK** | B → C | - | 연결 응답 |
| **PUBLISH** | C ↔ B | 0, 1, 2 | 메시지 발행 |
| **PUBACK** | B ↔ C | 1 | 발행 확인 |
| **PUBREC** | B ↔ C | 2 | 발행 수신 확인 (1단계) |
| **PUBREL** | B ↔ C | 2 | 발행 릴리즈 (2단계) |
| **PUBCOMP** | B ↔ C | 2 | 발행 완료 (3단계) |
| **SUBSCRIBE** | C → B | - | 구독 요청 |
| **SUBACK** | B → C | - | 구독 확인 |
| **UNSUBSCRIBE** | C → B | - | 구독 해제 요청 |
| **UNSUBACK** | B → C | - | 구독 해제 확인 |
| **PINGREQ** | C → B | - | Keep Alive 하트비트 |
| **PINGRESP** | B → C | - | Keep Alive 하트비트 응답 |
| **DISCONNECT** | C → B | - | 정상 연결 해제 |

---

## 기능 상호작용 매트릭스

| 기능 | cleanSession=true | cleanSession=false | QoS 0 | QoS 1/2 |
|------|:-----------------:|:------------------:|:-----:|:-------:|
| 구독 유지 (재연결 시) | ❌ | ✅ | - | - |
| 오프라인 메시지 큐잉 | ❌ | ✅ | ❌ (항상 불가) | ✅ |
| Retained 메시지 수신 | ✅ | ✅ | ✅ | ✅ |
| LWT 동작 | ✅ | ✅ | ✅ | ✅ |
| CONNACK sessionPresent | false | true (세션 있을 때) | - | - |

---

## 참고 자료

| 항목 | 링크 |
|------|------|
| HiveMQ MQTT Essentials | https://www.hivemq.com/mqtt-essentials/ |
| Eclipse Mosquitto 공식 문서 | https://mosquitto.org/documentation/ |
| PubSubClient (Arduino) | https://github.com/knolleary/pubsubclient |
| umqtt (MicroPython) | https://github.com/micropython/micropython-lib/tree/master/micropython/umqtt.robust |
| MQTTX (GUI 클라이언트) | https://mqttx.app/ |
| MQTT v3.1.1 공식 스펙 | https://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html |
