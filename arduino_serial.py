import serial
import time
import random

device = serial.Serial("/dev/ttyACM0", 9600, timeout=1)
time.sleep(3)
array = ["1", "-1"]
while True:
    device.reset_input_buffer()
    tmpStr = random.choice(array)
    device.write(bytes(tmpStr, "ascii"))
    print(f"Data sent: {tmpStr}")
    time.sleep(5)

device.close()