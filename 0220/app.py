from flask import Flask, render_template, jsonify
import time
from datetime import datetime 
import serial

app = Flask(__name__)

def read_sensor():
    try:
        ser = serial.Serial('/dev/ttyUSB0', 9600, timeout=3)
        time.sleep(2)
        line = ser.readline().decode('utf-8').rstrip()
        ser.close()

        humidity, celsius = line.split(',')
        return {
            "temperature": float(celsius),
            "humidity": float(humidity)
        }
    except Exception as e:
        print("센서 오류: ", e)
        return {"temperature": None, "humidity":None}

@app.route('/')
def index():
    #data = {"temperature": 28.1, "humidity": 60.5}
    #return "온도: 25.3, 습도: 60.5"
    #return "<h1>온도: 25.3</h1><p>습도: 60.5</p>"
    #return render_template("index.html", sensor=data)

    #records = [
        #{"temperature": 25.3, "humidity":60.5},
        #{"temperature": 27.1, "humidity":58.2},
        #{"temperature": 29.4, "humidity":55.0},
        #{"temperature": 23.8, "humidity":65.3},
    #]
    data = read_sensor()
    #now = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    now = datetime.now().strftime("%Y년%m월%d일 %H:%M")
    #return render_template("index.html", records=records)
    #return render_template("index.html", data=records, now=now)
    #return render_template("index.html", data=[], now=now)
    return render_template("index.html", sensor=data, now=now)

#@app.route('/api/sensor')
#def api_sensor():
    #data = {"temperature": 25.3, "humidity":60.5}
    #return jsonify(data)

if __name__ == '__main__':

    app.run(debug=True)
