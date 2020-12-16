#include "quaternionFilters.h"
#include "MPU9250.h"

#define I2Cclock 400000
#define I2Cport Wire
#define MPU9250_ADDRESS MPU9250_ADDRESS_AD0 // Use either this line or the next to select which I2C address your device is using
//#define MPU9250_ADDRESS MPU9250_ADDRESS_AD1

MPU9250 myIMU(MPU9250_ADDRESS_AD0, I2Cport, I2Cclock);

byte dir_9250 = 0x00;
byte dir_8963 = 0x00;
bool ledOn = true;

void setup()
{
    Wire.begin();
    // put your setup code here, to run once:
    Serial.begin(115200);
    while (!Serial)
    {
    };

    pinMode(13, OUTPUT);
    myIMU.initMPU9250();
    myIMU.initAK8963(myIMU.factoryMagCalibration);
    dir_9250 = myIMU.readByte(MPU9250_ADDRESS_AD0, WHO_AM_I_MPU9250);
    dir_8963 = myIMU.readByte(AK8963_ADDRESS, WHO_AM_I_AK8963);
    CalibrateMPU9250(dir_9250, dir_8963);
    myIMU.count = millis();
    myIMU.sumCount = 0;
    myIMU.sum = 0;
}

void loop()
{
    digitalWrite(13, ledOn);
    ledOn = !ledOn;
    delay(100);
    read_values();
    print();
}

void CalibrateMPU9250(byte dir_mpu, byte dir_mag)
{
    if (dir_mpu != 0xFF)
    {
        Serial.println(F("MPU9250 is online..."));
        myIMU.MPU9250SelfTest(myIMU.selfTest);
        Serial.print(F("x-axis self test: acc trim: "));
        Serial.print(myIMU.selfTest[0], 1);
        Serial.println("% of factory value");
        Serial.print(F("y-axis self test: acc trim: "));
        Serial.print(myIMU.selfTest[1], 1);
        Serial.println("% of factory value");
        Serial.print(F("z-axis self test: acc trim: "));
        Serial.print(myIMU.selfTest[2], 1);
        Serial.println("% of factory value");
        Serial.print(F("x-axis self test: gyr trim: "));
        Serial.print(myIMU.selfTest[3], 1);
        Serial.println("% of factory value");
        Serial.print(F("y-axis self test: gyr trim: "));
        Serial.print(myIMU.selfTest[4], 1);
        Serial.println("% of factory value");
        Serial.print(F("z-axis self test: gyr trim: "));
        Serial.print(myIMU.selfTest[5], 1);
        Serial.println("% of factory value");
        // Calibrate gyro and accelerometers, load biases in bias registers
        myIMU.calibrateMPU9250(myIMU.gyroBias, myIMU.accelBias);
    }
    if (dir_mag == 0x48)
    {
        Serial.print("X-Axis factory sensitivity adjust value ");
        Serial.println(myIMU.factoryMagCalibration[0], 2);
        Serial.print("Y-Axis factory sensitivity adjust value ");
        Serial.println(myIMU.factoryMagCalibration[1], 2);
        Serial.print("Z-Axis factory sensitivity adjust value ");
        Serial.println(myIMU.factoryMagCalibration[2], 2);

        // myIMU.magCalMPU9250(myIMU.magBias, myIMU.magScale);
    }
    Serial.print("Received mpu9250: 0x");
    Serial.println(dir_mpu, HEX);
    Serial.print("Received AK8963: 0x");
    Serial.println(dir_mag, HEX);

    // Get sensor resolutions, only need to do this once
    myIMU.getAres();
    myIMU.getGres();
    myIMU.getMres();
}

void read_values()
{

    if (myIMU.readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01)
    {
        myIMU.readAccelData(myIMU.accelCount); // Read the x/y/z adc values

        // Now we'll calculate the accleration value into actual g's
        // This depends on scale being set
        myIMU.ax = (float)myIMU.accelCount[0] * myIMU.aRes; // - myIMU.accelBias[0];
        myIMU.ay = (float)myIMU.accelCount[1] * myIMU.aRes; // - myIMU.accelBias[1];
        myIMU.az = (float)myIMU.accelCount[2] * myIMU.aRes; // - myIMU.accelBias[2];

        myIMU.readGyroData(myIMU.gyroCount); // Read the x/y/z adc values

        // Calculate the gyro value into actual degrees per second
        // This depends on scale being set
        myIMU.gx = (float)myIMU.gyroCount[0] * myIMU.gRes;
        myIMU.gy = (float)myIMU.gyroCount[1] * myIMU.gRes;
        myIMU.gz = (float)myIMU.gyroCount[2] * myIMU.gRes;

        myIMU.readMagData(myIMU.magCount); // Read the x/y/z adc values

        // Calculate the magnetometer values in milliGauss
        // Include factory calibration per data sheet and user environmental
        // corrections
        // Get actual magnetometer value, this depends on scale being set
        myIMU.mx = (float)myIMU.magCount[0]; //* myIMU.mRes * myIMU.factoryMagCalibration[0] - myIMU.magBias[0];
        myIMU.my = (float)myIMU.magCount[1]; //* myIMU.mRes * myIMU.factoryMagCalibration[1] - myIMU.magBias[1];
        myIMU.mz = (float)myIMU.magCount[2]; //* myIMU.mRes * myIMU.factoryMagCalibration[2] - myIMU.magBias[2];
    }                                        // if (readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01)

    // Must be called before updating quaternions!
    myIMU.updateTime();
}

void print()
{
    myIMU.delt_t = millis() - myIMU.count;
    if (myIMU.delt_t > 1000)
    {
        myIMU.count = millis();

        // Print acceleration values in milligs!
        Serial.print("X-acceleration: ");
        Serial.print(1000 * myIMU.ax);
        Serial.print(" mg ");
        Serial.print("Y-acceleration: ");
        Serial.print(1000 * myIMU.ay);
        Serial.print(" mg ");
        Serial.print("Z-acceleration: ");
        Serial.print(1000 * myIMU.az);
        Serial.println(" mg ");

        // Print gyro values in degree/sec
        Serial.print("X-gyro rate: ");
        Serial.print(myIMU.gx, 3);
        Serial.print(" degrees/sec ");
        Serial.print("Y-gyro rate: ");
        Serial.print(myIMU.gy, 3);
        Serial.print(" degrees/sec ");
        Serial.print("Z-gyro rate: ");
        Serial.print(myIMU.gz, 3);
        Serial.println(" degrees/sec");

        // Print mag values in degree/sec
        Serial.print("X-mag field: ");
        Serial.print(myIMU.mx);
        Serial.print(" mG ");
        Serial.print("Y-mag field: ");
        Serial.print(myIMU.my);
        Serial.print(" mG ");
        Serial.print("Z-mag field: ");
        Serial.print(myIMU.mz);
        Serial.println(" mG");

        myIMU.tempCount = myIMU.readTempData(); // Read the adc values
        // Temperature in degrees Centigrade
        myIMU.temperature = ((float)myIMU.tempCount) / 333.87 + 21.0;
        // Print temperature in degrees Centigrade
        Serial.print("Temperature is ");
        Serial.print(myIMU.temperature, 1);
        Serial.println(" degrees C");
    }
}