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

#include <MAX31341/MAX31341.h>
   

#define GET_BIT_VAL(val, pos, mask)     ( ( (val) & mask) >> pos )
#define SET_BIT_VAL(val, pos, mask)     ( ( ((int)val) << pos) & mask )

#define BCD2BIN(val) (((val) & 15) + ((val) >> 4) * 10)
#define BIN2BCD(val) ((((val) / 10) << 4) + (val) % 10)


#define ALL_IRQ  (	MAX31341_F_INT_EN_A1IE 	 | \
					MAX31341_F_INT_EN_A2IE 	 | \
					MAX31341_F_INT_EN_TIE 	 | \
					MAX31341_F_INT_EN_EIE1 	 | \
					MAX31341_F_INT_EN_ANA_IE | \
					MAX31341_F_INT_EN_DOSF	 )

int MAX31341::read_register(uint8_t reg, uint8_t *buf, uint8_t len/*=1*/)
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

int MAX31341::write_register(uint8_t reg, const uint8_t *buf, uint8_t len/*=1*/)
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

/*****************************************************************************/
MAX31341::MAX31341(TwoWire *i2c, uint8_t i2c_addr)
{
	if (i2c == NULL) {
		while (1) {
            ;
        }
	}
	m_i2c = i2c;
	m_slave_addr = i2c_addr;
}

void MAX31341::begin(void)
{
    m_i2c->begin();
    
    sw_reset_release();
    rtc_start();
    irq_disable();
}

int MAX31341::get_version(uint8_t &version)
{
    int ret;
    uint8_t val8;

	ret = read_register(MAX31341_R_REV_ID, &val8);
    version = GET_BIT_VAL(val8, MAX31341_F_REV_ID_REVID_POS, MAX31341_F_REV_ID_REVID);

    return ret;
}

int MAX31341::get_status(reg_status_t &stat)
{
    int ret;
    uint8_t val8;

    ret = read_register(MAX31341_R_INT_STATUS, &val8);
    if (ret) {
        return ret;
    }

    stat.bits.a1f    = GET_BIT_VAL(val8, MAX31341_F_INT_STATUS_A1IF_POS,    MAX31341_F_INT_STATUS_A1IF);
    stat.bits.a2f    = GET_BIT_VAL(val8, MAX31341_F_INT_STATUS_A2IF_POS,    MAX31341_F_INT_STATUS_A2IF);
    stat.bits.tif    = GET_BIT_VAL(val8, MAX31341_F_INT_STATUS_TIF_POS,     MAX31341_F_INT_STATUS_TIF);
    stat.bits.eif1   = GET_BIT_VAL(val8, MAX31341_F_INT_STATUS_EIF1_POS,  	MAX31341_F_INT_STATUS_EIF1);
    stat.bits.ana_if = GET_BIT_VAL(val8, MAX31341_F_INT_STATUS_ANA_IF_POS,  MAX31341_F_INT_STATUS_ANA_IF);
    stat.bits.osf    = GET_BIT_VAL(val8, MAX31341_F_INT_STATUS_OSF_POS,     MAX31341_F_INT_STATUS_OSF);
    stat.bits.los    = GET_BIT_VAL(val8, MAX31341_F_INT_STATUS_LOS_POS,   	MAX31341_F_INT_STATUS_LOS);

    return ret;
}

int MAX31341::get_configuration(reg_cfg_t &cfg)
{
    int ret;
    uint8_t regs[2];

    ret = read_register(MAX31341_R_CFG1, regs, 2);
    if (ret) {
        return ret;
    }

	// configuration byte 1
    cfg.bits.swrstn  = GET_BIT_VAL(regs[0], MAX31341_F_CFG1_SWRSTN_POS,  MAX31341_F_CFG1_SWRSTN);
    cfg.bits.rs  	 = GET_BIT_VAL(regs[0], MAX31341_F_CFG1_RS_POS,   	 MAX31341_F_CFG1_RS);
    cfg.bits.osconz  = GET_BIT_VAL(regs[0], MAX31341_F_CFG1_OSCONZ_POS,  MAX31341_F_CFG1_OSCONZ);
    cfg.bits.clksel  = GET_BIT_VAL(regs[0], MAX31341_F_CFG1_CLKSEL_POS,  MAX31341_F_CFG1_CLKSEL);
    cfg.bits.intcn   = GET_BIT_VAL(regs[0], MAX31341_F_CFG1_INTCN_POS,   MAX31341_F_CFG1_INTCN);
    cfg.bits.eclk  	 = GET_BIT_VAL(regs[0], MAX31341_F_CFG1_ECLK_POS,    MAX31341_F_CFG1_ECLK);

    // configuration byte 2
    cfg.bits.set_rtc  	 = GET_BIT_VAL(regs[1], MAX31341_F_CFG2_SET_RTC_POS,     MAX31341_F_CFG2_SET_RTC);
    cfg.bits.rd_rtc  	 = GET_BIT_VAL(regs[1], MAX31341_F_CFG2_RD_RTC_POS,   	 MAX31341_F_CFG2_RD_RTC);
    cfg.bits.i2c_timeout = GET_BIT_VAL(regs[1], MAX31341_F_CFG2_I2C_TIMEOUT_POS, MAX31341_F_CFG2_I2C_TIMEOUT);
    cfg.bits.bref	 	 = GET_BIT_VAL(regs[1], MAX31341_F_CFG2_BREF_POS,  		 MAX31341_F_CFG2_BREF);
    cfg.bits.data_reten	 = GET_BIT_VAL(regs[1], MAX31341_F_CFG2_DATA_RETEN_POS,  MAX31341_F_CFG2_DATA_RETEN);
   
    return ret;
}

int MAX31341::set_configuration(reg_cfg_t cfg)
{
    int ret;
    uint8_t regs[2] = {0, };

    // configuration byte 1
    regs[0] |= SET_BIT_VAL(cfg.bits.swrstn, MAX31341_F_CFG1_SWRSTN_POS,  MAX31341_F_CFG1_SWRSTN);
    regs[0] |= SET_BIT_VAL(cfg.bits.rs, 	MAX31341_F_CFG1_RS_POS,   	 MAX31341_F_CFG1_RS);
    regs[0] |= SET_BIT_VAL(cfg.bits.osconz, MAX31341_F_CFG1_OSCONZ_POS,  MAX31341_F_CFG1_OSCONZ);
    regs[0] |= SET_BIT_VAL(cfg.bits.clksel, MAX31341_F_CFG1_CLKSEL_POS,  MAX31341_F_CFG1_CLKSEL);
    regs[0] |= SET_BIT_VAL(cfg.bits.intcn, 	MAX31341_F_CFG1_INTCN_POS,   MAX31341_F_CFG1_INTCN);
    regs[0] |= SET_BIT_VAL(cfg.bits.eclk,   MAX31341_F_CFG1_ECLK_POS,  	 MAX31341_F_CFG1_ECLK);

    // configuration byte 2
    regs[1] |= SET_BIT_VAL(cfg.bits.set_rtc, 	MAX31341_F_CFG2_SET_RTC_POS,     MAX31341_F_CFG2_SET_RTC);
    regs[1] |= SET_BIT_VAL(cfg.bits.rd_rtc, 	MAX31341_F_CFG2_RD_RTC_POS,   	 MAX31341_F_CFG2_RD_RTC);
    regs[1] |= SET_BIT_VAL(cfg.bits.i2c_timeout,MAX31341_F_CFG2_I2C_TIMEOUT_POS, MAX31341_F_CFG2_I2C_TIMEOUT);
    regs[1] |= SET_BIT_VAL(cfg.bits.bref, 		MAX31341_F_CFG2_BREF_POS,  		 MAX31341_F_CFG2_BREF);
    regs[1] |= SET_BIT_VAL(cfg.bits.data_reten, MAX31341_F_CFG2_DATA_RETEN_POS,  MAX31341_F_CFG2_DATA_RETEN);
  
    ret = write_register(MAX31341_R_CFG1, regs, 2);

    return ret;
}

int MAX31341::get_time(struct tm *time)
{
	int ret;
	regs_rtc_time_t regs;

	if (time == NULL) {
		return -1;
	}

	ret = read_register(MAX31341_R_SECONDS, (uint8_t *) &regs, sizeof(regs));
	if (ret) {
		return ret;
	}

	/* tm_sec seconds [0,61] */
	time->tm_sec = BCD2BIN(regs.seconds.bcd.value);
	/* tm_min minutes [0,59] */
	time->tm_min = BCD2BIN(regs.minutes.bcd.value);
	/* tm_hour hour [0,23] */
	time->tm_hour = BCD2BIN(regs.hours.bcd.value);
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

int MAX31341::set_time(const struct tm *time)
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
	regs.hours.bcd.value= BIN2BCD(time->tm_hour);
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

    ret = write_register(MAX31341_R_SECONDS, (uint8_t *)&regs, sizeof(regs));
	if (ret) {
		return ret;
	}

    
    /*
     *  Set RTC
     */
    uint8_t val8;

	/* Toggle Set_RTC bit to set RTC registers */
    ret = read_register(MAX31341_R_CFG2, &val8);
	if (ret) {
		return ret;
	}

	val8 &= ~MAX31341_F_CFG2_SET_RTC;
	ret = write_register(MAX31341_R_CFG2, &val8);
    if (ret) {
		return ret;
	}

    delay(10);

	val8 |= MAX31341_F_CFG2_SET_RTC;
	ret = write_register(MAX31341_R_CFG2, &val8);
    if (ret) {
		return ret;
	}

	/* SET_RTC bit should be kept high at least 10ms */
    delay(10);

	val8 &= ~MAX31341_F_CFG2_SET_RTC;
	ret = write_register(MAX31341_R_CFG2, &val8);


    return ret;
}

int MAX31341::set_alarm(alarm_no_t alarm_no, const struct tm *alarm_time, alarm_period_t period)
{
	int ret;
	regs_alarm_t regs;

	if (alarm_no == ALARM2) {
		switch (period) {
			case ALARM_PERIOD_EVERYSECOND:
				return -1; // not support for alarm 2
		}
	}

	/*
	 *  Set period
	 */
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

	/* 
	 * Convert time structure to alarm registers 
	 */
	regs.sec.bcd.value = BIN2BCD(alarm_time->tm_sec);
	regs.min.bcd.value = BIN2BCD(alarm_time->tm_min);
	regs.hrs.bcd.value = BIN2BCD(alarm_time->tm_hour);

	if (regs.day_date.bits.dy_dt == 0) {
		/* Date match */
		regs.day_date.bcd_date.value = BIN2BCD(alarm_time->tm_mday);
	} else {
		/* Day match */
		regs.day_date.bcd_day.value = BIN2BCD(alarm_time->tm_wday);
	}
	//regs.mon.bcd.value = BIN2BCD(alarm_time->tm_mon);
	if (ret) {
		return ret;
	}

    /* 
     *  Write Registers 
     */
	uint8_t *ptr_regs = (uint8_t *)&regs;

	if (alarm_no == ALARM1) {
		ret = write_register(MAX31341_R_ALM1_SEC, &ptr_regs[0], sizeof(regs_alarm_t));
	} else {
		/* Discard sec starts from min register */
		ret = write_register(MAX31341_R_ALM2_MIN, &ptr_regs[1], sizeof(regs_alarm_t)-1);
	}

	return ret;
}

int MAX31341::get_alarm(alarm_no_t alarm_no, struct tm *alarm_time, alarm_period_t *period, bool *is_enabled)
{
	int ret;
	regs_alarm_t regs;
	uint8_t *ptr_regs = (uint8_t *)&regs;

	if (alarm_no == ALARM1) {
		ret = read_register(MAX31341_R_ALM1_SEC, &ptr_regs[0], sizeof(regs_alarm_t));
	} else {
		regs.sec.raw = 0;	/* zeroise second register for alarm2 */
		/* starts from min register (no sec register) */
		ret = read_register(MAX31341_R_ALM2_MIN, &ptr_regs[1], sizeof(regs_alarm_t)-1);
	}
    if (ret) {
        return ret;
    }

    /* 
     *  Convert alarm registers to time structure 
     */
	alarm_time->tm_sec = BCD2BIN(regs.sec.bcd.value);
	alarm_time->tm_min = BCD2BIN(regs.min.bcd.value);
	alarm_time->tm_hour = BCD2BIN(regs.hrs.bcd.value);

	if (regs.day_date.bits.dy_dt == 0) { /* date */
		alarm_time->tm_mday = BCD2BIN(regs.day_date.bcd_date.value);
	} else { /* day */
		alarm_time->tm_wday = BCD2BIN(regs.day_date.bcd_day.value);
	}
	//alarm_time->tm_mon = BCD2BIN(regs.mon.bcd.value) - 1;
	//alarm_time->tm_year = BCD2BIN(regs.year.bcd.value) + 100;	/* XXX no century bit */


    /*
     *  Find period
     */
	*period = (alarm_no == ALARM1) ? ALARM_PERIOD_EVERYSECOND : ALARM_PERIOD_EVERYMINUTE;

	if ((alarm_no == ALARM1) && (regs.sec.bits.axm1 == 0)) *period = ALARM_PERIOD_EVERYMINUTE;
	if (regs.min.bits.axm2 == 0) *period = ALARM_PERIOD_HOURLY;
	if (regs.hrs.bits.axm3 == 0) *period = ALARM_PERIOD_DAILY;
	if (regs.day_date.bits.axm4 == 0) *period = ALARM_PERIOD_WEEKLY;
 	if (regs.day_date.bits.dy_dt == 0) *period = ALARM_PERIOD_MONTHLY;
	//if ((alarm_no == ALARM1) && (regs.mon.bits.axm5 == 0)) *period = ALARM_PERIOD_YEARLY;
	//if ((alarm_no == ALARM1) && (regs.mon.bits.axm6 == 0)) *period = ALARM_PERIOD_ONETIME;


    /*
     *  Get enable status
     */
 	uint8_t val8;
	ret = read_register(MAX31341_R_INT_EN, &val8);
	if (ret) {
		return ret;
	}

	if (alarm_no == ALARM1) {
		*is_enabled = (val8 & (1 << INTR_ID_ALARM1)) != 0;
	} else {
		*is_enabled = (val8 & (1 << INTR_ID_ALARM2)) != 0;
	}

	return ret;
}

int MAX31341::set_power_mgmt_mode(power_mgmt_mode_t mode)
{
	int ret;
	uint8_t val8;

	ret = read_register(MAX31341_R_PWR_MGMT, &val8);
	if (ret) {
		return ret;
	}

	val8 &= ~MAX31341_F_PWR_MGMT_D_MODE;
	val8 |= SET_BIT_VAL(mode, MAX31341_F_PWR_MGMT_D_MODE_POS, MAX31341_F_PWR_MGMT_D_MODE);

	ret = write_register(MAX31341_R_PWR_MGMT, &val8);

	return ret;
}

int MAX31341::comparator_threshold_level(comp_thresh_t th)
{
	int ret;
	uint8_t val8;

	ret = read_register(MAX31341_R_CFG2, &val8);
	if (ret) {
		return ret;
	}

	val8 &= MAX31341_F_CFG2_BREF;
	val8 |= SET_BIT_VAL(th, MAX31341_F_CFG2_BREF_POS, MAX31341_F_CFG2_BREF);

	ret = write_register(MAX31341_R_CFG2, &val8);

	return ret;
}

int MAX31341::supply_select(power_mgmt_supply_t supply)
{
	int ret;
	uint8_t val8;

	ret = read_register(MAX31341_R_PWR_MGMT, &val8);
	if (ret) {
		return ret;
	}

	switch (supply) {
		case POW_MGMT_SUPPLY_SEL_VCC:
			val8 |= MAX31341_F_PWR_MGMT_DMAN_SEL;
			val8 &= ~MAX31341_F_PWR_MGMT_D_VBACK_SEL;
			break;
		case POW_MGMT_SUPPLY_SEL_AIN:
			val8 |= MAX31341_F_PWR_MGMT_DMAN_SEL;
			val8 |= MAX31341_F_PWR_MGMT_D_VBACK_SEL;
			break;
		case POW_MGMT_SUPPLY_SEL_AUTO:
		default:
			val8 &= ~MAX31341_F_PWR_MGMT_DMAN_SEL;
			break;
	}

	ret = write_register(MAX31341_R_PWR_MGMT, &val8);

	return ret;
}

int MAX31341::trickle_charger_enable(trickle_charger_ohm_t res)
{
	int ret;
	uint8_t val8 = 0;

	val8 |= SET_BIT_VAL(res, MAX31341_F_TRICKLE_D_TRICKLE_POS, MAX31341_F_TRICKLE_D_TRICKLE);
	ret = write_register(MAX31341_R_TRICKLE, &val8);

	return ret;
}

int MAX31341::trickle_charger_disable()
{
	int ret;
	uint8_t val8;

	val8 = 0;
	ret = write_register(MAX31341_R_TRICKLE, &val8);

	return ret;
}

int MAX31341::set_square_wave_frequency(sqw_out_freq_t freq)
{
	int ret;
	uint8_t val8;

	ret = read_register(MAX31341_R_CFG1, &val8);
	if (ret) {
		return ret;
	}

	val8 &= ~MAX31341_F_CFG1_RS;
	val8 |= SET_BIT_VAL(freq, MAX31341_F_CFG1_RS_POS, MAX31341_F_CFG1_RS);

	ret = write_register(MAX31341_R_CFG1, &val8);

	return ret;
}

int MAX31341::set_clock_sync_delay(sync_delay_t delay)
{
	int ret;
	uint8_t val8;

	ret = read_register(MAX31341_R_CLOCK_SYNC, &val8);
	if (ret) {
		return ret;
	}

	val8 &= ~MAX31341_F_CLOCK_SYNC_SYNC_DELAY;
	val8 |= SET_BIT_VAL(delay, MAX31341_F_CLOCK_SYNC_SYNC_DELAY_POS, MAX31341_F_CLOCK_SYNC_SYNC_DELAY);

	ret = write_register(MAX31341_R_CLOCK_SYNC, &val8);
    
	return ret;
}

int MAX31341::set_clkin_frequency(clkin_freq_t freq)
{
	int ret;
	uint8_t val8;

	ret = read_register(MAX31341_R_CFG1, &val8);
	if (ret) {
		return ret;
	}

	val8 &= ~MAX31341_F_CFG1_CLKSEL;
	val8 |= SET_BIT_VAL(freq, MAX31341_F_CFG1_CLKSEL_POS, MAX31341_F_CFG1_CLKSEL);

	ret = write_register(MAX31341_R_CFG1, &val8);
	if (ret) {
		return ret;
	}

	if (freq == CLKIN_FREQ_1HZ) {
		ret = set_clock_sync_delay(SYNC_DLY_LESS_THAN_1SEC);
	} else {
		ret = set_clock_sync_delay(SYNC_DLY_LESS_THAN_100MS);
	}

	return ret;
}

int MAX31341::configure_intb_clkout_pin(config_intb_clkout_pin_t sel)
{
	int ret;
	uint8_t val8;

	ret = read_register(MAX31341_R_CFG1, &val8);
	if (ret) {
		return ret;
	}

	if (sel == CONFIGURE_PIN_AS_INTB) {
		val8 |= MAX31341_F_CFG1_INTCN;
	} else {
		val8 &= ~MAX31341_F_CFG1_INTCN;
	}

	ret = write_register(MAX31341_R_CFG1, &val8);

	return ret;
}

int MAX31341::configure_inta_clkin_pin(config_inta_clkin_pin_t sel)
{
	int ret;
	uint8_t val8;

	ret = read_register(MAX31341_R_CFG1, &val8);
	if (ret) {
		return ret;
	}

	if (sel == CONFIGURE_PIN_AS_CLKIN) {
		val8 |= MAX31341_F_CFG1_ECLK;
	} else {
		val8 &= ~MAX31341_F_CFG1_ECLK;
	}

	ret = write_register(MAX31341_R_CFG1, &val8);
	if (ret) {
		return ret;
	}

	if (sel == CONFIGURE_PIN_AS_CLKIN) {
		/* Default synchronization delay for external clock mode */
		ret = set_clock_sync_delay(SYNC_DLY_LESS_THAN_1SEC);
	} else {
		/* Synchronization delay for internal oscillator mode */
		ret = set_clock_sync_delay(SYNC_DLY_LESS_THAN_10MS);
	}

	return ret;
}

int MAX31341::timer_init(uint8_t initial_value, bool repeat, timer_freq_t freq)
{
	int ret;
	uint8_t val8;

	ret = read_register(MAX31341_R_TIMER_CFG, &val8);
	if (ret) {
		return ret;
	}

	val8 &= ~MAX31341_F_TIMER_CFG_TE;
	val8 |= MAX31341_F_TIMER_CFG_TPAUSE;
	if (repeat) {
		val8 |= MAX31341_F_TIMER_CFG_TRPT;
	} else {
		val8 &= ~MAX31341_F_TIMER_CFG_TRPT;
	}
	val8 &= ~MAX31341_F_TIMER_CFG_TFS;
	val8 |= SET_BIT_VAL(freq, MAX31341_F_TIMER_CFG_TFS_POS, MAX31341_F_TIMER_CFG_TFS);

	ret = write_register(MAX31341_R_TIMER_CFG, &val8);
	if (ret) {
		return ret;
	}

	ret = write_register(MAX31341_R_TIMER_INIT, &initial_value);

	return ret;
}

int MAX31341::timer_get(uint8_t &count)
{
	int ret;
	uint8_t val8;

	ret = read_register(MAX31341_R_TIMER_COUNT, &val8);
	if (ret) {
		return ret;
	}
	count = val8;

	return ret;
}

int MAX31341::timer_start()
{
	int ret;
	uint8_t val8;

	ret = read_register(MAX31341_R_TIMER_CFG, &val8);
	if (ret) {
		return ret;
	}

	val8 |= MAX31341_F_TIMER_CFG_TE;
	val8 &= ~MAX31341_F_TIMER_CFG_TPAUSE;

	ret = write_register(MAX31341_R_TIMER_CFG, &val8);

	return ret;
}

int MAX31341::timer_pause()
{
	int ret;
	uint8_t val8;

	ret = read_register(MAX31341_R_TIMER_CFG, &val8);
	if (ret) {
		return ret;
	}

	val8 |= MAX31341_F_TIMER_CFG_TE;
	val8 |= MAX31341_F_TIMER_CFG_TPAUSE;

	ret = write_register(MAX31341_R_TIMER_CFG, &val8);

	return ret;
}

int MAX31341::timer_continue()
{
	int ret;
	uint8_t val8;

	ret = read_register(MAX31341_R_TIMER_CFG, &val8);
	if (ret) {
		return ret;
	}

	val8 |= MAX31341_F_TIMER_CFG_TE;
	val8 &= ~MAX31341_F_TIMER_CFG_TPAUSE;

	ret = write_register(MAX31341_R_TIMER_CFG, &val8);

	return ret;
}

int MAX31341::timer_stop()
{
	int ret;
	uint8_t val8;

	ret = read_register(MAX31341_R_TIMER_CFG, &val8);
	if (ret) {
		return ret;
	}

	val8 &= ~MAX31341_F_TIMER_CFG_TE;
	val8 |= MAX31341_F_TIMER_CFG_TPAUSE;

	ret = write_register(MAX31341_R_TIMER_CFG, &val8);

	return ret;
}

int MAX31341::set_data_retention_mode(bool enable)
{
	int ret;
	uint8_t regs[2];

	ret = read_register(MAX31341_R_CFG1, regs, 2);
	if (ret) {
		return ret;
	}

	if (enable) {
		regs[0] |= MAX31341_F_CFG1_OSCONZ; 
		regs[1] |= MAX31341_F_CFG2_DATA_RETEN; 
	} else {
		regs[0] &= ~MAX31341_F_CFG1_OSCONZ; 
		regs[1] &= ~MAX31341_F_CFG2_DATA_RETEN; 
	}

	ret = write_register(MAX31341_R_CFG1, regs, 2);

	return ret;
}

int MAX31341::irq_enable(intr_id_t id/*=INTR_ID_ALL*/)
{
	int ret;
	uint8_t val8;

	ret = read_register(MAX31341_R_INT_EN, &val8);
	if (ret) {
		return ret;
	}

    if (id == INTR_ID_ALL) {
        val8 |= ALL_IRQ;
    } else {
        val8 |= id;
    }

	ret = write_register(MAX31341_R_INT_EN, &val8);

	return ret;
}

int MAX31341::irq_disable(intr_id_t id/*=INTR_ID_ALL*/)
{
	int ret;
	uint8_t val8;

	ret = read_register(MAX31341_R_INT_EN, &val8);
	if (ret) {
		return ret;
	}

    if (id == INTR_ID_ALL) {
        val8 &= ~ALL_IRQ;
    } else {
		val8 &= ~id;
    }

	ret = write_register(MAX31341_R_INT_EN, &val8);

	return ret;
}

int MAX31341::irq_clear_flag(intr_id_t id/*=INTR_ID_ALL*/)
{
    int ret;
	uint8_t val8;

	// read status register to clear flags
	ret = read_register(MAX31341_R_INT_STATUS, &val8);

    return ret;
}

int MAX31341::sw_reset_assert()
{
	int ret;
	uint8_t val8;

	ret = read_register(MAX31341_R_CFG1, &val8);
	if (ret) {
		return ret;
	}

	/* Put device in reset state */
	val8 &= ~MAX31341_F_CFG1_SWRSTN;

	ret = write_register(MAX31341_R_CFG1, &val8);

	return ret;
}

int MAX31341::sw_reset_release()
{
	int ret;
	uint8_t val8;

	ret = read_register(MAX31341_R_CFG1, &val8);
	if (ret) {
		return ret;
	}

	/* Remove device from reset state */
	val8 |= MAX31341_F_CFG1_SWRSTN;
	ret = write_register(MAX31341_R_CFG1, &val8);

	return ret;
}

int MAX31341::rtc_start()
{
	int ret;
	uint8_t val8;

	ret = read_register(MAX31341_R_CFG1, &val8);
	if (ret) {
		return ret;
	}

	/* Enable the oscillator */
	val8 &= ~MAX31341_F_CFG1_OSCONZ; 
	ret = write_register(MAX31341_R_CFG1, &val8);

	return ret;
}
int MAX31341::rtc_stop()
{
	int ret;
	uint8_t val8;

	ret = read_register(MAX31341_R_CFG1, &val8);
	if (ret) {
		return ret;
	}

	/* Disable the oscillator */
	val8 |= MAX31341_F_CFG1_OSCONZ; 
	ret = write_register(MAX31341_R_CFG1, &val8);

	return ret;
}

int MAX31341::nvram_size()
{
	return (MAX31341_R_RAM_END - MAX31341_R_RAM_START) + 1;
}

int MAX31341::nvram_write(const uint8_t *buffer, int offset, int length)
{
    int ret;
	int totlen;

	totlen = (MAX31341_R_RAM_END - MAX31341_R_RAM_START) + 1;

	if ((offset + length) > totlen) {
		return -1;
	}

	if (length == 0) {
		return 0;
	}

    ret = write_register(MAX31341_R_RAM_START + offset, buffer, length);

	return ret;
}

int MAX31341::nvram_read(uint8_t *buffer, int offset, int length)
{
    int ret;
	int totlen;

	totlen = (MAX31341_R_RAM_END - MAX31341_R_RAM_START) + 1;

	if ((offset + length) > totlen) {
		return -1;
	}

	if (length == 0) {
		return 0;
	}

    ret = read_register(MAX31341_R_RAM_START + offset, buffer, length);

	return ret;
}
