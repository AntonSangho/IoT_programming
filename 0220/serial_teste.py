import serial
import time 

#시리얼 포트 설정 
ser = serial.Serial('/dev/ttyUSB0', 9600, timeout=1) #timeout=1은 Arduino에서 데이터가 오지않을때 1초까지 기다린다. 
time.sleep(2)  # 리셋

try:
	while True:
		if ser.in_waiting > 0:
			line = ser.readline().decode('utf-8').rstrip()

			# decode() 가 있을 때 : "6,30"
			# decode() 가 없을 때 : b"6,30\r\n"
			#line = ser.readline().rstrip()
			humidity, celsius = line.split(',')
			humidity = float(humidity)
			celsius = float(celsius)
			print(f"습도: {humidity}%, 온도: {celsius} C")

except KeyboardInterrupt:
	print("\n프로그램 종료")
	ser.close() 
