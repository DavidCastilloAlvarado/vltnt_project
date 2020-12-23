// Variables de comunicación del robot
Adafruit_PWMServoDriver servo_pwm = Adafruit_PWMServoDriver();
String inData;
String data_send;

// Variables donde se almacenarán los conteos de paso de cada motor
// los conteos inician en la posición home, conteo = 0
unsigned long m0_step; // contadores
unsigned long m1_step;
unsigned long m2_step;
unsigned long m3_step;
unsigned long sv1_ang; // almacenadores de PWM duty service
unsigned long sv2_ang;
// Velocidad del movimiento de los motores en ms
int SpeedMult = 200; // aumentar para hacer un movimiento aún más suave
// posicion en la memoria EEPROM DEL VALOR DE CADA CONTADOR Y DUTY CYCLE
struct POSmemo
{
    byte m0_step = 0;
    byte m1_step = 5;
    byte m2_step = 10;
    byte m3_step = 15;
    byte sv1_ang = 20;
    byte sv2_ang = 25;
};
/**------------------- Pines Motores a Paso (Step and DIR) y servos --------------**/
// los mx_flag seran consigurados como pullup input
struct PINmotor
{
    byte rst_all = 7; // resetea todos los drivers, en caso de falla

    byte m0_step = A1;
    byte m0_dir = A0;
    byte m0_fault = A7; // flag de driver que indica si el motor o driver a fallado, (generalmente por sobrecorriente)

    byte m1_step = A3;
    byte m1_dir = A2;
    byte m1_fault = A6;

    byte m2_step = 9;
    byte m2_dir = 10;
    byte m2_fault = 2;

    byte m3_step = 12;
    byte m3_dir = 11;
    byte m3_fault = 8;

    byte sv1p = 1; // representan al slot que ocuparan en la placa PCA9685
    byte sv2p = 2;
};
const PINmotor pimmotor;
/**------------------- Limites StepperMotor y servos --------------**/
struct LIMmotor
{
    unsigned long m0_min = 0; // representan la cantidad de pasos realizados
    unsigned long m0_max = 5000;

    unsigned long m1_min = 0;
    unsigned long m1_max = 5000;

    unsigned long m2_min = 0;
    unsigned long m2_max = 5000;

    unsigned long m3_min = 0;
    unsigned long m3_max = 5000;

    unsigned long sv1p = 900; // representan al slot que ocuparan en la placa PCA9685
    unsigned long sv2p = 900; // hasta 4096, usando el servodriver PCA9685
};
const LIMmotor limmotor;
/**------------------- PINES Finales de carrera  --------------**/
// FInales de carrera en pullup
struct PINfc
{
    byte fc1 = 3;
    byte fc2 = 4;
    byte fc3 = 5;
    byte fc4 = 6;
};
const PINfc pinfc;
/**------------------- Dirección de losmotores en CW  --------------**/
// Segun corresponda con el diseño en físico se puede setar por firmaware como se desea
// formatear el comando recibido por serial.
struct CWdir
{
    byte j0 = 1;
    byte j1 = 1;
    byte j2 = 1;
    byte j3 = 1;
};
const CWdir cwdir;