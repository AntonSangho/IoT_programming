# [수정] request 추가 - ESP32에서 보낸 POST 데이터를 받기 위해 필요
from flask import Flask, render_template, jsonify, request
# [삭제] import time - HTTP 방식에서는 불필요
# [삭제] import serial - USB 시리얼 통신 대신 HTTP 통신 사용
from datetime import datetime

app = Flask(__name__)

# [추가] ESP32에서 HTTP로 받은 최신 센서 데이터를 저장하는 변수
latest_sensor = {"temperature": None, "humidity": None}

# [삭제] read_sensor() 함수 - Serial 통신 방식이므로 더 이상 불필요
#def read_sensor():
#    try:
#        ser = serial.Serial('/dev/ttyUSB0', 9600, timeout=3)
#        time.sleep(2)
#        line = ser.readline().decode('utf-8').rstrip()
#        ser.close()
#
#        humidity, celsius = line.split(',')
#        return {
#            "temperature": float(celsius),
#            "humidity": float(humidity)
#        }
#    except Exception as e:
#        print("센서 오류: ", e)
#        return {"temperature": None, "humidity":None}

@app.route('/')
def index():
    # [수정] read_sensor() 대신 latest_sensor 변수에서 데이터를 가져옴
    now = datetime.now().strftime("%Y년%m월%d일 %H:%M")
    return render_template("index.html", sensor=latest_sensor, now=now)

# [추가] ESP32가 센서 데이터를 POST로 보내는 엔드포인트
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

# [추가] 현재 저장된 센서 데이터를 확인하는 GET 엔드포인트 (테스트용)
@app.route('/api/sensor', methods=['GET'])
def get_sensor():
    return jsonify(latest_sensor)

if __name__ == '__main__':
    # [수정] host='0.0.0.0' 추가 - ESP32(외부 기기)가 접속할 수 있도록 허용
    app.run(host='0.0.0.0', port=5000, debug=True)
