#!/usr/bin/env python
import os
import json
import pandas
import _thread
from flask import Flask, render_template, Response
#from ino_read import serial_arduino , get_json_ino
from com_arduino import read_ino
# Iniciando el buffer de lecturas de sensor y server de control de movimiento
try:
   _thread.start_new_thread(read_ino,())
except:
   print ("Error: unable to start thread")
#####
app = Flask(__name__)
#read_ino = serial_arduino()

@app.route('/datos')
def datos():
	try:
	    dataframe = pandas.read_csv("lecturas_ino.csv", delim_whitespace=True, header=None)
	    dataset = dataframe.values
	    ang_x = dataset[-1,0]
	    ang_y = dataset[-1,1]
	    dist = dataset[-1,2]
	    status_dir1 = dataset[-1,3] 
	    status_dir2 = dataset[-1,4]

	    angX= "Ang x= "+str(ang_x) 
	    angY= "Ang y= "+str(ang_y) 
	    distM= str(dist)+" cm"

	    sensor_json = {'st1': status_dir1, 'dist': distM, 'ang_y': angY, 'st2': status_dir2, 'ang_x': angX}
	except:
		sensor_json = {'st1': 'wait', 'dist': 'wait', 'ang_y': 'wait', 'st2': 'wait', 'ang_x': 'wait'}
	msg_json = json.dumps(sensor_json)
	return Response(msg_json)


if __name__ == '__main__':
    port = 8010
    print("Servidor de telemetría   http://127.0.1.0:"+str(port))
    app.run(host='0.0.0.0' ,port=port, threaded=True)

    """
    $.ajax({
        url: 'http://127.0.1.0:8080/datos',
        type: 'GET',
        dataType: "json",
        success: displayAll
    });
    function displayAll(data){
        console.log(data);
    }
    """
