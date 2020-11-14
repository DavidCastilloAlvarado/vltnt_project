/** La función para el chasssi no necesita calibración de inicio de operaciones
 * sin embargo el del brazo robótico si lo neecesita, ademas de que necesita tener
 * un contador de pasos totales acumulados dentro de la memoria eeprom.
 * **/
#include <avr/wdt.h>

String inData;
String data_send;
String buffer;
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
  data_send = String("{")+String("'ang_x':") + String(10) + String(",") + String("'ang_y':") + String(10) + String(",") + String("'dist':") + String(10) + String(",")+ String("'DIR':") + Status_dir + String(",")+ String("'SP':") + Status_speed + String("}");
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
  wdt_enable(WDTO_1S);     // Iniciando el WDT
}

void loop() {
  stop_motor_time_run();
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