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

#include <MAX31343/MAX31343.h>
#include <stdarg.h>


#define GET_BIT_VAL(val, pos, mask)     ( ( (val) & mask) >> pos )
#define SET_BIT_VAL(val, pos, mask)     ( ( ((int)val) << pos) & mask )

#define BCD2BIN(val) (((val) & 15) + ((val) >> 4) * 10)
#define BIN2BCD(val) ((((val) / 10) << 4) + (val) % 10)


#define ALL_IRQ  (	MAX31343_F_INT_EN_A1IE 	 | \
					MAX31343_F_INT_EN_A2IE 	 | \
					MAX31343_F_INT_EN_TIE 	 | \
					MAX31343_F_INT_EN_TSIE 	 | \
					MAX31343_F_INT_EN_PFAILE | \
					MAX31343_F_INT_EN_DOSF	 )


int MAX31343::read_register(uint8_t reg, uint8_t *buf, uint8_t len/*=1*/)
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

int MAX31343::write_register(uint8_t reg, const uint8_t *buf, uint8_t len/*=1*/)
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

/***********************************************************************************/
MAX31343::MAX31343(TwoWire *i2c, uint8_t i2c_addr)
{
	if (i2c == NULL) {
		while (1);
	}

	m_i2c = i2c;
	m_slave_addr = i2c_addr;
}

void MAX31343::begin(void)
{
	m_i2c->begin();

	sw_reset_release();
	rtc_start();
	irq_disable();
}

int MAX31343::get_status(reg_status_t &stat)
{
    int ret;
    uint8_t val8;

    ret = read_register(MAX31343_R_STATUS, &val8);
    if (ret) {
        return ret;
    }

    stat.bits.a1f    = GET_BIT_VAL(val8, MAX31343_F_STATUS_A1F_POS,      MAX31343_F_STATUS_A1F);
    stat.bits.a2f    = GET_BIT_VAL(val8, MAX31343_F_STATUS_A2F_POS,      MAX31343_F_STATUS_A2F);
    stat.bits.tif    = GET_BIT_VAL(val8, MAX31343_F_STATUS_TIF_POS,      MAX31343_F_STATUS_TIF);
    stat.bits.tsf    = GET_BIT_VAL(val8, MAX31343_F_STATUS_TSF_POS,  	 MAX31343_F_STATUS_TSF);
    stat.bits.pfail  = GET_BIT_VAL(val8, MAX31343_F_STATUS_PFAIL_POS,    MAX31343_F_STATUS_PFAIL);
    stat.bits.osf    = GET_BIT_VAL(val8, MAX31343_F_STATUS_OSF_POS,      MAX31343_F_STATUS_OSF);
    stat.bits.psdect = GET_BIT_VAL(val8, MAX31343_F_STATUS_PSDECT_POS,   MAX31343_F_STATUS_PSDECT);

    return ret;
}

int MAX31343::get_configuration(reg_cfg_t &cfg)
{
    int ret;
    uint8_t regs[2];

    ret = read_register(MAX31343_R_CFG1, regs, 2);
    if (ret) {
        return ret;
    }

	// configuration byte 1
    cfg.bits.enosc  	 = GET_BIT_VAL(regs[0], MAX31343_F_CFG1_ENOSC_POS,   	 MAX31343_F_CFG1_ENOSC);
    cfg.bits.i2c_timeout = GET_BIT_VAL(regs[0], MAX31343_F_CFG1_I2C_TIMEOUT_POS, MAX31343_F_CFG1_I2C_TIMEOUT);
    cfg.bits.data_ret  	 = GET_BIT_VAL(regs[0], MAX31343_F_CFG1_DATA_RET_POS,    MAX31343_F_CFG1_DATA_RET);
    // configuration byte 2
    cfg.bits.sqw_hz  	 = GET_BIT_VAL(regs[1], MAX31343_F_CFG2_SQW_HZ_POS,   MAX31343_F_CFG2_SQW_HZ);
    cfg.bits.clko_hz	 = GET_BIT_VAL(regs[1], MAX31343_F_CFG2_CLKO_HZ_POS,  MAX31343_F_CFG2_CLKO_HZ);
    cfg.bits.enclko	 	 = GET_BIT_VAL(regs[1], MAX31343_F_CFG2_ENCLKO_POS,   MAX31343_F_CFG2_ENCLKO);
   
    return ret;
}

int MAX31343::set_configuration(reg_cfg_t cfg)
{
    int ret;
    uint8_t regs[2] = {0, };

    regs[0] |= SET_BIT_VAL(cfg.bits.enosc, 		 MAX31343_F_CFG1_ENOSC_POS,   	  MAX31343_F_CFG1_ENOSC);
    regs[0] |= SET_BIT_VAL(cfg.bits.i2c_timeout, MAX31343_F_CFG1_I2C_TIMEOUT_POS, MAX31343_F_CFG1_I2C_TIMEOUT);
    regs[0] |= SET_BIT_VAL(cfg.bits.data_ret, 	 MAX31343_F_CFG1_DATA_RET_POS,    MAX31343_F_CFG1_DATA_RET);
    // configuration byte 2
    regs[1] |= SET_BIT_VAL(cfg.bits.sqw_hz,  MAX31343_F_CFG2_SQW_HZ_POS,   MAX31343_F_CFG2_SQW_HZ);
    regs[1] |= SET_BIT_VAL(cfg.bits.clko_hz, MAX31343_F_CFG2_CLKO_HZ_POS,  MAX31343_F_CFG2_CLKO_HZ);
    regs[1] |= SET_BIT_VAL(cfg.bits.enclko,  MAX31343_F_CFG2_ENCLKO_POS,   MAX31343_F_CFG2_ENCLKO);
    
    ret = write_register(MAX31343_R_CFG1, regs, 2);

    return ret;
}

int MAX31343::get_time(struct tm *time)
{
	int ret;
	rtc_time_regs_t regs;

	if (time == NULL) {
		return -1;
	}

	ret = read_register(MAX31343_R_SECONDS, (uint8_t *) &regs, sizeof(regs));
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

int MAX31343::set_time(const struct tm *time)
{
	int ret;
	rtc_time_regs_t regs;

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
		return -1;
	}

	ret = write_register(MAX31343_R_SECONDS, (const uint8_t *) &regs, sizeof(regs)); 

	return ret;
}

int MAX31343::set_alarm(alarm_no_t alarm_no, const struct tm *alarm_time, alarm_period_t period)
{
	int ret;
	regs_alarm_t regs;

	if (alarm_no == ALARM2) {
		switch (period) {
			case ALARM_PERIOD_EVERYSECOND:
			case ALARM_PERIOD_ONETIME:
			case ALARM_PERIOD_YEARLY:
				return -1; // not support for alarm 2
		}
	}


	/*
	 *  Set period
	 */

	// clear all flag, default onetime 
	regs.sec.bits.a1m1 = 0;
	regs.min.bits.a1m2 = 0;
	regs.hrs.bits.a1m3 = 0;
	regs.day_date.bits.a1m4 = 0;
	regs.mon.bits.a1m5 = 0;
	regs.mon.bits.a1m6 = 0;
	regs.day_date.bits.dy_dt = 0;

	switch (period) {
		case ALARM_PERIOD_ONETIME:
			// do nothing
			break;
		case ALARM_PERIOD_YEARLY:
			regs.mon.bits.a1m6 = 1;
			break;
		case ALARM_PERIOD_MONTHLY:
			regs.mon.bits.a1m5 = 1;
			regs.mon.bits.a1m6 = 1;
			break;
		case ALARM_PERIOD_WEEKLY:
			regs.mon.bits.a1m5 = 1;
			regs.mon.bits.a1m6 = 1;
			regs.day_date.bits.dy_dt = 1;
			break;
		case ALARM_PERIOD_DAILY:
			regs.day_date.bits.a1m4 = 1;
			regs.mon.bits.a1m5 = 1;
			regs.mon.bits.a1m6 = 1;
			break;
		case ALARM_PERIOD_HOURLY:
			regs.hrs.bits.a1m3 = 1;
			regs.day_date.bits.a1m4 = 1;
			regs.mon.bits.a1m5 = 1;
			regs.mon.bits.a1m6 = 1;
			break;
		case ALARM_PERIOD_EVERYMINUTE:
			regs.min.bits.a1m2 = 1;
			regs.hrs.bits.a1m3 = 1;
			regs.day_date.bits.a1m4 = 1;
			regs.mon.bits.a1m5 = 1;
			regs.mon.bits.a1m6 = 1;
			break;
		case ALARM_PERIOD_EVERYSECOND:
			regs.sec.bits.a1m1 = 1;
			regs.min.bits.a1m2 = 1;
			regs.hrs.bits.a1m3 = 1;
			regs.day_date.bits.a1m4 = 1;
			regs.mon.bits.a1m5 = 1;
			regs.mon.bits.a1m6 = 1;
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
	regs.mon.bcd.value = BIN2BCD(alarm_time->tm_mon+1);
    
    if (alarm_time->tm_year >= 200) {
		regs.year.bcd.value = BIN2BCD(alarm_time->tm_year - 200);
	} else if (alarm_time->tm_year >= 100) {
		regs.year.bcd.value = BIN2BCD(alarm_time->tm_year - 100);
	} else {
		return -1;
	}


    /* 
     *  Write Registers 
     */
    uint8_t *ptr_regs = (uint8_t *)&regs;

    if (alarm_no == ALARM1) {
        ret = write_register(MAX31343_R_ALM1_SEC, &ptr_regs[0], sizeof(regs_alarm_t));
    } else {
        /* XXX discard min, mon & sec registers */
        ret = write_register(MAX31343_R_ALM2_MIN, &ptr_regs[1], sizeof(regs_alarm_t)-3);
    }

	return ret;
}

int MAX31343::get_alarm(alarm_no_t alarm_no, struct tm *alarm_time, alarm_period_t *period, bool *is_enabled)
{
	int ret;
	regs_alarm_t regs;
	uint8_t reg;
    uint8_t *ptr_regs = (uint8_t *)&regs;

    /*
     *  Read registers
     */
    if (alarm_no == ALARM1) {
        ret = read_register(MAX31343_R_ALM1_SEC, &ptr_regs[0], sizeof(regs_alarm_t));
    } else {
        regs.sec.raw = 0;  /* zeroise second register for alarm2 */
        /* XXX discard mon & sec registers */
        ret = read_register(MAX31343_R_ALM2_MIN, &ptr_regs[1], sizeof(regs_alarm_t)-2);
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
	alarm_time->tm_mon = BCD2BIN(regs.mon.bcd.value) - 1;
	alarm_time->tm_year = BCD2BIN(regs.year.bcd.value) + 100;	/* XXX no century bit */


    /*
     *  Find period
     */
    if(alarm_no == ALARM1) {
        int alarm = regs.sec.bits.a1m1 | (regs.min.bits.a1m2<<1)
                    | (regs.hrs.bits.a1m3<<2) | (regs.day_date.bits.a1m4<<3)
                    | (regs.mon.bits.a1m5<<4) | (regs.mon.bits.a1m6<<5)
                    | (regs.day_date.bits.dy_dt<<6);

        switch(alarm) {
            case 0b1111111:
            case 0b0111111:
                *period = ALARM_PERIOD_EVERYSECOND;
                break;
            case 0b1111110:
            case 0b0111110:
                *period = ALARM_PERIOD_EVERYMINUTE;
                break;
            case 0b1111100:
            case 0b0111100:
                *period = ALARM_PERIOD_HOURLY;
                break;
            case 0b0111000:
            case 0b1111000:
                *period = ALARM_PERIOD_DAILY;
                break;
            case 0b0110000:
                *period = ALARM_PERIOD_MONTHLY;
                break;
            case 0b0100000:
                *period = ALARM_PERIOD_YEARLY;
                break;
            case 0b0000000:
                *period = ALARM_PERIOD_ONETIME;
                break;
            case 0b1110000:
                *period = ALARM_PERIOD_WEEKLY;
        }
    } else {
        int alarm = (regs.min.bits.a1m2) | (regs.hrs.bits.a1m3<<1)
                    | (regs.day_date.bits.a1m4<<2)
                    | (regs.day_date.bits.dy_dt<<3);

        switch(alarm) {
            case 0b1111:
            case 0b0111:
                *period = ALARM_PERIOD_EVERYMINUTE;
                break;
            case 0b1110:
            case 0b0110:
                *period = ALARM_PERIOD_HOURLY;
                break;
            case 0b1100:
            case 0b0100:
                *period = ALARM_PERIOD_DAILY;
                break;
            case 0b0000:
                *period = ALARM_PERIOD_MONTHLY;
                break;
            case 0b1000:
                *period = ALARM_PERIOD_WEEKLY;
        }
    }

    /*
     *  Get enable status
     */
	ret = read_register(MAX31343_R_INT_EN, (uint8_t *)&reg, 1);
	if (ret) {
		return ret;
	}

	if (alarm_no == ALARM1) {
		*is_enabled = (reg & (1 << INTR_ID_ALARM1)) != 0;
	} else {
		*is_enabled = (reg & (1 << INTR_ID_ALARM2)) != 0;
	}

	return ret;
}

int MAX31343::powerfail_threshold_level(comp_thresh_t th)
{
	int ret;
	uint8_t reg;

	ret = read_register(MAX31343_R_PWR_MGMT, &reg, 1);
	if (ret) {
		return ret;
	}

	reg &= ~MAX31343_F_PWR_MGMT_PFVT;
	reg |= SET_BIT_VAL(th, MAX31343_F_PWR_MGMT_PFVT_POS, MAX31343_F_PWR_MGMT_PFVT);
	
	ret = write_register(MAX31343_R_PWR_MGMT, &reg, 1);

	return ret;
}

int MAX31343::supply_select(power_mgmt_supply_t supply)
{
	int ret;
	uint8_t reg;

	ret = read_register(MAX31343_R_PWR_MGMT, &reg, 1);
	if (ret) {
		return ret;
	}

	switch (supply) {
	case POW_MGMT_SUPPLY_SEL_VCC:
		reg |= MAX31343_F_PWR_MGMT_DMAN_SEL;
		reg &= ~MAX31343_F_PWR_MGMT_D_VBACK_SEL;
		break;
	case POW_MGMT_SUPPLY_SEL_VBACK:
		reg |= MAX31343_F_PWR_MGMT_DMAN_SEL;
		reg |= MAX31343_F_PWR_MGMT_D_VBACK_SEL;
		break;
	case POW_MGMT_SUPPLY_SEL_AUTO:
	default:
		reg &= ~MAX31343_F_PWR_MGMT_DMAN_SEL;
		break;
	}

	ret = write_register(MAX31343_R_PWR_MGMT, &reg, 1);

	return ret;
}

int MAX31343::trickle_charger_enable(trickle_charger_ohm_t path)
{
    int ret;
    uint8_t reg = 0;

    // 0x5 is trickle enable code
	reg |= SET_BIT_VAL(5, MAX31343_F_TRICKLE_TCHE_POS, MAX31343_F_TRICKLE_TCHE);
	reg |= SET_BIT_VAL(path, MAX31343_F_TRICKLE_D_TRICKLE_POS, MAX31343_F_TRICKLE_D_TRICKLE);

	ret = write_register(MAX31343_R_TRICKLE, (uint8_t *)&reg, 1);

	return ret;
}

int MAX31343::trickle_charger_disable()
{
    int ret;
    uint8_t reg;
    
	ret = read_register(MAX31343_R_TRICKLE, &reg, 1);
	if (ret) {
		return ret;
	}

	reg &= ~MAX31343_F_TRICKLE_TCHE;
	ret = write_register(MAX31343_R_TRICKLE, &reg, 1);

	return ret;
}

int MAX31343::set_square_wave_frequency(sqw_out_freq_t freq)
{
	int ret;
	uint8_t cfg2;

	ret = read_register(MAX31343_R_CFG2, &cfg2, 1);
	if (ret) {
		return ret;
	}

	cfg2 &= ~MAX31343_F_CFG2_SQW_HZ;
	cfg2 |= SET_BIT_VAL(freq, MAX31343_F_CFG2_SQW_HZ_POS, MAX31343_F_CFG2_SQW_HZ);
	
	ret = write_register(MAX31343_R_CFG2, &cfg2, 1);

	return ret;
}

int MAX31343::clko_enable(clko_freq_t freq)
{
	int ret;
	uint8_t cfg2;

	ret = read_register(MAX31343_R_CFG2, (uint8_t *)&cfg2, 1);
	if (ret) {
		return ret;
	}

	cfg2 |= MAX31343_F_CFG2_ENCLKO;
	//
	cfg2 &= ~MAX31343_F_CFG2_CLKO_HZ;
	cfg2 |= SET_BIT_VAL(freq, MAX31343_F_CFG2_CLKO_HZ_POS, MAX31343_F_CFG2_CLKO_HZ);

	ret = write_register(MAX31343_R_CFG2, &cfg2, 1);

	return ret;
}

int MAX31343::clko_disable()
{
	int ret;
	uint8_t cfg2;

	ret = read_register(MAX31343_R_CFG2, &cfg2, 1);
	if (ret) {
		return ret;
	}

	cfg2 &= ~MAX31343_F_CFG2_ENCLKO;
	ret = write_register(MAX31343_R_CFG2, &cfg2, 1);

	return ret;
}

int MAX31343::timer_init(uint8_t initial_value, bool repeat, timer_freq_t freq)
{
	int ret;
	uint8_t cfg;

	ret = read_register(MAX31343_R_TIMER_CONFIG, &cfg, 1);
	if (ret) {
		return ret;
	}

	cfg &= ~MAX31343_F_TIMER_CONFIG_TE; /* timer is reset */
	cfg |= MAX31343_F_TIMER_CONFIG_TPAUSE; /* timer is paused */
	/* Timer repeat mode */
	if (repeat) {
		cfg |= MAX31343_F_TIMER_CONFIG_TRPT;
	} else {
		cfg &= ~MAX31343_F_TIMER_CONFIG_TRPT;
	}
	cfg &= ~MAX31343_F_TIMER_CONFIG_TFS;
	cfg |= SET_BIT_VAL(freq, MAX31343_F_TIMER_CONFIG_TFS_POS, MAX31343_F_TIMER_CONFIG_TFS);

	ret = write_register(MAX31343_R_TIMER_CONFIG, &cfg, 1);
	if (ret) {
		return ret;
	}

	ret = write_register(MAX31343_R_TIMER_INIT, &initial_value, 1);

	return ret;
}

int MAX31343::timer_get(uint8_t &val)
{
	int ret;
	uint8_t reg;

	ret = read_register(MAX31343_R_TIMER_COUNT, (uint8_t *)&reg, 1);
	if (ret) {
		return ret;
	}
	val = reg;

	return ret;
}

int MAX31343::timer_start()
{
	int ret;
	uint8_t cfg;

	ret = read_register(MAX31343_R_TIMER_CONFIG, &cfg, 1);
	if (ret) {
		return ret;
	}

	cfg |= MAX31343_F_TIMER_CONFIG_TE;
	cfg &= ~MAX31343_F_TIMER_CONFIG_TPAUSE;

	ret = write_register(MAX31343_R_TIMER_CONFIG, &cfg, 1);

	return ret;
}

int MAX31343::timer_pause()
{
	int ret;
	uint8_t cfg;

	ret = read_register(MAX31343_R_TIMER_CONFIG, &cfg, 1);
	if (ret) {
		return ret;
	}

	cfg |= MAX31343_F_TIMER_CONFIG_TE;
	cfg |= MAX31343_F_TIMER_CONFIG_TPAUSE;

	ret = write_register(MAX31343_R_TIMER_CONFIG, &cfg, 1);

	return ret;
}

int MAX31343::timer_continue()
{
	int ret;
	uint8_t cfg;

	ret = read_register(MAX31343_R_TIMER_CONFIG, &cfg, 1);
	if (ret) {
		return ret;
	}

	cfg |= MAX31343_F_TIMER_CONFIG_TE;
	cfg &= ~MAX31343_F_TIMER_CONFIG_TPAUSE;

	ret = write_register(MAX31343_R_TIMER_CONFIG, &cfg, 1);

	return ret;
}

int MAX31343::timer_stop()
{
	int ret;
	uint8_t cfg;

	ret = read_register(MAX31343_R_TIMER_CONFIG, &cfg, 1);
	if (ret) {
		return ret;
	}

	cfg &= ~MAX31343_F_TIMER_CONFIG_TE;
	cfg |= MAX31343_F_TIMER_CONFIG_TPAUSE;

	ret = write_register(MAX31343_R_TIMER_CONFIG, &cfg, 1);

	return ret;
}

int MAX31343::set_data_retention_mode(bool enable)
{
	int ret;
	uint8_t cfg1;

	ret = read_register(MAX31343_R_CFG1, &cfg1, 1);
	if (ret) {
		return ret;
	}

	if (enable) {
		cfg1 &= ~MAX31343_F_CFG1_ENOSC;
		cfg1 |= MAX31343_F_CFG1_DATA_RET;
	} else {
		cfg1 |= MAX31343_F_CFG1_ENOSC;
		cfg1 &= ~MAX31343_F_CFG1_DATA_RET;
	}

	ret = write_register(MAX31343_R_CFG1, &cfg1, 1);

	return ret;
}

int MAX31343::start_temp_conversion(bool automode/*=false*/, ttsint_t interval/*=TTS_INTERNAL_1SEC*/)
{
	int ret;
	uint8_t reg;

	ret = read_register(MAX31343_R_TS_CONFIG, &reg, 1);
	if (ret) {
		return ret;
	}

	if (automode) {
		reg |= MAX31343_F_TS_CONFIG_AUTO_MODE;
		//
		reg &= ~MAX31343_F_TS_CONFIG_TTSINT;
		reg |= SET_BIT_VAL(interval, MAX31343_F_TS_CONFIG_TTSINT_POS, MAX31343_F_TS_CONFIG_TTSINT);
	} else {
		reg &= ~MAX31343_F_TS_CONFIG_AUTO_MODE;
		reg |= MAX31343_F_TS_CONFIG_ONESHOT_MODE;
	}

	ret = write_register(MAX31343_R_TS_CONFIG, &reg, 1);

	return ret;
}

int MAX31343::is_temp_ready(void)
{
    int ret;
    uint8_t reg;

	ret = read_register(MAX31343_R_TS_CONFIG, &reg, 1);
	if (ret) {
		return ret;
	}


    if (reg & MAX31343_F_TS_CONFIG_ONESHOT_MODE) {
        return MAX31343_ERR_BUSY; // means temperatrue NOT ready
    } else {
        return 0; // means temperature ready 
    }

    return ret;
}

int MAX31343::get_temp(float &temp)
{
    int ret;
    uint8_t  buf[2];
    uint16_t count;

    #define TEMP_RESOLUTION_FOR_10_BIT      (0.25f)

    ret = read_register(MAX31343_R_TEMP_MSB, buf, 2);
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

int MAX31343::irq_enable(intr_id_t id/*=INTR_ID_ALL*/)
{
	int ret;
	uint8_t val8;

	ret = read_register(MAX31343_R_INT_EN, &val8, 1);
	if (ret) {
		return ret;
	}

    if (id == INTR_ID_ALL) {
        val8 |= ALL_IRQ;
    } else {
        val8 |= id;
    }
    
	ret = write_register(MAX31343_R_INT_EN, &val8, 1);

	return ret;
}

int MAX31343::irq_disable(intr_id_t id/*=INTR_ID_ALL*/)
{
	int ret;
	uint8_t val8;

	ret = read_register(MAX31343_R_INT_EN, &val8, 1);
	if (ret) {
		return ret;
	}

    if (id == INTR_ID_ALL) {
        val8 &= ~ALL_IRQ;
    } else {
		val8 &= ~id;
    }

	ret = write_register(MAX31343_R_INT_EN, &val8, 1);

	return ret;
}

int MAX31343::irq_clear_flag(intr_id_t id/*=INTR_ID_ALL*/)
{
	int ret;
	uint8_t val8;

	// read status register to clear flags
	ret = read_register(MAX31343_R_STATUS, &val8, 1);

	return ret;
}

int MAX31343::sw_reset_assert()
{
	int ret;
	uint8_t reg;

	reg = 1; /* Put device in reset state */
	ret = write_register(MAX31343_R_RTC_RESET, &reg, 1);

	return ret;
}

int MAX31343::sw_reset_release()
{
	int ret;
	uint8_t reg;

	/* Remove device from reset state */
	reg = 0;
	ret = write_register(MAX31343_R_RTC_RESET, &reg, 1);

	return ret;
}

int MAX31343::rtc_start()
{
	int ret;
	uint8_t cfg1;

	ret = read_register(MAX31343_R_CFG1, &cfg1, 1);
	if (ret) {
		return ret;
	}
	
	/* Enable the oscillator */
	cfg1 |= MAX31343_F_CFG1_ENOSC;	
	ret = write_register(MAX31343_R_CFG1, &cfg1, 1);

	return ret;
}

int MAX31343::rtc_stop()
{
	int ret;
	uint8_t cfg1;

	ret = read_register(MAX31343_R_CFG1, &cfg1, 1);
	if (ret) {
		return ret;
	}

	/* Disable the oscillator */
	cfg1 &= ~MAX31343_F_CFG1_ENOSC;	

	ret = write_register(MAX31343_R_CFG1, &cfg1, 1);

	return ret;
}

int MAX31343::nvram_size()
{
	return (MAX31343_R_RAM_REG_END - MAX31343_R_RAM_REG_START) + 1;
}

int MAX31343::nvram_write(int offset, const uint8_t *buffer, int length)
{
    int ret;
	int totlen;

	totlen = (MAX31343_R_RAM_REG_END - MAX31343_R_RAM_REG_START) + 1;

	if ((offset + length) > totlen) {
		return -1;
	}

	if (length == 0) {
		return 0;
	}

    ret = write_register(MAX31343_R_RAM_REG_START + offset, buffer, length);

	return ret;
}

int MAX31343::nvram_read(int offset, uint8_t *buffer, int length)
{
    int ret;
	int totlen;

	totlen = (MAX31343_R_RAM_REG_END - MAX31343_R_RAM_REG_START) + 1;

	if ((offset + length) > totlen) {
		return -1;
	}

	if (length == 0) {
		return 0;
	}

    ret = read_register(MAX31343_R_RAM_REG_START + offset, buffer, length);

	return ret;
}
