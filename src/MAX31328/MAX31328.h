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


#ifndef _MAX31328_H_
#define _MAX31328_H_


#include <MAX31328/MAX31328_registers.h>

#include "Arduino.h"
#include <Wire.h>
#include <time.h>


#define MAX31328_DRV_VERSION        "v1.0.1"

#define MAX31328_ERR_UNKNOWN          (-1)
#define MAX31328_ERR_BUSY             (-3)


class MAX31328
{
    public:
        typedef enum {
            INTR_ID_ALARM1 = 1<<0,  /**< Alarm1 flag */
            INTR_ID_ALARM2 = 1<<1,  /**< Alarm2 flag */
            INTR_ID_ALL    = 0xFF
        } intr_id_t;

        typedef enum {
            ALARM1 = 1<<0, // Alarm number 1
            ALARM2 = 1<<1, // Alarm number 2
        } alarm_no_t;

        /**
        * @brief    Alarm periodicity selection
        */
        typedef enum {
            ALARM_PERIOD_EVERYSECOND,   /**< Once per second */
            ALARM_PERIOD_EVERYMINUTE,   /**< Second match / Once per minute */
            ALARM_PERIOD_HOURLY,        /**< Second and Minute match */
            ALARM_PERIOD_DAILY,         /**< Hour, Minute and Second match*/
            ALARM_PERIOD_WEEKLY,        /**< Day and Time match */
            ALARM_PERIOD_MONTHLY,       /**< Date and Time match */
        } alarm_period_t;

        /**
        * @brief    Square wave output frequency selection on CLKOUT pin
        */
        typedef enum {
            SQW_OUT_FREQ_1HZ   = 0,  // 1Hz
            SQW_OUT_FREQ_4KHZ  = 1,  // 4.098kHz
            SQW_OUT_FREQ_8KHZ  = 2,  // 8.192kHz
            SQW_OUT_FREQ_32KHZ = 3,  // 32.768kHz
        } sqw_out_freq_t;

         typedef union {
            uint8_t raw;
            struct {
                uint8_t a1f     : 1;
                uint8_t a2f     : 1;
                uint8_t bsy     : 1;
                uint8_t en32kHz : 1;
                uint8_t         : 3; // not used
                uint8_t osf     : 1; 
            } bits;
        } reg_status_t;

        typedef union {
            uint8_t raw;
            struct {
                uint8_t a1ie   : 1;
                uint8_t a2ie   : 1;
                uint8_t intcn  : 1;
                uint8_t rs     : 2;
                uint8_t conv   : 1; 
                uint8_t bbsqw  : 1;
                uint8_t eosc   : 1; 
            } bits;
        } reg_cfg_t;


        MAX31328(TwoWire *i2c, uint8_t i2c_addr=MAX3128_I2C_ADDRESS);
        
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
        * @brief        Set status byte
        *
        * @param[in]    stat: Decoded status byte
        *
        * @returns      0 on success, negative error code on failure.
        */
        int set_status(reg_status_t stat);

        /**
        * @brief        Get control byte
        *
        * @param[out]   cfg: Decoded configuration byte
        *
        * @returns      0 on success, negative error code on failure.
        */
        int get_configuration(reg_cfg_t &cfg);

        /**
        * @brief        Set control byte
        *
        * @param[in]    cfg: Decoded configuration byte
        *
        * @returns      0 on success, negative error code on failure.
        */
        int set_configuration(reg_cfg_t cfg);

        /**
        * @brief        Read time info from RTC.
        *
        * @param[out]   rtc_time Time info from RTC.
        *
        * @returns      0 on success, negative error code on failure.
        */
        int get_time(struct tm *rtc_ctime);

        /**
        * @brief        Set time info to RTC.
        *
        * @param[in]    rtc_time Time info to be written to RTC.
        *
        * @returns      0 on success, negative error code on failure.
        */
        int set_time(const struct tm *rtc_ctime);
        
        /**
        * @brief        Set an alarm condition
        *
        * @param[in]    alarm_no Alarm number, ALARM1 or ALARM2
        * @param[in]    alarm_time Pointer to alarm time to be set
        * @param[in]    period Alarm periodicity, one of ALARM_PERIOD_*
        *
        * @return       0 on success, error code on failure
        */
        int set_alarm(alarm_no_t alarm_no, const struct tm *alarm_time, alarm_period_t period);

        /**
        * @brief        Get alarm data & time
        *
        * @param[in]    alarm_no Alarm number, ALARM1 or ALARM2
        * @param[out]   alarm_time Pointer to alarm time to be filled in
        * @param[out]   period Pointer to the period of alarm, one of ALARM_PERIOD_*
        * @param[out]   is_enabled Pointer to the state of alarm
        *
        * @return       0 on success, error code on failure
        */
        int get_alarm(alarm_no_t alarm_no, struct tm *alarm_time, alarm_period_t *period, bool *is_enabled);

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
        * @brief        Set square wave output frequency selection
        *
        * @param[in]    freq Clock frequency, one of SQW_OUT_FREQ_*
        *
        * @return       0 on success, error code on failure
        */
        int set_square_wave_frequency(sqw_out_freq_t freq);

        /**
        * @brief        Trigger temperature conversion
        *
        * @return       0 on success, error code on failure
        */
        int start_temp_conversion(void);

        /**
        * @brief        To decide temperature conversion finished or not
        *
        * @return       0 on TempReady, MAX31328_ERR_BUSY on not ready, error code on failure
        */
        int is_temp_ready(void);

        /**
        * @brief        Read temperature
        *
        * @param[in]    temp, temperature value will be put inside it
        *
        * @return       0 on success, error code on failure
        */
        int get_temp(float &temp);

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
                uint8_t raw;
                struct {
                    uint8_t seconds : 4;    // RTC seconds value
                    uint8_t sec_10  : 3;    // RTC seconds in multiples of 10
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
                    uint8_t minutes : 4;    // RTC minutes value
                    uint8_t min_10  : 3;    // RTC minutes in multiples of 10
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
                    uint8_t hour       : 4; // RTC hours value
                    uint8_t hr_10      : 1; // RTC hours in multiples of 10
                    uint8_t hr20_am_pm : 1; // 0x0: Indicates AM in 12-hr format, 0x1: Indicates PM in 12-hr format
                    uint8_t f24_12     : 1; // 0x0: 24-hr format, 0x1: 12-hr format
                    uint8_t            : 1;
                } bits;
                struct {
                    uint8_t value      : 6;
                    uint8_t            : 2;
                } bcd_format24;
                struct {
                    uint8_t value      : 5;
                    uint8_t            : 3;
                } bcd_format12;
            } hours;

            union {
                uint8_t raw;
                struct {
                    uint8_t day     : 3;    // RTC days
                    uint8_t         : 5;
                } bits;
                struct {
                    uint8_t value   : 3;
                    uint8_t         : 5;
                } bcd;
            } day;

            union {
                uint8_t raw;
                struct {
                    uint8_t date    : 4;    // RTC date
                    uint8_t date_10 : 2;    // RTC date in multiples of 10
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
                    uint8_t month    : 4;   // RTC months
                    uint8_t month_10 : 1;   // RTC month in multiples of 10
                    uint8_t          : 2;
                    uint8_t century  : 1;   // 0x0: Year is in current century, 0x1: Year is in the next century
                } bits;
                struct {
                    uint8_t value   : 5;
                    uint8_t         : 3;
                } bcd;
            } month;

            union {
                uint8_t raw;
                struct {
                    uint8_t year    : 4;    // RTC years
                    uint8_t year_10 : 4;    // RTC year multiples of 10
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
                    uint8_t seconds : 4;    // Alarm1 seconds
                    uint8_t sec_10  : 3;    // Alarm1 seconds in multiples of 10
                    uint8_t axm1    : 1;    // Alarm1 mask bit for minutes
                } bits;
                struct {
                    uint8_t value   : 7;
                    uint8_t         : 1;
                } bcd;
            } sec;

            union {
                uint8_t raw;
                struct {
                    uint8_t minutes : 4;    // Alarm1 minutes
                    uint8_t min_10  : 3;    // Alarm1 minutes in multiples of 10
                    uint8_t axm2    : 1;    // Alarm1 mask bit for minutes
                } bits;
                struct {
                    uint8_t value   : 7;
                    uint8_t         : 1;
                } bcd;
            } min;

            union {
                uint8_t raw;
                struct {
                    uint8_t hour       : 4; // Alarm1 hours
                    uint8_t hr_10      : 1; // Alarm1 hours in multiples of 10
                    uint8_t hr20_am_pm : 1; // 0x0: Indicates AM in 12-hr format, 0x1: Indicates PM in 12-hr format
                    uint8_t f24_12     : 1; // 0x0: 24-hr format, 0x1: 12-hr format
                    uint8_t axm3       : 1; // Alarm1 mask bit for hours
                } bits;
                struct {
                    uint8_t value      : 6;
                    uint8_t            : 2;
                } bcd_format24;
                struct {
                    uint8_t value      : 5;
                    uint8_t            : 3;
                } bcd_format12;
            } hrs;

            union {
                uint8_t raw;
                struct {
                    uint8_t day_date : 4;   // Alarm1 day/date
                    uint8_t date_10  : 2;   // Alarm1 date in multiples of 10
                    uint8_t dy_dt    : 1;
                    uint8_t axm4     : 1;   // Alarm1 mask bit for day/date
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
        uint8_t  m_slave_addr;
};
#endif /* _MAX31328_H_ */
