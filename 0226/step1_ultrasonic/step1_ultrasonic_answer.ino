/******************************************************************************
 * step1_ultrasonic_answer.ino
 * 1단계 해설 코드: 초음파 센서 데이터 읽기 (로컬 테스트)
 *
 * 목표: HC-SR04 초음파 센서로 거리를 측정하고 시리얼 모니터에 출력
 *       이 단계에서는 MQTT를 사용하지 않는다.
 *
 * 배선:
 *   HC-SR04 VCC  → NodeMCU Vin (5V)
 *   HC-SR04 GND  → NodeMCU GND
 *   HC-SR04 TRIG → NodeMCU D1 (GPIO 5)
 *   HC-SR04 ECHO → NodeMCU D2 (GPIO 4) ※ 전압분배(1kΩ+2kΩ) 권장
 ******************************************************************************/

// ===== 초음파 센서 핀 설정 =====
const int TRIG_PIN = 5;   // D1
const int ECHO_PIN = 4;   // D2

// ===== 타이밍 설정 =====
unsigned long previousMillis = 0;
const long interval = 2000;   // 2초마다 측정

// ===== 초음파 거리 측정 함수 =====
float measureDistance() {
  // 1. TRIG 핀을 LOW로 2μs 유지 (안정화)
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  // 2. TRIG 핀을 HIGH로 10μs 유지 → 초음파 발사
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  // 3. TRIG 핀을 다시 LOW
  digitalWrite(TRIG_PIN, LOW);

  // 4. ECHO 핀에서 왕복 시간(μs) 측정, 30ms 타임아웃
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  // 5. 타임아웃이면 -1.0 반환 (센서 범위 밖)
  if (duration == 0) {
    return -1.0;
  }

  // 6. 거리(cm) = 왕복시간 × 0.034 / 2
  //    음속 340m/s = 0.034cm/μs, 왕복이므로 /2
  float distance = duration * 0.034 / 2.0;
  return distance;
}

void setup() {
  Serial.begin(115200);

  // TRIG는 출력, ECHO는 입력으로 설정
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  Serial.println("1단계: 초음파 센서 테스트");
  Serial.println("========================");
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    float distance = measureDistance();

    if (distance > 0) {
      Serial.print("거리: ");
      Serial.print(distance, 1);   // 소수점 1자리
      Serial.println(" cm");
    } else {
      Serial.println("측정 실패 (범위 초과)");
    }
  }
}
