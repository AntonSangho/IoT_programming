// ESP32 → Flask 서버 HTTP 접속 테스트 코드
// 센서 없이 더미 데이터로 통신 확인용

#include <WiFi.h>
#include <HTTPClient.h>

// ===== WiFi 설정 =====
// [수정 필요] 사용하는 WiFi 이름과 비밀번호로 변경
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// ===== Flask 서버 설정 =====
// [수정 필요] Flask 서버가 실행 중인 PC의 IP 주소로 변경
// PC에서 ifconfig 또는 ip addr 명령어로 확인
String serverURL = "http://192.168.0.100:5000/api/sensor";

void setup() {
  Serial.begin(115200);

  // WiFi 연결
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
  // WiFi 연결 상태 확인 후 HTTP POST 전송
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    http.begin(serverURL);
    http.addHeader("Content-Type", "application/json");

    // 더미 데이터로 테스트 (센서 연결 전 통신 확인용)
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

  // 5초마다 전송
  delay(5000);
}
