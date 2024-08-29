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

#ifndef MAX3133X_HPP_
#define MAX3133X_HPP_

#include <Arduino.h>
#include <time.h>
#include <Wire.h>
#include "MAX3133X_registers.h"

enum max3133x_error_codes{
    MAX3133X_NO_ERR,
    MAX3133X_NULL_VALUE_ERR                 = -1,
    MAX3133X_READ_REG_ERR                   = -2,
    MAX3133X_WRITE_REG_ERR                  = -3,
    MAX3133X_INVALID_TIME_ERR               = -4,
    MAX3133X_INVALID_DATE_ERR               = -5,
    MAX3133X_INVALID_MASK_ERR               = -6,
    MAX3133X_INVALID_ALARM_PERIOD_ERR       = -7,
    MAX3133X_ALARM_ONETIME_NOT_SUPP_ERR     = -8,
    MAX3133X_ALARM_YEARLY_NOT_SUPP_ERR      = -9,
    MAX3133X_ALARM_EVERYMINUTE_NOT_SUPP_ERR = -10,
    MAX3133X_ALARM_EVERYSECOND_NOT_SUPP_ERR = -11,
    MAX3133X_I2C_BUFF_ERR                   = -12,
    MAX3133X_I2C_END_TRANS_ERR              = -13
};

class MAX3133X
{
public:
    /* PUBLIC FUNCTION DECLARATIONS */
 
    /**
    * @brief Read from a register.
    *
    * @param[in]    reg Address of a register to be read.
    * @param[out]   value Pointer to save result value.
    * @param[in]    len Size of result to be read.
    *
    * @returns 0 on success, negative error code on failure.
    */
    int read_register(uint8_t reg, uint8_t *value, uint8_t len);
 
    /**
    * @brief Write to a register.
    *
    * @param[in]    reg Address of a register to be written.
    * @param[out]   value Pointer of value to be written to register.
    * @param[in]    len Size of result to be written.
    *
    * @returns 0 on success, negative error code on failure.
    */
    int write_register(uint8_t reg, const uint8_t *value, uint8_t len);

    /**
    * @brief First initialization, must be call before using class function
    *
    */
    int begin(void);

    /**
    * @brief        Read time info from RTC.
    *
    * @param[out]   rtc_ctime Time info from RTC.
    *
    * @returns      0 on success, negative error code on failure.
    */
    int get_time(struct tm *rtc_ctime, uint16_t *sub_sec = NULL);

    /**
    * @brief Selection of 24hr-12hr hour format
    */
    typedef enum {
        HOUR24 = 0, /**< 24-Hour format */
        HOUR12 = 1, /**< 12-Hour format */
    } hour_format_t;

    /**
    * @brief        Set time info to RTC.
    *
    * @param[in]    rtc_ctime Time info to be written to RTC.
    *
    * @returns      0 on success, negative error code on failure.
    */
    int set_time(const struct tm *rtc_ctime, hour_format_t format = HOUR24);

    /**
    * @brief Alarm periodicity selection
    */
    typedef enum {
        ALARM_PERIOD_EVERYSECOND,   /**< Once a second */
        ALARM_PERIOD_EVERYMINUTE,   /**< Seconds match */
        ALARM_PERIOD_HOURLY,        /**< Seconds and Minutes match */
        ALARM_PERIOD_DAILY,         /**< Hours, Minutes and Seconds match*/
        ALARM_PERIOD_WEEKLY,        /**< Day and Time match */
        ALARM_PERIOD_MONTHLY,       /**< Date and Time match */
        ALARM_PERIOD_YEARLY,        /**< Month, Date and Time match */
        ALARM_PERIOD_ONETIME,       /**< Year, Month, Date and Time match */
    } alarm_period_t;

    /**
    * @brief Alarm number selection
    */
    typedef enum {
        ALARM1, /**< Alarm number 1 */
        ALARM2, /**< Alarm number 2 */
    } alarm_no_t;

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
     * @brief       Gets Status Register Value
     *
     * @param[in]   status_reg
     *
     * @returns     0 on success, negative error code on failure.
     */
    int get_status_reg(max3133x_status_reg_t * status_reg);

    /**
     * @brief       Gets Interrupt Enable Register Value
     *
     * @param[in]   int_en_reg
     *
     * @returns     0 on success, negative error code on failure.
     */
    int get_interrupt_reg(max3133x_int_en_reg_t * int_en_reg);

    /*Interrupt Enable Register Masks*/
    #define A1IE        0b00000001  /*Alarm1 interrupt mask*/
    #define A2IE        0b00000010  /*Alarm2 interrupt mask*/
    #define TIE         0b00000100  /*Timer interrupt mask*/
    #define DIE         0b00001000  /*Digital (DIN) interrupt mask*/
    #define VBATLOWIE   0b00010000  /*VBAT Low Interrupt mask*/
    #define PFAILE      0b00100000  /*Power fail Interrupt mask*/
    #define DOSF        0b01000000  /*Disable oscillator flag*/
    #define INT_ALL     0b01111111  /*All Interrupts*/
    #define NUM_OF_INT    6         /*Number of Interrupts*/

    /**
     * @brief       Enables Interrupts
     *
     * @param[in]   mask
     *
     * @returns     0 on success, negative error code on failure.
     */
    int interrupt_enable(uint8_t mask);

    /**
     * @brief       Disables Interrupts
     *
     * @param[in]   mask
     *
     * @returns     0 on success, negative error code on failure.
     */
    int interrupt_disable(uint8_t mask);

    /**
    * @brief    Put device into reset state
   *
    * @return   0 on success, error code on failure
    */
    int sw_reset_assert();

    /**
    * @brief    Release device from state state
    *
    * @return   0 on success, error code on failure
    */
    int sw_reset_release();

    /**
     * @brief   Resets the digital block and the I2C programmable registers except for RAM registers and RTC_reset.
     *
     * @returns 0 on success, negative error code on failure.
     */
    int sw_reset();

    /**
     * @brief EN_IO Configuration
     *
     * @details
     *  - Register     : RTC_CONFIG1
     *  - Bit Fields   : [6]
     *  - Default      : 0x1
     *  - Description  : Active-high enable DIN pin when running on VBAT
    */
    typedef enum{
        DISABLE_DIN,
        ENABLE_DIN
    }en_io_t;
	
    /**
     * @brief Data retention mode enable/disable Configuration
     *
     * @details
     *  - Register      : RTC_CONFIG1
     *  - Bit Fields    : [2]
     *  - Default       : 0x0
     *  - Description   : Data retention mode enable/disable.
     */
    typedef enum{
        NORMAL_OP_MODE,
        DATA_RETENTION_MODE
    }data_ret_t;

    /**
     * @brief I2C timeout enable Configuration
     *
     * @details
     *  - Register      : RTC_CONFIG1
     *  - Bit Fields    : [1]
     *  - Default       : 0x1
     *  - Description   : I2C timeout enable
     */
    typedef enum{
        DISABLE_I2C_TIMEOUT,
        ENABLE_I2C_TIMEOUT
    }i2c_timeout_t;

    /**
     * @brief Active-high enable for the crystal oscillator Configuration
     *
     * @details
     *  - Register      : RTC_CONFIG1
     *  - Bit Fields    : [0]
     *  - Default       : 0x1
     *  - Description   : Active-high enable for the crystal oscillator
     */
    typedef enum{
        DISABLE_OSCILLATOR,
        ENABLE_OSCILLATOR
    }en_osc_t;

    /**
     * @brief Digital (DIN) pin Sleep Entry Enable Configuration
     *
     * @details
     *  - Register      : RTC_CONFIG2
     *  - Bit Fields    : [4]
     *  - Default       : 0x0
     *  - Description   : Digital (DIN) pin Sleep Entry Enable
     */
    typedef enum{
        DIN_SLEEP_ENTRY_DISABLE,
        DIN_SLEEP_ENTRY_ENABLE
    }dse_t;

    /**
     * @brief Digital (DIN) pin Debounce Enable Configuration
     *
     * @details
     *  - Register      : RTC_CONFIG2
     *  - Bit Fields    : [3]
     *  - Default       : 0x0
     *  - Description   : Digital (DIN) pin Debounce Enable
     */
    typedef enum{
        DIN_DEBOUNCE_DISABLE,
        DIN_DEBOUNCE_ENABLE
    }ddb_t;

    /**
     * @brief CLKOUT enable Configuration
     *
     * @details
     *  - Register      : RTC_CONFIG2
     *  - Bit Fields    : [2]
     *  - Default       : 0x0
     *  - Description   : CLKOUT enable
     */
    typedef enum{
        INTERRUPT,
        CLOCK_OUTPUT
    }enclko_t;

    /**
     * @brief Register Configuration
     *
     * @details
     *  - Register      : RTC_CONFIG1
     *  - Bit Fields    : [5:4]
     *  - Default       : 0x0
     *  - Description   : Alarm1 Auto Clear
     */
    typedef enum{
        BY_READING,     /**< 0x0: Alarm1 flag and interrupt can only be cleared by reading Status register via I2C */
        AFTER_10MS,     /**< 0x1: Alarm1 flag and interrupt are cleared ~10ms after assertion */
        AFTER_500MS,    /**< 0x2: Alarm1 flag and interrupt are cleared ~500ms after assertion */
        AFTER_5s        /**< 0x3: Alarm1 flag and interrupt are cleared ~5s after assertion. This option should not be used when Alarm1 is set to OncePerSec. */
    }a1ac_t;

    /**
     * @brief       Sets Alarm1 Auto Clear Mode
     *
     * @param[in]   a1ac A1AC bits.
     *
     * @returns     0 on success, negative error code on failure.
     */
    int set_alarm1_auto_clear(a1ac_t a1ac);

    /**
     * @brief Digital (DIN) interrupt polarity configuration
     *
     * @details
     *  - Register      : RTC_CONFIG1
     *  - Bit Fields    : [3]
     *  - Default       : 0x0
     *  - Description   : Digital (DIN) interrupt polarity
     */
    typedef enum{
        FALLING_EDGE,   /**< 0x0: Interrupt triggers on falling edge of DIN input. */
        RISING_EDGE     /**< 0x1: Interrupt triggers on rising edge of DIN input. */
    }dip_t;

    /**
     * @brief       Digital (DIN) interrupt polarity
     *
     * @param[in]   dip
     *
     * @returns     0 on success, negative error code on failure.
     */
    int set_din_polarity(dip_t dip);

    /**
    * @brief    Put device into data retention mode
    *
    * @return   0 on success, error code on failure
    */
    int data_retention_mode_enter();

    /**
    * @brief    Remove device from data retention mode
    *
    * @return   0 on success, error code on failure
    */
    int data_retention_mode_exit();

    /**
    * @brief    Enable I2C timeout
    *
    * @return   0 on success, error code on failure
    */
    int i2c_timeout_enable();

    /**
    * @brief    Disable I2C timeout
    *
    * @return   0 on success, error code on failure
    */
    int i2c_timeout_disable();

    /**
    * @brief    Enable the crystal oscillator.
    *
    * @return   0 on success, error code on failure
    */
    int oscillator_enable();

    /**
    * @brief    Disable the crystal oscillator.
    *
    * @return   0 on success, error code on failure
    */
    int oscillator_disable();

    /**
    * @brief    Enable the CLKOUT. Sets INTBb/CLKOUT pin as CLKO (clock output).
    *
    * @return   0 on success, error code on failure
    */
    int clkout_enable();

    /**
    * @brief    Disable the CLKOUT. Sets INTBb/CLKOUT pin as INTBb (interrupt).
    *
    * @return   0 on success, error code on failure
    */
    int clkout_disable();

    /**
     * @brief Set output clock frequency on INTBb/CLKOUT pin Configuration
     *
     * @details
     *  - Register      : RTC_CONFIG2
     *  - Bit Fields    : [1:0]
     *  - Default       : 0x3
     *  - Description   : Output clock frequency on INTBb/CLKOUT pin
     */
    typedef enum{
        CLKOUT_1HZ,
        CLKOUT_64HZ,
        CLKOUT_1024KHZ,
        CLKOUT_32KHZ_UNCOMP
    }clko_hz_t;

    /**
     * @brief       Set output clock frequency on INTBb/CLKOUT pin
     *
     * @param[in]   clko_hz
     *
     * @returns     0 on success, negative error code on failure.
     */
    int set_clko_freq(clko_hz_t    clko_hz);

    /**
     * @brief       Get output clock frequency on INTBb/CLKOUT pin
     *
     * @param[out]  clko_hz
     *
     * @returns     0 on success, negative error code on failure.
     */
    int get_clko_freq(clko_hz_t    *clko_hz);

    /**
     * @brief   Enable the Timestamp Function
     *
     * @returns 0 on success, negative error code on failure.
     */
    int timestamp_function_enable();

    /**
     * @brief   Disable the Timestamp Function
     *
     * @returns 0 on success, negative error code on failure.
     */
    int timestamp_function_disable();

    /**
     * @brief   All Timestamp registers are reset to 0x00. If the Timestamp Function is Enabled, timestamp recording will start again.
     *
     * @returns 0 on success, negative error code on failure.
     */
    int timestamp_registers_reset();

    /**
    * @brief    Enable Timestamp Overwrite mode
    *
    * @details  More than 4 timestamps are recorded by overwriting oldest timestamp. Latest timestamp is always stored in the TS0 bank; earliest timestamp will be stored in the TS3 bank.
    *
    * @return   0 on success, error code on failure
    */
    int timestamp_overwrite_enable();

    /**
    * @brief    Disable Timestamp Overwrite mode
    *
    * @details  Timestamps are recorded (TS0 -> .. -> TS3). Latest timestamp is always stored in the TS0 bank. Further TS trigger events do not record timestamps.
    * 
    * @return   0 on success, error code on failure
    */
    int timestamp_overwrite_disable();

    /*Timestamp Config Register Masks*/
    #define TSVLOW  0b00100000  /*Record Timestamp on VBATLOW detection */
    #define TSPWM   0b00010000  /*Record Timestamp on power supply switch (VCC <-> VBAT)*/
    #define TSDIN   0b00001000  /*Record Timestamp on DIN transition. Polarity controlled by DIP bitfield in RTC_Config1 register.*/

    /**
     * @brief       Enable Timestamp Records
     *
     * @param[in]   record_enable_mask one or more of TS*
     *
     * @returns     0 on success, negative error code on failure.
     */
    int timestamp_record_enable(uint8_t record_enable_mask);

    /**
     * @brief       Disable Timestamp Records
     *
     * @param[in]   record_disable_mask one or more of TS*
     *
     * @returns     0 on success, negative error code on failure.
     */
    int timestamp_record_disable(uint8_t record_disable_mask);

    /**
     * @brief Timer frequency selection Configuration
     *
     * @details
     *  - Register      : TIMER_CONFIG
     *  - Bit Fields    : [1:0]
     *  - Default       : 0x0
     *  - Description   : Timer frequency selection
     */
    typedef enum {
        TIMER_FREQ_1024HZ,  /**< 1024Hz */
        TIMER_FREQ_256HZ,   /**< 256Hz */
        TIMER_FREQ_64HZ,    /**< 64Hz */
        TIMER_FREQ_16HZ,    /**< 16Hz */
    } timer_freq_t;

    /**
    * @brief    Enable timer
    *
    * @return   0 on success, error code on failure
    */
    int timer_start();

    /**
    * @brief    Pause timer, timer value is preserved
    *
    * @return   0 on success, error code on failure
    */
    int timer_pause();

    /**
    * @brief    Start timer from the paused value
    *
    * @return   0 on success, error code on failure
    */
    int timer_continue();

    /**
    * @brief    Disable timer
    *
    * @return   0 on success, error code on failure
    */
    int timer_stop();

    /**
    * @brief    Turn-on the Battery Voltage Detector Function
    *
    * @return   0 on success, error code on failure
    */
    int battery_voltage_detector_enable();

    /**
    * @brief    Turn-off the Battery Voltage Detector Function
    * 
    * @return   0 on success, error code on failure
    */
    int battery_voltage_detector_disable();

    /**
    * @brief Supply voltage select.
    */
    typedef enum {
        POW_MGMT_SUPPLY_SEL_AUTO,   /**< Circuit decides whether to use VCC or VBACKUP */
        POW_MGMT_SUPPLY_SEL_VCC,    /**< Use VCC as supply */
        POW_MGMT_SUPPLY_SEL_VBAT,   /**< Use VBAT as supply */
    } power_mgmt_supply_t;

    /**
    * @brief        Select device power source
    *
    * @param[in]    supply Supply selection, one of POW_MGMT_SUPPLY_SEL_*
    *
    * @return       0 on success, error code on failure
    */
    int supply_select(power_mgmt_supply_t supply);

    /**
    * @brief Selection of charging path's resistor value
    */
    typedef enum {
        TRICKLE_CHARGER_3K,     /**< 3000 Ohm */
        TRICKLE_CHARGER_3K_2,   /**< 3000 Ohm */
        TRICKLE_CHARGER_6K,     /**< 6000 Ohm */
        TRICKLE_CHARGER_11K,    /**< 11000 Ohm */
    } trickle_charger_ohm_t;

    /**
    * @brief        Configure trickle charger charging path, also enable it
    *
    * @param[in]    res Value of resistor
    * @param[in]    diode Enable diode
    *
    * @return       0 on success, error code on failure
    */
    int trickle_charger_enable(trickle_charger_ohm_t res, bool diode);

    /**
    * @brief    Disable Trickle Charger
    *
    * @return   0 on success, error code on failure
    */
    int trickle_charger_disable();

    /**
    * @brief Selection of Timestamp
    */
    typedef enum {
        TS0,        /**< Timestamp 0 */
        TS1,        /**< Timestamp 1 */
        TS2,        /**< Timestamp 2 */
        TS3,        /**< Timestamp 3 */
        NUM_OF_TS   /**< Number of Timestamps */
    } ts_num_t;

    /**
    * @brief Timestamp Triggers
    */
    typedef enum {
        NOT_TRIGGERED,  /**< Not Triggered */
        DINF,           /**< triggered by DIN transition */
        VCCF,           /**< triggered by VBAT -> VCC switch */
        VBATF,          /**< triggered by VCC -> VBAT switch */
        VLOWF,          /**< triggered by VLOW detection */
        NUM_OF_TRIG     /**< Number of Triggers */
    } ts_trigger_t;

    typedef struct{
        ts_num_t     ts_num;
        ts_trigger_t ts_trigger;
        uint16_t     sub_sec;
        struct tm    ctime;
    }timestamp_t;

    /**
    * @brief        Read Timestamp info.
    *
    * @param[in]    ts_num Timestamp number.
    * @param[out]   timestamp Time info.
    *
    * @returns      0 on success, negative error code on failure.
    */
    int get_timestamp(int ts_num, timestamp_t *timestamp);

    /**
    * @brief        correct the clock accuracy on your board. refer the datasheet for additional informations
    *
    * @param[in]    meas Timestamp number.
    *
    * @returns      0 on success, negative error code on failure.
    */
    int offset_configuration(int meas);

    /**
     * @brief   Allow the OSF to indicate the oscillator status.
     *
     * @returns 0 on success, negative error code on failure.
     */
    int oscillator_flag_enable();

    /**
     * @brief   Disable the oscillator flag, irrespective of the oscillator status
     *
     * @returns 0 on success, negative error code on failure.
     */
    int oscillator_flag_disable();

protected:
    typedef struct {
        uint8_t     status_reg_addr;
        uint8_t     int_en_reg_addr;
        uint8_t     status2_reg_addr;
        uint8_t     int_en2_reg_addr;
        uint8_t     rtc_reset_reg_addr;
        uint8_t     rtc_config1_reg_addr;
        uint8_t     rtc_config2_reg_addr;
        uint8_t     timestamp_config_reg_addr;
        uint8_t     timer_config_reg_addr;
        uint8_t     sleep_config_reg_addr;
        uint8_t     seconds_1_128_reg_addr;
        uint8_t     seconds_reg_addr;
        uint8_t     minutes_reg_addr;
        uint8_t     hours_reg_addr;
        uint8_t     day_reg_addr;
        uint8_t     date_reg_addr;
        uint8_t     month_reg_addr;
        uint8_t     year_reg_addr;
        uint8_t     alm1_sec_reg_addr;
        uint8_t     alm1_min_reg_addr;
        uint8_t     alm1_hrs_reg_addr;
        uint8_t     alm1_day_date_reg_addr;
        uint8_t     alm1_mon_reg_addr;
        uint8_t     alm1_year_reg_addr;
        uint8_t     alm2_min_reg_addr;
        uint8_t     alm2_hrs_reg_addr;
        uint8_t     alm2_day_date_reg_addr;
        uint8_t     timer_count_reg_addr;
        uint8_t     timer_count2_reg_addr;
        uint8_t     timer_count1_reg_addr;
        uint8_t     timer_init_reg_addr;
        uint8_t     timer_init2_reg_addr;
        uint8_t     timer_init1_reg_addr;
        uint8_t     pwr_mgmt_reg_addr;
        uint8_t     trickle_reg_addr;
        uint8_t     aging_offset_reg_addr;
        uint8_t     offset_high_reg_addr;
        uint8_t     offset_low_reg_addr;
        uint8_t     ts_config_reg_addr;
        uint8_t     temp_alm_high_msb_reg_addr;
        uint8_t     temp_alm_high_lsb_reg_addr;
        uint8_t     temp_alm_low_msb_reg_addr;
        uint8_t     temp_alm_low_lsb_reg_addr;
        uint8_t     temp_data_msb_reg_addr;
        uint8_t     temp_data_lsb_reg_addr;
        uint8_t     ts0_sec_1_128_reg_addr;
        uint8_t     ts0_sec_reg_addr;
        uint8_t     ts0_min_reg_addr;
        uint8_t     ts0_hour_reg_addr;
        uint8_t     ts0_date_reg_addr;
        uint8_t     ts0_month_reg_addr;
        uint8_t     ts0_year_reg_addr;
        uint8_t     ts0_flags_reg_addr;
        uint8_t     ts1_sec_1_128_reg_addr;
        uint8_t     ts1_sec_reg_addr;
        uint8_t     ts1_min_reg_addr;
        uint8_t     ts1_hour_reg_addr;
        uint8_t     ts1_date_reg_addr;
        uint8_t     ts1_month_reg_addr;
        uint8_t     ts1_year_reg_addr;
        uint8_t     ts1_flags_reg_addr;
        uint8_t     ts2_sec_1_128_reg_addr;
        uint8_t     ts2_sec_reg_addr;
        uint8_t     ts2_min_reg_addr;
        uint8_t     ts2_hour_reg_addr;
        uint8_t     ts2_date_reg_addr;
        uint8_t     ts2_month_reg_addr;
        uint8_t     ts2_year_reg_addr;
        uint8_t     ts2_flags_reg_addr;
        uint8_t     ts3_sec_1_128_reg_addr;
        uint8_t     ts3_sec_reg_addr;
        uint8_t     ts3_min_reg_addr;
        uint8_t     ts3_hour_reg_addr;
        uint8_t     ts3_date_reg_addr;
        uint8_t     ts3_month_reg_addr;
        uint8_t     ts3_year_reg_addr;
        uint8_t     ts3_flags_reg_addr;
    } reg_addr_t;

    /* Constructors */
    MAX3133X(const reg_addr_t *reg_addr, TwoWire *i2c, uint8_t i2c_addr = MAX3133X_I2C_ADDRESS);

private:
    /* PRIVATE TYPE DECLARATIONS */

    /* PRIVATE VARIABLE DECLARATIONS */
    TwoWire *i2c_handler;

    uint8_t  slave_addr;

    /* PRIVATE CONSTANT VARIABLE DECLARATIONS */
    const reg_addr_t *reg_addr;

    /* PRIVATE FUNCTION DECLARATIONS */
    void rtc_regs_to_time(struct tm *time, const max3133x_rtc_time_regs_t *regs, uint16_t *sub_sec);

    int time_to_rtc_regs(max3133x_rtc_time_regs_t *regs, const struct tm *time, hour_format_t format);

    void timestamp_regs_to_time(timestamp_t *timestamp, const max3133x_ts_regs_t *timestamp_reg);

    int time_to_alarm_regs(max3133x_alarm_regs_t &regs, const struct tm *alarm_time, hour_format_t format);

    void alarm_regs_to_time(alarm_no_t alarm_no, struct tm *alarm_time, const max3133x_alarm_regs_t *regs, hour_format_t format);

    int set_alarm_period(alarm_no_t alarm_no, max3133x_alarm_regs_t &regs, alarm_period_t period);

    int set_alarm_regs(alarm_no_t alarm_no, const max3133x_alarm_regs_t *regs);

    void to_12hr(uint8_t hr, uint8_t *hr_12, uint8_t *pm);

    int get_rtc_time_format(hour_format_t *format);

    int data_retention_mode_config(bool enable);

    int battery_voltage_detector_config(bool enable);

    int clkout_config(bool enable);

    int i2c_timeout_config(bool enable);

    int oscillator_config(bool enable);

    int timestamp_overwrite_config(bool enable);

    int oscillator_flag_config(bool enable);

    int8_t hours_reg_to_hour(const max3133x_hours_reg_t *hours_reg);

    /**
    * @brief    Interrupt handler function
    */
    void interrupt_handler();

    /**
    * @brief Post interrupt jobs after interrupt is detected.
    */
    void post_interrupt_work();

    struct handler {
        void (*func)(void *);
        void *cb;
    };

    handler interrupt_handler_list[NUM_OF_INT];
};

/** MAX31335 Device Class
*
* Hold configurations for the MAX31335
*/
class MAX31335 : public MAX3133X
{
private:
    static const reg_addr_t reg_addr;

public:
    typedef struct{
        en_io_t         en_io;      /*RTC_CONFIG1 - DIN pin enable when running on VBAT*/
        a1ac_t          a1ac;       /*RTC_CONFIG1 - Alarm1 Auto Clear */
        dip_t           dip;        /*RTC_CONFIG1 - Digital (DIN) interrupt polarity */
        data_ret_t      data_ret;   /*RTC_CONFIG1 - Data retention mode enable/disable. */
        i2c_timeout_t   i2c_timeout;/*RTC_CONFIG1 - I2C timeout enable */
        en_osc_t        en_osc;     /*RTC_CONFIG1 - Active-high enable for the crystal oscillator */
        enclko_t        enclko;     /*RTC_CONFIG2 - CLKOUT enable */
        clko_hz_t       clko_hz;    /*RTC_CONFIG2 - Set output clock frequency on INTBb/CLKOUT pin */
    }rtc_config_t;

    /**
    * @brief        Configure the device
    *
    * @param[in]    max31335_config Device configuration
    *
    * @return       0 on success, error code on failure
    *
    * @note         RTC_CONFIG1 and RTC_CONFIG2 registers are set.
    */
    int rtc_config(rtc_config_t *max31335_config);

    /**
    * @brief        Get device configuration
    *
    * @param[out]   max31335_config Device configuration
    *
    * @return       0 on success, error code on failure
    *
    * @note         RTC_CONFIG1 and RTC_CONFIG2 register values are read.
    */
    int get_rtc_config(rtc_config_t *max31335_config);

    /**
    * @brief        Initialize timer
    *
    * @param[in]    init_val Timer initial value
    * @param[in]    repeat Timer repeat mode enable/disable
    * @param[in]    freq Timer frequency, one of TIMER_FREQ_*
    *
    * @return       0 on success, error code on failure
    */
    int timer_init(uint16_t init_val, bool repeat, timer_freq_t freq);

    /**
    * @brief    Read timer value
    *
    * @return   timer value on success, error code on failure
    */
    int timer_get();
    /**
     * @brief       Gets Status2 Register Value
     *
     * @param[in]   status_reg
     *
     * @returns     0 on success, negative error code on failure.
     */
    int get_status2_reg(max31335_status2_reg_t * status_reg);

    /**
     * @brief       Gets Interrupt Enable2 Register Value
     *
     * @param[in]   int_en_reg
     *
     * @returns     0 on success, negative error code on failure.
     */
    int get_interrupt2_reg(max31335_int_en2_reg_t * int_en_reg);
    
    /*Interrupt Enable1 Register Masks*/
    #define UTF         0b00000001  /*Under Temperature interrupt mask*/
    #define OTF         0b00000010  /*Over Temperature interrupt mask*/
    #define TEMP_RDY    0b00000100  /*Temperature Ready interrupt mask*/
    #define INT1_ALL    0b00000111  /*All Interrupts*/
    #define NUM_OF_INT1 3           /*Number of Interrupts*/

    /**
     * @brief       Enables Interrupt1
     *
     * @param[in]   mask
     *
     * @returns     0 on success, negative error code on failure.
     */
    int interrupt2_enable(uint8_t mask);

    /**
     * @brief       Disables Interrupt1
     *
     * @param[in]   mask
     *
     * @returns     0 on success, negative error code on failure.
     */
    int interrupt2_disable(uint8_t mask);

    MAX31335(TwoWire *i2c, uint8_t i2c_addr = MAX31335_I2C_ADDRESS) : MAX3133X(&reg_addr, i2c, i2c_addr) {}
};

/** MAX31334 Device Class
*
* Hold configurations for the MAX31334
*/
class MAX31334 : public MAX3133X
{
private:
    static const reg_addr_t reg_addr;

    int din_sleep_entry_config(bool enable);

    int din_pin_debounce_config(bool enable);

public:
    typedef struct{
        a1ac_t          a1ac;       /*RTC_CONFIG1 - Alarm1 Auto Clear */
        dip_t           dip;        /*RTC_CONFIG1 - Digital (DIN) interrupt polarity */
        data_ret_t      data_ret;   /*RTC_CONFIG1 - Data retention mode enable/disable. */
        i2c_timeout_t   i2c_timeout;/*RTC_CONFIG1 - I2C timeout enable */
        en_osc_t        en_osc;     /*RTC_CONFIG1 - Active-high enable for the crystal oscillator */
        dse_t           dse;        /*RTC_CONFIG2 - Digital (DIN) pin Sleep Entry Enable */
        ddb_t           ddb;        /*RTC_CONFIG2 - Digital (DIN) pin Debounce Enable */
        enclko_t        enclko;     /*RTC_CONFIG2 - CLKOUT enable */
        clko_hz_t       clko_hz;    /*RTC_CONFIG2 - Set output clock frequency on INTBb/CLKOUT pin */
    }rtc_config_t;

    /**
    * @brief        Configure the device
    *
    * @param[in]    max31334_config Device configuration
    *
    * @return       0 on success, error code on failure
    *
    * @note         RTC_CONFIG1 and RTC_CONFIG2 registers are set.
    */
    int rtc_config(rtc_config_t *max31334_config);

    /**
    * @brief        Get device configuration
    *
    * @param[out]   max31334_config Device configuration
    *
    * @return       0 on success, error code on failure
    *
    * @note         RTC_CONFIG1 and RTC_CONFIG2 register values are read.
    */
    int get_rtc_config(rtc_config_t *max31334_config);

    /**
    * @brief        Initialize timer
    *
    * @param[in]    init_val Timer initial value
    * @param[in]    repeat Timer repeat mode enable/disable
    * @param[in]    freq Timer frequency, one of TIMER_FREQ_*
    *
    * @return       0 on success, error code on failure
    */
    int timer_init(uint16_t init_val, bool repeat, timer_freq_t freq);

    /**
    * @brief    Read timer value
    *
    * @return   timer value on success, error code on failure
    */
    int timer_get();

    /**
    * @brief    Get Sleep State
    *
    * @return   Sleep State. 0: SLST=0 indicates the PSW SM is not in Sleep state,
    *                        1: SLST=1 indicates the PSW SM is in Sleep state.
    *                 negative: on failure
    */
    int get_sleep_state();

    /**
    * @brief    Enable the Digital (DIN) pin Sleep Entry
    *
    * @details  DIN pin can be used to enter sleep state.
    *
    * @return   0 on success, error code on failure
    */
    int din_sleep_entry_enable();

    /**
    * @brief    Disable the Digital (DIN) pin Sleep Entry
    *
    * @details  DIN pin cannot be used to enter sleep state (Sleep state entry is only possible by writing SLP=1 over I2C).
    *
    * @return   0 on success, error code on failure
    */
    int din_sleep_entry_disable();

    /**
    * @brief    Enable the Digital (DIN) pin Debounce function
    *
    * @details  50ms debounce on DIN pin enabled.
    *
    * @return   0 on success, error code on failure
    */
    int din_pin_debounce_enable();

    /**
    * @brief    Disable the Digital (DIN) pin Debounce function
    *
    * @details  No debounce on DIN pin.
    *
    * @return   0 on success, error code on failure
    */
    int din_pin_debounce_disable();

    /**
    * @brief    Put PSW SM into Sleep state.
    *
    * @return   0 on success, error code on failure
    */
    int sleep_enter();

    /**
    * @brief    Put PSW SM into Active state.
    *
    * @return   0 on success, error code on failure
    */
    int sleep_exit();

    /**
     * @brief Wait State Timeout Configuration
     *
     * @details
     *  - Register      : SLEEP_CONFIG
     *  - Bit Fields    : [6:4]
     *  - Default       : 0x0
     *  - Description   : Wait State Timeout. This bitfield must be set before writing SLP=1 if a finite wait state duration is
     *                    desired before entering the sleep state.
     */
    typedef enum {
        WSTO_0MS,   /**< 0ms */
        WSTO_8MS,   /**< 8ms */
        WSTO_16MS,  /**< 16ms */
        WSTO_24MS,  /**< 24ms */
        WSTO_32MS,  /**< 32ms */
        WSTO_40MS,  /**< 40ms */
        WSTO_48MS,  /**< 48ms */
        WSTO_56MS   /**< 56ms */
    } wsto_t;

    /**
    * @brief        Set Wait State Timeout
    *
    * @param[in]    wsto Wait State Timeout
    *
    * @return       0 on success, error code on failure
    */
    int set_wait_state_timeout(wsto_t wsto);

    /**
    * @brief        Get Wait State Timeout
    *
    * @param[out]   wsto Wait State Timeout
    *
    * @return       0 on success, error code on failure
    */
    int get_wait_state_timeout(wsto_t* wsto);

    /*Sleep Config Register Masks*/
    #define A1WE      0b00000001  /*Alarm1 Wakeup Enable */
    #define A2WE      0b00000010  /*Alarm2 Wakeup Enable */
    #define TWE       0b00000100  /*Timer Wakeup Enable */
    #define DWE       0b00001000  /*DIN Wakeup Enable */

    /**
     * @brief       Enable Wakeup
     *
     * @param[in]   wakeup_enable_mask one or more of Sleep Config Register Masks
     *
     * @returns     0 on success, negative error code on failure.
     */
    int wakeup_enable(uint8_t wakeup_enable_mask);

    /**
     * @brief       Disable Wakeup
     *
     * @param[in]   wakeup_disable_mask one or more of Sleep Config Register Masks
     *
     * @returns     0 on success, negative error code on failure.
     */
    int wakeup_disable(uint8_t wakeup_disable_mask);

    MAX31334(TwoWire *i2c, uint8_t i2c_addr = MAX3133X_I2C_ADDRESS) : MAX3133X(&reg_addr, i2c, i2c_addr) {}
};

/** MAX31331 Device Class
*
* Hold configurations for the MAX31331
*/
class MAX31331 : public MAX3133X
{
private:
    static const reg_addr_t reg_addr;

public:
    typedef struct{
        a1ac_t          a1ac;       /*RTC_CONFIG1 - Alarm1 Auto Clear */
        dip_t           dip;        /*RTC_CONFIG1 - Digital (DIN) interrupt polarity */
        data_ret_t      data_ret;   /*RTC_CONFIG1 - Data retention mode enable/disable. */
        i2c_timeout_t   i2c_timeout;/*RTC_CONFIG1 - I2C timeout enable */
        en_osc_t        en_osc;     /*RTC_CONFIG1 - Active-high enable for the crystal oscillator */
        enclko_t        enclko;     /*RTC_CONFIG2 - CLKOUT enable */
        clko_hz_t       clko_hz;    /*RTC_CONFIG2 - Set output clock frequency on INTBb/CLKOUT pin */
    }rtc_config_t;

    /**
    * @brief        Configure the device
    *
    * @param[in]    max31331_config Device configuration
    *
    * @return       0 on success, error code on failure
    *
    * @note         RTC_CONFIG1 and RTC_CONFIG2 registers are set.
    */
    int rtc_config(rtc_config_t *max31331_config);

    /**
    * @brief        Get device configuration
    *
    * @param[out]   max31331_config Device configuration
    *
    * @return       0 on success, error code on failure
    *
    * @note         RTC_CONFIG1 and RTC_CONFIG2 register values are read.
    */
    int get_rtc_config(rtc_config_t *max31331_config);

    /**
    * @brief        Initialize timer
    *
    * @param[in]    init_val Timer initial value
    * @param[in]    repeat Timer repeat mode enable/disable
    * @param[in]    freq Timer frequency, one of TIMER_FREQ_*
    *
    * @return       0 on success, error code on failure
    */
    int timer_init(uint8_t  init_val, bool repeat, timer_freq_t freq);

    /**
    * @brief    Read timer value
    *
    * @return   timer value on success, error code on failure
    */
    int timer_get();

    MAX31331(TwoWire *i2c, uint8_t i2c_addr = MAX3133X_I2C_ADDRESS) : MAX3133X(&reg_addr, i2c, i2c_addr) {}
};

#endif /* MAX3133X_HPP_ */