import requests
import json
import serial
import _thread
import csv
import time
import os
import pandas
import numpy as np
#import sys 
flag = 0
count = 0

# Toma de datos para la telemtría - Arduino envia data (almacena en un csv)
def read_ino():
                global flag
                global count
                n = 0
                while True:
                        try:
                                local = "/dev/ttyACM" +str(n) 
                                ser = serial.Serial(local, 115200)
                                
                                if flag == 0:
                                    print("Conexion Establecida con Arduino")
                                else: 
                                    print("Conexion Restablecida con Arduino")
                                flag = 0
                                time.sleep(4)
                                break 
                        except:
                                n=n+1
                                #print("Intento " + str(n))
                                if n >= 100 :
                                    print("Error : Arduino no Detectado - Intento 100 | Por Favor conecte el arduino")
                                    break
                                            #sys.exit()
                while True:
                        try: 
                                sensor = ser.readline()
                        except:
                                with open('lecturas_ino.csv', 'a', newline='') as csvfile: 
                                        regwriter = csv.writer(csvfile, delimiter=' ', quoting=csv.QUOTE_MINIMAL)
                                        regwriter.writerow(["wait", "wait", "wait","wait", "wait"])

                                count += 1
                                if count > 35:
                                        os.remove("lecturas_ino.csv")
                                        count = 0

                                time.sleep(2)
                                print("Error : Problemas con serial del arduino")
                                read_ino()
                        try:
                                sensor = sensor.decode('utf8').replace("'",'"')
                                HUD_data = json.loads(sensor)
                                # Obteniendo la lectura de los sens$
                                ang_x = HUD_data['ang_x']
                                ang_y = HUD_data['ang_y']
                                dist = HUD_data['dist']
                                status_dir1 = HUD_data['st1']
                                status_dir2 = "Speed: " + str(HUD_data['st2'])+"%"
                                with open('lecturas_ino.csv', 'a', newline='') as csvfile: 
                                        regwriter = csv.writer(csvfile, delimiter=' ', quoting=csv.QUOTE_MINIMAL)
                                        regwriter.writerow([str(ang_x), str(ang_y), str(dist),str(status_dir1), str(status_dir2)])

                                count += 1
                                if count > 35:
                                        os.remove("lecturas_ino.csv")
                                        count = 0
                                #print(count)
                        except:
                                pass


###############################
# Create two threads 
"""
try:
   _thread.start_new_thread(read_arduino,())
   _thread.start_new_thread(server_ras,())
   while True:
        time.sleep(1)
        print("Holas")
except:
   print ("Error: unable to start thread")
"""
