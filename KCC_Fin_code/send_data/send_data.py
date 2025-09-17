import serial
import requests
from datetime import datetime

port0 = '/dev/ttyACM0'

url = 'http://Mobius:7579/Mobius/20191546'

brate0 = 115200

headers = {
    'Accept' : 'application/json',
    'X-M2M-RI' : '12345',
    'X-M2M-Origin' : "20191546",
    'Content-Type' : 'application/vnd.onem2m-res+json; ty=4'
}

ser0 = serial.Serial(port0, baudrate = brate0, timeout = None)

print('%s and %s' %(ser0.name))
print('Time, Dust1, Dust2, Dust3\n') 

while True:
    now = datetime.now()
    time = now.strftime('%Y%m%d%H%M%S')

    if ser0.in_waiting != 0:
        contect0 = ser0.readline()

        # print('%s, %s, %s' %(time, contect0[:-2].decode(), contect1[:-2].decode()))
        con0_1 = "{}, {}".format(time, contect0[:-2].decode())

        data = {
            "m2m:cin" : {
                "con" : con0_1
            }
        }

        r = requests.post(url+'/Data', headers=headers, json=data)

        try:
            r.raise_for_status()
            jr = r.json()

        except Exception as exc:
            print('There was a problem: %s' % (exc))