// ESP32 → 라즈베리파이 Flask 접속 테스트
// 시리얼 모니터에서 결과 확인 (115200 baud)

#include <WiFi.h>
#include <HTTPClient.h>

// [수정 필요] WiFi 설정
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// [수정 필요] 라즈베리파이 IP (ip addr show wlan0 또는 eth0 으로 확인)
String serverURL = "http://라즈베리파이IP:5000/ping";

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("=== ESP32 접속 테스트 시작 ===");

  // 1단계: WiFi 연결
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
  // 2단계: Flask 서버 접속 테스트
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
