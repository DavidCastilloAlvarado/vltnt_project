"""
AUTO: Bach. David Rolando Castillo Alvarado
TEMA: Clase creada para inicializar los puertos seriales según el nombre especifico del robot,
enviar comandos de control al robot y recibir datos de telemetría del robot.
FUNCIONALIDAD: Se reliza la conección con los robor mediante puerto serial, el sistema tiene proteccion
contra desconexión, en caso sucediera, esta intentará reconetarse inmediatamente. sin la encesidad de
reiniciar ninguno de los dos equipos, ni el computador principal ni el robot esclavo.
"""
import serial
import time
import json

# class NoMynameFailed(Exception):
#     def __init__(self, mensaje):
#         self.mensaje = mensaje
#     def __str__(self):
#         return self.mensaje

class control_robot(object):
    """ 
    Envia: RQN0 solicita nombre, RQN1, solicita telemetría \n
    Control: {"MV": [key1,key2,key3, ... keyN]}\n
    default_response: {"signal1":xyz, "signal2":xyz, ...}\n
    serial_busy: Lista de numeros de seriales y ocupados\n
    name : Nombre del dispositivo a buscar. \n
    default_response: Es un diccionario con las llaves de telemetría seteadas para responder ante el error\n
    """
    def __init__(self, serial_busy, name_robot,default_response,control_format,trans_func= lambda x:x, baudrate=115200, cant_intentos= 30, timeout_read=0.05):
        self.name = name_robot
        self.control_format = control_format
        self.preprocess = trans_func
        self.busys = serial_busy
        self.baudrate = baudrate
        self.timeout_read = timeout_read
        self.id_response = default_response
        self.cant_int = cant_intentos
        self.serial_link,self.serial_number = self._get_serial()

    @staticmethod
    def _request_robot(option):
        """
        O :  obtener el nombte\n
        1 :  obtener telemetría\n
        """
        comand = "RQN"+str(option)+'\n' # PREFIJO PARA QUE EL ROBOT RECONOZCA QUE ES UNA ORDEN DE MOVIMIENTO
        send_comand = str.encode(comand)
        return send_comand

    def _readSerial2JSON(self):
        telemetry_data = self.serial_link.readline()
        telemetry_data = telemetry_data.decode('utf8').replace("'",'"')
        return json.loads(telemetry_data)

    def _writeSerial2Robot(self,data):
        if type(data) ==bytes:
            self.serial_link.write(data)
            self.serial_link.flushInput()
        elif type(data) == dict:
            data2send= "MV"     # PREFIJO PARA QUE EL ROBOT RECONOZCA QUE ES UNA ORDEN DE MOVIMIENTO
            for key in data:
                data2send += key+str(data[key])
            data2send = data2send + '\n'
            data2send = str.encode(data2send)
            self.serial_link.write(data2send) 
            self.serial_link.flushInput()

    def _get_serial(self,):
        number=0
        while True:
            try:
                if number in self.busys:
                    continue
                
                local = "/dev/ttyUSB" +str(number) #/dev/ttyACM
                ser = serial.Serial(local, self.baudrate,rtscts=False,dsrdtr=False)
                
                # Enviando solicitud de nombre al arduino
                # El Request con valor 0 es para solicitar el nombre del robot esclavo
                time.sleep(0.2)
                ser.write(self._request_robot(7))#Señal de lliberación No es relevante lo que se envía
                time.sleep(0.5)
                ser.write(self._request_robot(0))#_Solicita al robot que le retorne su nombre
                ser.flushInput()                 # Limpia el buffer de salida
                # Recibiendo la respuesta del arduino, todos nuestros robots envian por defecto JSON con un key "name"
                responsed = ser.readline()
                print("Intento")
                responsed = responsed.decode('utf8').replace("'",'"')
                responsed = json.loads(responsed)
                name_from_robot = responsed['name']
                print("arduino: {}  esperado: {}".format(name_from_robot,self.name))
                assert name_from_robot == self.name , "No es el dispositivo buscado"
                print(">>Dispositivo reconocido: {}".format(name_from_robot))
                break
            except:# Exception as err:
                #print(err)
                number+=1
                #print("Intento # " + str(n))
                if number >= self.cant_int :
                    print("No se detectaron dispositivos luego de {} intentos".format(number))
                    return None, None
        return ser, number

    def get_telemetry_data(self):
        """
        Función que devuelve la información que envía el robot esclavo\n
        Devuelve los datos de telemetria obtenidos, en caso de falla devuelve los datos \n
        por defecto.\n
        """
        try:
            self._writeSerial2Robot(self._request_robot(1))
            time.sleep(self.timeout_read)
            telemetry_data = self._readSerial2JSON()
        except:
            #self.serial_link.close()
            self.serial_link,self.serial_number = self._get_serial()
            return self.id_response
        return telemetry_data

    def control_device(self, data):
        """
        Función que envia comandos hacia el robot\n
        data : es un diccionario.\n
        Devuelve valor verdadero cuando se recibio correctamente el comando,\n
        caso contrario el Falso.\n
        """
        try:
            self._writeSerial2Robot(data)
            time.sleep(self.timeout_read)
            # telemetry_data = self._readSerial2JSON()
        except:
            #self.serial_link.close()
            self.serial_link,self.serial_number = self._get_serial()
            return False
        return True
