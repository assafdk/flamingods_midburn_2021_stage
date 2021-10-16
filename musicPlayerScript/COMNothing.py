import serial #for using COM ports

w = serial.Serial(port='COM7')
w.write(b'S')