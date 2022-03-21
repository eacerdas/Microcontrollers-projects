import serial
import csv
import time
import paho.mqtt.client as mqtt
import json

def on_connect(client, userdata, flags, rc):
    if rc==0:
        client.connected_flag=True #Si la conexion es correcta
        print("connected OK") #Se imprime que la conexion es es correcta
    else:
        print("Bad connection Returned code=",rc) #Si la conexion es incorrecta
        client.loop_stop()  #Se detiene la conexion

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))

client = mqtt.Client('python3')
client.on_connect = on_connect
client.on_message = on_message

mqtt.Client.connected_flag=False 
mqtt.Client.suppress_puback_flag=False

PuertoSerial = serial.Serial(port = '/tmp/ttyS1')

token = "DjDnAg7sKtWe2k6rk1oD"
topic = "v1/devices/me/telemetry"
topicreq = "v1/devices/me/attributes/request/1"

client.username_pw_set(token, password=None)

client.connect("iot.eie.ucr.ac.cr", 1883, 60)
while not(client.connected_flag):
	client.loop()
	time.sleep(1)
time.sleep(3)


data=dict()
for i in range(10):
	valor = PuertoSerial.readline().decode().split(',')
	
	data["Bateria"]=valor[0]
	data["Temperatura"]=valor[1]
	data["Humedad"]=valor[2]
	data["Viento"]=valor[3]
	data["Lluvia"]=valor[4]
	data["Luz"]=valor[5]
	data_out = json.dumps(data)
	
	print(valor)

	client.publish(topic,data_out,0)
	time.sleep(5)
	client.loop()
client.disconnect()

