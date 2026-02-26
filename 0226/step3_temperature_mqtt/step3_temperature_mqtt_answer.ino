/******************************************************************************
 * step3_temperature_mqtt_answer.ino
 * 3단계 해설 코드: 온도 센서 조건부 퍼블리시 (QoS 0)
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
const char *ssid     = "YOUR_WIFI_SSID";       // Wi-Fi 이름
const char *password = "YOUR_WIFI_PASSWORD";    // Wi-Fi 비밀번호

// ===== MQTT 설정 =====
const char *MQTT_ID = "temperature_01";          // 학생마다 고유하게 변경
const char *TOPIC   = "device/01/temperature";   // 01을 본인 번호로 변경

IPAddress broker(192, 168, 0, 81);               // 라즈베리파이 IP로 변경
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
//
// 방식 B(변화폭 기반)를 기본으로 사용한다.
// 방식 A를 사용하려면 아래 주석을 참고하여 교체한다.
//
bool shouldPublish(float temperature) {
  // === 방식 B: 변화폭 기반 (기본) ===
  float diff = temperature - lastSentTemp;
  if (diff < 0) diff = -diff;   // 절대값

  if (diff >= CHANGE_THRESHOLD) {
    lastSentTemp = temperature;  // 마지막 전송값 업데이트
    return true;
  }
  return false;

  // === 방식 A: 임계값 기반 (사용하려면 위 코드를 지우고 아래 주석 해제) ===
  // return (temperature >= THRESHOLD_TEMP);
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

    // 온도 읽기
    float temperature = dht.readTemperature();

    // 센서 읽기 실패 처리
    if (isnan(temperature)) {
      Serial.println("센서 읽기 실패!");
      return;
    }

    // 조건 확인 후 전송
    if (shouldPublish(temperature)) {
      char tempStr[10];
      dtostrf(temperature, 4, 1, tempStr);

      client.publish(TOPIC, tempStr);

      Serial.print("[전송] ");
      Serial.print(TOPIC);
      Serial.print(" => ");
      Serial.println(tempStr);
    } else {
      Serial.print("[스킵] 온도: ");
      Serial.print(temperature, 1);
      Serial.println(" ℃ - 조건 미충족");
    }
  }
}
