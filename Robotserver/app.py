#!/usr/bin/env python
import json
import jsonpickle
from flask import Flask, render_template, Response,request
from control_serial import control_robot

# Inicializamos los dos robots esclavos
default_chassis = {"status":"wait"}
default_arm     = {"status":"wait"}
chassis_robot = control_robot(serial_busy=[], 
                                name_robot="arm", 
                                default_response=default_arm,)
arm_robot     = control_robot(serial_busy=[chassis_robot.serial_number], 
                                name_robot="car", 
                                default_response=default_chassis,)
# Initialize the Flask application
app = Flask(__name__)

@app.route('/datos/<robot>', methods=['GET'])
def telemetry(robot):
    global chassis_robot
    global arm_robot

    if robot == "arm":
        data_json = arm_robot.get_telemetry_data()
    elif robot == "car":
        data_json = chassis_robot.get_telemetry_data()

    data_json = json.dumps(data_json)
    return Response(data_json)

@app.route('/datos/<robot>', methods=['POST'])
def control(robot):
    global chassis_robot
    global arm_robot
    # Recibiendo datos desde el frontend
    raw = request.data
    send_str = raw.decode('utf8').replace("'", '"')
    comand = json.loads(send_str)

    if robot == "arm":
        ctrl_resp = arm_robot.control_device(data=comand)
    elif robot == "car":
        ctrl_resp = chassis_robot.control_device(data=comand)
    
    response = {'message': 'Control {} delivered: {}'.format(robot, ctrl_resp)
                }
    # Codifica la respuesta using jsonpickle
    response_pickled = jsonpickle.encode(response)
    return Response(response=response_pickled, status=200, mimetype="application/json")

if __name__ == '__main__':
    port = 8010
    print("Servidor de telemetría   http://localhost:"+str(port))
    app.run(host='0.0.0.0' ,port=port, threaded=True)
