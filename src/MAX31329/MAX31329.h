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

#ifndef _MAX31329_H_
#define _MAX31329_H_

#include <MAX31329/MAX31329_registers.h>

#include <time.h>
#include <Wire.h>
#include <Arduino.h>


#define MAX31329_DRV_VERSION		"v1.0.1"

#define MAX31329_ERR_UNKNOWN          (-1)
#define MAX31329_ERR_BUSY             (-3)


class MAX31329
{
	public:
	    /**
	    * @brief	Uncompensated clock output frequency selection
	    */
	    typedef enum {
	        CLKO_FREQ_1HZ,		// 1Hz 
	        CLKO_FREQ_50HZ,		// 50Hz 
	        CLKO_FREQ_60HZ,		// 60Hz 
	        CLKO_FREQ_32000HZ,	// 32KHz
	    } clko_freq_t;

	    // /**
	    // * @brief	Square wave output frequency selection
	    // */
	    // typedef enum {
	    //     SQW_OUT_FREQ_1HZ  = 0,	//  1Hz
	    //     SQW_OUT_FREQ_2HZ,		//  2Hz
	    //     SQW_OUT_FREQ_4HZ,		//  4Hz
	    //     SQW_OUT_FREQ_8HZ,		//  8Hz
	    //     SQW_OUT_FREQ_16HZ,		// 16Hz
	    //     SQW_OUT_FREQ_32HZ,		// 32Hz
	    // } sqw_out_freq_t;

	    /**
	    * @brief	Power fail threshold voltage
	    */
	    typedef enum {
			COMP_THRESH_0V = 0,	// 1.85V
	        COMP_THRESH_1V85 = 1,	// 1.85V
	        COMP_THRESH_2V15,		// 2.15V
	        COMP_THRESH_2V40,		// 2.40V
	    } comp_thresh_t;

	    /**
	    * @brief	Supply voltage select.
	    */
	    typedef enum {
	        POW_MGMT_SUPPLY_SEL_AUTO,	/**< Circuit decides whether to use VCC or VBACKUP */
	        POW_MGMT_SUPPLY_SEL_VCC,	/**< Use VCC as supply */
	        POW_MGMT_SUPPLY_SEL_VBACK,	/**< Use VBACKUP as supply */
	    } power_mgmt_supply_t;

	    /**
	    * @brief	Selection of charging path's resistor value
	    */
	    typedef enum {
	        TRICKLE_CHARGER_3K_S,   /**< 3Kohm in series with a Schottky diode  */
	        TRICKLE_CHARGER_3K_2_S, /**< 3Kohm in series with a Schottky diode  */
	        TRICKLE_CHARGER_6K_S,	/**< 6Kohm in series with a Schottky diode */
	        TRICKLE_CHARGER_11K_S,    /**<  11Kohm in series with a Schottky diode */
	        TRICKLE_CHARGER_3K_D_S,   /**< 3Kohm in series with a diode+Schottky diode  */
	        TRICKLE_CHARGER_3K_2_D_S,   /**< 3Kohm in series with a diode+Schottky diode */
	        TRICKLE_CHARGER_6K_D_S,   /**< 6Kohm in series with a diode+Schottky diode */
	        TRICKLE_CHARGER_11K_D_S,  /**< 11Kohm in series with a diode+Schottky diode */
	        TRICKLE_CHARGER_NO_CONNECTION,   /**< No Connection  */
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
	    * @brief	Selection of interrupt ids
	    */
	    typedef enum {
	        INTR_ID_ALARM1 = 1<<0,	// Alarm1 flag
	        INTR_ID_ALARM2 = 1<<1,	// Alarm2 flag
	        INTR_ID_TIMER  = 1<<2,	// Timer interrupt flag
	        INTR_ID_DIF   = 1<<3,	// DIN interrupt flag
	        INTR_ID_PFAIL  = 1<<5,	// Power fail interrupt enable
	        INTR_ID_DOSF   = 1<<6,	// Disable Oscillator flag
	        INTR_ID_ALL	   = 0xFF
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
	        ALARM_PERIOD_MONTHLY,		/**< Date and Time match */
	        ALARM_PERIOD_YEARLY,		/**< Month, Date and Time match (Max31342 only) */
	        ALARM_PERIOD_ONETIME,		/**< Year, Month, Date and Time match (Max31342 only) */
	    } alarm_period_t;

	    /**
	    * @brief	Selection of INTA/INTB/CLKIN/CLKOUT pin function
	    */
	    typedef enum {
	        CONFIGURE_PIN_AS_CLKOUT,	/**< Output is square wave */
	        CONFIGURE_PIN_AS_INTB		/**< Output is interrupt */
	    } config_intb_clkout_pin_t;
		typedef enum {
			CONFIGURE_PIN_AS_CLKIN,	    /**< Input is square wave */
	        CONFIGURE_PIN_AS_INTA		/**< Output is interrupt */
	    } config_inta_clkin_pin_t;

		typedef union {
			uint8_t raw;
			struct {
				uint8_t a1f     : 1;
				uint8_t a2f     : 1;
				uint8_t tif     : 1;
				uint8_t dif     : 1;
				uint8_t los     : 1;
				uint8_t pfail   : 1;
				uint8_t osf 	: 1;
				uint8_t psdect  : 1;
			} bits;
		} reg_status_t;

		typedef union {
			uint16_t raw;
			struct {
				uint16_t enosc   	 : 1;
				uint16_t i2c_timeout : 1;
				uint16_t data_ret    : 1;
				uint16_t en_io       : 1;
				uint16_t 			 : 4; // not used
				uint16_t clkin_hz    : 2;
				uint16_t enclkin     : 1;
				uint16_t dip         : 1;
				uint16_t             : 1;
				uint16_t clko_hz     : 2;
				uint16_t enclko      : 1;
			} bits;
		} reg_cfg_t;

		MAX31329(TwoWire *i2c, uint8_t i2c_addr=MAX31329_I2C_ADDRESS);

	    /**
	    * @brief   First initialization, must be call before using class function
	    *
	    */
		void begin(void);

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
		* @brief		Set power fail threshold voltage
		*
		* @param[in]	th Set Analog Comparator Threshold level, one of COMP_THRESH_*
		*
		* @return		0 on success, error code on failure
		*/
		int powerfail_threshold_level(comp_thresh_t th);

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
	    * @param[in]	path Value of resister + diodes
	    *
	    * @return		0 on success, error code on failure
	    */
	    int trickle_charger_enable(trickle_charger_ohm_t path);

		/**
		* @brief		Disable trickle charger
		*
		* @return		0 on success, error code on failure
		*/
		int trickle_charger_disable();

		// /**
		// * @brief		Select square wave output frequency selection
		// *
		// * @param[in]	freq Clock frequency, one of SQUARE_WAVE_OUT_FREQ_*
		// *
		// * @return		0 on success, error code on failure
		// */
		// int set_square_wave_frequency(sqw_out_freq_t freq);

		/**
		* @brief		Enable CLKO
		*
		* @param[in]	freq Clock frequency, one of CLKO_FREQ_*
		*
		* @return		0 on success, error code on failure
		*/
		int clko_enable(clko_freq_t freq);

		/**
		* @brief		Disable CLKO
		*
		* @return		0 on success, error code on failure
		*/
		int clko_disable();
        /**
		* @brief		Disable CLKIN
		*
		* @return		0 on success, error code on failure
		*/
		int clkin_disable();

		/**
		* @brief		Initialize timer
		*
		* @param[in]	initial_value Timer initial value
		* @param[in]	repeat Timer repeat mode enable/disable
		* @param[in]	freq Timer frequency, one of TIMER_FREQ_*
		*
		* @return		0 on success, error code on failure
		*
		*/
		int timer_init(uint8_t initial_value, bool repeat, timer_freq_t freq);

		/**
		* @brief	Read timer value
		*
		* @param[out]	val: current timer count value
		* 
		* @return	0 on success, error code on failure
		*/
		int timer_get(uint8_t &val);

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
		* @brief		Configure temperature sensor
		*
		* @param[in]	automode temperature value
		* @param[in]	interval temperature read interval for automode, one of
		* 				TTS_INTERNAL_*
		*
		* @return		0 on success, error code on failure
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
		* @param[in]	offset Offset of location in NVRAM
		* @param[out]	buffer Pointer to the data to be written
		* @param[in]	length Number of bytes to write
		*
		* @return		0 on success, error code on failure
		*/
		int nvram_write(int offset, const uint8_t *buffer, int length);

		/**
		* @brief		Non-volatile memory read
		*
		* @param[in]	offset Offset of location in NVRAM
		* @param[in]	buffer Buffer to read in to
		* @param[in]	length Number of bytes to read
		*
		* @return		0 on success, error code on failure
		*/
		int nvram_read(int offset, uint8_t *buffer, int length);

        /**
        * @brief        Directly read value from register
        *
        * @param[in]    reg: Register address
        * @param[in]    buf: destionation that values will be written
        * @param[in]    len: expected read len
        *
        * @return       0 on success, error code on failure
        */
        int read_register(uint8_t reg, uint8_t *buf, uint8_t len=1);
        
        /**
        * @brief        Directly write value to register
        *
        * @param[in]    reg: Register address
        * @param[in]    buf: values that would like to be written
        * @param[in]    len: buf len
        *
        * @return       0 on success, error code on failure
        */
        int write_register(uint8_t reg, const uint8_t *buf, uint8_t len=1);

	private:
		typedef struct {
			union {
				unsigned char raw;
				struct {
					unsigned char seconds : 4;	/**< RTC seconds value. */
					unsigned char sec_10  : 3;	/**< RTC seconds in multiples of 10 */
					unsigned char         : 1;
				} bits;
				struct {
					unsigned char value   : 7;
					unsigned char         : 1;
				} bcd;
			} seconds;

			union {
				unsigned char raw;
				struct {
					unsigned char minutes : 4;	/**< RTC minutes value */
					unsigned char min_10  : 3;	/**< RTC minutes in multiples of 10 */
					unsigned char         : 1;
				} bits;
				struct {
					unsigned char value   : 7;
					unsigned char         : 1;
				} bcd;
			} minutes;

			union {
				unsigned char raw;
				struct {
					unsigned char hour       : 4;	/**< RTC hours value */
					unsigned char hr_10      : 1;	/**< RTC hours in multiples of 10 */
					unsigned char hr_20_AM_PM: 1;	/**< RTC hours in multiples of 20 in 24 hour format or AM/PM in 12 hour format */
					unsigned char f_24_12    : 1;	/**< RTC hours in 12-Hour or 24-Hour format*/
					unsigned char            : 1;
				} bits;
				struct {
					unsigned char value      : 7;
					unsigned char            : 1;
				} bcd;
			} hours;

			union {
				unsigned char raw;
				struct {
					unsigned char day : 3;	/**< RTC days */
					unsigned char     : 5;
				} bits;
				struct {
					unsigned char value : 3;
					unsigned char       : 5;
				} bcd;
			} day;

			union {
				unsigned char raw;
				struct {
					unsigned char date    : 4;	/**< RTC date */
					unsigned char date_10 : 2;	/**< RTC date in multiples of 10 */
					unsigned char         : 2;
				} bits;
				struct {
					unsigned char value   : 6;
					unsigned char         : 2;
				} bcd;
			} date;

			union {
				unsigned char raw;
				struct {
					unsigned char month    : 4;	/**< RTC months */
					unsigned char month_10 : 1;	/**< RTC month in multiples of 10 */
					unsigned char          : 2;
					unsigned char century  : 1;	/**< Century bit */
				} bits;
				struct {
					unsigned char value   : 5;
					unsigned char         : 3;
				} bcd;
			} month;

			union {
				unsigned char raw;
				struct {
					unsigned char year    : 4;	/**< RTC years */
					unsigned char year_10 : 4;	/**< RTC year multiples of 10 */
				} bits;
				struct {
					unsigned char value   : 8;
				} bcd;
			} year;
		} rtc_time_regs_t;

		typedef struct {
			union {
				unsigned char raw;
				struct {
					unsigned char seconds : 4;	/**< Alarm1 seconds */
					unsigned char sec_10  : 3;	/**< Alarm1 seconds in multiples of 10 */
					unsigned char a1m1    : 1;	/**< Alarm1 mask bit for minutes */
				} bits;
				struct {
					unsigned char value   : 7;
					unsigned char         : 1;
				} bcd;
			} sec;

			union {
				unsigned char raw;
				struct {
					unsigned char minutes : 4;	/**< Alarm1 minutes */
					unsigned char min_10  : 3;	/**< Alarm1 minutes in multiples of 10 */
					unsigned char a1m2    : 1;	/**< Alarm1 mask bit for minutes */
				} bits;
				struct {
					unsigned char value   : 7;
					unsigned char         : 1;
				} bcd;
			} min;

			union {
				unsigned char raw;
				struct {
					unsigned char hour       : 4;	/**< Alarm1 hours */
					unsigned char hr_10      : 1;	/**< Alarm1 hours in multiples of 10 */
					unsigned char hr_20_AM_PM: 1;   /**< Alarm1 hours in multiples of 20 or AM/PM format */
					unsigned char            : 1;   /**< Alarm1 hours in multiples of 20 or AM/PM format */
					unsigned char a1m3       : 1;	/**< Alarm1 mask bit for hours */
				} bits;
				struct {
					unsigned char value      : 6;
					unsigned char            : 2;
				} bcd;
			} hrs;

			union {
				unsigned char raw;
				struct {
					unsigned char day_date : 4;	/**< Alarm1 day/date */
					unsigned char date_10  : 2;	/**< Alarm1 date in multiples of 10 */
					unsigned char dy_dt    : 1;
					unsigned char a1m4     : 1;	/**< Alarm1 mask bit for day/date */
				} bits;
				struct {
					unsigned char value   : 3;
					unsigned char         : 5;
				} bcd_day;
				struct {
					unsigned char value   : 6;
					unsigned char         : 2;
				} bcd_date;
			} day_date;

			union {
				unsigned char raw;
				struct {
					unsigned char month    : 4;	/**< Alarm1 months */
					unsigned char month_10 : 1;	/**< Alarm1 months in multiples of 10 */
					unsigned char          : 1;
					unsigned char a1m6     : 1;	/**< Alarm1 mask bit for year */
					unsigned char a1m5     : 1;	/**< Alarm1 mask bit for month */
				} bits;
				struct {
					unsigned char value   : 5;
					unsigned char         : 3;
				} bcd;
			} mon;

			union {
				unsigned char raw;
				struct {
					unsigned char year    : 4;	/* Alarm1 years */
					unsigned char year_10 : 4;	/* Alarm1 multiples of 10 */
				} bits;
				struct {
					unsigned char value   : 8;
				} bcd;
			} year;
		} regs_alarm_t;

		TwoWire *m_i2c;
		uint8_t m_slave_addr;

};

#endif /* _MAX31329_H_ */
