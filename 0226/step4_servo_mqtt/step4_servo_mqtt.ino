/******************************************************************************
 * step4_servo_mqtt.ino
 * 4단계: 서보모터 제어 - MQTT 명령 수신 (Subscribe)
 *
 * 목표: MQTT 구독자(Subscriber)로서 명령을 받아 서보모터를 제어
 *       이전 단계(Publish)와 반대 방향 데이터 흐름
 * 토픽: device/{student_id}/servo/cmd
 * QoS:  Subscribe QoS 1 (PubSubClient 최대치)
 *       개념적으로는 QoS 2가 필요 (Exactly Once)
 *
 * 배선:
 *   SG90 빨간색 (VCC)    → NodeMCU Vin (5V) 또는 외부 전원
 *   SG90 갈색  (GND)     → NodeMCU GND
 *   SG90 주황색 (Signal)  → NodeMCU D5 (GPIO 14)
 *
 * 명령 전송 (라즈베리파이에서):
 *   mosquitto_pub -t "device/01/servo/cmd" -m "OPEN"
 *   mosquitto_pub -t "device/01/servo/cmd" -m "CLOSE"
 *   mosquitto_pub -t "device/01/servo/cmd" -m "45"
 ******************************************************************************/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>

// ===== Wi-Fi 설정 =====
const char *ssid     = "YOUR_WIFI_SSID";       // TODO: Wi-Fi 이름
const char *password = "YOUR_WIFI_PASSWORD";    // TODO: Wi-Fi 비밀번호

// ===== MQTT 설정 =====
const char *MQTT_ID = "servo_01";               // TODO: 학생마다 고유하게 변경
const char *TOPIC   = "device/01/servo/cmd";    // TODO: 01을 본인 번호로 변경

IPAddress broker(192, 168, 0, 81);              // TODO: 라즈베리파이 IP로 변경
const int MQTT_PORT = 1883;

WiFiClient wclient;
PubSubClient client(wclient);

// ===== 서보모터 설정 =====
const int SERVO_PIN = 14;   // D5 (GPIO 14)
Servo myServo;


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

// ===== MQTT 메시지 수신 콜백 =====
// 브로커로부터 메시지가 도착하면 자동으로 호출됨
void callback(char* topic, byte* payload, unsigned int length) {
  // TODO: 수신된 payload를 문자열로 변환
  //   힌트: char message[length + 1];
  //         for (int i = 0; i < length; i++) message[i] = (char)payload[i];
  //         message[length] = '\0';

  // TODO: 수신 내용 시리얼 출력
  //   예: "수신: [device/01/servo/cmd] OPEN"

  // TODO: 명령에 따라 서보모터 제어
  //   String cmd = String(message);
  //
  //   if (cmd == "OPEN") {
  //     서보 90도: myServo.write(90);
  //     시리얼 출력: "→ 서보 90도 (OPEN)"
  //   }
  //   else if (cmd == "CLOSE") {
  //     서보 0도: myServo.write(0);
  //     시리얼 출력: "→ 서보 0도 (CLOSE)"
  //   }
  //   else {
  //     숫자로 변환 시도: int angle = cmd.toInt();
  //     범위 확인 (0~180): if (angle >= 0 && angle <= 180)
  //     서보 해당 각도: myServo.write(angle);
  //     시리얼 출력: "→ 서보 {angle}도"
  //   }
}

// ===== MQTT 재연결 =====
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    if (client.connect(MQTT_ID)) {
      Serial.println("connected");

      // TODO: 토픽 구독 등록 (QoS 1)
      //   힌트: client.subscribe(TOPIC, 1);
      //   PubSubClient는 Subscribe QoS 0, 1을 지원

      Serial.print("Subscribed to: ");
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


void setup() {
  Serial.begin(115200);

  // TODO: 서보모터 초기화
  //   힌트: myServo.attach(SERVO_PIN);
  //         myServo.write(0);   // 초기 위치 0도

  delay(100);
  setup_wifi();
  client.setServer(broker, MQTT_PORT);

  // TODO: 콜백 함수 등록
  //   힌트: client.setCallback(callback);

  Serial.println("4단계: 서보모터 MQTT 제어");
  Serial.println("=========================");
  Serial.println("명령 대기 중...");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  // client.loop()가 콜백 함수를 호출한다
  // 메시지가 도착하면 callback() 자동 실행
  client.loop();
}
