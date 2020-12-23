/** 
 * control_arm = {"A": None, "B": None,
               "C": None, "D": None, "P": None, "R": None, "S": None}
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
    float AdjSpeed = (SpeedIn / 100);
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
    // Seteando la dirección deseada de movimiento (Por practicidad se setean todos los motores)
    set_direction(J0dir, cwdir.j0, pimmotor.m0_dir);
    set_direction(J1dir, cwdir.j1, pimmotor.m1_dir);
    set_direction(J2dir, cwdir.j2, pimmotor.m2_dir);
    set_direction(J3dir, cwdir.j3, pimmotor.m3_dir);

    // Creando las variables aux
    // creando el delay para el accionamiento de los motores
    float curDelay = (CalcDCCSpeed / Jactive);
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
    // Extras para motores no cinematicos de posicion
    int J2cur = 0;
    int J3cur = 0;
    //Calcular variables aux para realizar el movimiento total conjunto.
    calc_aux_values(J0_PE, J0_SE_1, J0_LO_1, J0_SE_2, J0_LO_2, HighStep, J0step);
    calc_aux_values(J1_PE, J1_SE_1, J1_LO_1, J1_SE_2, J1_LO_2, HighStep, J1step);

    // Movimiento de motores para el posicionamiento
    while (J1cur < J1step || J0cur < J0step)
    {
        if (J0cur < J0step)
        {
            Jmove(pimmotor.m0_step, curDelay, J0_PE, J0_SE_1, J0_LO_1, J0_SE_2, J0_LO_2, J0cur, J0_PEcur, J0_SE_1cur, J0_SE_2cur);
        }
        if (J1cur < J1step)
        {
            Jmove(pimmotor.m1_step, curDelay, J1_PE, J1_SE_1, J1_LO_1, J1_SE_2, J1_LO_2, J1cur, J1_PEcur, J1_SE_1cur, J1_SE_2cur);
        }
        if (J2cur < J2step)
        {
            stepper_move(pimmotor.m2_step, curDelay, J2cur);
        }
        if (J3cur < J3step)
        {
            stepper_move(pimmotor.m3_step, curDelay, J3cur);
        }
    }
}

void home_position()
{
}

void servo_move()
{
}

void Jmove(int jxstepPIN, float curDelay, int Jx_PE, int Jx_SE_1, int Jx_LO_1, int Jx_SE_2, int Jx_LO_2, int &Jxcur, int &Jx_PEcur, int &Jx_SE_1cur, int &Jx_SE_2cur)
{
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
}

void set_direction(byte Jxdir, byte Jxrotdir, byte JxPin)
{ /*
Jxdir: dirección enviada como comando
Jxrotdir: dirección cw seteada en este firmware 0 o 1
jxPin: que pin corresponde dir pin steppermotor
*/
    /////// Jx /////////
    if (Jxdir == 1 && Jxrotdir == 1)
    {
        digitalWrite(JxPin, LOW);
    }
    else if (Jxdir == 1 && Jxrotdir == 0)
    {
        digitalWrite(JxPin, HIGH);
    }
    else if (Jxdir == 0 && Jxrotdir == 1)
    {
        digitalWrite(JxPin, HIGH);
    }
    else if (Jxdir == 0 && Jxrotdir == 0)
    {
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