import serial
import csv
import time
import paho.mqtt.client as mqtt
import json

def on_connect(client, userdata, flags, rc):
    if rc==0:
        client.connected_flag=True #Si la conexion es correcta
        print("connected OK") #Se imprime que la conexion es es correcta
	client.subscribe(topicreq)
    else:
        print("Bad connection Returned code=",rc) #Si la conexion es incorrecta
        client.loop_stop()  #Se detiene la conexion

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print("MSG recieves")
    PuertoSerial.write('1')

def on_subscribe(client, userdata, mid, granted_qos):
	print("Subscribe Success")

client = mqtt.Client('python3')
client.on_connect = on_connect
client.on_message = on_message
client.on_subscribe = on_subscribe

mqtt.Client.connected_flag=False 
mqtt.Client.suppress_puback_flag=False

PuertoSerial = serial.Serial(port = '/tmp/ttyS1')

token = "TrkAY5lUZdZs4WCPH7oe"
topic = "v1/devices/me/telemetry"
topicreq = "v1/devices/me/rpc/request/+"

client.username_pw_set(token, password=None)

client.connect("iot.eie.ucr.ac.cr", 1883, 60)
while not(client.connected_flag):
	client.loop()
	time.sleep(1)
time.sleep(3)

client.loop_start()
data=dict()
for i in range(10):
	
	valor = PuertoSerial.readline().decode().split(',')

	data["Armed"]=valor[0]
	data["Alarm"]=valor[1]
	data["Door1"]=valor[2]
	data["Door2"]=valor[3]
	data["Lock1"]=valor[4]
	data["Lock2"]=valor[5]
	data["Window"]=valor[6]
	data["Camera1"]=valor[7]
	data["Camera2"]=valor[8]
	data_out = json.dumps(data)

	print(valor)
	client.publish(topic,data_out,0)

	time.sleep(5)
client.loop_stop(force=False)
client.disconnect()

