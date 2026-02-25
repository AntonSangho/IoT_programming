// ESP32 HTTP 접속 디버그 코드
// 어디서 문제가 발생하는지 단계별로 확인

#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "YOUR_WIFI_SSID";         // 수정 필요
const char* password = "YOUR_WIFI_PASSWORD";  // 수정 필요

String serverIP = "10.42.0.1";               // 수정 필요: 라즈베리파이 IP
int serverPort = 5000;

// [핵심] ESP32는 WiFiClient를 명시적으로 만들어야 하는 경우가 있음
WiFiClient client;

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("==============================");
  Serial.println("  ESP32 HTTP 디버그 테스트");
  Serial.println("==============================");

  // ===== 1단계: WiFi 연결 확인 =====
  Serial.println();
  Serial.println("[1단계] WiFi 연결 시도...");
  Serial.print("  SSID: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  int retry = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    retry++;
    if (retry > 20) {
      Serial.println();
      Serial.println("  WiFi 연결 실패! SSID와 비밀번호를 확인하세요.");
      Serial.print("  WiFi 상태 코드: ");
      Serial.println(WiFi.status());
      // 상태 코드 설명
      // 1: SSID 찾을 수 없음
      // 4: 연결 실패 (비밀번호 오류 가능)
      // 6: 연결 끊김
      return;
    }
  }

  Serial.println();
  Serial.println("  WiFi 연결 성공!");
  Serial.print("  ESP32 IP:  ");
  Serial.println(WiFi.localIP());
  Serial.print("  서브넷:    ");
  Serial.println(WiFi.subnetMask());
  Serial.print("  게이트웨이: ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("  신호세기:  ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");

  // ===== 2단계: IP 대역 비교 =====
  Serial.println();
  Serial.println("[2단계] 네트워크 대역 확인...");
  Serial.print("  ESP32 IP:  ");
  Serial.println(WiFi.localIP());
  Serial.print("  서버  IP:  ");
  Serial.println(serverIP);

  // 간단한 대역 비교 (앞 3자리)
  IPAddress myIP = WiFi.localIP();
  Serial.print("  ESP32 대역: ");
  Serial.print(myIP[0]); Serial.print("."); Serial.print(myIP[1]); Serial.print("."); Serial.println(myIP[2]);
  Serial.print("  서버  대역: ");
  Serial.println(serverIP.substring(0, serverIP.lastIndexOf(".")));

  // ===== 3단계: TCP 연결 테스트 =====
  Serial.println();
  Serial.println("[3단계] TCP 연결 테스트 (HTTP 전에 기본 연결 확인)...");
  Serial.print("  ");
  Serial.print(serverIP);
  Serial.print(":");
  Serial.print(serverPort);
  Serial.println(" 연결 시도...");

  if (client.connect(serverIP.c_str(), serverPort)) {
    Serial.println("  TCP 연결 성공!");
    client.stop();
  } else {
    Serial.println("  TCP 연결 실패!");
    Serial.println("  → 서버가 실행 중인지 확인하세요");
    Serial.println("  → IP 주소가 맞는지 확인하세요");
    Serial.println("  → 같은 네트워크인지 확인하세요");
  }
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi 끊김!");
    delay(3000);
    return;
  }

  // ===== 4단계: HTTP GET 테스트 =====
  Serial.println();
  Serial.println("[4단계] HTTP GET 테스트...");

  HTTPClient http;

  // 방법 1: WiFiClient를 전달하는 방식 (ESP32 권장)
  String url = "http://" + serverIP + ":" + String(serverPort) + "/ping";
  Serial.print("  URL: ");
  Serial.println(url);

  http.begin(client, url);

  int httpCode = http.GET();

  Serial.print("  응답 코드: ");
  Serial.println(httpCode);

  if (httpCode > 0) {
    String response = http.getString();
    Serial.print("  응답 내용: ");
    Serial.println(response);
    Serial.println("  ==> 성공!");
  } else {
    Serial.print("  에러! HTTPClient 에러코드: ");
    Serial.println(httpCode);
    Serial.println();
    Serial.println("  에러코드 설명:");
    Serial.println("  -1: 연결 거부 (서버 IP/포트 확인)");
    Serial.println("  -2: 전송 실패");
    Serial.println("  -3: 헤더 수신 실패");
    Serial.println("  -4: 본문 없음");
    Serial.println("  -11: 타임아웃");
  }

  http.end();

  delay(5000);
}
