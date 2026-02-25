# 1단계: ESP32 → 라즈베리파이 접속 테스트

## 목표

ESP32가 라즈베리파이 Flask 서버에 접속할 수 있는지 확인한다.

```
[ESP32] --WiFi GET 요청--> [라즈베리파이 Flask] --응답--> "pong"
```

## 사전 준비

### 라즈베리파이 IP 확인

```bash
# 무선 IP
ip addr show wlan0

# 유선 IP
ip addr show eth0
```

ESP32와 라즈베리파이가 **같은 WiFi 공유기**에 연결되어 있어야 한다.

---

## 라즈베리파이 코드 - test_server.py

```python
from flask import Flask

app = Flask(__name__)

@app.route('/ping')
def ping():
    print("ESP32에서 접속!")
    return "pong"

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
```

실행:
```bash
python test_server.py
```

---

## ESP32 코드 - esp32_http_test.ino

```cpp
#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "YOUR_WIFI_SSID";         // 수정 필요
const char* password = "YOUR_WIFI_PASSWORD";  // 수정 필요

String serverURL = "http://라즈베리파이IP:5000/ping";  // 수정 필요

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("=== ESP32 접속 테스트 시작 ===");

  WiFi.begin(ssid, password);
  Serial.print("WiFi 연결 중");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi 연결 성공!");
  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverURL);

    int httpCode = http.GET();

    if (httpCode == 200) {
      String response = http.getString();
      Serial.println("서버 접속 성공! 응답: " + response);
    } else {
      Serial.println("서버 접속 실패! 코드: " + String(httpCode));
    }

    http.end();
  } else {
    Serial.println("WiFi 끊김!");
  }

  delay(3000);
}
```

---

## 수정할 곳 (3곳)

| 항목 | 수정 내용 |
|------|----------|
| `ssid` | WiFi 이름 |
| `password` | WiFi 비밀번호 |
| `serverURL` | `http://라즈베리파이IP:5000/ping` |

---

## 확인 포인트

| 위치 | 성공 시 출력 |
|------|-------------|
| ESP32 시리얼 모니터 | `서버 접속 성공! 응답: pong` |
| 라즈베리파이 터미널 | `ESP32에서 접속!` |

---

## 트러블슈팅

| 증상 | 해결 방법 |
|------|----------|
| `WiFi 연결 중......` 계속됨 | ssid, password 확인 |
| `서버 접속 실패! 코드: -1` | 라즈베리파이 IP 확인, Flask 실행 여부 확인 |
| Flask는 실행되는데 접속 안됨 | 라즈베리파이에서 `curl http://localhost:5000/ping` 으로 자체 테스트 |

> 이 단계가 성공하면 [2단계](step2_post_get.md)로 넘어갑니다.
