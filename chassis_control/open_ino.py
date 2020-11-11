import serial

def run_ino():
        n=0
        while True:
                try:
                        local = "/dev/ttyACM" +str(n)
                        ser = serial.Serial(local, 115200)
                        break
                except:
                        n=n+1
                        #print("Intento # " + str(n))
                        if n >= 100 :
                                print("Error : Arduino no Detectado - Intento 100 | Por Favor conecte el arduino")
                                return 0
        return ser
