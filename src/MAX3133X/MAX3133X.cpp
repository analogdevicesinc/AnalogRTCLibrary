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

#include "MAX3133X.h"

#define BCD2BIN(val) (((val) & 15) + ((val) >> 4) * 10)
#define BIN2BCD(val) ((((val) / 10) << 4) + (val) % 10)
#define SWAPBYTES(val)  (((val & 0xFF) << 8) | ((val & 0xFF00) >> 8))

#define pr_err(msg) Serial.println("max3133x.cpp: " msg)

MAX3133X::MAX3133X(const reg_addr_t *reg_addr, TwoWire *i2c, uint8_t i2c_addr)
{
    if (i2c == NULL || reg_addr == NULL)
        pr_err("i2c object is invalid!");

    this->reg_addr = reg_addr;
    i2c_handler = i2c;
    slave_addr = i2c_addr;
}

int MAX3133X::begin(void)
{
    int ret;

    if (i2c_handler == NULL || reg_addr == NULL)
        return MAX3133X_NULL_VALUE_ERR;

    i2c_handler->begin();
    ret = sw_reset();
    if (ret != MAX3133X_NO_ERR)
        return ret;

    ret = interrupt_disable(INT_ALL);
    if (ret != MAX3133X_NO_ERR)
        return ret;

    return oscillator_enable();
}

int MAX3133X::read_register(uint8_t reg, uint8_t *value, uint8_t len)
{
    int counter = 0;

    if (value == NULL)
        return MAX3133X_NULL_VALUE_ERR;
    
    i2c_handler->beginTransmission(slave_addr);

    if (i2c_handler->write(reg) != 1) {
        i2c_handler->flush();
        return MAX3133X_WRITE_REG_ERR;
    }

    if (i2c_handler->endTransmission(false) != 0) {
        i2c_handler->flush();
        return MAX3133X_I2C_END_TRANS_ERR;
    }

    if (i2c_handler->requestFrom((uint8_t)slave_addr, len) != len) {
        i2c_handler->flush();
        return MAX3133X_I2C_BUFF_ERR;
    }

    while (i2c_handler->available()) { // slave may send less than requested
        value[counter++] = i2c_handler->read(); // receive a byte as character
    }

    if (counter != len) {
        i2c_handler->flush();
        return MAX3133X_READ_REG_ERR;
    }

    return MAX3133X_NO_ERR;
}

int MAX3133X::write_register(uint8_t reg, const uint8_t *value, uint8_t len)
{
    if (value == NULL) 
        return MAX3133X_NULL_VALUE_ERR;

    i2c_handler->beginTransmission(slave_addr);

    if (i2c_handler->write(reg) != 1) {
        i2c_handler->flush();
        return MAX3133X_WRITE_REG_ERR;
    }

    if (i2c_handler->write(value, len) != len) {
        i2c_handler->flush();
        return MAX3133X_WRITE_REG_ERR;
    }

    if (i2c_handler->endTransmission() != 0) {
        i2c_handler->flush();
        return MAX3133X_I2C_END_TRANS_ERR;
    }

    return MAX3133X_NO_ERR;
}

#define SET_BIT_FIELD(address, reg_name, bit_field_name, value)                 \
{   int ret;                                                                    \
    ret = read_register(address, (uint8_t *)&(reg_name), 1);                    \
    if (ret)                                                                    \
        return ret;                                                             \
    if (bit_field_name != value){                                               \
        bit_field_name = value;                                                 \
        ret = write_register(address, (uint8_t *)&(reg_name), 1);               \
        if (ret)                                                                \
            return ret;                                                         \
    }                                                                           \
}

inline int8_t MAX3133X::hours_reg_to_hour(const max3133x_hours_reg_t *hours_reg)
{
    hour_format_t format = hours_reg->bits_24hr.f_24_12 == 1 ? HOUR12 : HOUR24;
    if (format == HOUR24) {
        return BCD2BIN(hours_reg->bcd_24hr.value);
    } else {
        if (hours_reg->bits_12hr.am_pm) {
            if (hours_reg->bcd_12hr.value < 12)
                return (hours_reg->bcd_12hr.value + 12);
        } else {
            if (hours_reg->bcd_12hr.value == 12)
                return (hours_reg->bcd_12hr.value - 12);
        }
    }
}

inline void MAX3133X::rtc_regs_to_time(struct tm *time, const max3133x_rtc_time_regs_t *regs, uint16_t *sub_sec)
{
    if (sub_sec != NULL)
        *sub_sec = (1000 * regs->seconds_1_128_reg.raw) / 128.0;

    /* tm_sec seconds [0,61] */
    time->tm_sec = BCD2BIN(regs->seconds_reg.bcd.value);

    /* tm_min minutes [0,59] */
    time->tm_min = BCD2BIN(regs->minutes_reg.bcd.value);

    /* tm_hour hour [0,23] */
    time->tm_hour = hours_reg_to_hour(&regs->hours_reg);
    /*hour_format_t format = regs->hours_reg.bits_24hr.f_24_12 == 1 ? HOUR12 : HOUR24;
    if (format == HOUR24) {
        time->tm_hour = BCD2BIN(regs->hours_reg.bcd_24hr.value);
    } else if (format == HOUR12) {
        uint8_t hr24 = to_24hr(BCD2BIN(regs->hours_reg.bcd_12hr.value), regs->hours_reg.bits_12hr.am_pm);
        time->tm_hour = hr24;
    }*/

    /* tm_wday day of week [0,6] (Sunday = 0) */
    time->tm_wday = BCD2BIN(regs->day_reg.bcd.value) - 1;

    /* tm_mday day of month [1,31] */
    time->tm_mday = BCD2BIN(regs->date_reg.bcd.value);

    /* tm_mon month of year [0,11] */
    time->tm_mon = BCD2BIN(regs->month_reg.bcd.value) - 1;

    /* tm_year years since 2000 */
    if (regs->month_reg.bits.century)
        time->tm_year = BCD2BIN(regs->year_reg.bcd.value) + 200;
    else
        time->tm_year = BCD2BIN(regs->year_reg.bcd.value) + 100;

    /* tm_yday day of year [0,365] */
    time->tm_yday = 0; /* TODO */

    /* tm_isdst daylight savings flag */
    time->tm_isdst = 0; /* TODO */
}

int MAX3133X::time_to_rtc_regs(max3133x_rtc_time_regs_t *regs, const struct tm *time, hour_format_t format)
{
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
    regs->seconds_reg.bcd.value = BIN2BCD(time->tm_sec);

    regs->minutes_reg.bcd.value = BIN2BCD(time->tm_min);

    if (format == HOUR24) {
        regs->hours_reg.bcd_24hr.value = BIN2BCD(time->tm_hour);
        regs->hours_reg.bits_24hr.f_24_12 = HOUR24;
    } else if (format == HOUR12) {
        uint8_t hr_12, pm;
        to_12hr(time->tm_hour, &hr_12, &pm);
        regs->hours_reg.bcd_12hr.value = BIN2BCD(hr_12);
        regs->hours_reg.bits_12hr.f_24_12 = HOUR12;
        regs->hours_reg.bits_12hr.am_pm = pm;
    } else {
        pr_err("Invalid Hour Format!");
        return MAX3133X_INVALID_TIME_ERR;
    }

    regs->day_reg.bcd.value = BIN2BCD(time->tm_wday + 1);

    regs->date_reg.bcd.value = BIN2BCD(time->tm_mday);

    regs->month_reg.bcd.value = BIN2BCD(time->tm_mon + 1);

    if (time->tm_year >= 200) {
        regs->month_reg.bits.century = 1;
        regs->year_reg.bcd.value = BIN2BCD(time->tm_year - 200);
    } else if (time->tm_year >= 100) {
        regs->month_reg.bits.century = 0;
        regs->year_reg.bcd.value = BIN2BCD(time->tm_year - 100);
    } else {
        pr_err("Invalid set date!");
        return MAX3133X_INVALID_DATE_ERR;
    }

    return MAX3133X_NO_ERR;
}

int MAX3133X::get_time(struct tm *time, uint16_t *sub_sec)
{
    int ret;
    max3133x_rtc_time_regs_t max3133x_rtc_time_regs;
    if (time == NULL) {
        pr_err("time is invalid!");
        return MAX3133X_NULL_VALUE_ERR;
    }

    ret = read_register(reg_addr->seconds_1_128_reg_addr, 
                        (uint8_t *) &max3133x_rtc_time_regs.seconds_1_128_reg,
                        sizeof(max3133x_rtc_time_regs));
    if (ret != MAX3133X_NO_ERR) {
        pr_err("read time registers failed!");
        return ret;
    }

    rtc_regs_to_time(time, &max3133x_rtc_time_regs, sub_sec);
    return MAX3133X_NO_ERR;
}

int MAX3133X::set_time(const struct tm *time, hour_format_t format)
{
    int ret;
    max3133x_rtc_time_regs_t max3133x_rtc_time_regs;

    if (time == NULL) {
        pr_err("time is invalid!");
        return MAX3133X_NULL_VALUE_ERR;  
    }

    ret = time_to_rtc_regs(&max3133x_rtc_time_regs, time, format);
    if (ret != MAX3133X_NO_ERR)
        return ret;

    return write_register(reg_addr->seconds_reg_addr, &max3133x_rtc_time_regs.seconds_reg.raw, sizeof(max3133x_rtc_time_regs)-1);
}

inline void MAX3133X::timestamp_regs_to_time(timestamp_t *timestamp, const max3133x_ts_regs_t *timestamp_reg)
{
    /* tm_sec seconds [0,61] */
    timestamp->ctime.tm_sec = BCD2BIN(timestamp_reg->ts_sec_reg.bcd.value);

    /* tm_min minutes [0,59] */
    timestamp->ctime.tm_min = BCD2BIN(timestamp_reg->ts_min_reg.bcd.value);

    /* tm_hour hour [0,23] */
    timestamp->ctime.tm_hour = hours_reg_to_hour(&timestamp_reg->ts_hour_reg);
    /*hour_format_t format = timestamp_reg->ts_hour_reg.bits_24hr.f_24_12 ? HOUR12 : HOUR24;
    if (format == HOUR24) {
        timestamp->ctime.tm_hour = BCD2BIN(timestamp_reg->ts_hour_reg.bcd_24hr.value);
    } else if (format == HOUR12) {
        uint8_t hr24 = to_24hr(BCD2BIN(timestamp_reg->ts_hour_reg.bcd_12hr.value), timestamp_reg->ts_hour_reg.bits_12hr.am_pm);
        timestamp->ctime.tm_hour = hr24;
    }*/

    /* tm_mday day of month [1,31] */
    timestamp->ctime.tm_mday = BCD2BIN(timestamp_reg->ts_date_reg.bcd.value);

    /* tm_mon month of year [0,11] */
    timestamp->ctime.tm_mon = BCD2BIN(timestamp_reg->ts_month_reg.bcd.value) - 1;

    /* tm_year years since 2000 */
    if (timestamp_reg->ts_month_reg.bits.century)
        timestamp->ctime.tm_year = BCD2BIN(timestamp_reg->ts_year_reg.bcd.value) + 200;
    else
        timestamp->ctime.tm_year = BCD2BIN(timestamp_reg->ts_year_reg.bcd.value) + 100;

    /* tm_yday day of year [0,365] */
    timestamp->ctime.tm_yday = 0; /* TODO */

    /* tm_isdst daylight savings flag */
    timestamp->ctime.tm_isdst = 0; /* TODO */

    timestamp->sub_sec = (1000 * timestamp_reg->ts_sec_1_128_reg.raw) / 128.0;
}

int MAX3133X::get_status_reg(max3133x_status_reg_t * status_reg)
{
    return read_register(reg_addr->status_reg_addr, &status_reg->raw, 1);
}

int MAX3133X::get_interrupt_reg(max3133x_int_en_reg_t * int_en_reg)
{
    return read_register(reg_addr->int_en_reg_addr, &int_en_reg->raw, 1);
}

int MAX3133X::interrupt_enable(uint8_t mask)
{
    int ret;
    max3133x_int_en_reg_t int_en_reg;

    ret = read_register(reg_addr->int_en_reg_addr, &int_en_reg.raw, 1);
    if (ret != MAX3133X_NO_ERR)
        return ret;

    int_en_reg.raw |= mask;
    return write_register(reg_addr->int_en_reg_addr, &int_en_reg.raw, 1);
}

int MAX3133X::interrupt_disable(uint8_t mask)
{
    int ret;
    max3133x_int_en_reg_t int_en_reg;

    ret = read_register(reg_addr->int_en_reg_addr, &int_en_reg.raw, 1);
    if (ret != MAX3133X_NO_ERR)
        return ret;

    int_en_reg.raw &= ~mask;
    return write_register(reg_addr->int_en_reg_addr, &int_en_reg.raw, 1);
}

int MAX3133X::sw_reset_assert()
{
    max3133x_rtc_reset_reg_t rtc_reset_reg;
    SET_BIT_FIELD(reg_addr->rtc_reset_reg_addr, rtc_reset_reg, rtc_reset_reg.bits.swrst, 1);
    return MAX3133X_NO_ERR;
}

int MAX3133X::sw_reset_release()
{
    max3133x_rtc_reset_reg_t rtc_reset_reg;
    SET_BIT_FIELD(reg_addr->rtc_reset_reg_addr, rtc_reset_reg, rtc_reset_reg.bits.swrst, 0);
    return MAX3133X_NO_ERR;
}

int MAX3133X::sw_reset()
{
    int ret;
    ret = sw_reset_assert();
    if (ret != MAX3133X_NO_ERR)
        return ret;

    delay(500);
    return sw_reset_release();
}

int MAX31331::rtc_config(rtc_config_t *max31331_config)
{
    int ret;
    max3133x_rtc_config1_reg_t rtc_config1_reg;
    max31331_rtc_config2_reg_t rtc_config2_reg;

    rtc_config1_reg.bits.a1ac           = max31331_config->a1ac;
    rtc_config1_reg.bits.dip            = max31331_config->dip;
    rtc_config1_reg.bits.data_ret       = max31331_config->data_ret;
    rtc_config1_reg.bits.i2c_timeout    = max31331_config->i2c_timeout;
    rtc_config1_reg.bits.en_osc         = max31331_config->en_osc;

    ret =  write_register(reg_addr.rtc_config1_reg_addr, &rtc_config1_reg.raw, 1);
    if (ret != MAX3133X_NO_ERR)
        return ret;

    rtc_config2_reg.bits.clko_hz    = max31331_config->clko_hz;
    rtc_config2_reg.bits.enclko     = max31331_config->enclko;

    return write_register(reg_addr.rtc_config2_reg_addr, &rtc_config2_reg.raw, 1);
}

int MAX31334::rtc_config(rtc_config_t *max31334_config)
{
    int ret;
    max3133x_rtc_config1_reg_t rtc_config1_reg;
    max31334_rtc_config2_reg_t rtc_config2_reg;

    rtc_config1_reg.bits.a1ac           = max31334_config->a1ac;
    rtc_config1_reg.bits.dip            = max31334_config->dip;
    rtc_config1_reg.bits.data_ret       = max31334_config->data_ret;
    rtc_config1_reg.bits.i2c_timeout    = max31334_config->i2c_timeout;
    rtc_config1_reg.bits.en_osc         = max31334_config->en_osc;

    ret =  write_register(reg_addr.rtc_config1_reg_addr, &rtc_config1_reg.raw, 1);
    if (ret != MAX3133X_NO_ERR)
        return ret;

    rtc_config2_reg.bits.clko_hz    = max31334_config->clko_hz;
    rtc_config2_reg.bits.enclko     = max31334_config->enclko;
    rtc_config2_reg.bits.ddb        = max31334_config->ddb;
    rtc_config2_reg.bits.dse        = max31334_config->dse;

    return write_register(reg_addr.rtc_config2_reg_addr, &rtc_config2_reg.raw, 1);
}

int MAX31331::get_rtc_config(rtc_config_t *max31331_config)
{
    int ret;
    max3133x_rtc_config1_reg_t rtc_config1_reg;
    max31331_rtc_config2_reg_t rtc_config2_reg;

    ret = read_register(reg_addr.rtc_config1_reg_addr, &rtc_config1_reg.raw, 1);
    if (ret != MAX3133X_NO_ERR)
        return ret;

    max31331_config->a1ac           = (a1ac_t)rtc_config1_reg.bits.a1ac;
    max31331_config->dip            = (dip_t)rtc_config1_reg.bits.dip;
    max31331_config->data_ret       = (data_ret_t)rtc_config1_reg.bits.data_ret;
    max31331_config->i2c_timeout    = (i2c_timeout_t)rtc_config1_reg.bits.i2c_timeout;
    max31331_config->en_osc         = (en_osc_t)rtc_config1_reg.bits.en_osc;

    ret = read_register(reg_addr.rtc_config2_reg_addr, &rtc_config2_reg.raw, 1);
    if (ret != MAX3133X_NO_ERR)
        return ret;

    max31331_config->clko_hz        = (clko_hz_t)rtc_config2_reg.bits.clko_hz;
    max31331_config->enclko         = (enclko_t)rtc_config2_reg.bits.enclko;
    return MAX3133X_NO_ERR;
}

int MAX31334::get_rtc_config(rtc_config_t *max31334_config)
{
    int ret;
    max3133x_rtc_config1_reg_t rtc_config1_reg;
    max31334_rtc_config2_reg_t rtc_config2_reg;

    ret =  read_register(reg_addr.rtc_config1_reg_addr, &rtc_config1_reg.raw, 1);
    if (ret != MAX3133X_NO_ERR)
        return ret;

    max31334_config->a1ac           = (a1ac_t)rtc_config1_reg.bits.a1ac;
    max31334_config->dip            = (dip_t)rtc_config1_reg.bits.dip;
    max31334_config->data_ret       = (data_ret_t)rtc_config1_reg.bits.data_ret;
    max31334_config->i2c_timeout    = (i2c_timeout_t)rtc_config1_reg.bits.i2c_timeout;
    max31334_config->en_osc         = (en_osc_t)rtc_config1_reg.bits.en_osc;

    ret = read_register(reg_addr.rtc_config2_reg_addr, &rtc_config2_reg.raw, 1);
    if (ret != MAX3133X_NO_ERR)
        return ret;

    max31334_config->clko_hz        = (clko_hz_t)rtc_config2_reg.bits.clko_hz;
    max31334_config->ddb            = (ddb_t)rtc_config2_reg.bits.ddb;
    max31334_config->dse            = (dse_t)rtc_config2_reg.bits.dse;
    max31334_config->enclko         = (enclko_t)rtc_config2_reg.bits.enclko;

    return MAX3133X_NO_ERR;
}

int MAX3133X::set_alarm1_auto_clear(a1ac_t a1ac)
{
    max3133x_rtc_config1_reg_t rtc_config1_reg;
    SET_BIT_FIELD(reg_addr->rtc_config1_reg_addr, rtc_config1_reg, rtc_config1_reg.bits.a1ac, a1ac);
    return MAX3133X_NO_ERR;
}

int MAX3133X::set_din_polarity(dip_t dip)
{
    max3133x_rtc_config1_reg_t rtc_config1_reg;
    SET_BIT_FIELD(reg_addr->rtc_config1_reg_addr, rtc_config1_reg, rtc_config1_reg.bits.dip, dip);
    return MAX3133X_NO_ERR;
}

int MAX3133X::data_retention_mode_config(bool enable)
{
    max3133x_rtc_config1_reg_t rtc_config1_reg;
    SET_BIT_FIELD(reg_addr->rtc_config1_reg_addr, rtc_config1_reg, rtc_config1_reg.bits.data_ret, enable);
    return MAX3133X_NO_ERR;
}

int MAX3133X::data_retention_mode_enter()
{
    return data_retention_mode_config(1);
}

int MAX3133X::data_retention_mode_exit()
{
    return data_retention_mode_config(0);
}

int MAX3133X::i2c_timeout_config(bool enable)
{
    max3133x_rtc_config1_reg_t rtc_config1_reg;
    SET_BIT_FIELD(reg_addr->rtc_config1_reg_addr, rtc_config1_reg, rtc_config1_reg.bits.i2c_timeout, enable);
    return MAX3133X_NO_ERR;
}

int MAX3133X::i2c_timeout_enable()
{
    return i2c_timeout_config(1);
}

int MAX3133X::i2c_timeout_disable()
{
    return i2c_timeout_config(0);
}

int MAX3133X::oscillator_config(bool enable)
{
    max3133x_rtc_config1_reg_t rtc_config1_reg;
    SET_BIT_FIELD(reg_addr->rtc_config1_reg_addr, rtc_config1_reg, rtc_config1_reg.bits.en_osc, enable);
    return MAX3133X_NO_ERR;
}

int MAX3133X::oscillator_enable()
{
    return oscillator_config(1);
}

int MAX3133X::oscillator_disable()
{
    return oscillator_config(0);
}

int MAX31334::get_sleep_state()
{
    int ret;
    max31334_rtc_config2_reg_t rtc_config2_reg;

    ret = read_register(reg_addr.rtc_config2_reg_addr, &rtc_config2_reg.raw, 1);
    if (ret != MAX3133X_NO_ERR)
        return ret;

    return rtc_config2_reg.bits.slst;
}

int MAX31334::din_sleep_entry_config(bool enable)
{
    max31334_rtc_config2_reg_t rtc_config2_reg;
    SET_BIT_FIELD(reg_addr.rtc_config2_reg_addr, rtc_config2_reg, rtc_config2_reg.bits.dse, enable);
    return MAX3133X_NO_ERR;
}

int MAX31334::din_sleep_entry_enable()
{
    return din_sleep_entry_config(1);
}

int MAX31334::din_sleep_entry_disable()
{
    return din_sleep_entry_config(0);
}

int MAX31334::din_pin_debounce_config(bool enable)
{
    max31334_rtc_config2_reg_t rtc_config2_reg;
    SET_BIT_FIELD(reg_addr.rtc_config2_reg_addr, rtc_config2_reg, rtc_config2_reg.bits.ddb, enable);
    return MAX3133X_NO_ERR;
}

int MAX31334::din_pin_debounce_enable()
{
    return din_pin_debounce_config(1);
}

int MAX31334::din_pin_debounce_disable()
{
    return din_pin_debounce_config(0);
}

int MAX3133X::clkout_config(bool enable)
{
    max31334_rtc_config2_reg_t rtc_config2_reg;
    SET_BIT_FIELD(reg_addr->rtc_config2_reg_addr, rtc_config2_reg, rtc_config2_reg.bits.enclko, enable);
    return MAX3133X_NO_ERR;
}

int MAX3133X::clkout_enable()
{
    return clkout_config(1);
}

int MAX3133X::clkout_disable()
{
    return clkout_config(0);
}

int MAX3133X::set_clko_freq(clko_hz_t clko_hz)
{
    max31334_rtc_config2_reg_t rtc_config2_reg;
    SET_BIT_FIELD(reg_addr->rtc_config2_reg_addr, rtc_config2_reg, rtc_config2_reg.bits.clko_hz, clko_hz);
    return MAX3133X_NO_ERR;
}

int MAX3133X::get_clko_freq(clko_hz_t *clko_hz)
{
    int ret;
    max31334_rtc_config2_reg_t rtc_config2_reg;

    ret = read_register(reg_addr->rtc_config2_reg_addr, &rtc_config2_reg.raw, 1);
    if (ret != MAX3133X_NO_ERR)
        return ret;

    *clko_hz = (clko_hz_t)rtc_config2_reg.bits.clko_hz;
    return MAX3133X_NO_ERR;
}

int MAX3133X::timestamp_function_enable()
{
    max3133x_timestamp_config_reg_t timestamp_config_reg;
    SET_BIT_FIELD(reg_addr->timestamp_config_reg_addr, timestamp_config_reg, timestamp_config_reg.bits.tse, 1);
    return MAX3133X_NO_ERR;
}

int MAX3133X::timestamp_function_disable()
{
    max3133x_timestamp_config_reg_t timestamp_config_reg;
    SET_BIT_FIELD(reg_addr->timestamp_config_reg_addr, timestamp_config_reg, timestamp_config_reg.bits.tse, 0);
    return MAX3133X_NO_ERR;
}

int MAX3133X::timestamp_registers_reset()
{
    max3133x_timestamp_config_reg_t timestamp_config_reg;
    SET_BIT_FIELD(reg_addr->timestamp_config_reg_addr, timestamp_config_reg, timestamp_config_reg.bits.tsr, 1);
    return MAX3133X_NO_ERR;
}

int MAX3133X::timestamp_overwrite_config(bool enable)
{
    max3133x_timestamp_config_reg_t timestamp_config_reg;
    SET_BIT_FIELD(reg_addr->timestamp_config_reg_addr, timestamp_config_reg, timestamp_config_reg.bits.tsow, enable);
    return MAX3133X_NO_ERR;
}

int MAX3133X::timestamp_overwrite_enable()
{
    return timestamp_overwrite_config(1);
}

int MAX3133X::timestamp_overwrite_disable()
{
    return timestamp_overwrite_config(0);
}

int MAX3133X::timestamp_record_enable(uint8_t record_enable_mask)
{
    int ret;
    max3133x_timestamp_config_reg_t timestamp_config_reg;

    if (record_enable_mask > (TSVLOW | TSPWM | TSDIN))
        return MAX3133X_INVALID_MASK_ERR;

    ret = read_register(reg_addr->timestamp_config_reg_addr, &timestamp_config_reg.raw, 1);
    if (ret != MAX3133X_NO_ERR)
        return ret;

    timestamp_config_reg.raw |= record_enable_mask;
    return write_register(reg_addr->timestamp_config_reg_addr, &timestamp_config_reg.raw, 1);
}

int MAX3133X::timestamp_record_disable(uint8_t record_disable_mask)
{
    int ret;
    max3133x_timestamp_config_reg_t timestamp_config_reg;

    if (record_disable_mask > (TSVLOW | TSPWM | TSDIN))
        return MAX3133X_INVALID_MASK_ERR;

    ret = read_register(reg_addr->timestamp_config_reg_addr, &timestamp_config_reg.raw, 1);
    if (ret != MAX3133X_NO_ERR)
        return ret;

    timestamp_config_reg.raw &= ~record_disable_mask;
    return write_register(reg_addr->timestamp_config_reg_addr, &timestamp_config_reg.raw, 1);
}

int MAX31331::timer_init(uint8_t timer_init, bool repeat, timer_freq_t freq)
{
    int ret;
    max3133x_timer_config_reg_t timer_config_reg;

    ret = read_register(reg_addr.timer_config_reg_addr, &timer_config_reg.raw, 1);
    if (ret != MAX3133X_NO_ERR)
        return ret;

    timer_config_reg.bits.te = 0;                   /* timer is reset */
    timer_config_reg.bits.tpause = 1;               /* timer is paused */
    timer_config_reg.bits.trpt = repeat ? 1 : 0;    /* Timer repeat mode */
    timer_config_reg.bits.tfs = freq;               /* Timer frequency */

    ret = write_register(reg_addr.timer_config_reg_addr, &timer_config_reg.raw, 1);
    if (ret != MAX3133X_NO_ERR)
        return ret;

    return write_register(reg_addr.timer_init_reg_addr, &timer_init, 1);
}

int MAX31334::timer_init(uint16_t timer_init, bool repeat, timer_freq_t freq)
{
    int ret;
    max3133x_timer_config_reg_t timer_config_reg;

    ret = read_register(reg_addr.timer_config_reg_addr, &timer_config_reg.raw, 1);
    if (ret != MAX3133X_NO_ERR)
        return ret;

    timer_config_reg.bits.te = 0;                   /* timer is reset */
    timer_config_reg.bits.tpause = 1;               /* timer is paused */
    timer_config_reg.bits.trpt = repeat ? 1 : 0;    /* Timer repeat mode */
    timer_config_reg.bits.tfs = freq;               /* Timer frequency */

    ret = write_register(reg_addr.timer_config_reg_addr, &timer_config_reg.raw, 1);
    if (ret != MAX3133X_NO_ERR)
        return ret;

    timer_init = SWAPBYTES(timer_init);

    return write_register(reg_addr.timer_init2_reg_addr, (uint8_t *)&timer_init, 2);
}

int MAX31331::timer_get()
{
    int ret;
    uint8_t timer_count;

    ret = read_register(reg_addr.timer_count_reg_addr, &timer_count, 1);
    if (ret != MAX3133X_NO_ERR)
        return ret;

    return timer_count;
}

int MAX31334::timer_get()
{
    int ret;
    uint16_t timer_count;

    ret = read_register(reg_addr.timer_count2_reg_addr, (uint8_t *)&timer_count, 2);
    if (ret != MAX3133X_NO_ERR)
        return ret;

    return SWAPBYTES(timer_count);
}

int MAX3133X::timer_start()
{
    int ret;
    max3133x_timer_config_reg_t timer_config_reg;

    ret = read_register(reg_addr->timer_config_reg_addr, &timer_config_reg.raw, 1);
    if (ret != MAX3133X_NO_ERR)
        return ret;

    timer_config_reg.bits.te        = 1;
    timer_config_reg.bits.tpause    = 0;

    return write_register(reg_addr->timer_config_reg_addr, &timer_config_reg.raw, 1);
}

int MAX3133X::timer_pause()
{
    int ret;
    max3133x_timer_config_reg_t timer_config_reg;

    ret = read_register(reg_addr->timer_config_reg_addr, &timer_config_reg.raw, 1);
    if (ret != MAX3133X_NO_ERR)
        return ret;

    timer_config_reg.bits.te        = 1;
    timer_config_reg.bits.tpause    = 1;

    return write_register(reg_addr->timer_config_reg_addr, &timer_config_reg.raw, 1);
}

int MAX3133X::timer_continue()
{
    int ret;
    max3133x_timer_config_reg_t timer_config_reg;

    ret = read_register(reg_addr->timer_config_reg_addr, &timer_config_reg.raw, 1);
    if (ret != MAX3133X_NO_ERR)
        return ret;

    timer_config_reg.bits.te        = 1;
    timer_config_reg.bits.tpause    = 0;

    return write_register(reg_addr->timer_config_reg_addr, &timer_config_reg.raw, 1);
}

int MAX3133X::timer_stop()
{
    int ret;
    max3133x_timer_config_reg_t timer_config_reg;

    ret = read_register(reg_addr->timer_config_reg_addr, &timer_config_reg.raw, 1);
    if (ret != MAX3133X_NO_ERR)
        return ret;

    timer_config_reg.bits.te        = 0;
    timer_config_reg.bits.tpause    = 1;

    return write_register(reg_addr->timer_config_reg_addr, &timer_config_reg.raw, 1);
}

int MAX31334::sleep_enter()
{
    max31334_sleep_config_reg_t sleep_config_reg;
    SET_BIT_FIELD(reg_addr.sleep_config_reg_addr, sleep_config_reg, sleep_config_reg.bits.slp, 1);
    return MAX3133X_NO_ERR;
}

int MAX31334::sleep_exit()
{
    max31334_sleep_config_reg_t sleep_config_reg;
    SET_BIT_FIELD(reg_addr.sleep_config_reg_addr, sleep_config_reg, sleep_config_reg.bits.slp, 0);
    return MAX3133X_NO_ERR;
}

int MAX31334::set_wait_state_timeout(wsto_t wsto)
{
    max31334_sleep_config_reg_t sleep_config_reg;
    SET_BIT_FIELD(reg_addr.sleep_config_reg_addr, sleep_config_reg, sleep_config_reg.bits.wsto, wsto);
    return MAX3133X_NO_ERR;
}

int MAX31334::get_wait_state_timeout(wsto_t* wsto)
{
    int ret;
    max31334_sleep_config_reg_t sleep_config_reg;

    ret = read_register(reg_addr.sleep_config_reg_addr, &sleep_config_reg.raw, 1);
    if (ret != MAX3133X_NO_ERR)
        return ret;

    *wsto = (wsto_t)sleep_config_reg.bits.wsto;
    return MAX3133X_NO_ERR;
}

int MAX31334::wakeup_enable(uint8_t wakeup_enable_mask)
{
    int ret;
    max31334_sleep_config_reg_t sleep_config_reg;

    ret = read_register(reg_addr.sleep_config_reg_addr, &sleep_config_reg.raw, 1);
    if (ret != MAX3133X_NO_ERR)
        return ret;

    sleep_config_reg.raw |= wakeup_enable_mask;
    return write_register(reg_addr.sleep_config_reg_addr, &sleep_config_reg.raw, 1);
}

int MAX31334::wakeup_disable(uint8_t wakeup_disable_mask)
{
    int ret;
    max31334_sleep_config_reg_t sleep_config_reg;

    ret = read_register(reg_addr.sleep_config_reg_addr, &sleep_config_reg.raw, 1);
    if (ret != MAX3133X_NO_ERR)
        return ret;

    sleep_config_reg.raw &= ~wakeup_disable_mask;
    return write_register(reg_addr.sleep_config_reg_addr, &sleep_config_reg.raw, 1);
}

int MAX3133X::battery_voltage_detector_config(bool enable)
{
    max3133x_pwr_mgmt_reg_t pwr_mgmt_reg;
    SET_BIT_FIELD(reg_addr->pwr_mgmt_reg_addr, pwr_mgmt_reg, pwr_mgmt_reg.bits.en_vbat_detect, enable);
    return MAX3133X_NO_ERR;
}

int MAX3133X::battery_voltage_detector_enable()
{
    return battery_voltage_detector_config(1);
}

int MAX3133X::battery_voltage_detector_disable()
{
    return battery_voltage_detector_config(0);
}

int MAX3133X::supply_select(power_mgmt_supply_t supply)
{
    int ret;
    max3133x_pwr_mgmt_reg_t pwr_mgmt_reg;

    ret = read_register(reg_addr->pwr_mgmt_reg_addr, &pwr_mgmt_reg.raw, 1);
    if (ret != MAX3133X_NO_ERR)
        return ret;

    switch (supply) {
        case POW_MGMT_SUPPLY_SEL_VCC:
            pwr_mgmt_reg.bits.manual_sel    = 1;
            pwr_mgmt_reg.bits.vback_sel     = 0;
            break;
        case POW_MGMT_SUPPLY_SEL_VBAT:
            pwr_mgmt_reg.bits.manual_sel    = 1;
            pwr_mgmt_reg.bits.vback_sel     = 1;
            break;
        case POW_MGMT_SUPPLY_SEL_AUTO:
        default:
            pwr_mgmt_reg.bits.manual_sel    = 0;
            break;
    }

    return write_register(reg_addr->pwr_mgmt_reg_addr, &pwr_mgmt_reg.raw, 1);
}

int MAX3133X::trickle_charger_enable(trickle_charger_ohm_t res, bool diode)
{
    max3133x_trickle_reg_reg_t trickle_reg_reg;
    trickle_reg_reg.bits.trickle = res;

    if (diode)
        trickle_reg_reg.bits.trickle |= 0x04;

    trickle_reg_reg.bits.en_trickle = true;

    return write_register(reg_addr->trickle_reg_addr, &trickle_reg_reg.raw, 1);
}

int MAX3133X::trickle_charger_disable()
{
    max3133x_trickle_reg_reg_t trickle_reg_reg;
    SET_BIT_FIELD(reg_addr->trickle_reg_addr, trickle_reg_reg, trickle_reg_reg.bits.en_trickle, 0);
    return MAX3133X_NO_ERR;
}

int MAX3133X::get_timestamp(int ts_num, timestamp_t *timestamp)
{
    int ret;
    max3133x_ts_regs_t timestamp_reg;
    max3133x_ts_flags_reg_t ts_flags_reg;
    uint8_t ts_reg_addr;
    uint8_t ts_flag_reg_addr = reg_addr->ts0_flags_reg_addr + sizeof(max3133x_ts_regs_t)*ts_num;

    ret = read_register(ts_flag_reg_addr, (uint8_t *)&ts_flags_reg, 1);
    if (ret != MAX3133X_NO_ERR)
        return ret;

    timestamp->ts_num       = (ts_num_t)ts_num;
    timestamp->ts_trigger   = (ts_trigger_t)(ts_flags_reg.raw & 0xF);

    if (ts_flags_reg.raw == NOT_TRIGGERED)
        return ret;

    ts_reg_addr = reg_addr->ts0_sec_1_128_reg_addr + sizeof(max3133x_ts_regs_t)*ts_num;
    ret = read_register(ts_reg_addr, (uint8_t *)&timestamp_reg, sizeof(max3133x_ts_regs_t)-1);
    if (ret != MAX3133X_NO_ERR)
        return ret;
    
    timestamp_regs_to_time(timestamp, &timestamp_reg);
    return MAX3133X_NO_ERR;
}

int MAX3133X::offset_configuration(int meas)
{
    short int offset;
    double acc = (meas - 32768)*30.5175;

    offset = (short int)(acc/0.477);

    return write_register(reg_addr->offset_high_reg_addr, (uint8_t *)&offset, 2);
}

int MAX3133X::oscillator_flag_config(bool enable)
{
    max3133x_int_en_reg_t int_en_reg;
    SET_BIT_FIELD(reg_addr->int_en_reg_addr, int_en_reg, int_en_reg.bits.dosf, !enable);

    return MAX3133X_NO_ERR;
}

inline void MAX3133X::to_12hr(uint8_t hr, uint8_t *hr_12, uint8_t *pm) {
    if (hr == 0) {
        *hr_12 = 12;
        *pm = 0;
    } else if (hr < 12) {
        *hr_12 = hr;
        *pm = 0;
    } else if (hr == 12) {
        *hr_12 = 12;
        *pm = 1;
    } else {
        *hr_12 = hr - 12;
        *pm = 1;
    }
}

int MAX3133X::set_alarm_period(alarm_no_t alarm_no, max3133x_alarm_regs_t &regs, alarm_period_t period)
{
    regs.sec.bits.am1       = 1;
    regs.min.bits.am2       = 1;
    regs.hrs.bits_24hr.am3  = 1;
    regs.day_date.bits.am4  = 1;
    regs.mon.bits.am5       = 1;
    regs.mon.bits.am6       = 1;
    regs.day_date.bits.dy_dt_match  = 1;

    switch (period) {
        case ALARM_PERIOD_ONETIME:
            if (alarm_no == ALARM2) {
                pr_err("Alarm2 does not support onetime alarm");
                return MAX3133X_ALARM_ONETIME_NOT_SUPP_ERR;
            }
            regs.mon.bits.am6 = 0;
        case ALARM_PERIOD_YEARLY:
            if (alarm_no == ALARM2) {
                pr_err("Alarm2 does not support once per year alarm");
                return MAX3133X_ALARM_YEARLY_NOT_SUPP_ERR;
            }
            regs.mon.bits.am5 = 0;
        case ALARM_PERIOD_MONTHLY:
            regs.day_date.bits.dy_dt_match = 0;
        case ALARM_PERIOD_WEEKLY:
            regs.day_date.bits.am4 = 0;
        case ALARM_PERIOD_DAILY:
            regs.hrs.bits_24hr.am3 = 0;
        case ALARM_PERIOD_HOURLY:
            regs.min.bits.am2 = 0;
        case ALARM_PERIOD_EVERYMINUTE:
            if ((alarm_no == ALARM2) && (period == ALARM_PERIOD_EVERYMINUTE)) {
                pr_err("Alarm2 does not support every minute alarm");
                return MAX3133X_ALARM_EVERYMINUTE_NOT_SUPP_ERR;
            }
            regs.sec.bits.am1 = 0;
        case ALARM_PERIOD_EVERYSECOND:
            if ((alarm_no == ALARM2) && (period == ALARM_PERIOD_EVERYSECOND)) {
                pr_err("Alarm2 does not support once per second alarm");
                return MAX3133X_ALARM_EVERYSECOND_NOT_SUPP_ERR;
            }
            break;
        default:
            pr_err("Invalid alarm period");
            return MAX3133X_INVALID_ALARM_PERIOD_ERR;
    }
    return MAX3133X_NO_ERR;
}

int MAX3133X::time_to_alarm_regs(max3133x_alarm_regs_t &regs, const struct tm *alarm_time, hour_format_t format)
{
    regs.sec.bcd.value = BIN2BCD(alarm_time->tm_sec);
    regs.min.bcd.value = BIN2BCD(alarm_time->tm_min);

    if (format == HOUR24) {
        regs.hrs.bcd_24hr.value = BIN2BCD(alarm_time->tm_hour);
    } else if (format == HOUR12) {
        uint8_t hr_12, pm;
        to_12hr(alarm_time->tm_hour, &hr_12, &pm);
        regs.hrs.bcd_12hr.value = BIN2BCD(hr_12);
        regs.hrs.bits_12hr.am_pm = pm;
    } else {
        pr_err("Invalid Hour Format!");
        return MAX3133X_INVALID_TIME_ERR;
    }

    if (regs.day_date.bits.dy_dt_match == 0) /* Date match */
        regs.day_date.bcd_date.value = BIN2BCD(alarm_time->tm_mday);
    else /* Day match */
        regs.day_date.bcd_day.value = BIN2BCD(alarm_time->tm_wday);

    regs.mon.bcd.value = BIN2BCD(alarm_time->tm_mon + 1);

    if (alarm_time->tm_year >= 200) {
        regs.year.bcd.value = BIN2BCD(alarm_time->tm_year - 200);
    } else if (alarm_time->tm_year >= 100) {
        regs.year.bcd.value = BIN2BCD(alarm_time->tm_year - 100);
    } else {
        pr_err("Invalid set year!");
        return MAX3133X_INVALID_DATE_ERR;
    }

    return MAX3133X_NO_ERR;
}

int MAX3133X::set_alarm_regs(alarm_no_t alarm_no, const max3133x_alarm_regs_t *regs)
{
    uint8_t len = sizeof(max3133x_alarm_regs_t);

    if (alarm_no == ALARM1)
        return write_register(reg_addr->alm1_sec_reg_addr, &regs->sec.raw, len);
    else
        return write_register(reg_addr->alm2_min_reg_addr, &regs->min.raw, len-3);
}

int MAX3133X::get_rtc_time_format(hour_format_t *format)
{
    int ret;
    max3133x_hours_reg_t hours_reg;
    ret = read_register(reg_addr->hours_reg_addr, (uint8_t *)&hours_reg.raw, 1);
    if (ret != MAX3133X_NO_ERR)
        return ret;

    *format = (hour_format_t)hours_reg.bits_24hr.f_24_12;

    return MAX3133X_NO_ERR;
}

int MAX3133X::set_alarm(alarm_no_t alarm_no, const struct tm *alarm_time, alarm_period_t period)
{
    int ret;
    max3133x_alarm_regs_t alarm_regs;
    hour_format_t format;

    ret = set_alarm_period(alarm_no, alarm_regs, period);
    if (ret != MAX3133X_NO_ERR)
        return ret;

    ret = get_rtc_time_format(&format);
    if (ret != MAX3133X_NO_ERR)
        return ret;

    /* Convert time structure to alarm registers */
    ret = time_to_alarm_regs(alarm_regs, alarm_time, format);
    if (ret != MAX3133X_NO_ERR)
        return ret;

    return set_alarm_regs(alarm_no, &alarm_regs);
}

inline void MAX3133X::alarm_regs_to_time(alarm_no_t alarm_no, struct tm *alarm_time, 
                                 const max3133x_alarm_regs_t *regs, hour_format_t format)
{
    alarm_time->tm_min = BCD2BIN(regs->min.bcd.value);

    if (format == HOUR24) {
        alarm_time->tm_hour = BCD2BIN(regs->hrs.bcd_24hr.value);
    } else if (format == HOUR12) {
        if (regs->hrs.bits_12hr.am_pm) {
            if (BCD2BIN(regs->hrs.bcd_12hr.value) < 12)
                alarm_time->tm_hour = BCD2BIN(regs->hrs.bcd_12hr.value) + 12;
        } else {
            if (BCD2BIN(regs->hrs.bcd_12hr.value) == 12)
                alarm_time->tm_hour = BCD2BIN(regs->hrs.bcd_12hr.value) - 12;
        }
    }

    if (regs->day_date.bits.dy_dt_match == 0) { /* date */
        alarm_time->tm_mday = BCD2BIN(regs->day_date.bcd_date.value);
        alarm_time->tm_wday = 0;
    } else { /* day */
        alarm_time->tm_wday = BCD2BIN(regs->day_date.bcd_day.value);
        alarm_time->tm_mday = 0;
    }

    if (alarm_no == ALARM1) {
        alarm_time->tm_sec = BCD2BIN(regs->sec.bcd.value);
        alarm_time->tm_mon = BCD2BIN(regs->mon.bcd.value) - 1;
        alarm_time->tm_year = BCD2BIN(regs->year.bcd.value) + 100;  /* XXX no century bit */
    }
}

int MAX3133X::get_alarm(alarm_no_t alarm_no, struct tm *alarm_time, 
                        alarm_period_t *period, bool *is_enabled)
{
    int ret;
    max3133x_alarm_regs_t alarm_regs;
    max3133x_int_en_reg_t int_en_reg;
    uint8_t len = sizeof(max3133x_alarm_regs_t);
    hour_format_t format;

    ret = get_rtc_time_format(&format);
    if (ret != MAX3133X_NO_ERR)
        return ret;

    if (alarm_no == ALARM1)
        ret = read_register(reg_addr->alm1_sec_reg_addr, &alarm_regs.sec.raw, len);
    else
        ret = read_register(reg_addr->alm2_min_reg_addr, &alarm_regs.min.raw, len-3);

    if (ret != MAX3133X_NO_ERR)
        return ret;

    /* Convert alarm registers to time structure */
    alarm_regs_to_time(alarm_no, alarm_time, &alarm_regs, format);

    *period = ALARM_PERIOD_EVERYSECOND;

    if (alarm_no == ALARM1) {
        if (alarm_regs.sec.bits.am1 == 0) 
            *period = ALARM_PERIOD_EVERYMINUTE;
    }
        
    if (alarm_regs.min.bits.am2 == 0)
        *period = ALARM_PERIOD_HOURLY;
    if (alarm_regs.hrs.bits_24hr.am3 == 0)
        *period = ALARM_PERIOD_DAILY;
    if (alarm_regs.day_date.bits.am4 == 0)
        *period = ALARM_PERIOD_WEEKLY;
    if (alarm_regs.day_date.bits.dy_dt_match == 0)
        *period = ALARM_PERIOD_MONTHLY;
    if (alarm_no == ALARM1) {
        if (alarm_regs.mon.bits.am5 == 0) *period = ALARM_PERIOD_YEARLY;
        if (alarm_regs.mon.bits.am6 == 0) *period = ALARM_PERIOD_ONETIME;
    }

    ret = read_register(reg_addr->int_en_reg_addr, (uint8_t *)&int_en_reg.raw, 1);
    if (ret != MAX3133X_NO_ERR)
        return ret;

    if (alarm_no == ALARM1)
        *is_enabled = (int_en_reg.raw & (A1IE) == A1IE);
    else
        *is_enabled = (int_en_reg.raw & (A2IE) == A2IE);

    return MAX3133X_NO_ERR;
}

const MAX3133X::reg_addr_t MAX31334::reg_addr = {
MAX31334_STATUS,
MAX31334_INT_EN,
MAX31334_RTC_RESET,
MAX31334_RTC_CONFIG1,
MAX31334_RTC_CONFIG2,
MAX31334_TIMESTAMP_CONFIG,
MAX31334_TIMER_CONFIG,
MAX31334_SLEEP_CONFIG,
MAX31334_SECONDS_1_128,
MAX31334_SECONDS,
MAX31334_MINUTES,
MAX31334_HOURS,
MAX31334_DAY,
MAX31334_DATE,
MAX31334_MONTH,
MAX31334_YEAR,
MAX31334_ALM1_SEC,
MAX31334_ALM1_MIN,
MAX31334_ALM1_HRS,
MAX31334_ALM1_DAY_DATE,
MAX31334_ALM1_MON,
MAX31334_ALM1_YEAR,
MAX31334_ALM2_MIN,
MAX31334_ALM2_HRS,
MAX31334_ALM2_DAY_DATE,
REG_NOT_AVAILABLE,
MAX31334_TIMER_COUNT2,
MAX31334_TIMER_COUNT1,
REG_NOT_AVAILABLE,
MAX31334_TIMER_INIT2,
MAX31334_TIMER_INIT1,
MAX31334_PWR_MGMT,
MAX31334_TRICKLE_REG,
MAX31334_OFFSET_HIGH,
MAX31334_OFFSET_LOW,
MAX31334_TS0_SEC_1_128,
MAX31334_TS0_SEC,
MAX31334_TS0_MIN,
MAX31334_TS0_HOUR,
MAX31334_TS0_DATE,
MAX31334_TS0_MONTH,
MAX31334_TS0_YEAR,
MAX31334_TS0_FLAGS,
MAX31334_TS1_SEC_1_128,
MAX31334_TS1_SEC,
MAX31334_TS1_MIN,
MAX31334_TS1_HOUR,
MAX31334_TS1_DATE,
MAX31334_TS1_MONTH,
MAX31334_TS1_YEAR,
MAX31334_TS1_FLAGS,
MAX31334_TS2_SEC_1_128,
MAX31334_TS2_SEC,
MAX31334_TS2_MIN,
MAX31334_TS2_HOUR,
MAX31334_TS2_DATE,
MAX31334_TS2_MONTH,
MAX31334_TS2_YEAR,
MAX31334_TS2_FLAGS,
MAX31334_TS3_SEC_1_128,
MAX31334_TS3_SEC,
MAX31334_TS3_MIN,
MAX31334_TS3_HOUR,
MAX31334_TS3_DATE,
MAX31334_TS3_MONTH,
MAX31334_TS3_YEAR,
MAX31334_TS3_FLAGS,
};

const MAX3133X::reg_addr_t MAX31331::reg_addr = {
MAX31331_STATUS,
MAX31331_INT_EN,
MAX31331_RTC_RESET,
MAX31331_RTC_CONFIG1,
MAX31331_RTC_CONFIG2,
MAX31331_TIMESTAMP_CONFIG,
MAX31331_TIMER_CONFIG,
REG_NOT_AVAILABLE,
MAX31331_SECONDS_1_128,
MAX31331_SECONDS,
MAX31331_MINUTES,
MAX31331_HOURS,
MAX31331_DAY,
MAX31331_DATE,
MAX31331_MONTH,
MAX31331_YEAR,
MAX31331_ALM1_SEC,
MAX31331_ALM1_MIN,
MAX31331_ALM1_HRS,
MAX31331_ALM1_DAY_DATE,
MAX31331_ALM1_MON,
MAX31331_ALM1_YEAR,
MAX31331_ALM2_MIN,
MAX31331_ALM2_HRS,
MAX31331_ALM2_DAY_DATE,
MAX31331_TIMER_COUNT,
REG_NOT_AVAILABLE,
REG_NOT_AVAILABLE,
MAX31331_TIMER_INIT,
REG_NOT_AVAILABLE,
REG_NOT_AVAILABLE,
MAX31331_PWR_MGMT,
MAX31331_TRICKLE_REG,
MAX31331_OFFSET_HIGH,
MAX31331_OFFSET_LOW,
MAX31331_TS0_SEC_1_128,
MAX31331_TS0_SEC,
MAX31331_TS0_MIN,
MAX31331_TS0_HOUR,
MAX31331_TS0_DATE,
MAX31331_TS0_MONTH,
MAX31331_TS0_YEAR,
MAX31331_TS0_FLAGS,
MAX31331_TS1_SEC_1_128,
MAX31331_TS1_SEC,
MAX31331_TS1_MIN,
MAX31331_TS1_HOUR,
MAX31331_TS1_DATE,
MAX31331_TS1_MONTH,
MAX31331_TS1_YEAR,
MAX31331_TS1_FLAGS,
MAX31331_TS2_SEC_1_128,
MAX31331_TS2_SEC,
MAX31331_TS2_MIN,
MAX31331_TS2_HOUR,
MAX31331_TS2_DATE,
MAX31331_TS2_MONTH,
MAX31331_TS2_YEAR,
MAX31331_TS2_FLAGS,
MAX31331_TS3_SEC_1_128,
MAX31331_TS3_SEC,
MAX31331_TS3_MIN,
MAX31331_TS3_HOUR,
MAX31331_TS3_DATE,
MAX31331_TS3_MONTH,
MAX31331_TS3_YEAR,
MAX31331_TS3_FLAGS,
};