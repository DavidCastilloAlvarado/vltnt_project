import serial
import time
import json

# class NoMynameFailed(Exception):
#     def __init__(self, mensaje):
#         self.mensaje = mensaje
#     def __str__(self):
#         return self.mensaje

class control_robot(object):
    # serial_busy: Lista de numeros de seriales y ocupados
    # name : Nombre del dispositivo a buscar
    # default_response: Es un diccionario
    def __init__(self, serial_busy, name_robot,default_response, baudrate=115200, cant_intentos= 30):
        self.name = name_robot
        self.busys = serial_busy
        self.baudrate = baudrate
        self.id_response = default_response
        self.serial_number = None
        self.cant_int = cant_intentos
        self.serial_link = self._get_serial()

    @staticmethod
    def _request_robot(option):
        """
        O :  obtener el nombte
        1 :  obtener telemetría
        """
        comand = "req="+str(option)
        send_comand = str.encode(comand)
        return send_comand

    def _readSerial2JSON(self):
        telemetry_data = self.serial_link.readline()
        telemetry_data = telemetry_data.decode('utf8').replace("'",'"')
        return json.loads(telemetry_data)

    def _writeSerial2Robot(self,data):
        if type(data) ==bytes:
            self.serial_link.write(data)
        elif type(data) == dict:
            data2send= ""
            for key in data:
                data2send += key+"="+str(data[key])+","
            data2send = str.encode(data2send)
            self.serial_link.write(data2send)

    def _get_serial(self,):
        number=0
        while True:
            try:
                if number in self.busys:
                    continue
                local = "/dev/ttyACM" +str(number)
                ser = serial.Serial(local, self.baudrate)
                # Enviando solicitud de nombre al arduino
                # El Request con valor 0 es para solicitar el nombre del robot esclavo
                ser.write(_request_robot(0))
                # Recibiendo la respuesta del arduino, todos nuestros robots envian por defecto JSON con un key "name"
                responsed = ser.readline()
                responsed = responsed.decode('utf8').replace("'",'"')
                responsed = json.loads(responsed)
                name_from_robot = responsed['name']
                assert name_from_robot == self.name , "No es el dispositivo buscado"
                break
            except:
                number+=1
                #print("Intento # " + str(n))
                if number >= self.cant_int :
                    print("No se detectaron dispositivos luego de {} intentos".format(number))
                    return None
        self.serial_number = number
        return ser

    def get_telemetry_data(self):
        """
        Función que devuelve la información que envía el robot esclavo
        Devuelve los datos de telemetria obtenidos, en caso de falla devuelve los datos 
        por defecto.
        """
        try:
            self._writeSerial2Robot(self._request_robot(1))
            time.sleep(0.03)
            telemetry_data = self._readSerial2JSON()
        except:
            self.serial_link.close()
            self.serial_link = self._get_serial()
            return self.id_response
        return telemetry_data

    def control_device(self, data):
        """
        Función que envia comandos hacia el robot
        data : es un diccionario.
        Devuelve valor verdadero cuando se recibio correctamente el comando,
        caso contrario el Falso.
        """
        try:
            self._writeSerial2Robot(data)
            time.sleep(0.03)
            # telemetry_data = self._readSerial2JSON()
        except:
            self.serial_link.close()
            self.serial_link = self._get_serial()
            return False
        return True
