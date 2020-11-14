import json

def WebJson2InoJson(json_incoming, robot_serial):
    """
    La entrada robot_serial es un objeto de clase "control_robot"

    # ARM
    input WEBJSON

    +-X, +-Z, +-gripper_rotate, +-gripper_open_close, cam_yaw, cam_pitch 

    Output

    A, B ,C, D, P, R

    # CAR

    input

    DIRECCION, VELOCIDAD

    Output

    D, S
    """
    robot_name            = robot_serial.name
    json_format_outcoming = robot_serial.control_format
    if robot_name == "car":
        assert len(json_incoming) == len(json_format_outcoming), "Dimensión incorrecta de los datos de entrada con los de salida - car"
        for key_in, key_out in zip(json_incoming, json_format_outcoming):
            json_format_outcoming[key_out] = string2number(json_incoming[key_in])

        return json_format_outcoming

    elif robot_name == "arm":
        # input WEBJSON
        # +-X, +-Z, +-gripper_rotate, +-gripper_open_close, cam_yaw, cam_pitch 
        # Output
        # A, B ,C, D, P, R
        # TODO: Crear la clase que tradusca el movimiento del efector final, a movimientos que los motores de paso puedan interpretar.
        return json_format_outcoming

def string2number(input_val):
    if type(input_val) == int:
        return input_val
    elif type(input_val)== str:
        input_val = input_val.lower()
        if input_val == "forward":
            return 1
        elif input_val== "backward":
            return 2
        elif input_val == "right":
            return 3
        elif input_val == "left":
            return 4
        elif input_val == "stop":
            return 0