SF fusion;
String inData;
String data_send;
/*--------------------------MPU9250---------------------------------*/
// La dirección del MPU9250 puede ser 0x68 o 0x69, dependiendo
// del estado de AD0. Si no se especifica, 0x68 estará implicito
MPU9250 IMU(Wire, 0x68);
byte mpu_started = 0;
float pitch = 0;
float roll = 0;
float yaw = 0;
// direcciones para los valores floats de calibración del magnetometro en el EEPROM
struct drrMAGBS
{
    int biasx = 0;
    int biasy = 5;
    int biasz = 10;
    int scalx = 15;
    int scaly = 20;
    int scalz = 25;
};
/*---------------------------HC RC04---------------------------------*/
const int Trigger = 9; //Pin digital 2 para el Trigger del sensor
const int Echo = 10;   //Pin digital 3 para el Echo del sensor
long t;                //timpo que demora en llegar el eco
long dist_obst;        //distancia en centimetros
long d_max = 150;      // distancia maxima detectada
int sum_d = 0;
int i = 0;
int j = 0;
int dis_val[5] = {0, 0, 0, 0, 0}; //pos[0,1,2,3,4]
long time_out;
/*---------------------------Control de Motor------------------------*/
//                           M1     M2  COnfiguracion de Pines L298N
int IN1[2] = {4, 7};  // 13     8
int IN2[2] = {3, 8};  // 12     9
int PWMp[2] = {5, 6}; // 11    10
int forWARDS = 1;
int backWARDS = 0;
unsigned long time_run = 250;    // Tiempo que permanecera con la orden en milisegundos
unsigned long time_run_prev = 0; // Variable global en donde se almacenará el tiempo previo
String Status_dir = "";
String Status_speed = "";
String Status_cal = "";