
void send_status()
{
    data_send = String("{") +
                String("'st':") + String(status) +
                String(",") + String("'m0':") + String(m0_step) +
                String(",") + String("'m1':") + String(m1_step) +
                String(",") + String("'m2':") + String(m2_step) +
                String(",") + String("'m3':") + String(m3_step) +
                String(",") + String("'sv1':") + String(int(sv1_ang)) +
                String(",") + String("'sv2':") + String(int(sv2_ang)) +
                String("}");
    Serial.println(data_send);
}

void response_order(int order)
{
    if (order == 0) // Llamar al nombre del robot
    {
        Serial.println("{'name':\"arm\"}");
    }
    else if (order == 1) // LLamar al estado del robot
    {
        send_status();
    }
    else if (order == 2) // LLamar a la posicion home
    {
        home_position();
    }
    else if (order == 3) // Reset motor drivers
    {
        digitalWrite(pimmotor.rst_all, HIGH);
        delay(100);
        digitalWrite(pimmotor.rst_all, LOW);
    }
    else if (order == 4)
    {
        fc_status();
    }
}

void serialEvent()
{
    char incomingByte = (char)Serial.read();
    inData += incomingByte;
    if (incomingByte == '\n')
    {
        String function = inData.substring(0, 2);
        if (function == "RQ")
        {
            int orderStart = inData.indexOf("N");
            int order = inData.substring(orderStart + 1).toInt();
            response_order(order);
        }
        else if (function == "MV")
        {
            robot_command(inData);
        }
        //Serial.println(inData);
        inData = "";
    }
}
