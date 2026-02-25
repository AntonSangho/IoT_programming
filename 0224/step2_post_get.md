# 2단계: POST/GET으로 데이터 주고받기

## 목표

ESP32가 더미 데이터를 POST로 보내고, 브라우저에서 GET으로 확인한다.

```
[ESP32] --POST JSON--> [Flask /api/sensor] --GET--> [브라우저에서 확인]
```

---

## 라즈베리파이 코드 - app.py

```python
from flask import Flask, jsonify, request

app = Flask(__name__)

latest_sensor = {"temperature": None, "humidity": None}

# ESP32가 데이터를 보내는 엔드포인트
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

# 저장된 데이터를 확인하는 엔드포인트
@app.route('/api/sensor', methods=['GET'])
def get_sensor():
    return jsonify(latest_sensor)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
```

실행:
```bash
python app.py
```

---

## ESP32 코드 - esp32_http.ino

```cpp
#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "YOUR_WIFI_SSID";         // 수정 필요
const char* password = "YOUR_WIFI_PASSWORD";  // 수정 필요

String serverURL = "http://라즈베리파이IP:5000/api/sensor";  // 수정 필요

void setup() {
  Serial.begin(115200);

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
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    http.begin(serverURL);
    http.addHeader("Content-Type", "application/json");

    // 더미 데이터로 POST 전송
    String jsonData = "{\"temperature\":25.3,\"humidity\":60.5}";
    int httpCode = http.POST(jsonData);

    if (httpCode > 0) {
      String response = http.getString();
      Serial.print("서버 응답 코드: ");
      Serial.println(httpCode);
      Serial.print("서버 응답: ");
      Serial.println(response);
    } else {
      Serial.print("전송 실패, 에러코드: ");
      Serial.println(httpCode);
    }

    http.end();
  } else {
    Serial.println("WiFi 연결 끊김!");
  }

  delay(5000);
}
```

---

## 1단계와 달라진 점

| 항목 | 1단계 | 2단계 |
|------|-------|-------|
| HTTP 메서드 | GET | **POST** |
| 엔드포인트 | `/ping` | `/api/sensor` |
| 데이터 | 없음 | **JSON (온도, 습도)** |
| Content-Type | 없음 | `application/json` |

---

## 확인 포인트

| 확인 방법 | 예상 결과 |
|-----------|----------|
| ESP32 시리얼 모니터 | `서버 응답: {"status":"ok"}` |
| 라즈베리파이 터미널 | `수신: 온도=25.3, 습도=60.5` |
| 브라우저 `http://라즈베리파이IP:5000/api/sensor` | `{"humidity":60.5,"temperature":25.3}` |

---

## curl로 먼저 테스트하기 (ESP32 없이)

라즈베리파이에서 직접 POST를 보내 Flask가 정상 동작하는지 확인할 수 있다.

```bash
# POST 테스트
curl -X POST http://localhost:5000/api/sensor \
  -H "Content-Type: application/json" \
  -d '{"temperature": 25.3, "humidity": 60.5}'

# GET으로 확인
curl http://localhost:5000/api/sensor
```

---

## 트러블슈팅

| 증상 | 해결 방법 |
|------|----------|
| `전송 실패, 에러코드: -1` | serverURL의 IP, 포트 확인 |
| 서버에서 400 에러 | `Content-Type: application/json` 헤더 확인 |
| 브라우저에서 `null` 표시 | ESP32가 아직 데이터를 보내지 않은 상태 |

> 더미 데이터가 정상적으로 오가면 [3단계](step3_sensor.md)로 넘어갑니다.
