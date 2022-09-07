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

#ifndef _MAX31341_H_
#define _MAX31341_H_

#include <MAX31341/MAX31341_registers.h>

#include <Arduino.h>
#include <Wire.h>
#include <time.h>


#define MAX31341_DRV_VERSION        "v1.0.1"


class MAX31341
{
public:
	/**
	* @brief	Mode of the comparator
	*/
	typedef enum {
	    POW_MGMT_MODE_COMPARATOR,		/**< Comparator */
	    POW_MGMT_MODE_POWER_MANAGEMENT,	/**< Power Management / Trickle Charger Mode */
	} power_mgmt_mode_t;

	/**
	* @brief	Analog comparator threshold voltage
	*/
	typedef enum {
	    COMP_THRESH_1V3,	/**< 1.3V */
	    COMP_THRESH_1V7,	/**< 1.7V */
	    COMP_THRESH_2V0,	/**< 2.0V */
	    COMP_THRESH_2V2,	/**< 2.2V */
	} comp_thresh_t;

	/**
	* @brief	Supply voltage select.
	*/
	typedef enum {
	    POW_MGMT_SUPPLY_SEL_AUTO,	/**< Circuit decides whether to use VCC or VBACKUP */
	    POW_MGMT_SUPPLY_SEL_VCC,	/**< Use VCC as supply */
	    POW_MGMT_SUPPLY_SEL_AIN,	/**< Use AIN as supply */
	} power_mgmt_supply_t;

	/**
	* @brief	Selection of charging path's resistor value
	*/
	typedef enum {
	    TRICKLE_CHARGER_NO_CONNECTION = 0,   /**< No connect */
	    TRICKLE_CHARGER_3K_S  = 0x08,   /**< 3Kohm in series with a Schottky diode. */
	    TRICKLE_CHARGER_6K_S  = 0x0A,   /**< 6Kohm in series with a Schottky diode. */
	    TRICKLE_CHARGER_11K_S = 0x0B,   /**< 11Kohm in series with a Schottky diode. */
	    TRICKLE_CHARGER_3K_S_2  = 0x0C,   /**< 3Kohm in series with a diode in series with a Schottky diode. */
	    TRICKLE_CHARGER_6K_S_2  = 0x0E,   /**< 6Kohm in series with a diode in series with a Schottky diode. */
	    TRICKLE_CHARGER_11K_S_2 = 0x0F,   /**< 11Kohm in series with a diode in series with a Schottky diode. */
	} trickle_charger_ohm_t;

	/**
	* @brief	Timer frequency selection
	*/
	typedef enum {
	    TIMER_FREQ_1024HZ,	/**< 1024Hz */
	    TIMER_FREQ_256HZ,	/**< 256Hz */
	    TIMER_FREQ_64HZ,	/**< 64Hz */
	    TIMER_FREQ_16HZ,	/**< 16Hz */
	} timer_freq_t;

	/**
	* @brief	CLKIN frequency selection
	*/
	typedef enum {
	    CLKIN_FREQ_1HZ,		/**< 1Hz*/
	    CLKIN_FREQ_50HZ,	/**< 50Hz */
	    CLKIN_FREQ_60HZ,	/**< 60Hz */
	    CLKIN_FREQ_32768HZ,	/**< 32.768kHz */
	} clkin_freq_t;

	/**
	* @brief	Square wave output frequency selection on CLKOUT pin
	*/
	typedef enum {
	    SQW_OUT_FREQ_1HZ,	/**< 1Hz */
	    SQW_OUT_FREQ_4KHZ,	/**< 4.098kHz */
	    SQW_OUT_FREQ_8KHZ,	/**< 8.192kHz */
	    SQW_OUT_FREQ_32KHZ,	/**< 32.768kHz */
	} sqw_out_freq_t;

	/**
	* @brief	Selection of interrupt ids
	*/
	typedef enum {
	    INTR_ID_ALARM1   = 1<<0,	/**< Alarm1 flag */
	    INTR_ID_ALARM2   = 1<<1,	/**< Alarm2 flag */
	    INTR_ID_TIMER    = 1<<2,	/**< Timer interrupt flag */
	    INTR_ID_RESERVED = 1<<3,
	    INTR_ID_EXTERNAL = 1<<4,	/**< External interrupt flag for DIN1 */
	    INTR_ID_ANALOG   = 1<<5,	/**< Analog Interrupt flag / Power fail flag  */
	    INTR_ID_OSF      = 1<<6,	/**< Oscillator stop flag */
	    INTR_ID_ALL 	 = 0xFF
	} intr_id_t;

	/**
	* @brief	Alarm number selection
	*/
	typedef enum {
	    ALARM1,	/**< Alarm number 1 */
	    ALARM2,	/**< Alarm number 2 */
	} alarm_no_t;

	/**
	* @brief	Alarm periodicity selection
	*/
	typedef enum {
	    ALARM_PERIOD_EVERYSECOND,	/**< Once per second */
	    ALARM_PERIOD_EVERYMINUTE,	/**< Second match / Once per minute */
	    ALARM_PERIOD_HOURLY,		/**< Second and Minute match */
	    ALARM_PERIOD_DAILY,			/**< Hour, Minute and Second match*/
	    ALARM_PERIOD_WEEKLY,		/**< Day and Time match */
	    ALARM_PERIOD_MONTHLY 		/**< Date and Time match */
	} alarm_period_t;

	/**
	* @brief	Selection of INTA/CLKIN pin function
	*/
	typedef enum {
	    CONFIGURE_PIN_AS_INTA,	/**< Configure pin as interrupt out */
	    CONFIGURE_PIN_AS_CLKIN	/**< Configure pin as external clock in  */
	} config_inta_clkin_pin_t;

	/**
	* @brief	Selection of INTB/CLKOUT pin function
	*/
	typedef enum {
	    CONFIGURE_PIN_AS_CLKOUT,	/**< Output is square wave */
	    CONFIGURE_PIN_AS_INTB		/**< Output is interrupt */
	} config_intb_clkout_pin_t;

	/**
	 * @brief	Selection of sync delay
	 */
	typedef enum {
	    SYNC_DLY_LESS_THAN_1SEC = 0,	/**<  Sync delay less than 1 second, recommended for external 1Hz clock */
	    SYNC_DLY_LESS_THAN_100MS,		/**<  Sync delay less than 100 msec, recommended for external 50Hz/60Hz/32KHz clock */
	    SYNC_DLY_LESS_THAN_10MS			/**<  Sync delay less than 10 msec, recommended for internal clock */
	} sync_delay_t;

	typedef union {
		uint8_t raw;
		struct {
			uint8_t a1f     : 1;
			uint8_t a2f     : 1;
			uint8_t tif     : 1;
			uint8_t 	    : 1;
			uint8_t eif1    : 1;
			uint8_t ana_if 	: 1;
			uint8_t osf     : 1;
			uint8_t los     : 1;
		} bits;
	} reg_status_t;

	typedef union {
		uint16_t raw;
		struct {
			uint16_t swrstn      	: 1; 
			uint16_t rs   	 		: 2;
			uint16_t osconz	     	: 1; 
			uint16_t clksel 		: 2;
			uint16_t intcn    		: 1;
			uint16_t eclk			: 1; 
			uint16_t 				: 1; // not used 
			uint16_t set_rtc 	 	: 1; 
			uint16_t rd_rtc  	 	: 1; 
			uint16_t i2c_timeout    : 1;
			uint16_t bref      		: 2;
			uint16_t data_reten     : 1;
			uint16_t 			    : 1; // not used
		} bits;
	} reg_cfg_t;

	/**
	* @brief	Base class constructor.
	*
	* @param[in]	i2c Pointer to I2C bus object for this device.
	* @param[in]	i2c_addr slave addr
	*/
	MAX31341(TwoWire *i2c, uint8_t i2c_addr);
    
    /**
	* @brief  First initialization, must be called before using class function
	*
	*/
    void begin(void);

    /**
	* @brief		Get Revision ID of sensor
	*
	* @param[out]	version Pointer to save version number.
	*
	* @returns		0 on success, negative error code on failure.
	*/
    int get_version(uint8_t &version);

    /**
    * @brief        Read status byte
    *
    * @param[out]   stat: Decoded status byte
    *
    * @returns      0 on success, negative error code on failure.
    */
    int get_status(reg_status_t &stat);

    /**
    * @brief        Get configuration bytes
    *
    * @param[out]   cfg: configuration values
    *
    * @returns      0 on success, negative error code on failure.
    */
    int get_configuration(reg_cfg_t &cfg);

    /**
    * @brief        Set configuration bytes
    *
    * @param[in]    cfg: configuration values
    *
    * @returns      0 on success, negative error code on failure.
    */
    int set_configuration(reg_cfg_t cfg);

	/**
	* @brief		Read time info from RTC.
	*
	* @param[out]	rtc_time Time info from RTC.
	*
	* @returns		0 on success, negative error code on failure.
	*/
	int get_time(struct tm *rtc_ctime);

	/**
	* @brief		Set time info to RTC.
	*
	* @param[in]	rtc_time Time info to be written to RTC.
	*
	* @returns		0 on success, negative error code on failure.
	*/
	int set_time(const struct tm *rtc_ctime);

	/**
	* @brief		Set an alarm condition
	*
	* @param[in]	alarm_no Alarm number, ALARM1 or ALARM2
	* @param[in]	alarm_time Pointer to alarm time to be set
	* @param[in]	period Alarm periodicity, one of ALARM_PERIOD_*
	*
	* @return		0 on success, error code on failure
	*/
	int set_alarm(alarm_no_t alarm_no, const struct tm *alarm_time, alarm_period_t period);

	/**
	* @brief		Get alarm data & time
	*
	* @param[in]	alarm_no Alarm number, ALARM1 or ALARM2
	* @param[out]	alarm_time Pointer to alarm time to be filled in
	* @param[out]	period Pointer to the period of alarm, one of ALARM_PERIOD_*
	* @param[out]	is_enabled Pointer to the state of alarm
	*
	* @return		0 on success, error code on failure
	*/
	int get_alarm(alarm_no_t alarm_no, struct tm *alarm_time, alarm_period_t *period, bool *is_enabled);

	/**
	* @brief		Select power management mode of operation
	*
	* @param[in]	mode Mode selection, one of COMP_MODE_*
	*
	* @return		0 on success, error code on failure
	*/
	int set_power_mgmt_mode(power_mgmt_mode_t mode);

	/**
	* @brief		Set comparator threshold
	*
	* @param[in]	th Set Analog Comparator Threshold level, one of COMP_THRESH_*
	*
	* @return		0 on success, error code on failure
	*/
	int comparator_threshold_level(comp_thresh_t th);

	/**
	* @brief		Select device power source
	*
	* @param[in]	supply Supply selection, one of POW_MGMT_SUPPLY_SEL_*
	*
	* @return		0 on success, error code on failure
	*/
	int supply_select(power_mgmt_supply_t supply);

	/**
	* @brief		Configure trickle charger charging path, also enable it
	*
	* @param[in]	res Value of resister
	*
	* @return		0 on success, error code on failure
	*/
	int trickle_charger_enable(trickle_charger_ohm_t res);

	/**
	* @brief		Disable trickle charger
	*
	* @return		0 on success, error code on failure
	*/
	int trickle_charger_disable();

	/**
	* @brief		Select square wave output frequency selection
	*
	* @param[in]	freq Clock frequency, one of CLKOUT_FREQ_*
	*
	* @return		0 on success, error code on failure
	*/
	int set_square_wave_frequency(sqw_out_freq_t freq);

	/**
	* @brief		Select external clock input frequency
	*
	* @param[in]	freq Clock frequency, one of CLKIN_FREQ_*
	*
	* @return		0 on success, error code on failure
	*/
	int set_clkin_frequency(clkin_freq_t freq);

	/**
	* @brief		Select direction of INTB/CLKOUT pin
	*
	* @param[in]	sel Pin function, one of CONFIGURE_PIN_B3_AS_INTB or CONFIGURE_PIN_B3_AS_CLKOUT
	*
	* @return		0 on success, error code on failure
	*/
	int configure_intb_clkout_pin(config_intb_clkout_pin_t sel);

	/**
	* @brief		Select direction of INTA/CLKIN pin
	*
	* @param[in]	sel Pin function, one of CONFIGURE_PIN_B3_AS_INTA or CONFIGURE_PIN_B3_AS_CLKIN
	*
	* @return		0 on success, error code on failure
	*/
	int configure_inta_clkin_pin(config_inta_clkin_pin_t sel);

	/**
	* @brief		Initialize timer
	*
	* @param[in]	value Timer initial value
	* @param[in]	repeat Timer repeat mode enable/disable
	* @param[in]	freq Timer frequency, one of TIMER_FREQ_*
	* @param[in]	mode Timer mode, 0 or 1
	*
	* @return		0 on success, error code on failure
	*
	* @note			\p mode controls the countdown timer interrupt function
	* along with \p repeat.
	* 	Pulse interrupt when \p mode = 0, irrespective of \p repeat (true or false)
	* 	Pulse interrupt when \p mode = 1 and \p repeat = true
	* 	Level interrupt when \p mode = 1 and \p repeat = false
	*/
	int timer_init(uint8_t value, bool repeat, timer_freq_t freq);

	/**
	* @brief	Read timer value
	*
	* @return	0 on success, error code on failure
	*/
	int timer_get(uint8_t &count);

	/**
	* @brief	Enable timer
	*
	* @return	0 on success, error code on failure
	*/
	int timer_start();

	/**
	* @brief	Pause timer, timer value is preserved
	*
	* @return	0 on success, error code on failure
	*/
	int timer_pause();

	/**
	* @brief	Start timer from the paused value
	*
	* @return	0 on success, error code on failure
	*/
	int timer_continue();

	/**
	* @brief	Disable timer
	*
	* @return	0 on success, error code on failure
	*/
	int timer_stop();

	/**
	* @brief	Put device into data retention mode
	*
	* @param[in] enable: true to enter data retain mode, 
	* 					 false to exit from data retain mode
	* 
	* @return	0 on success, error code on failure
	*/
	int set_data_retention_mode(bool enable);

    /**
    * @brief        Enable interrupt
    *
    * @param[in]    id Interrupt id, one of INTR_ID_*
    *
    * @return       0 on success, error code on failure
    */
    int irq_enable(intr_id_t id=INTR_ID_ALL);

    /**
    * @brief        Disable interrupt
    *
    * @param[in]    id Interrupt id, one of INTR_ID_*
    *
    * @return       0 on success, error code on failure
    */
    int irq_disable(intr_id_t id=INTR_ID_ALL);
    
    /**
    * @brief    Clear the interrupt flag
    *
    * @return   0 on success, error code on failure
    */
    int irq_clear_flag(intr_id_t id=INTR_ID_ALL);
    
	/**
	* @brief	Put device into reset state
	*
	* @return 	0 on success, error code on failure
	*/
	int sw_reset_assert();

	/**
	* @brief	Release device from state state
	*
	* @return	0 on success, error code on failure
	*/
	int sw_reset_release();

	/**
	* @brief	Enable the RTC oscillator
	*
	* @return	0 on success, error code on failure
	*/
	int rtc_start();

	/**
	* @brief	Disable the RTC oscillator
	*
	* @return	0 on success, error code on failure
	*/
	int rtc_stop();

	/**
	* @brief		NVRAM size of the part
	*
	* @return		0 if part does not have a NVRAM, otherwise returns size
	*/
	int nvram_size();
	
	/**
	* @brief		Non-volatile memory write
	*
	* @param[out]	buffer Pointer to the data to be written
	* @param[in]	offset Offset of location in NVRAM
	* @param[in]	length Number of bytes to write
	*
	* @return		0 on success, error code on failure
	*/
	int nvram_write(const uint8_t *buffer, int offset, int length);

	/**
	* @brief		Non-volatile memory read
	*
	* @param[in]	buffer Buffer to read in to
	* @param[in]	offset Offset of location in NVRAM
	* @param[in]	length Number of bytes to read
	*
	* @return		0 on success, error code on failure
	*/
	int nvram_read(uint8_t *buffer, int offset, int length);

    /**
	* @brief		Read from a register.
	*
	* @param[in]	reg Address of a register to be read.
	* @param[out]	value Pointer to save result value.
	* @param[in]	len Size of result to be read.
	*
	* @returns		0 on success, negative error code on failure.
	*/
	int read_register(uint8_t reg, uint8_t *dst, uint8_t len=1);

	/**
	* @brief		Write to a register.
	*
	* @param[in]	reg Address of a register to be written.
	* @param[out]	value Pointer of value to be written to register.
	* @param[in]	len Size of result to be written.
	*
	* @returns		0 on success, negative error code on failure.
	*/
	int write_register(uint8_t reg, const uint8_t *src, uint8_t len=1);

private:
	typedef struct {
	    union {
	        uint8_t raw;
	        struct {
	            uint8_t seconds : 4;    /**< RTC seconds value. */
	            uint8_t sec_10  : 3;    /**< RTC seconds in multiples of 10 */
	            uint8_t         : 1;
	        } bits;
	        struct {
	            uint8_t value   : 7;
	            uint8_t         : 1;
	        } bcd;
	    } seconds;

	    union {
	        uint8_t raw;
	        struct {
	            uint8_t minutes : 4;    /**< RTC minutes value */
	            uint8_t min_10  : 3;    /**< RTC minutes in multiples of 10 */
	            uint8_t         : 1;
	        } bits;
	        struct {
	            uint8_t value   : 7;
	            uint8_t         : 1;
	        } bcd;
	    } minutes;

	    union {
	        uint8_t raw;
	        struct {
	            uint8_t hour       : 4; /**< RTC hours value */
	            uint8_t hr_10      : 2; /**< RTC hours in multiples of 10 */
	            uint8_t            : 2;
	        } bits;
	        struct {
	            uint8_t value      : 6;
	            uint8_t            : 2;
	        } bcd;
	    } hours;

	    union {
	        uint8_t raw;
	        struct {
	            uint8_t day : 3;    /**< RTC days */
	            uint8_t     : 5;
	        } bits;
	        struct {
	            uint8_t value : 3;
	            uint8_t       : 5;
	        } bcd;
	    } day;

	    union {
	        uint8_t raw;
	        struct {
	            uint8_t date    : 4;    /**< RTC date */
	            uint8_t date_10 : 2;    /**< RTC date in multiples of 10 */
	            uint8_t         : 2;
	        } bits;
	        struct {
	            uint8_t value   : 6;
	            uint8_t         : 2;
	        } bcd;
	    } date;

	    union {
	        uint8_t raw;
	        struct {
	            uint8_t month    : 4;   /**< RTC months */
	            uint8_t month_10 : 1;   /**< RTC month in multiples of 10 */
	            uint8_t          : 2;
	            uint8_t century  : 1;   /**< Century bit */
	        } bits;
	        struct {
	            uint8_t value   : 5;
	            uint8_t         : 3;
	        } bcd;
	    } month;

	    union {
	        uint8_t raw;
	        struct {
	            uint8_t year    : 4;    /**< RTC years */
	            uint8_t year_10 : 4;    /**< RTC year multiples of 10 */
	        } bits;
	        struct {
	            uint8_t value   : 8;
	        } bcd;
	    } year;
	} regs_rtc_time_t;


	typedef struct {
	    union {
	        uint8_t raw;
	        struct {
	            uint8_t seconds : 4;    /**< Alarm1 seconds */
	            uint8_t sec_10  : 3;    /**< Alarm1 seconds in multiples of 10 */
	            uint8_t axm1    : 1;    /**< Alarm1 mask bit for minutes */
	        } bits;
	        struct {
	            uint8_t value   : 7;
	            uint8_t         : 1;
	        } bcd;
	    } sec;

	    union {
	        uint8_t raw;
	        struct {
	            uint8_t minutes : 4;    /**< Alarm1 minutes */
	            uint8_t min_10  : 3;    /**< Alarm1 minutes in multiples of 10 */
	            uint8_t axm2    : 1;    /**< Alarm1 mask bit for minutes */
	        } bits;
	        struct {
	            uint8_t value   : 7;
	            uint8_t         : 1;
	        } bcd;
	    } min;

	    union {
	        uint8_t raw;
	        struct {
	            uint8_t hour       : 4; /**< Alarm1 hours */
	            uint8_t hr_10      : 2; /**< Alarm1 hours in multiples of 10 */
	            uint8_t            : 1;
	            uint8_t axm3       : 1; /**< Alarm1 mask bit for hours */
	        } bits;
	        struct {
	            uint8_t value      : 6;
	            uint8_t            : 2;
	        } bcd;
	    } hrs;

	    union {
	        uint8_t raw;
	        struct {
	            uint8_t day_date : 4;   /**< Alarm1 day/date */
	            uint8_t date_10  : 2;   /**< Alarm1 date in multiples of 10 */
	            uint8_t dy_dt    : 1;
	            uint8_t axm4     : 1;   /**< Alarm1 mask bit for day/date */
	        } bits;
	        struct {
	            uint8_t value   : 3;
	            uint8_t         : 5;
	        } bcd_day;
	        struct {
	            uint8_t value   : 6;
	            uint8_t         : 2;
	        } bcd_date;
	    } day_date;
	} regs_alarm_t;

	TwoWire *m_i2c;
	uint8_t m_slave_addr;

	int set_clock_sync_delay(sync_delay_t delay);
};

#endif /* _MAX31341_H_ */
