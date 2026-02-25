# ESP32 - 라즈베리파이 HTTP 통신 다이어그램

## 전체 구조

```mermaid
graph LR
    subgraph ESP32
        S[DHT 센서] --> E[ESP32]
    end

    subgraph 라즈베리파이
        F[Flask 서버]
        W[웹페이지]
    end

    subgraph 사용자
        B[브라우저]
    end

    E -- "WiFi / HTTP POST" --> F
    F --> W
    B -- "HTTP GET" --> W
```

---

## 네트워크 구조

```mermaid
graph TB
    R["공유기 (WiFi)"]

    subgraph 같은 네트워크 192.168.0.x
        E["ESP32<br/>192.168.0.xxx<br/>(WiFi)"]
        RP_W["라즈베리파이 wlan0<br/>192.168.0.10<br/>(WiFi)"]
        RP_E["라즈베리파이 eth0<br/>192.168.0.20<br/>(유선)"]
    end

    R --- E
    R --- RP_W
    R --- RP_E

    E -- "접속 가능" --> RP_W
    E -- "접속 가능" --> RP_E

    style E fill:#f9f,stroke:#333
    style RP_W fill:#bbf,stroke:#333
    style RP_E fill:#bbf,stroke:#333
```

> `host='0.0.0.0'`으로 Flask를 실행하면 wlan0, eth0 **둘 다** 접속 가능

---

## 1단계: 접속 테스트 (ping/pong)

```mermaid
sequenceDiagram
    participant E as ESP32
    participant F as Flask (라즈베리파이)

    Note over E: WiFi 연결
    E->>E: WiFi.begin(ssid, password)
    E->>E: WiFi 연결 성공!

    loop 3초마다
        E->>F: GET /ping
        F->>F: print("ESP32에서 접속!")
        F-->>E: "pong"
        E->>E: 시리얼 모니터: 서버 접속 성공!
    end
```

---

## 2단계: POST/GET 데이터 전송

```mermaid
sequenceDiagram
    participant E as ESP32
    participant F as Flask (라즈베리파이)
    participant B as 브라우저

    Note over E: 더미 데이터 전송

    loop 5초마다
        E->>F: POST /api/sensor<br/>{"temperature":25.3, "humidity":60.5}
        F->>F: latest_sensor에 저장
        F-->>E: {"status": "ok"}
    end

    B->>F: GET /api/sensor
    F-->>B: {"temperature":25.3, "humidity":60.5}
```

---

## 3단계: 센서 데이터 + 웹페이지

```mermaid
sequenceDiagram
    participant S as DHT11 센서
    participant E as ESP32
    participant F as Flask (라즈베리파이)
    participant B as 브라우저

    loop 5초마다
        S->>E: 온도: 26.1 / 습도: 58.3
        E->>F: POST /api/sensor<br/>{"temperature":26.1, "humidity":58.3}
        F->>F: latest_sensor에 저장
        F-->>E: {"status": "ok"}
    end

    loop 5초마다 자동 새로고침
        B->>F: GET /
        F-->>B: 웹페이지 (온도: 26.1C, 습도: 58.3%)
    end
```

---

## 단계별 진행 흐름

```mermaid
graph LR
    A["1단계<br/>접속 테스트<br/>GET /ping → pong"] --> B["2단계<br/>데이터 전송<br/>POST 더미 데이터"]
    B --> C["3단계<br/>센서 연결<br/>POST 실측 데이터"]

    style A fill:#e8f5e9,stroke:#333
    style B fill:#e3f2fd,stroke:#333
    style C fill:#fff3e0,stroke:#333
```
