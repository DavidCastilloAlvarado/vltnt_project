/**
 * AUTO: Bach. David Rolando Castillo Alvarado
 * TEMA: Firmware desarrollado para el control de movimiento del chassis, perteneciente al
 * proyecto VLTNT, el cual es un robot integrado que se desplaza en el plano mediante 
 * ruedas de oruga y que ademas posee un manipulador robotico de 4 grados de libertad
 * OPERCACION: El firmware trabaja en conjunto con el software del API REST creado para
 * controlar las funcionalidades del robot.
 * INPUTS: Las ordenes para comandar al robot, ingresan por el serial, el cual está unido al 
 * raspberry pi. Al chasis se le envía RQN0 para solicitar su nombre, RQN1 para solicitar
 * sus datos de telemetría. 
 * Para controlar el movimiento MVD0S1 para ordenar moverse en la dirección 0 a la velocidad 1,
 * las direcciones son [0,1,2,3,4] = [STOP, forward, backward, left, right], y las velocidades
 * van de [0 - 9]
 *  La función para el chasssi no necesita calibración de inicio de operaciones
 * sin embargo el del brazo robótico si lo neecesita, ademas de que necesita tener
 * un contador de pasos totales acumulados dentro de la memoria eeprom.
 * **/
#include <avr/wdt.h>
#include <EEPROM.h>
#include "MPU9250.h"
#include "SensorFusion.h" //SF
#include "settings.h"

void setup()
{
  wdt_disable();
  Serial.begin(115200);  //Iniciando puerto serial
  Serial.setTimeout(50); // ms de tiempo de corte y espera a la comunicación
  inData.reserve(100);
  data_send.reserve(100);
  init_Motor_control(); // inicializar los parametros para el motor
  init_MPU_sensor();    // inicializar el IMU
  init_HC_RC04();       // inicializar el sensor de distancia
  wdt_enable(WDTO_1S);  // Iniciando el WDT
}

void loop()
{
  stop_motor_time_run(); //detener el motor,
  MPU_sensor();          // reller el IMU
  HC_RC04();
  wdt_reset(); // reset watch dog
}
