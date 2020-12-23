/**
 * AUTO: Bach. David Rolando Castillo Alvarado
 * TEMA: Firmware desarrollado para el control de movimiento del manipulador robótico, perteneciente al
 * proyecto VLTNT, el cual es un robot integrado que se desplaza en el plano mediante 
 * ruedas de oruga y que ademas posee un manipulador robotico de 4 grados de libertad
 * OPERACACION: El firmware trabaja en conjunto con el software del API REST creado para
 * controlar las funcionalidades del robot.
 * INPUTS: Las ordenes para el movimiento de los motores llegan desde el API. Este API maneja una
 * objeto creado para controlar la cinemática del robot, es ella quien maneja laas cinemáticas. 
 * Este firmware realiza la funcion de mover los motores a paso del manipulador. Es decir recibirá comandos
 * de movimiento de angularidad traducidos a pasos y dirección de giro.
 * **/
#include <avr/wdt.h>
#include <EEPROM.h>
#include <Adafruit_PWMServoDriver.h>
#include "settings.h"

void setup()
{
    wdt_disable();
    read_eeprom();
    Serial.begin(115200);  //Iniciando puerto serial
    Serial.setTimeout(50); // ms de tiempo de corte y espera a la comunicación
    inData.reserve(100);
    data_send.reserve(100);
    wdt_enable(WDTO_1S); // Iniciando el WDT
}

void loop()
{
    wdt_reset(); // reset watch dog
}