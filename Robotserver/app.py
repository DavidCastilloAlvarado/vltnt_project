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
from flask import Flask, render_template, Response, request
from control_serial import control_robot
from serializer import WebJson2InoJson
from settings import API_KEY
import os

# Inicializamos los dos robots esclavos
default_chassis = {"status": "wait"}
# A,B,C,D son los 4 ejes, PyR son las posiciones de los servos de la camara: EJE:|DIR(0,1)|STEP(INT)
control_arm = {"A": None, "B": None,
               "C": None, "D": None, "P": None, "R": None}
default_arm = {"status": "wait"}
# D: es la direccion de movimiento (forward(1), backward(2), left(3), right(4)), S: velocidad del robot (0 a 9)
control_car = {"D": None, "S": None}
arm_robot = control_robot(serial_busy=[],
                          name_robot="arm",
                          control_format=control_arm,
                          default_response=default_arm,)
chassis_robot = control_robot(serial_busy=[arm_robot.serial_number],
                              name_robot="car",
                              control_format=control_car,
                              default_response=default_chassis,)
# Initialize the Flask application
app = Flask(__name__)


@app.route('/datos/<robot>', methods=['GET'])
def telemetry(robot):
    global chassis_robot
    global arm_robot

    if robot == "arm":
        data_json, linked = arm_robot.get_telemetry_data()
    elif robot == "car":
        data_json, linked = chassis_robot.get_telemetry_data()

    data_json = json.dumps(data_json)
    return Response(response=data_json, status=200 if linked else 400, mimetype="application/json")


@app.route('/control/<robot>', methods=['POST'])
def control(robot):
    global chassis_robot
    global arm_robot
    # Recibiendo datos desde el frontend
    raw = request.data
    send_str = raw.decode('utf8').replace("\'", "\"")
    command = json.loads(send_str)
    assert command["key"] == API_KEY, "API_KEY ERROR"
    command.pop("key")

    if not "aux" in command:
        if robot == "arm":
            # El comando para tranformar las senales de avance y retroceso para el manipulador, falta implementar
            # TODO: transformar command (direcciones de movimiento to movement on the robot) a posiciones(angulos) en el robot
            command = WebJson2InoJson(command, arm_robot)
            ctrl_resp = arm_robot.control_device(data=command)
        elif robot == "car":
            command = WebJson2InoJson(command, chassis_robot)
            ctrl_resp = chassis_robot.control_device(data=command)
        elif robot == "reset":
            os.system("sudo reboot")

    else:
        assert command["aux"] > 1, "Auxiliar command error"
        if robot == "arm":
            ctrl_resp = arm_robot.auxiliar_command(command["aux"])
        elif robot == "car":
            ctrl_resp = chassis_robot.auxiliar_command(command["aux"])
    response = {'message': 'Control {} delivered'.format(robot), "status": ctrl_resp,
                }
    # Codifica la respuesta using jsonpickle
    response_pickled = jsonpickle.encode(response)
    return Response(response=response_pickled, status=200 if ctrl_resp else 400, mimetype="application/json")


@app.route("/")
def home():
    return render_template("index.html")


@app.route("/info")
def info():
    return render_template("info.html")


@app.route("/calibrar")
def calibrar():
    return render_template("calibrar.html")


@app.route("/vltnt")
def vltnt():
    return render_template("controlpanel.html")


if __name__ == '__main__':
    port = 8010
    print("Servidor de telemetría&Control   http://localhost:"+str(port))
    app.run(host='0.0.0.0', port=port, threaded=True, debug=1)
