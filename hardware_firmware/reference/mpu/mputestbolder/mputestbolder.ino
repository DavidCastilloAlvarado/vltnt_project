#include "MPU9250.h"
// #include "math.h"
#include "SensorFusion.h" //SF
SF fusion;
// an MPU9250 object with the MPU-9250 sensor on I2C bus 0 with address 0x68
/*-------------------------- MPU9250---------------------------------*/
// La dirección del MPU9250 puede ser 0x68 o 0x69, dependiendo
// del estado de AD0. Si no se especifica, 0x68 estará implicito
MPU9250 IMU(Wire, 0x68);
// Valores RAW (sin procesar) del acelerometro y giroscopio en los ejes x,y,z
// float gx, gy, gz, ax, ay, az, mx, my, mz;
float pitch, roll, yaw;
// float deltat;
float temp;
int status;

void setup()
{
    // serial to display data
    Serial.begin(115200);
    init_MPU();
    // calibrate_MPU();
}

void loop()
{
    // read the sensor
    read_MPU();
}

void init_MPU()
{
    status = IMU.begin();
    if (status)
    {
        // Serial.println(F("IMU initialization unsuccessful"));
        // Serial.println(F("Check IMU wiring or try cycling power"));
        // Serial.print(F("Status: "));
        // Serial.println(status);
        // setting the accelerometer full scale range to +/-8G
        IMU.setAccelRange(MPU9250::ACCEL_RANGE_8G);
        // setting the gyroscope full scale range to +/-500 deg/s
        IMU.setGyroRange(MPU9250::GYRO_RANGE_500DPS);
        // setting DLPF bandwidth to 20 Hz
        // IMU.setDlpfBandwidth(MPU9250::DLPF_BANDWIDTH_20HZ);
        // setting SRD to 19 for a 50 Hz update rate
        IMU.setSrd(19);
    }
}

void read_MPU()
{
    IMU.readSensor();
    // gx = IMU.getGyroX_rads();
    // gy = IMU.getGyroY_rads();
    // gz = IMU.getGyroZ_rads();
    // ax = IMU.getAccelX_mss();
    // ay = IMU.getAccelY_mss();
    // az = IMU.getAccelZ_mss();
    // mx = IMU.getMagX_uT();
    // my = IMU.getMagY_uT();
    // mz = IMU.getMagZ_uT();
    temp = IMU.getTemperature_C();
    // deltat = fusion.deltatUpdate();                                    //this have to be done before calling the fusion update
    fusion.MadgwickUpdate(IMU.getGyroX_rads(),
                          IMU.getGyroY_rads(),
                          IMU.getGyroZ_rads(),
                          IMU.getAccelX_mss(),
                          IMU.getAccelY_mss(),
                          IMU.getAccelZ_mss(),
                          IMU.getMagX_uT(),
                          IMU.getMagY_uT(),
                          IMU.getMagZ_uT(),
                          fusion.deltatUpdate()); //else use the magwick, it is slower but more accurate
    // fusion.MahonyUpdate(gx, gy, gz, ax, ay, az, deltat);  //mahony is suggested if there isn't the mag and the mcu is slow
    pitch = -1 * fusion.getPitch();
    roll = 180 - fusion.getRoll() > 180 ? -180.0 - fusion.getRoll() : 180 - fusion.getRoll(); //you could also use getRollRadians() ecc
    yaw = fusion.getYaw();

    // Serial.print(gx, 6);
    // Serial.print("\t");
    // Serial.print(gy, 6);
    // Serial.print("\t");
    // Serial.print(gz, 6);
    // Serial.print("\t");
    // Serial.print(ax, 6);
    // Serial.print("\t");
    // Serial.print(ay, 6);
    // Serial.print("\t");
    // Serial.print(az, 6);
    // Serial.print("\t");
    // Serial.print(mx, 6);
    // Serial.print("\t");
    // Serial.print(my, 6);
    // Serial.print("\t");
    // Serial.print(mz, 6);
    // Serial.print("\t");
    // Serial.println(temp, 6);

    // Serial.print(F("Pitch:\t"));
    Serial.println(pitch);
    // // Serial.print(F("Roll:\t"));
    Serial.println(roll);
    // // Serial.print(F("Yaw:\t"));
    Serial.println(yaw);
    Serial.println();
}

void calibrate_MPU()
{
    if (status)
    {
        Serial.print("mag cal");
        int status_cal = IMU.calibrateMag();
        Serial.print("mag ready");
        delay(5000);
        int status_gyro = IMU.calibrateGyro();
        int status_acc = IMU.calibrateAccel();
    }
}