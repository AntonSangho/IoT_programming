/******************************************************************************
 * step3_temperature_mqtt.ino
 * 3단계: 온도 센서 조건부 퍼블리시 (QoS 0)
 *
 * 목표: 온도가 특정 조건을 만족할 때만 MQTT 메시지를 전송
 *       불필요한 전송을 줄여 네트워크 트래픽 절약
 * 토픽: device/{student_id}/temperature
 * QoS:  0 (주기적 데이터, 유실 허용)
 *
 * 배선:
 *   DHT11 VCC   → NodeMCU 3.3V
 *   DHT11 GND   → NodeMCU GND
 *   DHT11 DATA  → NodeMCU D4 (GPIO 2) + 10kΩ 풀업 저항
 *
 * 라이브러리 설치:
 *   Arduino IDE → 라이브러리 관리자 → "DHT sensor library" (Adafruit) 설치
 *   의존성 "Adafruit Unified Sensor"도 함께 설치
 *
 * 확인: 라즈베리파이에서
 *   mosquitto_sub -t "device/01/temperature" -v
 ******************************************************************************/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// ===== Wi-Fi 설정 =====
const char *ssid     = "YOUR_WIFI_SSID";       // TODO: Wi-Fi 이름
const char *password = "YOUR_WIFI_PASSWORD";    // TODO: Wi-Fi 비밀번호

// ===== MQTT 설정 =====
const char *MQTT_ID = "temperature_01";          // TODO: 학생마다 고유하게 변경
const char *TOPIC   = "device/01/temperature";   // TODO: 01을 본인 번호로 변경

IPAddress broker(192, 168, 0, 81);               // TODO: 라즈베리파이 IP로 변경
const int MQTT_PORT = 1883;

WiFiClient wclient;
PubSubClient client(wclient);

// ===== DHT 센서 설정 =====
#define DHT_PIN   2          // D4 (GPIO 2)
#define DHT_TYPE  DHT11      // DHT11 또는 DHT22
DHT dht(DHT_PIN, DHT_TYPE);

// ===== 타이밍 설정 =====
unsigned long previousMillis = 0;
const long interval = 2000;   // 2초마다 측정

// ===== 조건부 전송 설정 =====
// 방식 A: 임계값 기반
const float THRESHOLD_TEMP = 30.0;    // 이 온도 이상일 때만 전송

// 방식 B: 변화폭 기반 (방식 A 대신 사용 가능)
const float CHANGE_THRESHOLD = 1.0;   // 이전 전송값과 1℃ 이상 차이 시 전송
float lastSentTemp = -999.0;          // 마지막으로 전송한 온도값


// ===== Wi-Fi 연결 =====
void setup_wifi() {
  Serial.print("\nConnecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// ===== MQTT 재연결 =====
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    if (client.connect(MQTT_ID)) {
      Serial.println("connected");
      Serial.print("Publishing to: ");
      Serial.println(TOPIC);
      Serial.println();
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

// ===== 전송 조건 판단 함수 =====
// 방식을 선택하여 하나만 사용할 것
bool shouldPublish(float temperature) {
  // TODO: 조건부 전송 로직 구현
  //
  // === 방식 A: 임계값 기반 ===
  // 힌트: temperature >= THRESHOLD_TEMP 이면 true
  //
  // === 방식 B: 변화폭 기반 ===
  // 힌트: abs(temperature - lastSentTemp) >= CHANGE_THRESHOLD 이면 true
  //       전송 시 lastSentTemp = temperature 로 업데이트

  return false;  // TODO: 조건에 따라 true/false 반환
}


void setup() {
  Serial.begin(115200);

  dht.begin();

  delay(100);
  setup_wifi();
  client.setServer(broker, MQTT_PORT);

  Serial.println("3단계: 온도 조건부 MQTT 퍼블리시");
  Serial.println("=================================");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // TODO: 온도 읽기
    //   힌트: float temperature = dht.readTemperature();

    // TODO: 센서 읽기 실패 처리
    //   힌트: isnan(temperature) 이면 에러 출력 후 return

    // TODO: 조건 확인 후 전송
    //   if (shouldPublish(temperature)) {
    //     - 온도를 문자열로 변환: dtostrf()
    //     - MQTT 전송: client.publish(TOPIC, tempStr)
    //     - 시리얼 출력: "[전송] device/01/temperature => 26.8"
    //   } else {
    //     - 시리얼 출력: "[스킵] 온도: 25.3 ℃ - 조건 미충족"
    //   }
  }
}
