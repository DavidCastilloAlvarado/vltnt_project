void init_HC_RC04()
{
    pinMode(Trigger, OUTPUT);   //pin como salida
    pinMode(Echo, INPUT);       //pin como entrada
    digitalWrite(Trigger, LOW); //Inicializamos el pin con 0
    time_out = d_max * 58.823;
}

void HC_RC04()
{
    digitalWrite(Trigger, HIGH);
    delayMicroseconds(10); //Enviamos un pulso de 10us
    digitalWrite(Trigger, LOW);

    t = pulseIn(Echo, HIGH, time_out); //obtenemos el ancho del pulso
    if (t == 0)
    {
        dist_obst = d_max; // Tiempo mayor al esperado =  más distancia Dmáx
    }
    else
    {
        dist_obst = t / 58.823; // Distancia en centimetros
    }

    if (i < 5)
    {                           // Acumulamos 5 lecturas
        dis_val[i] = dist_obst; // para sacar la media
        i++;
    }
    if (i == 5)
    {
        for (j = 0; j < 5; j++)
        { // calcula el promedio de mediciones
            sum_d += dis_val[j];
            dist_obst = sum_d / 5;
        }
        for (j = 0; j < 4; j++)
        { // Desplaza los valores dentro del array
            dis_val[j] = dis_val[j + 1];
        }
        i--;
        sum_d = 0;
    }
    if (dist_obst > d_max)
    {
        dist_obst = d_max;
    }
}
