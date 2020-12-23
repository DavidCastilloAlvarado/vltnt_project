void robot_move(String &inData)
{
    // TODO: Usar el string inData para ahorrar memoria dinámica
    int DIRstart = inData.indexOf('D');
    int SPstart = inData.indexOf('S');
    int DIR = inData.substring(DIRstart + 1, SPstart).toInt();
    int SPEED = inData.substring(SPstart + 1).toInt();
    time_run_prev = millis();
    Motor_action(DIR, SPEED);
}

void stop_motor_time_run()
{
    unsigned long timedif = millis() - time_run_prev;
    if (timedif > time_run || time_run_prev > millis())
    { // El motor se parará cuando pase el tiempo de corte o cuando exista un revalse en el valor del millis()
        Motor_action(0, 0);
    }
}

void init_Motor_control()
{
    //Motor 1
    pinMode(IN1[0], OUTPUT);
    pinMode(IN2[0], OUTPUT);
    pinMode(PWMp[0], OUTPUT);
    // Motor 2
    pinMode(IN1[1], OUTPUT);
    pinMode(IN2[1], OUTPUT);
    pinMode(PWMp[1], OUTPUT);
    Motor_action(0, 0);
}

void shaftrev(int in1, int in2, int PWM, int sentido, int Wpulse)
{
    switch (sentido)
    {
        // Sentido:
    case 0: // BACKWARD
        digitalWrite(in2, HIGH);
        digitalWrite(in1, LOW);
        analogWrite(PWM, Wpulse);
        break;
    case 1: // FORWARD
        digitalWrite(in2, LOW);
        digitalWrite(in1, HIGH);
        analogWrite(PWM, Wpulse);
        break;
    case 3:                      // STOP
        digitalWrite(in2, HIGH); // se está usando un circuito optoacoplado, por lo que se tuvo que invertir
        digitalWrite(in1, HIGH); // se invitió de LOW a HIGH para evitar el esfuerzo del
        analogWrite(PWM, 255);   // Wpulse se reemplazo por 255
        break;
    }
}

void Motor_action(int direct, int pwmS)
{
    pwmS = pwmS * 255 / 9;
    Status_speed = String(int(pwmS / 255.0 * 100));
    switch (direct)
    {
    case 1: // FOrward
        shaftrev(IN1[0], IN2[0], PWMp[0], forWARDS, pwmS);
        shaftrev(IN1[1], IN2[1], PWMp[1], forWARDS, pwmS);
        Status_dir = "\"FORWARD\"";
        break;
    case 2: // BackWard
        shaftrev(IN1[0], IN2[0], PWMp[0], backWARDS, pwmS);
        shaftrev(IN1[1], IN2[1], PWMp[1], backWARDS, pwmS);
        Status_dir = "\"BACKWARD\"";
        break;
    case 4: // Left
        shaftrev(IN1[0], IN2[0], PWMp[0], backWARDS, pwmS);
        shaftrev(IN1[1], IN2[1], PWMp[1], forWARDS, pwmS);
        Status_dir = "\"LEFT\"";
        break;
    case 3: // Right
        shaftrev(IN1[0], IN2[0], PWMp[0], forWARDS, pwmS);
        shaftrev(IN1[1], IN2[1], PWMp[1], backWARDS, pwmS);
        Status_dir = "\"RIGHT\"";
        break;
    case 0: // Stop
        shaftrev(IN1[0], IN2[0], PWMp[0], 3, 0);
        shaftrev(IN1[1], IN2[1], PWMp[1], 3, 0);
        Status_dir = "\"STOP\"";
        Status_speed = "0";
        break;
    default: // Stop
        shaftrev(IN1[0], IN2[0], PWMp[0], 3, 0);
        shaftrev(IN1[1], IN2[1], PWMp[1], 3, 0);
        Status_dir = "\"STOP\"";
        Status_speed = "0";
        break;
    }
}
