
void send_telemetry()
{
    data_send = String("{") +
                String("'roll':") + String(int(roll)) +
                String(",") + String("'pitch':") + String(int(pitch)) +
                String(",") + String("'yaw':") + String(int(yaw)) +
                String(",") + String("'Tm':") + String(int(IMU.getTemperature_C())) +
                String(",") + String("'dist':") + String(int(dist_obst)) +
                String(",") + String("'DIR':") + Status_dir +
                String(",") + String("'SP':") + Status_speed +
                String(",") + String("'C':") + Status_cal +
                String("}");
    Serial.println(data_send);
}

void response_order(int order)
{
    if (order == 0)
    {
        Serial.println("{'name':\"car\"}");
    }
    else if (order == 1)
    {
        send_telemetry();
    }
}

void serialEvent()
{
    //while(Serial.available() ){
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
            robot_move(inData);
        }
        else if (function == "CA")
        {
            MPU_calibrate();
        }
        //Serial.println(inData);
        inData = "";
    }
}
