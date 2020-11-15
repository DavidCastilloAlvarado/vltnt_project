#!/usr/bin/env python
"""
AUTO: Bach. David Rolando Castillo Alvarado
TEMA: servidor para el API REST que controlará las funcionalidades del robot, este API REST
tendrá comunicación con el servidor de aplicación de la pagina web de la interface de 
usuario.
FUNCIONALIDAD: Se cuenta con dos funciones, una para entregar telemetría y otro para proporcionar control
a los dos robots declarados al inicio, el chassis y el arm. Cada uno de estos de declara usando una 
clase especialmente diseñada para este proposito, abrir puertos seriales 
usando nombres especificos con ordenes personalizadas.
"""
import json
import jsonpickle
from flask import Flask, render_template, Response,request
from control_serial import control_robot
from utiles import WebJson2InoJson

# Inicializamos los dos robots esclavos
default_chassis = {"status":"wait"}
control_arm     = {"A":None,"B":None,"C":None,"D":None,"P":None,"R":None}# A,B,C,D son los 4 ejes, PyR son las posiciones de los servos de la camara: EJE:|DIR(0,1)|STEP(INT) 
default_arm     = {"status":"wait"}
control_car     = {"D":None,"S":None} # D: es la direccion de movimiento (forward(1), backward(2), left(3), right(4)), S: velocidad del robot
arm_robot        = control_robot(serial_busy=[], 
                                name_robot="arm", 
                                control_format = control_arm,
                                default_response=default_arm,)
chassis_robot    = control_robot(serial_busy=[arm_robot.serial_number], 
                                name_robot="car", 
                                control_format = control_car,
                                default_response=default_chassis,)
# Initialize the Flask application
app = Flask(__name__)

@app.route('/datos/<robot>', methods=['GET'])
def telemetry(robot):
    global chassis_robot
    global arm_robot

    if   robot == "arm":
        data_json = arm_robot.get_telemetry_data()
    elif robot == "car":
        data_json = chassis_robot.get_telemetry_data()

    data_json = json.dumps(data_json)
    return Response(data_json)

@app.route('/control/<robot>', methods=['POST'])
def control(robot):
    global chassis_robot
    global arm_robot
    # Recibiendo datos desde el frontend
    raw = request.data
    send_str = raw.decode('utf8').replace("\'", "\"")
    comand = json.loads(send_str)
    
    if robot == "arm":
        # El comando para tranformar las senales de avance y retroceso para el manipulador, falta implementar
        # TODO: transformar command (direcciones de movimiento to movement on the robot) a posiciones(angulos) en el robot
        comand = WebJson2InoJson(comand,arm_robot)
        ctrl_resp = arm_robot.control_device(data=comand)
    elif robot == "car":
        comand = WebJson2InoJson(comand,chassis_robot)
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
