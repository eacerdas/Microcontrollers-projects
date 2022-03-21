import serial
import csv

PuertoSerial = serial.Serial(port = '/dev/ttyS0') #fuente: https://maker.pro/pic/tutorial/introduction-to-python-serial-ports

f= open("valor.csv",'w') 
writer = csv.writer(f) #fuente: https://www.pythontutorial.net/python-basics/python-write-csv-file/

while(1):
    	valor = PuertoSerial.readline().decode().split(' ')
    	writer.writerow(valor)
	print(valor)

f.close()