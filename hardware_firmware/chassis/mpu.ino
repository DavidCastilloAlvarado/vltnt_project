void init_MPU_sensor()
{
    mpu_started = IMU.begin();
    if (mpu_started)
    {
        drrMAGBS drrparm;
        // setting the accelerometer full scale range to +/-8G
        IMU.setAccelRange(MPU9250::ACCEL_RANGE_8G);
        // setting the gyroscope full scale range to +/-500 deg/s
        IMU.setGyroRange(MPU9250::GYRO_RANGE_500DPS);
        // setting DLPF bandwidth to 20 Hz
        // IMU.setDlpfBandwidth(MPU9250::DLPF_BANDWIDTH_20HZ);
        // setting SRD to 19 for a 50 Hz update rate
        IMU.setSrd(19);
        delay(100);
        float bias = 0.00f;
        float scal = 0.00f;
        EEPROM.get(drrparm.biasx, bias);
        EEPROM.get(drrparm.scalx, scal);
        scal = scal == 0.00 ? 1.00 : scal;
        IMU.setMagCalX(bias, scal);

        bias = 0.00f;
        scal = 0.00f;
        EEPROM.get(drrparm.biasy, bias);
        EEPROM.get(drrparm.scaly, scal);
        scal = scal == 0.00 ? 1.00 : scal;
        IMU.setMagCalY(bias, scal);

        bias = 0.00f;
        scal = 0.00f;
        EEPROM.get(drrparm.biasz, bias);
        EEPROM.get(drrparm.scalz, scal);
        scal = scal == 0.00 ? 1.00 : scal;
        IMU.setMagCalZ(bias, scal);
    }
}

void MPU_sensor()
{
    if (mpu_started)
    {
        IMU.readSensor();
        IMU.getTemperature_C();
        fusion.MadgwickUpdate(IMU.getGyroX_rads(),
                              IMU.getGyroY_rads(),
                              IMU.getGyroZ_rads(),
                              IMU.getAccelX_mss(),
                              IMU.getAccelY_mss(),
                              IMU.getAccelZ_mss(),
                              IMU.getMagX_uT(),
                              IMU.getMagY_uT(),
                              IMU.getMagZ_uT(),
                              fusion.deltatUpdate());
        pitch = -1 * fusion.getPitch();
        roll = 180 - fusion.getRoll() > 180 ? -180.0 - fusion.getRoll() : 180 - fusion.getRoll();
        yaw = fusion.getYaw();
    }
}

void MPU_calibrate()
{
    if (mpu_started)
    {
        drrMAGBS drrparm;
        wdt_disable(); // Detiene el watdog para que la calibración sea posible, de lo contrario reiniciará el arduino antes de acabar
        IMU.calibrateMag();
        wdt_enable(WDTO_1S); // ReIniciando el WDT

        EEPROM.put(drrparm.biasx, IMU.getMagBiasX_uT());
        EEPROM.put(drrparm.biasy, IMU.getMagBiasY_uT());
        EEPROM.put(drrparm.biasz, IMU.getMagBiasZ_uT());
        EEPROM.put(drrparm.scalx, IMU.getMagScaleFactorX());
        EEPROM.put(drrparm.scaly, IMU.getMagScaleFactorY());
        EEPROM.put(drrparm.scalz, IMU.getMagScaleFactorZ());
    }
}
