
void send_status()
{
    data_send = String("{") +
                String("'m0':") + String(m0_step) +
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
    if (order == 0)
    {
        Serial.println("{'name':\"arm\"}");
    }
    else if (order == 1)
    {
        send_status();
    }
    else if (order == 2)
    {
        home_position();
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
