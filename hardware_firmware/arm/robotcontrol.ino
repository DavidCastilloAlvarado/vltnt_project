/** 
 * control_arm = {"A": None, "B": None,
               "C": None, "D": None, "Y": None, "P": None, "S": None}
**/

void robot_command(String &inData)
{
    // Extrayendo datos de movimiento del robot
    int J0start = inData.indexOf('A');
    int J1start = inData.indexOf('B');
    int J2start = inData.indexOf('C');
    int J3start = inData.indexOf('D');
    int SV1start = inData.indexOf('Y');
    int SV2start = inData.indexOf('P');
    int SPEEDstart = inData.indexOf('S');

    int J0dir = inData.substring(J0start + 1, J0start + 2).toInt();
    int J1dir = inData.substring(J1start + 1, J1start + 2).toInt();
    int J2dir = inData.substring(J2start + 1, J2start + 2).toInt();
    int J3dir = inData.substring(J3start + 1, J3start + 2).toInt();

    int J0step = inData.substring(J0start + 2, J1start).toInt();
    int J1step = inData.substring(J1start + 2, J2start).toInt();
    int J2step = inData.substring(J2start + 2, J3start).toInt();
    int J3step = inData.substring(J3start + 2, SV1start).toInt();

    int sv1pos = inData.substring(SV1start + 1, SV2start).toInt();
    int sv2pos = inData.substring(SV2start + 1, SPEEDstart).toInt();
    int SpeedIn = inData.substring(SPEEDstart + 1).toInt();

    // Calculo de la velocidad
    float AdjSpeed = (SpeedIn / 100.0);
    float CalcDCCSpeed = (SpeedMult / AdjSpeed);

    // Encontrando el maximo paso a desplazar
    int HighStep = J0step;
    if (J1step > HighStep)
    {
        HighStep = J1step;
    }
    // Determinando cuantos ejes involucrados en la posición se desea mover.
    int Jactive = 0;
    if (J0step >= 1)
    {
        Jactive = ++Jactive;
    }
    if (J1step >= 1)
    {
        Jactive = ++Jactive;
    }
    // Seteando la dirección deseada de movimiento según la conexión y posición de los motores físicos.
    set_direction(J0dir, cwdir.j0, pimmotor.m0_dir, cwst.j0);
    set_direction(J1dir, cwdir.j1, pimmotor.m1_dir, cwst.j1);
    set_direction(J2dir, cwdir.j2, pimmotor.m2_dir, cwst.j2);
    set_direction(J3dir, cwdir.j3, pimmotor.m3_dir, cwst.j3);

    // Creando las variables aux
    // creando el delay para el accionamiento de los motores
    float curDelay = (CalcDCCSpeed / (Jactive)*1.0);
    /*-----------------------J0-----------------------*/
    // calculados
    int J0_PE = 0;
    int J0_SE_1 = 0;
    int J0_LO_1 = 0;
    int J0_SE_2 = 0;
    int J0_LO_2 = 0;
    // Dinamicos
    int J0cur = 0;
    int J0_PEcur = 0;
    int J0_SE_1cur = 0;
    int J0_SE_2cur = 0;
    /*-----------------------J1-----------------------*/
    // Calculados
    int J1_PE = 0;
    int J1_SE_1 = 0;
    int J1_LO_1 = 0;
    int J1_SE_2 = 0;
    int J1_LO_2 = 0;
    // Dinámicos
    int J1cur = 0;
    int J1_PEcur = 0;
    int J1_SE_1cur = 0;
    int J1_SE_2cur = 0;
    /*-----------------------EXTRAS-----------------------*/
    // Extras para motores no cinematicos
    int J2cur = 0;
    int J3cur = 0;
    bool passLim = true;
    bool Fail = false;
    //Calcular variables aux para realizar el movimiento total conjunto.
    calc_aux_values(J0_PE, J0_SE_1, J0_LO_1, J0_SE_2, J0_LO_2, HighStep, J0step);
    calc_aux_values(J1_PE, J1_SE_1, J1_LO_1, J1_SE_2, J1_LO_2, HighStep, J1step);

    // Movimiento de motores para el posicionamiento
    Serial.print(Jactive);
    Serial.print(" ");
    Serial.println(curDelay);
    while (J0cur < J0step || J1cur < J1step && passLim && (Fail == false))
    {
        if (J0cur < J0step)
        {
            if (read_fc_limits(pinfc.fc01, pinfc.fc02, cwst.j0, passLim) && (fail_status(Fail) == false))
            {
                Jmove(pimmotor.m0_step, curDelay, J0_PE, J0_SE_1, J0_LO_1, J0_SE_2, J0_LO_2, J0cur, J0_PEcur, J0_SE_1cur, J0_SE_2cur);
            }
            else
            {
                break;
            }
        }
        if (J1cur < J1step)
        {
            if (read_fc_limits(pinfc.fc11, pinfc.fc12, cwst.j1, passLim) && (fail_status(Fail) == false))
            {
                Jmove(pimmotor.m1_step, curDelay, J1_PE, J1_SE_1, J1_LO_1, J1_SE_2, J1_LO_2, J1cur, J1_PEcur, J1_SE_1cur, J1_SE_2cur);
            }
            else
            {
                break;
            }
        }
        wdt_reset(); // reset watch dog
        // Establecer tiempo de espera en caso solo se cuente con un único motor activado
        Jactive == 1 ? delayMicroseconds(curDelay) : delayMicroseconds(0);
    }
    // El motor que controla el gripper tiene mucha perdida por lo que necesita más torque para ser movido
    curDelay = curDelay < 500 ? 500 : curDelay;
    while (J3cur < J3step || J2cur < J2step)
    {
        if (J2cur < J2step)
        {
            if (fail_status(Fail))
            {
                break;
            }
            stepper_move(pimmotor.m2_step, curDelay, J2cur);
        }
        if (J3cur < J3step)
        {
            if (fail_status(Fail))
            {
                break;
            }
            stepper_move(pimmotor.m3_step, curDelay, J3cur);
        }
        wdt_reset(); // reset watch dog
    }
    status = passLim ? "ok" : "fcLim";
    status = Fail ? "Failmotor" : status;

    // Movimiento de los servos
    servo_move(sv1pos, sv2pos);

    send_status();
}

void home_position()
{
    set_direction(1, cwdir.j0, pimmotor.m0_dir, cwst.j0); // mover en CCW
    set_direction(0, cwdir.j1, pimmotor.m1_dir, cwst.j1); // mover en CW
    bool passLim_0 = true;
    bool passLim_1 = true;
    bool Fail = false;
    int J0cur = 0;
    int J1cur = 0;
    float curDelay = 300 / 2;
    while (passLim_0 || passLim_1) // va ha parar el movimiento cuando se active el final de carrera
    {
        wdt_reset();
        if (read_fc_limits(pinfc.fc01, pinfc.fc02, cwst.j0, passLim_0))
        {
            if (fail_status(Fail))
            {
                break;
            }
            stepper_move(pimmotor.m0_step, curDelay, J0cur);
        }

        if (read_fc_limits(pinfc.fc11, pinfc.fc12, cwst.j1, passLim_1))
        {
            if (fail_status(Fail))
            {
                break;
            }
            stepper_move(pimmotor.m1_step, curDelay, J1cur);
        }
    }

    status = passLim_0 && passLim_1 ? "ok" : "fcLim";
    status = Fail ? "Failmotor" : status;
}

void servo_move(float servo1pos, float servo2pos)
{
    ////// Servo 1 /////
    if (servo1pos > limmotor.sv1pmax)
    {
        servo1pos = limmotor.sv1pmax;
    }
    else if (servo1pos < limmotor.sv1pmin)
    {
        servo1pos = limmotor.sv1pmin;
    }
    servopwm.setPWM(pimmotor.sv1p, 0, servo1pos);
    ////// Servo 2 //////
    if (servo2pos > limmotor.sv2pmax)
    {
        servo2pos = limmotor.sv2pmax;
    }
    else if (servo2pos < limmotor.sv2pmin)
    {
        servo2pos = limmotor.sv2pmin;
    }
    servopwm.setPWM(pimmotor.sv2p, 0, servo2pos);
}

bool read_fc_limits(byte fc1, byte fc2, bool cw_state, bool &pass)
{
    // se genera el error cuando la dirección de movimiento es en dirección(CW) del final de carrera activado
    if (digitalRead(fc1) == 0 and cw_state == true)
    {
        pass = false;
        return pass;
    }
    // se genera el error cuando la dirección de movimiento es en dirección(CCW) del final de carrera activado
    else if (digitalRead(fc2) == 0 and (cw_state == false))
    {
        pass = false;
        return pass;
    }
    else
    {
        return true;
    }
}

bool fail_status(bool &fail)
{
    if (digitalRead(pimmotor.m3_fault) || digitalRead(pimmotor.m2_fault) || digitalRead(pimmotor.m1_fault) || digitalRead(pimmotor.m0_fault))
    {
        fail = true;
        return fail;
    }
    else
    {
        fail = false;
        return fail;
    }
}

void Jmove(int jxstepPIN, float curDelay, int Jx_PE, int Jx_SE_1, int Jx_LO_1, int Jx_SE_2, int Jx_LO_2, int &Jxcur, int &Jx_PEcur, int &Jx_SE_1cur, int &Jx_SE_2cur)
{
    // Serial.println(jxstepPIN);
    if (Jx_SE_2 == 0)
    {
        Jx_SE_2cur = (Jx_SE_2 + 1);
    }
    if (Jx_SE_2cur != Jx_SE_2)
    {
        Jx_SE_2cur = ++Jx_SE_2cur;
        if (Jx_SE_1 == 0)
        {
            Jx_SE_1cur = (Jx_SE_1 + 1);
        }
        if (Jx_SE_1cur != Jx_SE_1)
        {
            Jx_SE_1cur = ++Jx_SE_1cur;
            Jx_PEcur = ++Jx_PEcur;
            if (Jx_PEcur == Jx_PE)
            {
                Jx_PEcur = 0;
                stepper_move(jxstepPIN, curDelay, Jxcur);
            }
        }
        else
        {
            Jx_SE_1cur = 0;
        }
    }
    else
    {
        Jx_SE_2cur = 0;
    }
}

void stepper_move(int jxstepPIN, float curDelay, int &Jxcur)
{
    Jxcur = ++Jxcur;
    digitalWrite(jxstepPIN, LOW);
    delayMicroseconds(curDelay);
    digitalWrite(jxstepPIN, HIGH);
    // delayMicroseconds(250.0); // delay de espera para la siguiente instrucción
}

void set_direction(byte Jxdir, byte Jxrotdir, byte JxPin, bool &cw)
{ /*
Jxdir: dirección enviada como comando
Jxrotdir: dirección cw seteada en este firmware 0 o 1
jxPin: que pin corresponde dir pin steppermotor
*/
    /////// Jx /////////
    if (Jxdir == 1 && Jxrotdir == 1)
    {
        cw = true;
        digitalWrite(JxPin, LOW);
    }
    else if (Jxdir == 1 && Jxrotdir == 0)
    {
        cw = true;
        digitalWrite(JxPin, HIGH);
    }
    else if (Jxdir == 0 && Jxrotdir == 1)
    {
        cw = false;
        digitalWrite(JxPin, HIGH);
    }
    else if (Jxdir == 0 && Jxrotdir == 0)
    {
        cw = false;
        digitalWrite(JxPin, LOW);
    }
}

void calc_aux_values(int &Jx_PE, int &Jx_SE_1, int &Jx_LO_1, int &Jx_SE_2, int &Jx_LO_2, int &HighStep, int &Jxstep)
{
    Jx_PE = (HighStep / Jxstep);
    ///find left over 1
    Jx_LO_1 = (HighStep - (Jxstep * Jx_PE)); // residuo de  (HighStep / J1step)
    ///find skip 1
    if (Jx_LO_1 > 0)
    {
        Jx_SE_1 = (HighStep / Jx_LO_1); // cociente entero
    }
    else
    {
        Jx_SE_1 = 0;
    }
    ///find left over 2
    if (Jx_SE_1 > 0)
    { //(1-e)*J1_LO_1, "e" siempre menor que 1, e=(HighStep- J1_LO_1)/HighStep
        Jx_LO_2 = HighStep - ((Jxstep * Jx_PE) + ((Jxstep * Jx_PE) / Jx_SE_1));
    }
    else
    {
        Jx_LO_2 = 0;
    }
    ///find skip 2
    if (Jx_LO_2 > 0)
    {
        Jx_SE_2 = (HighStep / Jx_LO_2);
    }
    else
    {
        Jx_SE_2 = 0;
    }
}

void init_robot_pins()
{
    // iniciando los pines de los finales de carrera
    pinMode(pinfc.fc01, INPUT_PULLUP);
    pinMode(pinfc.fc02, INPUT_PULLUP);
    pinMode(pinfc.fc11, INPUT_PULLUP);
    pinMode(pinfc.fc12, INPUT_PULLUP);

    //iniciando los pines relacionados al control de los motores
    digitalWrite(pimmotor.rst_all, LOW);
    pinMode(pimmotor.rst_all, OUTPUT);

    pinMode(pimmotor.m0_step, OUTPUT);
    pinMode(pimmotor.m0_dir, OUTPUT);
    pinMode(pimmotor.m0_fault, INPUT_PULLUP);

    pinMode(pimmotor.m1_step, OUTPUT);
    pinMode(pimmotor.m1_dir, OUTPUT);
    pinMode(pimmotor.m1_fault, INPUT_PULLUP);

    pinMode(pimmotor.m2_step, OUTPUT);
    pinMode(pimmotor.m2_dir, OUTPUT);
    pinMode(pimmotor.m2_fault, INPUT_PULLUP);

    pinMode(pimmotor.m3_step, OUTPUT);
    pinMode(pimmotor.m3_dir, OUTPUT);
    pinMode(pimmotor.m3_fault, INPUT_PULLUP);

    // Seteando los motor_step en high(se están usando optoacopladores que invierten el comportamiento)
    digitalWrite(pimmotor.m0_step, HIGH);
    digitalWrite(pimmotor.m1_step, HIGH);
    digitalWrite(pimmotor.m2_step, HIGH);
    digitalWrite(pimmotor.m3_step, HIGH);
}

void init_servos()
{
    servopwm.begin();
    servopwm.setPWMFreq(FREQ_);
}

void fc_status()
{
    bool fc01 = digitalRead(pinfc.fc01);
    bool fc02 = digitalRead(pinfc.fc02);
    bool fc11 = digitalRead(pinfc.fc11);
    bool fc12 = digitalRead(pinfc.fc12);
    Serial.println("{" + String(fc01) + String(" ") +
                   String(fc02) + String(" ") +
                   String(fc11) + String(" ") +
                   String(fc12) + String(" ") +
                   "}");
}
