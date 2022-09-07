/*******************************************************************************
* Copyright(C) Analog Devices Inc., All Rights Reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files(the "Software"),
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
* IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY CLAIM, DAMAGES
* OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*
* Except as contained in this notice, the name of Analog Devices Inc.
* shall not be used except as stated in the Analog Devices Inc.
* Branding Policy.
*
* The mere transfer of this software does not imply any licenses
* of trade secrets, proprietary technology, copyrights, patents,
* trademarks, maskwork rights, or any other form of intellectual
* property whatsoever. Analog Devices Inc.retains all ownership rights.
********************************************************************************
*/


#include <MAX31328/MAX31328.h>
#include <stdarg.h>


#define GET_BIT_VAL(val, pos, mask)     ( ( (val) & mask) >> pos )
#define SET_BIT_VAL(val, pos, mask)     ( ( ((int)val) << pos) & mask )

#define BCD2BIN(val) (((val) & 15) + ((val) >> 4) * 10)
#define BIN2BCD(val) ((((val) / 10) << 4) + (val) % 10)


int MAX31328::read_register(uint8_t reg, uint8_t *buf, uint8_t len/*=1*/)
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
    m_i2c->requestFrom((char)m_slave_addr, (char)len, true);

    while (m_i2c->available()) { // slave may send less than requested
        buf[counter++] = m_i2c->read(); // receive a byte as character
    }

    //
    if (counter != len) {
        m_i2c->begin(); // restart
        ret = -1;
    }

    return ret;
}

int MAX31328::write_register(uint8_t reg, const uint8_t *buf, uint8_t len/*=1*/)
{
    int ret;

    m_i2c->beginTransmission(m_slave_addr);
    m_i2c->write(reg);
    m_i2c->write(buf, len);
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

/********************************************************************************/
MAX31328::MAX31328(TwoWire *i2c, uint8_t i2c_addr)
{
    if (i2c == NULL) {
        while (1);
    }

    m_i2c = i2c;
    m_slave_addr = i2c_addr;
}

void MAX31328::begin(void)
{
    m_i2c->begin();
}

int MAX31328::get_status(reg_status_t &stat)
{
    int ret;
    uint8_t val8;

    ret = read_register(MAX31328_R_STATUS, &val8);
    if (ret) {
        return ret;
    }

    stat.bits.a1f     = GET_BIT_VAL(val8, MAX31328_F_STATUS_A1F_POS,      MAX31328_F_STATUS_A1F);
    stat.bits.a2f     = GET_BIT_VAL(val8, MAX31328_F_STATUS_A2F_POS,      MAX31328_F_STATUS_A2F);
    stat.bits.bsy     = GET_BIT_VAL(val8, MAX31328_F_STATUS_BSY_POS,      MAX31328_F_STATUS_BSY);
    stat.bits.en32kHz = GET_BIT_VAL(val8, MAX31328_F_STATUS_EN32KHZ_POS,  MAX31328_F_STATUS_EN32KHZ);
    stat.bits.osf     = GET_BIT_VAL(val8, MAX31328_F_STATUS_OSF_POS,      MAX31328_F_STATUS_OSF);

    return ret;
}

int MAX31328::set_status(reg_status_t stat)
{
    int ret;
    uint8_t val8 = 0;

    val8 |= SET_BIT_VAL(stat.bits.a1f,     MAX31328_F_STATUS_A1F_POS,      MAX31328_F_STATUS_A1F);
    val8 |= SET_BIT_VAL(stat.bits.a2f,     MAX31328_F_STATUS_A2F_POS,      MAX31328_F_STATUS_A2F);
    val8 |= SET_BIT_VAL(stat.bits.bsy,     MAX31328_F_STATUS_BSY_POS,      MAX31328_F_STATUS_BSY);
    val8 |= SET_BIT_VAL(stat.bits.en32kHz, MAX31328_F_STATUS_EN32KHZ_POS,  MAX31328_F_STATUS_EN32KHZ);
    val8 |= SET_BIT_VAL(stat.bits.osf,     MAX31328_F_STATUS_OSF_POS,      MAX31328_F_STATUS_OSF);

    ret = write_register(MAX31328_R_STATUS, &val8);

    return ret;
}

int MAX31328::get_configuration(reg_cfg_t &cfg)
{
    int ret;
    uint8_t val8;

    ret = read_register(MAX31328_R_CONTROL, &val8);
    if (ret) {
        return ret;
    }

    cfg.bits.a1ie  = GET_BIT_VAL(val8, MAX31328_F_CTRL_A1IE_POS,   MAX31328_F_CTRL_A1IE);
    cfg.bits.a2ie  = GET_BIT_VAL(val8, MAX31328_F_CTRL_A2IE_POS,   MAX31328_F_CTRL_A2IE);
    cfg.bits.intcn = GET_BIT_VAL(val8, MAX31328_F_CTRL_INTCN_POS,  MAX31328_F_CTRL_INTCN);
    cfg.bits.rs    = GET_BIT_VAL(val8, MAX31328_F_CTRL_RS_POS,     MAX31328_F_CTRL_RS);
    cfg.bits.conv  = GET_BIT_VAL(val8, MAX31328_F_CTRL_CONV_POS,   MAX31328_F_CTRL_CONV);
    cfg.bits.bbsqw = GET_BIT_VAL(val8, MAX31328_F_CTRL_BBSQW_POS,  MAX31328_F_CTRL_BBSQW);
    cfg.bits.eosc  = GET_BIT_VAL(val8, MAX31328_F_CTRL_EOSC_POS,   MAX31328_F_CTRL_EOSC);

    return ret;
}

int MAX31328::set_configuration(reg_cfg_t cfg)
{
    int ret;
    uint8_t val8 = 0;

    val8 |= SET_BIT_VAL(cfg.bits.a1ie,  MAX31328_F_CTRL_A1IE_POS,   MAX31328_F_CTRL_A1IE);
    val8 |= SET_BIT_VAL(cfg.bits.a2ie,  MAX31328_F_CTRL_A2IE_POS,   MAX31328_F_CTRL_A2IE);
    val8 |= SET_BIT_VAL(cfg.bits.intcn, MAX31328_F_CTRL_INTCN_POS,  MAX31328_F_CTRL_INTCN);
    val8 |= SET_BIT_VAL(cfg.bits.rs,    MAX31328_F_CTRL_RS_POS,     MAX31328_F_CTRL_RS);
    val8 |= SET_BIT_VAL(cfg.bits.conv,  MAX31328_F_CTRL_CONV_POS,   MAX31328_F_CTRL_CONV);
    val8 |= SET_BIT_VAL(cfg.bits.bbsqw, MAX31328_F_CTRL_BBSQW_POS,  MAX31328_F_CTRL_BBSQW);
    val8 |= SET_BIT_VAL(cfg.bits.eosc,  MAX31328_F_CTRL_EOSC_POS,   MAX31328_F_CTRL_EOSC);

    ret = write_register(MAX31328_R_CONTROL, &val8);

    return ret;
}

int MAX31328::set_time(const struct tm *time)
{
    int ret;
    regs_rtc_time_t regs;

    if (time == NULL) {
        return -1;
    }

    /*********************************************************
     * +----------+------+---------------------------+-------+
     * | Member   | Type | Meaning                   | Range |
     * +----------+------+---------------------------+-------+
     * | tm_sec   | int  | seconds after the minute  | 0-61* |
     * | tm_min   | int  | minutes after the hour    | 0-59  |
     * | tm_hour  | int  | hours since midnight      | 0-23  |
     * | tm_mday  | int  | day of the month          | 1-31  |
     * | tm_mon   | int  | months since January      | 0-11  |
     * | tm_year  | int  | years since 1900          |       |
     * | tm_wday  | int  | days since Sunday         | 0-6   |
     * | tm_yday  | int  | days since January 1      | 0-365 |
     * | tm_isdst | int  | Daylight Saving Time flag |       |
     * +----------+------+---------------------------+-------+
     * * tm_sec is generally 0-59. The extra range is to accommodate for leap
     *   seconds in certain systems.
     *********************************************************/
    regs.seconds.bcd.value = BIN2BCD(time->tm_sec);

    regs.minutes.bcd.value = BIN2BCD(time->tm_min);

    regs.hours.bcd_format24.value= BIN2BCD(time->tm_hour);

    regs.day.bcd.value = BIN2BCD(time->tm_wday + 1);

    regs.date.bcd.value = BIN2BCD(time->tm_mday);

    regs.month.bcd.value = BIN2BCD(time->tm_mon + 1);

    if (time->tm_year >= 200) {
        regs.month.bits.century = 1;
        regs.year.bcd.value = BIN2BCD(time->tm_year - 200);
    } else if (time->tm_year >= 100) {
        regs.month.bits.century = 0;
        regs.year.bcd.value = BIN2BCD(time->tm_year - 100);
    } else {
        //Invalid set date!
        return -1;
    }

    ret = write_register(MAX31328_R_SECONDS, (uint8_t *) &regs, sizeof(regs));
    
    return ret;
}

int MAX31328::set_alarm(alarm_no_t alarm_no, const struct tm *alarm_time, alarm_period_t period)
{
    int ret;
    regs_alarm_t regs;

    if ((alarm_no == ALARM2) && (period == ALARM_PERIOD_EVERYSECOND)) {
        return -1; /* Alarm2 does not support "once per second" alarm*/
    }

    /* Set Alarm Period */
    regs.sec.bits.axm1 = 1;
    regs.min.bits.axm2 = 1;
    regs.hrs.bits.axm3 = 1;
    regs.day_date.bits.axm4 = 1;
    regs.day_date.bits.dy_dt = 1;

    switch (period) {
        case ALARM_PERIOD_EVERYSECOND:
            // Do nothing
            break;
        case ALARM_PERIOD_EVERYMINUTE:
            regs.sec.bits.axm1 = 0;
            break;
        case ALARM_PERIOD_HOURLY:
            regs.sec.bits.axm1 = 0;
            regs.min.bits.axm2 = 0;
            break;
        case ALARM_PERIOD_DAILY:
            regs.sec.bits.axm1 = 0;
            regs.min.bits.axm2 = 0;
            regs.hrs.bits.axm3 = 0;
            break;
        case ALARM_PERIOD_WEEKLY:
            regs.sec.bits.axm1 = 0;
            regs.min.bits.axm2 = 0;
            regs.hrs.bits.axm3 = 0;
            regs.day_date.bits.axm4 = 0;
            break;
        case ALARM_PERIOD_MONTHLY:
            regs.sec.bits.axm1 = 0;
            regs.min.bits.axm2 = 0;
            regs.hrs.bits.axm3 = 0;
            regs.day_date.bits.axm4 = 0;
            regs.day_date.bits.dy_dt = 0;
            break;
        default:
            return -1;
    }

    /* Convert time structure to alarm registers */
    regs.sec.bcd.value = BIN2BCD(alarm_time->tm_sec);
    regs.min.bcd.value = BIN2BCD(alarm_time->tm_min);
    regs.hrs.bcd_format24.value = BIN2BCD(alarm_time->tm_hour);

    if (regs.day_date.bits.dy_dt == 0) {
        /* Date match */
        regs.day_date.bcd_date.value = BIN2BCD(alarm_time->tm_mday);
    } else {
        /* Day match */
        regs.day_date.bcd_day.value = BIN2BCD(alarm_time->tm_wday);
    }

    
    /* 
     *  Write Registers 
     */
    uint8_t *ptr_regs = (uint8_t *)&regs;

    if (alarm_no == ALARM1) {
        ret = write_register(MAX31328_R_ALRM1_SECONDS, &ptr_regs[0], sizeof(regs_alarm_t));
    } else {
        /* Discard sec starts from min register */
        ret = write_register(MAX31328_R_ALRM2_MINUTES, &ptr_regs[1], sizeof(regs_alarm_t)-1);
    }
    if (ret) {
        return ret;
    }

    return ret;
}

int MAX31328::get_alarm(alarm_no_t alarm_no, struct tm *alarm_time, alarm_period_t *period, bool *is_enabled)
{
    int ret;
    regs_alarm_t regs;
    uint8_t val8;
    uint8_t *ptr_regs = (uint8_t *)&regs;

    /*
     *  Read registers
     */
    if (alarm_no == ALARM1) {
        ret = read_register(MAX31328_R_ALRM1_SECONDS, &ptr_regs[0], sizeof(regs_alarm_t));
    } else {
        regs.sec.raw = 0;  /* zeroise second register for alarm2 */
        /* starts from min register (no sec register) */
        ret = read_register(MAX31328_R_ALRM2_MINUTES, &ptr_regs[1], sizeof(regs_alarm_t)-1);
    }

    if (ret) {
        return ret;
    }

    /* 
     *  Convert alarm registers to time structure 
     */
    alarm_time->tm_sec  = BCD2BIN(regs.sec.bcd.value);
    alarm_time->tm_min  = BCD2BIN(regs.min.bcd.value);
    alarm_time->tm_hour = BCD2BIN(regs.hrs.bcd_format24.value);

    if (regs.day_date.bits.dy_dt == 0) { /* date */
        alarm_time->tm_mday = BCD2BIN(regs.day_date.bcd_date.value);
    } else { /* day */
        alarm_time->tm_wday = BCD2BIN(regs.day_date.bcd_day.value);
    }

    /*
     *  Find period
     */
    *period = (alarm_no == ALARM1) ? ALARM_PERIOD_EVERYSECOND : ALARM_PERIOD_EVERYMINUTE;

    if ((alarm_no == ALARM1) && (regs.sec.bits.axm1 == 0)) *period = ALARM_PERIOD_EVERYMINUTE;
    if (regs.min.bits.axm2 == 0) *period = ALARM_PERIOD_HOURLY;
    if (regs.hrs.bits.axm3 == 0) *period = ALARM_PERIOD_DAILY;
    if (regs.day_date.bits.axm4 == 0) *period = ALARM_PERIOD_WEEKLY;
    if (regs.day_date.bits.dy_dt == 0) *period = ALARM_PERIOD_MONTHLY;


    /*
     *  Get enable status
     */
    ret = read_register(MAX31328_R_CONTROL, &val8);
    if (ret) {
        return ret;
    }
    *is_enabled = (val8 & (uint8_t)alarm_no) ? true: false;

    return ret;
}

int MAX31328::get_time(struct tm *time)
{
    int ret;
    regs_rtc_time_t regs;

    if (time == NULL) {
        return -1;
    }

    ret = read_register(MAX31328_R_SECONDS, (uint8_t *) &regs, sizeof(regs));
    if (ret) {
        return -1;
    }

    /* tm_sec seconds [0,61] */
    time->tm_sec = BCD2BIN(regs.seconds.bcd.value);

    /* tm_min minutes [0,59] */
    time->tm_min = BCD2BIN(regs.minutes.bcd.value);

    /* tm_hour hour [0,23] */
    if (regs.hours.bits.f24_12) {
        time->tm_hour = BCD2BIN(regs.hours.bcd_format12.value);

        if (regs.hours.bits.hr20_am_pm  && (time->tm_hour != 12)) {
            time->tm_hour += 12;
        }
    } else {
        time->tm_hour = BCD2BIN(regs.hours.bcd_format24.value);
    }

    /* tm_wday day of week [0,6] (Sunday = 0) */
    time->tm_wday = BCD2BIN(regs.day.bcd.value) - 1;

    /* tm_mday day of month [1,31] */
    time->tm_mday = BCD2BIN(regs.date.bcd.value);

    /* tm_mon month of year [0,11] */
    time->tm_mon = BCD2BIN(regs.month.bcd.value) - 1;

    /* tm_year years since 2000 */
    if (regs.month.bits.century) {
        time->tm_year = BCD2BIN(regs.year.bcd.value) + 200;
    } else {
        time->tm_year = BCD2BIN(regs.year.bcd.value) + 100;
    }

    /* tm_yday day of year [0,365] */
    time->tm_yday = 0; /* TODO */

    /* tm_isdst daylight savings flag */
    time->tm_isdst = 0; /* TODO */

    return ret;
}

int MAX31328::irq_enable(intr_id_t id/*=INTR_ID_ALL*/)
{
    int ret;
    uint8_t val8;

    ret = read_register(MAX31328_R_CONTROL, &val8);
    if (ret) {
        return ret;
    }
    
    val8 |= MAX31328_F_CTRL_INTCN;
    if (id == INTR_ID_ALL) {
        val8 |= MAX31328_F_CTRL_A2IE | MAX31328_F_CTRL_A1IE;
    } else {
        val8 |= id;
    }

    ret = write_register(MAX31328_R_CONTROL, &val8);

    return ret;
}

int MAX31328::irq_disable(intr_id_t id/*=INTR_ID_ALL*/)
{
    int ret;
    uint8_t val8;

    ret = read_register(MAX31328_R_CONTROL, &val8);
    if (ret) {
        return ret;
    }

    if (id == INTR_ID_ALL) {
        val8 &= ~MAX31328_F_CTRL_INTCN;
        val8 &= ~(MAX31328_F_CTRL_A2IE | MAX31328_F_CTRL_A1IE);
    } else {
        val8 &= ~id; 
    }

    val8 &= ~id;
    ret = write_register(MAX31328_R_CONTROL, &val8);

    return ret;
}

int MAX31328::irq_clear_flag(intr_id_t id/*=INTR_ID_ALL*/)
{
    int ret;
    uint8_t val8;

    ret = read_register(MAX31328_R_STATUS, &val8);
    if (ret) {
        return ret;
    }

    if (id == INTR_ID_ALL) {
        val8 &= ~(MAX31328_F_STATUS_A2F | MAX31328_F_STATUS_A1F);
    } else {
        val8 &= ~id; 
    }
    ret = write_register(MAX31328_R_STATUS, &val8);

    return ret;
}

int MAX31328::set_square_wave_frequency(sqw_out_freq_t freq)
{
    int ret;
    uint8_t val8;

    ret = read_register(MAX31328_R_CONTROL, &val8);
    if (ret) {
        return ret;
    }

    // Set mode to SQW
    val8 &= ~MAX31328_F_CTRL_INTCN;

    // Set Frequency
    val8 &= ~MAX31328_F_CTRL_RS;
    val8 |= SET_BIT_VAL(freq, MAX31328_F_CTRL_RS_POS, MAX31328_F_CTRL_RS);

    ret = write_register(MAX31328_R_CONTROL, &val8);

    return ret;
}

int MAX31328::start_temp_conversion(void)
{
    int ret;
    uint8_t cfg;
    uint8_t stat;

    ret = read_register(MAX31328_R_STATUS, &stat);
    if (ret) {
        return ret;
    }

    if (stat & MAX31328_F_STATUS_BSY) {
        return MAX31328_ERR_BUSY; // Device is busy
    }

    ret = read_register(MAX31328_R_CONTROL, &cfg);
    if (ret) {
        return ret;
    }

    cfg |= MAX31328_F_CTRL_CONV;

    ret = write_register(MAX31328_R_CONTROL, &cfg);

    return ret;
}

int MAX31328::is_temp_ready(void)
{
    int ret;
    uint8_t cfg;

    ret = read_register(MAX31328_R_CONTROL, &cfg);
    if (ret) {
        return ret;
    }

    if (cfg & MAX31328_F_CTRL_CONV) {
        return MAX31328_ERR_BUSY; // means temperatrue NOT ready
    } else {
        return 0; // means temperature ready 
    }

    return ret;
}

int MAX31328::get_temp(float &temp)
{
    int ret;
    uint8_t  buf[2];
    uint16_t count;

    #define TEMP_RESOLUTION_FOR_10_BIT      (0.25f)

    ret = read_register(MAX31328_R_MSB_TEMP, buf, 2);
    if (ret) {
        return ret;
    }

    // buf[0] includes upper 8 bits, buf[1](7:6 bits) includes lower 2 bits
    count =(buf[0]<<2) | ( (buf[1]>>6) & 0x03 );

    count &= 0x3FF; // Resolution is 10 bits, mask rest of it

    // convert count to temperature, 10th bit is sign bit
    if (count & (1<<9) ) {
        count = (count ^ 0x3FF) + 1;
        temp  = count * TEMP_RESOLUTION_FOR_10_BIT;
        temp  = 0 - temp; // convert to negative
    } else {
        temp   = count * TEMP_RESOLUTION_FOR_10_BIT;
    }

    return ret;    
}
