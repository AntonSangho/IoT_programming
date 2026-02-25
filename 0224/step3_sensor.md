# 3단계: 실제 센서 데이터 연결

## 목표

DHT 센서의 실측 데이터를 Flask 웹페이지에 실시간 표시한다.

```
[ESP32 + DHT11] --POST 실측 데이터--> [Flask] --웹페이지--> [브라우저]
```

---

## 회로 연결

```
ESP32        DHT11
------       -----
3.3V    -->  VCC
GND     -->  GND
GPIO 4  -->  DATA
```

---

## 2단계와 달라진 점

| 항목 | 2단계 | 3단계 |
|------|-------|-------|
| ESP32 데이터 | 더미 `25.3, 60.5` | **DHT 센서 실측값** |
| Flask | API만 | **웹페이지 + API** |
| 라이브러리 | WiFi, HTTPClient | + **DHT** |

---

## 라즈베리파이 코드 - app.py

```python
from flask import Flask, render_template, jsonify, request
from datetime import datetime

app = Flask(__name__)

latest_sensor = {"temperature": None, "humidity": None}

@app.route('/')
def index():
    now = datetime.now().strftime("%Y년%m월%d일 %H:%M")
    return render_template("index.html", sensor=latest_sensor, now=now)

@app.route('/api/sensor', methods=['POST'])
def receive_sensor():
    global latest_sensor
    data = request.get_json()
    latest_sensor = {
        "temperature": data.get("temperature"),
        "humidity": data.get("humidity")
    }
    print(f"수신: 온도={latest_sensor['temperature']}, 습도={latest_sensor['humidity']}")
    return jsonify({"status": "ok"})

@app.route('/api/sensor', methods=['GET'])
def get_sensor():
    return jsonify(latest_sensor)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
```

---

## 라즈베리파이 - templates/index.html

```html
<!DOCTYPE html>
<html>
<head>
    <title>센서 데이터</title>
    <meta http-equiv="refresh" content="5">
</head>
<body>
    <h1>센서 기록</h1>
    <p> 측정 시각  : {{ now }}</p>

    {% if sensor.temperature %}
        <p> 온도 : {{ sensor.temperature }} C</p>
        <p> 습도 : {{ sensor.humidity }} %</p>

        {% if sensor.temperature >= 28 %}
            <p> 더움 </p>
        {% elif sensor.temperature >= 24 %}
            <p> 보통 </p>
        {% else %}
            <p> 쾌적 </p>
        {% endif %}
    {% else %}
        <p> 센서 데이터를 기다리는 중... </p>
    {% endif %}
</body>
</html>
```

---

## ESP32 코드 - esp32_sensor.ino

```cpp
#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

const char* ssid = "YOUR_WIFI_SSID";         // 수정 필요
const char* password = "YOUR_WIFI_PASSWORD";  // 수정 필요

String serverURL = "http://라즈베리파이IP:5000/api/sensor";  // 수정 필요

#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  dht.begin();

  WiFi.begin(ssid, password);
  Serial.print("WiFi 연결 중");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("WiFi 연결 완료! IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("센서 읽기 실패!");
    delay(2000);
    return;
  }

  Serial.print("온도: ");
  Serial.print(temperature);
  Serial.print(" / 습도: ");
  Serial.println(humidity);

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    http.begin(serverURL);
    http.addHeader("Content-Type", "application/json");

    String jsonData = "{\"temperature\":" + String(temperature) + ",\"humidity\":" + String(humidity) + "}";
    int httpCode = http.POST(jsonData);

    if (httpCode > 0) {
      String response = http.getString();
      Serial.println("서버 응답: " + response);
    } else {
      Serial.println("전송 실패! 코드: " + String(httpCode));
    }

    http.end();
  }

  delay(5000);
}
```

### Arduino IDE 라이브러리 설치

3단계에서는 DHT 라이브러리가 필요하다.

1. Arduino IDE → 스케치 → 라이브러리 관리
2. `DHT sensor library` 검색 → Adafruit 버전 설치
3. 의존성 `Adafruit Unified Sensor`도 함께 설치

---

## 최종 확인

| 확인 방법 | 예상 결과 |
|-----------|----------|
| ESP32 시리얼 모니터 | `온도: 26.1 / 습도: 58.3` + `서버 응답: {"status":"ok"}` |
| 브라우저 `http://라즈베리파이IP:5000` | 웹페이지에 실시간 온도/습도 표시 |
| 브라우저 `http://라즈베리파이IP:5000/api/sensor` | `{"humidity":58.3,"temperature":26.1}` |

---

## 트러블슈팅

| 증상 | 해결 방법 |
|------|----------|
| `센서 읽기 실패!` 반복 | 배선 확인, DHTPIN 번호 확인 |
| 온도가 비정상적 (0 또는 nan) | DHT11/DHT22 타입 맞는지 확인 |
| 웹페이지에 `센서 데이터를 기다리는 중` | ESP32가 아직 데이터를 보내지 않은 상태, 시리얼 모니터 확인 |

---

## 전체 단계 요약

| 단계 | 목표 | ESP32 | Flask | 센서 |
|------|------|-------|-------|------|
| [1단계](step1_ping_test.md) | 접속 확인 | GET → `/ping` | `pong` 응답 | 없음 |
| [2단계](step2_post_get.md) | 데이터 전송 | POST 더미 데이터 | API 저장/조회 | 없음 |
| **3단계** | **실제 운영** | **POST 실측 데이터** | **웹페이지 + API** | **DHT11** |
