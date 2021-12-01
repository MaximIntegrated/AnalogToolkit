/*******************************************************************************
* Copyright (C) 2021 Maxim Integrated Products, Inc., All Rights Reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
* OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*
* Except as contained in this notice, the name of Maxim Integrated
* Products, Inc. shall not be used except as stated in the Maxim Integrated
* Products, Inc. Branding Policy.
*
* The mere transfer of this software does not imply any licenses
* of trade secrets, proprietary technology, copyrights, patents,
* trademarks, maskwork rights, or any other form of intellectual
* property whatsoever. Maxim Integrated Products, Inc. retains all
* ownership rights.
*******************************************************************************
*/

#include <MAX40080/MAX40080.h>

// General purpose global float variable, 
// the function that return float variables like: convert_count_2_current
// does not work for local float decleration and return
// so general purpose global float variable added here
static float g_float;

// 
static uint8_t crc_table[] = {
     0x00, 0x07, 0x0e, 0x09, 0x1c, 0x1b, 0x12, 0x15, 0x38, 0x3f, 0x36, 0x31,
     0x24, 0x23, 0x2a, 0x2d, 0x70, 0x77, 0x7e, 0x79, 0x6c, 0x6b, 0x62, 0x65,
     0x48, 0x4f, 0x46, 0x41, 0x54, 0x53, 0x5a, 0x5d, 0xe0, 0xe7, 0xee, 0xe9,
     0xfc, 0xfb, 0xf2, 0xf5, 0xd8, 0xdf, 0xd6, 0xd1, 0xc4, 0xc3, 0xca, 0xcd,
     0x90, 0x97, 0x9e, 0x99, 0x8c, 0x8b, 0x82, 0x85, 0xa8, 0xaf, 0xa6, 0xa1,
     0xb4, 0xb3, 0xba, 0xbd, 0xc7, 0xc0, 0xc9, 0xce, 0xdb, 0xdc, 0xd5, 0xd2,
     0xff, 0xf8, 0xf1, 0xf6, 0xe3, 0xe4, 0xed, 0xea, 0xb7, 0xb0, 0xb9, 0xbe,
     0xab, 0xac, 0xa5, 0xa2, 0x8f, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9d, 0x9a,
     0x27, 0x20, 0x29, 0x2e, 0x3b, 0x3c, 0x35, 0x32, 0x1f, 0x18, 0x11, 0x16,
     0x03, 0x04, 0x0d, 0x0a, 0x57, 0x50, 0x59, 0x5e, 0x4b, 0x4c, 0x45, 0x42,
     0x6f, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7d, 0x7a, 0x89, 0x8e, 0x87, 0x80,
     0x95, 0x92, 0x9b, 0x9c, 0xb1, 0xb6, 0xbf, 0xb8, 0xad, 0xaa, 0xa3, 0xa4,
     0xf9, 0xfe, 0xf7, 0xf0, 0xe5, 0xe2, 0xeb, 0xec, 0xc1, 0xc6, 0xcf, 0xc8,
     0xdd, 0xda, 0xd3, 0xd4, 0x69, 0x6e, 0x67, 0x60, 0x75, 0x72, 0x7b, 0x7c,
     0x51, 0x56, 0x5f, 0x58, 0x4d, 0x4a, 0x43, 0x44, 0x19, 0x1e, 0x17, 0x10,
     0x05, 0x02, 0x0b, 0x0c, 0x21, 0x26, 0x2f, 0x28, 0x3d, 0x3a, 0x33, 0x34,
     0x4e, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5c, 0x5b, 0x76, 0x71, 0x78, 0x7f,
     0x6a, 0x6d, 0x64, 0x63, 0x3e, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2c, 0x2b,
     0x06, 0x01, 0x08, 0x0f, 0x1a, 0x1d, 0x14, 0x13, 0xae, 0xa9, 0xa0, 0xa7,
     0xb2, 0xb5, 0xbc, 0xbb, 0x96, 0x91, 0x98, 0x9f, 0x8a, 0x8d, 0x84, 0x83,
     0xde, 0xd9, 0xd0, 0xd7, 0xc2, 0xc5, 0xcc, 0xcb, 0xe6, 0xe1, 0xe8, 0xef,
     0xfa, 0xfd, 0xf4, 0xf3
};

/************************************ Private Functions ***********************/
static uint8_t calc_crc8(uint8_t crc, uint8_t *buf, int len)
{
    int i;
 
    for (i=0; i<len; i++) {
        crc = crc_table[crc ^ buf[i]];
    }
 
    return crc;
}

int MAX40080::read_register(uint8_t reg, uint8_t *buf, uint8_t len/*=1*/)
{
    int ret;
    int counter = 0;

    m_i2c->beginTransmission(m_slave_addr);
    m_i2c->write(reg);

    /*
        stop = true, sends a stop message after transmission, releasing the I2C bus.
        
        stop = false, sends a restart message after transmission. 
          The bus will not be released, which prevents another master device from transmitting between messages. 
          This allows one master device to send multiple transmissions while in control.
    */
    ret = m_i2c->endTransmission(false);
    /*
        0:success
        1:data too long to fit in transmit buffer
        2:received NACK on transmit of address
        3:received NACK on transmit of data
        4:other error
    */
    if (ret != 0) {
        m_i2c->begin(); // restart
        return -1;
    }

    // Read
    int r_Len = len;
    if (m_reg_cfg.bits.pec) {
        r_Len += 1; // +1 for PEC byte
    }

    m_i2c->requestFrom((char)m_slave_addr, (char)r_Len, false);

    while (m_i2c->available()) { // slave may send less than requested
        buf[counter++] = m_i2c->read(); // receive a byte as character
    }
    
    m_i2c->endTransmission();

    //
    if (counter == r_Len) {
        if (m_reg_cfg.bits.pec) {
            uint8_t crc = 0;
            uint8_t addr = m_slave_addr<<1;

            crc = calc_crc8(crc, &addr, 1);
            crc = calc_crc8(crc, &reg, 1);
            
            addr += 1; // for read
            crc = calc_crc8(crc, &addr, 1);
            crc = calc_crc8(crc, buf,  len);
            
            if (crc != buf[r_Len-1]) {
                // CRC deos not match with PEC byte
                //return -2;
            }
        }
    } else {
        m_i2c->begin(); // restart
        ret = -1;
    }

    switch (len) {
        case 1: //single byte read
            break;
        case 2: // read word, convert big endian to little endian
            uint8_t tmp;
            tmp = buf[0];
            buf[0] = buf[1];
            buf[1] = tmp;
            break;
        case 4: // read double word, swap bytes
             #if 0
                uint8_t tmp;
                tmp = buf[0];
                buf[0] = buf[3];
                buf[3] = tmp;

                tmp = buf[1];
                buf[1] = buf[2];
                buf[2] = tmp; 
            #endif
            break;
    }

    return ret;
}

int MAX40080::write_register(uint8_t reg, const uint8_t *data, uint8_t len/*=1*/)
{
    int ret;
    uint8_t buf[4];

    switch (len) {
        case 1: //single byte read
            buf[0] = data[0];
            break;
        case 2: // read word, convert big endian to little endian
            buf[0] = data[1];
            buf[1] = data[0];
            break;
        case 4: // read double word, swap bytes
            #if 0
                buf[0] = data[3];
                buf[1] = data[2];
                buf[2] = data[1];
                buf[3] = data[0]; 
            #else
                buf[0] = data[0];
                buf[1] = data[1];
                buf[2] = data[2];
                buf[3] = data[3]; 
            #endif
            break;
        default:
            return -1;
    }

    m_i2c->beginTransmission(m_slave_addr);
    m_i2c->write(reg);
    m_i2c->write(buf, len);

    if (m_reg_cfg.bits.pec) {
        uint8_t crc = 0;
        uint8_t addr = m_slave_addr<<1;

        crc = calc_crc8(crc, &addr, 1);
        crc = calc_crc8(crc, &reg, 1);
        crc = calc_crc8(crc, buf,  len);
        
        m_i2c->write(crc);
    }

    ret = m_i2c->endTransmission();
    /*
        0:success
        1:data too long to fit in transmit buffer
        2:received NACK on transmit of address
        3:received NACK on transmit of data
        4:other error
    */

    if (ret != 0) {
        m_i2c->begin(); // restart
    }

    return ret;
}

uint16_t MAX40080::convert_voltage_2_count(float voltage, int resolution)
{
    uint16_t count;
    int max_val = (1<<resolution) - 1;

    count = (uint16_t) ( (voltage / MAX40080_FULL_RANGE_VOLTAGE) * (float)max_val);
    
    return count;
}

float MAX40080::convert_count_2_voltage(uint16_t count, int resolution)
{
    float voltage;
    int max_val = (1<<resolution) - 1;

    count &= max_val;
    voltage = count * (MAX40080_FULL_RANGE_VOLTAGE / (float)max_val);

    return voltage;
}

uint16_t MAX40080::convert_current_2_count(float current)
{
    uint16_t count;
    float step_per_bit;

    if (m_reg_cfg.bits.input_range == INPUT_RANGE_50mV) {
        step_per_bit = 0.050 / 4095.0;
    } else {
        step_per_bit = 0.010 / 4095.0;
    }

    if (current < 0) {
        current = 0 - current;
        count   = (uint16_t) ((current * m_shuntResistor) / step_per_bit);
        count   = (count - 1) ^ 0x1FFF;
    } else {
        count =(uint16_t) ((current * m_shuntResistor) / step_per_bit);
    }

    return count;
}

float MAX40080::convert_count_2_current(uint16_t count)
{
    float step_per_bit;

    if (m_reg_cfg.bits.input_range == INPUT_RANGE_50mV) {
        step_per_bit = 0.050f / 4095.0f;
    } else {
        step_per_bit = 0.010f / 4095.0f;
    }

    count &= 0x1FFF; // mask other bits
    if (count & (1<<12) ) { // check sign bit
        count   = (count ^ 0x1FFF) + 1;
        g_float = (count * step_per_bit) / m_shuntResistor ;
        g_float = 0 - g_float; // convert to negative
    } else {
        g_float = (count * step_per_bit) / m_shuntResistor;
    }

    return g_float;
}

/************************************ Public Functions ***********************/
MAX40080::MAX40080(TwoWire *i2c, uint8_t i2c_addr, float shuntResistor)
{
    if (i2c == NULL) {
        while (1) {
            ;
        }
    }
    
    m_i2c = i2c;
    m_slave_addr = i2c_addr;
    m_shuntResistor = shuntResistor;
}

void MAX40080::begin(void)
{
    m_i2c->begin();

    /* 
        set default values, for more info check UG
    */
    m_reg_cfg.bits.pec = 1; // on default pec enable
    m_reg_cfg.bits.mode = OP_MODE_STANDBY;
    m_reg_cfg.bits.input_range = INPUT_RANGE_50mV;
    m_reg_cfg.bits.i2c_timeout = 0;
    m_reg_cfg.bits.alert = 0;
    m_reg_cfg.bits.stay_hs_mode = 0;
    m_reg_cfg.bits.adc_sample_rate = 0;
    m_reg_cfg.bits.digital_filter = 0;
    m_reg_cfg.bits.internal_use = 0;
}

int MAX40080::get_status(reg_status_t &stat)
{
    int ret = 0;

    ret = read_register(MAX40080_R_STATUS, (uint8_t *)&stat.raw, 2);

    return ret;
}

int MAX40080::set_interrupt_status(intr_id_t interrupt, bool status)
{
    int ret;
    uint8_t byt;

    ret = read_register(MAX40080_R_INT_EN, &byt);
    if (ret) {
        return ret;
    }

    if (status) {
        byt |= interrupt;
    } else {
        byt &= ~interrupt; 
    }

    ret = write_register(MAX40080_R_INT_EN, &byt);

    return ret;
}

int MAX40080::clear_interrupt_flag(intr_id_t interrupt)
{
    int ret;
    reg_status_t stat;

    ret = get_status(stat);
    if (ret) {
        return ret;
    }

    stat.raw |= (uint8_t)interrupt;

    ret = write_register(MAX40080_R_INT_EN, (uint8_t *)&stat.raw, 2);

    return ret;
}

int MAX40080::get_configuration(reg_cfg_t &cfg)
{
    int ret;

    ret = read_register(MAX40080_R_CFG, (uint8_t *)&cfg.raw, 2);

    return ret;
}

int MAX40080::set_configuration(reg_cfg_t cfg)
{
    int ret;

    ret = write_register(MAX40080_R_CFG, (uint8_t *)&cfg.raw, 2);
    if (ret == 0) {
        m_reg_cfg = cfg;
    }

    return ret;
}

int MAX40080::fifo_configure(measure_type_t typ, uint8_t overflow_thrshld/*=0x34*/, bool rool_over_status/*=false*/)
{
    int ret;
    reg_fifo_cfg_t reg;

    ret = read_register(MAX40080_R_FIFO_CFG, (uint8_t *)&reg.raw, 2);
    if (ret) {
        return ret;
    }

    reg.bits.store_v_i = typ;
    reg.bits.overflow_thrshld = overflow_thrshld;
    reg.bits.rool_over = rool_over_status;
    
    ret = write_register(MAX40080_R_FIFO_CFG, (uint8_t *)&reg.raw, 2);

    return ret;
}

int MAX40080::flush_fifo(void)
{
    int ret;
    reg_fifo_cfg_t reg;

    ret = read_register(MAX40080_R_FIFO_CFG, (uint8_t *)&reg.raw, 2);
    if (ret) {
        return ret;
    }

    reg.bits.flush = 1;
    ret = write_register(MAX40080_R_FIFO_CFG, (uint8_t *)&reg.raw, 2);

    return ret;
}

int MAX40080::get_current(float &current)
{
    int ret;
    int16_t val16;

    ret = read_register(MAX40080_R_CURRENT, (uint8_t *)&val16, 2);
    if (ret) {
        return ret;
    }

    if (val16 & (1<<15)) { // is valid
        current = convert_count_2_current( val16 );
    } else {
        ret = -1;
    }

    return ret;
}

int MAX40080::get_voltage(float &voltage)
{
    int ret;
    int16_t val16;

    ret = read_register(MAX40080_R_VOLTAGE, (uint8_t *)&val16, 2);
    if (ret) {
        return ret;
    }

    if (val16 & (1<<15)) { // is valid
        voltage = convert_count_2_voltage(val16, 12);// 12 bit ADC
    } else {
        ret = -1;
    }

    return ret;
}

int MAX40080::get_current_and_voltage(float &current, float &voltage)
{
    int ret;
    int32_t val32;

    ret = read_register(MAX40080_R_CURRENT_AND_VOLTAGE, (uint8_t *)&val32, 4);
    if (ret) {
        return ret;
    }

    if (val32 & 1<<31) { // is valid
        // 12 bits, Voltage allways positive
        voltage = convert_count_2_voltage( (uint16_t)(val32>>16), 12);// 12 bit ADC

        // convert current
        current = convert_count_2_current( (uint16_t)(val32 & 0xffff) );
    } else {    
        ret = -1;
    }

    return ret;
}

int MAX40080::get_max_peak_current(float &current)
{
    int ret;
    uint16_t val16;

    ret = read_register(MAX40080_R_MAX_PEAK_CURRENT, (uint8_t *)&val16, 2);
    if (ret) {
        return ret;
    }
    
    current = convert_count_2_current( val16 );
    
    return ret;
}

int MAX40080::get_threshold_over_current(float &current)
{
    int ret;
    uint8_t val8;

    ret = read_register(MAX40080_R_THRSHLD_OVER_CURRENT, &val8);
    if (ret) {
        return ret;
    }    

    #if 0 // below case seems cause stack overflow
        current = convert_count_2_current( val8<<6 ); // it keeps msb 6 bits
    #else
        float step_per_bit;
        uint16_t count = val8<<6;

        if (m_reg_cfg.bits.input_range == INPUT_RANGE_50mV) {
            step_per_bit = 0.050f / 4095.0f;
        } else {
            step_per_bit = 0.010f / 4095.0f;
        }

        count &= 0x1FFF; // mask other bits
        if (count & (1<<12) ) { // check sign bit
            count   = (count ^ 0x1FFF) + 1;
            current = (count * step_per_bit) / m_shuntResistor ;
            current = 0 - current; // convert to negative
        } else {
            current = (count * step_per_bit) / m_shuntResistor;
        }
    #endif
    
    return ret;
}

int MAX40080::get_threshold_over_voltage(float &voltage)
{
    int ret;
    uint8_t val8;

    ret = read_register(MAX40080_R_THRSHLD_OVER_VOLTAGE, &val8);
    if (ret) {
        return ret;
    }

    voltage = convert_count_2_voltage( (uint16_t)val8, 6); // full range 6 bits
    
    return ret;
}

int MAX40080::get_threshold_under_voltage(float &voltage)
{
    int ret;
    uint8_t val8;

    ret = read_register(MAX40080_R_THRSHLD_UNDER_VOLTAGE, &val8);
    if (ret) {
        return ret;
    }

    voltage = convert_count_2_voltage( (uint16_t)val8, 6); // full range 6 bits
    
    return ret;
}

int MAX40080::get_wakeup_current(float &current)
{
    int ret;
    uint8_t val8;

    ret = read_register(MAX40080_R_WAKEUP_CURRENT, &val8);
    if (ret) {
        return ret;
    }

    current = convert_count_2_current( val8<<6 ); // it keeps msb 6 bits
    
    return ret;
}

int MAX40080::set_threshold_over_current(float current)
{
    int ret;
    uint8_t val8;

    val8 = (uint8_t) (convert_current_2_count(current) >> 6); // it keeps msb 6 bits

    ret = write_register(MAX40080_R_THRSHLD_OVER_CURRENT, &val8);
    
    return ret;
}

int MAX40080::set_threshold_over_voltage(float voltage)
{
    int ret;
    uint8_t val8;

    val8 = convert_voltage_2_count(voltage, 6); // full range 6 bits

    ret = write_register(MAX40080_R_THRSHLD_OVER_VOLTAGE, &val8);
    
    return ret;
}

int MAX40080::set_threshold_under_voltage(float voltage)
{
    int ret;
    uint8_t val8;

    val8 = convert_voltage_2_count(voltage, 6); // full range 6 bits

    ret = write_register(MAX40080_R_THRSHLD_UNDER_VOLTAGE, &val8);
    
    return ret;
}

int MAX40080::set_wakeup_current(float  current)
{
    int ret;
    uint8_t val8;

    val8 = (uint8_t) (convert_current_2_count(current) >> 6); // it keeps msb 6 bits

    ret = write_register(MAX40080_R_WAKEUP_CURRENT, &val8);
    
    return ret;
}