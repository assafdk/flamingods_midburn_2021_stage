import serial #for using COM ports

w = serial.Serial(port='COM4')
w.write(b'C')