void update_eeprom()
{
    POSmemo eeprom_memo;
    EEPROM.put(eeprom_memo.m0_step, m0_step);
    EEPROM.put(eeprom_memo.m1_step, m1_step);
    EEPROM.put(eeprom_memo.m2_step, m2_step);
    EEPROM.put(eeprom_memo.m3_step, m3_step);
    EEPROM.put(eeprom_memo.sv1_ang, sv1_ang);
    EEPROM.put(eeprom_memo.sv2_ang, sv2_ang);
}

void read_eeprom()
{
    POSmemo eeprom_memo;
    EEPROM.get(eeprom_memo.m0_step, m0_step);
    EEPROM.get(eeprom_memo.m1_step, m1_step);
    EEPROM.get(eeprom_memo.m2_step, m2_step);
    EEPROM.get(eeprom_memo.m3_step, m3_step);
    EEPROM.get(eeprom_memo.sv1_ang, sv1_ang);
    EEPROM.get(eeprom_memo.sv2_ang, sv2_ang);
}