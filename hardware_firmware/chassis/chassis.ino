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
#include "I2Cdev.h"
#include "MPU6050.h"
#include "Wire.h"
#include "math.h"
#include <avr/wdt.h>

String inData;
String data_send;
/*--------------------------MPU 6050---------------------------------*/
// La dirección del MPU6050 puede ser 0x68 o 0x69, dependiendo 
// del estado de AD0. Si no se especifica, 0x68 estará implicito
MPU6050 sensor_ang;
// Valores RAW (sin procesar) del acelerometro y giroscopio en los ejes x,y,z
bool started_sensor = false;
int ax, ay, az;
int gx, gy, gz;
double  ang_x, ang_y;
float ang_x_prev = 0; 
float ang_y_prev = 0;
long tiempo_prev;
float dt;
/*---------------------------HC RC04---------------------------------*/
const int Trigger = A0;   //Pin digital 2 para el Trigger del sensor
const int Echo    = A1;   //Pin digital 3 para el Echo del sensor
long t;                  //timpo que demora en llegar el eco
long dist_obst;          //distancia en centimetros
long d_max = 150; // distancia maxima detectada
int sum_d = 0;
int i = 0;
int j = 0;
int dis_val[5]={0,0,0,0,0}; //pos[0,1,2,3,4] 
long time_out;

/*---------------------------Control de Motor------------------------*/
//                           M1     M2  COnfiguracion de Pines L298N
int IN1[2] ={13,8};       // 13     8
int IN2[2] ={12,9};       // 12     9
int PWMp[2]={11,10};      // 11    10
int forWARDS  = 1; 
int backWARDS = 0;
unsigned long time_run = 250; // Tiempo que permanecera con la orden en milisegundos
unsigned long time_run_prev=0;// Variable global en donde se almacenará el tiempo previo
String Status_dir  ="";
String Status_speed="";


void send_telemetry(){
  data_send = String("{")+String("'ang_x':") + String(int(ang_x_prev)) + String(",") + String("'ang_y':") + String(int(ang_y_prev)) + String(",") + String("'dist':") + String(int(dist_obst)) + String(",")+ String("'DIR':") + Status_dir + String(",")+ String("'SP':") + Status_speed + String("}");
  Serial.println(data_send);
}

void response_order(int order){
  if (order == 0){
    Serial.println("{'name':\"car\"}");
  }
  else if (order == 1){
    send_telemetry();
  }
}

void robot_move(String &inData){
  // TODO: Usar el string inData para ahorrar memoria dinámica
  int DIRstart = inData.indexOf('D');
  int SPstart  = inData.indexOf('S');
  int DIR      = inData.substring(DIRstart + 1, SPstart).toInt();
  int SPEED    = inData.substring(SPstart+1).toInt();
  time_run_prev = millis();
  Motor_action(DIR,SPEED);
}

void serialEvent(){
  //while(Serial.available() ){
    char incomingByte = (char)Serial.read();
    inData += incomingByte;
    if (incomingByte == '\n'){
      String function = inData.substring(0, 2);
      if (function == "RQ"){
        int orderStart = inData.indexOf("N");
        int order = inData.substring(orderStart + 1).toInt();
        response_order(order);
      }
      else if(function == "MV"){
        robot_move(inData);
      }
      //Serial.println(inData);
      inData= "";
    }
}

void setup() {
  wdt_disable();
  Serial.begin(115200);    //Iniciando puerto serial
  Serial.setTimeout(50);
  inData.reserve(100);
  data_send.reserve(100);
  init_Motor_control();
  init_MPU_sensor(); 
  init_HC_RC04();

  wdt_enable(WDTO_1S);     // Iniciando el WDT
}

void loop() {
  stop_motor_time_run();
  MPU_sensor();
  HC_RC04();
  // put your main code here, to run repeatedly:
  wdt_reset();
}

void stop_motor_time_run(){
  unsigned long timedif= millis()-time_run_prev;
  if (timedif > time_run || time_run_prev > millis()){ // El motor se parará cuando pase el tiempo de corte o cuando exista un revalse en el valor del millis()
    Motor_action(0,0);
  }
}

void init_Motor_control(){
  //Motor 1
  pinMode(IN1[0], OUTPUT);
  pinMode(IN2[0], OUTPUT);
  pinMode(PWMp[0], OUTPUT);
  // Motor 2
  pinMode(IN1[1], OUTPUT);
  pinMode(IN2[1], OUTPUT);
  pinMode(PWMp[1], OUTPUT);
}

void shaftrev(int in1, int in2, int PWM, int sentido,int Wpulse){  
  switch (sentido) {
    // Sentido: 
      case 0: // BACKWARD
        digitalWrite(in2, HIGH);
        digitalWrite(in1, LOW);
        analogWrite(PWM,Wpulse);
        break;
      case 1: // FORWARD
        digitalWrite(in2, LOW);
        digitalWrite(in1, HIGH);
        analogWrite(PWM,Wpulse);
        break;
      case 3: // STOP
        digitalWrite(in2, LOW);
        digitalWrite(in1, LOW);
        analogWrite(PWM,Wpulse);
        break;
  }
}

void Motor_action(int direct,int pwmS ){
  pwmS=pwmS*255/9;
  Status_speed = String(int(pwmS/255.0*100));
  switch (direct) {
      case 1: // FOrward
        shaftrev(IN1[0],IN2[0],PWMp[0],forWARDS, pwmS);
        shaftrev(IN1[1],IN2[1],PWMp[1],forWARDS, pwmS);
        Status_dir = "\"FORWARD\"";
        break;
      case 2: // BackWard
        shaftrev(IN1[0],IN2[0],PWMp[0],backWARDS, pwmS);
        shaftrev(IN1[1],IN2[1],PWMp[1],backWARDS, pwmS);
        Status_dir = "\"BACKWARD\"";
        break;
      case 4: // Left
        shaftrev(IN1[0],IN2[0],PWMp[0],backWARDS,pwmS);
        shaftrev(IN1[1],IN2[1],PWMp[1],forWARDS, pwmS);
        Status_dir = "\"LEFT\"";
        break;
      case 3: // Right
        shaftrev(IN1[0],IN2[0],PWMp[0],forWARDS, pwmS);
        shaftrev(IN1[1],IN2[1],PWMp[1],backWARDS, pwmS);
        Status_dir = "\"RIGHT\"";
        break;
      case 0: // Stop
        shaftrev(IN1[0],IN2[0],PWMp[0],3, 0);
        shaftrev(IN1[1],IN2[1],PWMp[1],3, 0);
        Status_dir = "\"STOP\"";
        Status_speed = "0";
        break;
      default: // Stop
        shaftrev(IN1[0],IN2[0],PWMp[0],3, 0);
        shaftrev(IN1[1],IN2[1],PWMp[1],3, 0);
        Status_dir = "\"STOP\"";
        Status_speed = "0";
        break;
  }
}

void init_MPU_sensor(){
  Wire.begin();           //Iniciando I2C  
  sensor_ang.initialize();    //Iniciando el sensor_ang
  if (sensor_ang.testConnection()) {
    started_sensor= true;
    }
  else{
    started_sensor= false;
  }
}

void init_HC_RC04(){
  pinMode(Trigger, OUTPUT); //pin como salida
  pinMode(Echo, INPUT);  //pin como entrada
  digitalWrite(Trigger, LOW);//Inicializamos el pin con 0
  time_out = d_max*58.823;
}

void MPU_sensor(){
  if (started_sensor){
    // Leer las aceleraciones y velocidades angulares
    sensor_ang.getAcceleration(&ax, &ay, &az);
    sensor_ang.getRotation(&gx, &gy, &gz);
    // Correxion de Offsets
    ax=ax-1277;
    ay=ay+428;
    az=az+1723;
    gx=gx+637;
    gy=gy-319;
    gz=gz-224;
    dt = (millis()-tiempo_prev)/1000.0;
    tiempo_prev=millis();
    
    //Calcular los ángulos con acelerometro
    float accel_ang_x=atan2(ay,sqrt(pow(ax,2) + pow(az,2)))*(180.0/3.14);
    float accel_ang_y=atan2(-ax,sqrt(pow(ay,2) + pow(az,2)))*(180.0/3.14);
    
    //Calcular angulo de rotación con giroscopio y filtro complementario  
    ang_x = 0.98*(ang_x_prev+(gx/127)*dt) + 0.02*accel_ang_x;
    ang_y = 0.98*(ang_y_prev+(gy/160)*dt) + 0.02*accel_ang_y;
    
    ang_x_prev=ang_x;
    ang_y_prev=ang_y;
  }
}

void HC_RC04(){
  digitalWrite(Trigger, HIGH);
  delayMicroseconds(10);      //Enviamos un pulso de 10us
  digitalWrite(Trigger, LOW);
  
  t = pulseIn(Echo, HIGH,time_out );    //obtenemos el ancho del pulso
  if (t == 0 ){ 
    dist_obst = d_max; // Tiempo mayor al esperado =  más distancia Dmáx
  } 
  else{
    dist_obst = t/58.823;       // Distancia en centimetros
  } 
  
  if (i <5){                  // Acumulamos 5 lecturas 
    dis_val[i] = dist_obst;   // para sacar la media
    i++;
  }
  if (i == 5){
    for (j = 0; j < 5; j++){  // calcula el promedio de mediciones
      sum_d += dis_val[j];
      dist_obst=sum_d/5;
    }
    for (j = 0; j < 4; j++){  // Desplaza los valores dentro del array
      dis_val[j] = dis_val[j+1]; 
    } 
    i--;
    sum_d = 0;
  }
  if (dist_obst > d_max ){ dist_obst = d_max; }
}
