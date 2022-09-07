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

#ifndef MAX3133X_REGS_HPP_
#define MAX3133X_REGS_HPP_

#define MAX3133X_I2C_ADDRESS    0x68
#define REG_NOT_AVAILABLE       0xFF

/**
 * @brief STATUS Register
 */
typedef union {
    unsigned char raw;
    struct {
        unsigned char a1f       : 1;
        unsigned char a2f       : 1;
        unsigned char tif       : 1;
        unsigned char dif       : 1;
        unsigned char vbatlow   : 1;
        unsigned char pfail     : 1;
        unsigned char osf       : 1;
        unsigned char psdect    : 1;
    } bits;
} max3133x_status_reg_t;

/**
 * @brief ENT_EN Register
 */
typedef union {
    unsigned char raw;
    struct {
        unsigned char a1ie      : 1;
        unsigned char a2ie      : 1;
        unsigned char tie       : 1;
        unsigned char die       : 1;
        unsigned char vbatlowie : 1;
        unsigned char pfaile    : 1;
        unsigned char dosf      : 1;
        unsigned char           : 1;
    } bits;
} max3133x_int_en_reg_t;

/**
 * @brief RTC_RESET Register
 */
typedef union {
    unsigned char raw;
    struct {
        unsigned char swrst     : 1;
        unsigned char           : 7;
    } bits;
} max3133x_rtc_reset_reg_t;

/**
 * @brief RTC_CONFIG1 Register
 */
typedef union {
    unsigned char raw;
    struct {
        unsigned char en_osc        : 1;
        unsigned char i2c_timeout   : 1;
        unsigned char data_ret      : 1;
        unsigned char dip           : 1;
        unsigned char a1ac          : 2;
        unsigned char               : 2;
    } bits;
} max3133x_rtc_config1_reg_t;

/**
 * @brief RTC_CONFIG2 Register
 */
typedef union {
    unsigned char raw;
    struct {
        unsigned char clko_hz   : 2;
        unsigned char enclko    : 1;
        unsigned char           : 5;
    } bits;
} max31331_rtc_config2_reg_t;

/**
 * @brief RTC_CONFIG2 Register
 */
typedef union {
    unsigned char raw;
    struct {
        unsigned char clko_hz   : 2;
        unsigned char enclko    : 1;
        unsigned char ddb       : 1;
        unsigned char dse       : 1;
        unsigned char           : 2;
        unsigned char slst      : 1;
    } bits;
} max31334_rtc_config2_reg_t;

/**
 * @brief TIMESTAMP_CONFIG Register
 */
typedef union {
    unsigned char raw;
    struct {
        unsigned char tse       : 1;
        unsigned char tsr       : 1;
        unsigned char tsow      : 1;
        unsigned char tsdin     : 1;
        unsigned char tspwm     : 1;
        unsigned char tsvlow    : 1;
        unsigned char           : 2;
    } bits;
} max3133x_timestamp_config_reg_t;

/**
 * @brief TIMER_CONFIG Register
 */
typedef union {
    unsigned char raw;
    struct {
        unsigned char tfs       : 2;
        unsigned char trpt      : 1;
        unsigned char tpause    : 1;
        unsigned char te        : 1;
        unsigned char           : 3;

    } bits;
} max3133x_timer_config_reg_t;

/**
 * @brief SLEEP_CONFIG Register
 */
typedef union {
    unsigned char raw;
    struct {
        unsigned char a1we      : 1;
        unsigned char a2we      : 1;
        unsigned char twe       : 1;
        unsigned char dwe       : 1;
        unsigned char wsto      : 3;
        unsigned char slp       : 1;
    } bits;
} max31334_sleep_config_reg_t;

/**
 * @brief SECONDS_1_128 Register
 */
typedef union {
    unsigned char raw;
    struct {
        unsigned char _1_128s   : 1;
        unsigned char _1_64s    : 1;
        unsigned char _1_32s    : 1;
        unsigned char _1_16s    : 1;
        unsigned char _1_8s     : 1;
        unsigned char _1_4s     : 1;
        unsigned char _1_2s     : 1;
        unsigned char           : 1;
    } bits;
} max3133x_seconds_1_128_reg_t;

/**
 * @brief SECONDS Register
 */
typedef union {
    unsigned char raw;
    struct {
        unsigned char seconds   : 4;
        unsigned char sec_10    : 3;
        unsigned char           : 1;
    } bits;
    struct {
        unsigned char value     : 7;
        unsigned char           : 1;
    } bcd;
} max3133x_seconds_reg_t;

/**
 * @brief MINUTES Register
 */
typedef union {
    unsigned char raw;
    struct {
        unsigned char minutes   : 4;
        unsigned char min_10    : 3;
        unsigned char           : 1;
    } bits;
    struct {
        unsigned char value     : 7;
        unsigned char           : 1;
    } bcd;
} max3133x_minutes_reg_t;

/**
 * @brief HOURS Register
 */
typedef union {
    unsigned char raw;
    struct {
        unsigned char hour      : 4;
        unsigned char hr_10     : 1;
        unsigned char am_pm     : 1;
        unsigned char f_24_12   : 1;
        unsigned char           : 1;
    } bits_12hr;
    struct {
        unsigned char value     : 5;
        unsigned char           : 3;
    } bcd_12hr;
    struct {
        unsigned char hour      : 4;
        unsigned char hr_10     : 2;
        unsigned char f_24_12   : 1;
        unsigned char           : 1;
    } bits_24hr;
    struct {
        unsigned char value     : 6;
        unsigned char           : 2;
    } bcd_24hr;
} max3133x_hours_reg_t;

/**
 * @brief DAY Register
 */
typedef union {
    unsigned char raw;
    struct {
        unsigned char day       : 3;
        unsigned char           : 5;
    } bits;
    struct {
        unsigned char value     : 3;
        unsigned char           : 5;
    } bcd;
} max3133x_day_reg_t;

/**
 * @brief DATE Register
 */
typedef union {
    unsigned char raw;
    struct {
        unsigned char date      : 4;
        unsigned char date_10   : 2;
        unsigned char           : 2;
    } bits;
    struct {
        unsigned char value     : 6;
        unsigned char           : 2;
    } bcd;
} max3133x_date_reg_t;

/**
 * @brief MONTH Register
 */
typedef union {
    unsigned char raw;
    struct {
        unsigned char month     : 4;
        unsigned char month_10  : 1;
        unsigned char           : 2;
        unsigned char century   : 1;
    } bits;
    struct {
        unsigned char value     : 5;
        unsigned char           : 3;
    } bcd;
} max3133x_month_reg_t;

/**
 * @brief YEAR Register
 */
typedef union {
    unsigned char raw;
    struct {
        unsigned char year      : 4;
        unsigned char year_10   : 4;
    } bits;
    struct {
        unsigned char value     : 8;
    } bcd;
} max3133x_year_reg_t;

/**
 * @brief ALM_SEC Register
 */
typedef union {
    unsigned char raw;
    struct {
        unsigned char seconds   : 4;
        unsigned char sec_10    : 3;
        unsigned char am1       : 1;
    } bits;
    struct {
        unsigned char value     : 7;
        unsigned char           : 1;
    } bcd;
} max3133x_alm_sec_reg_t;

/**
 * @brief ALM_MIN Register
 */
typedef union {
    unsigned char raw;
    struct {
        unsigned char minutes   : 4;
        unsigned char min_10    : 3;
        unsigned char am2       : 1;
    } bits;
    struct {
        unsigned char value     : 7;
        unsigned char           : 1;
    } bcd;
} max3133x_alm_min_reg_t;

/**
 * @brief ALM_HRS Register
 */
typedef union {
    unsigned char raw;
    struct {
        unsigned char hour      : 4;
        unsigned char hr_10     : 1;
        unsigned char am_pm     : 1;
        unsigned char           : 1;
        unsigned char am3       : 1;
    } bits_12hr;
    struct {
        unsigned char value     : 5;
        unsigned char           : 3;
    } bcd_12hr;
    struct {
        unsigned char hour      : 4;
        unsigned char hr_10     : 2;
        unsigned char           : 1;
        unsigned char am3       : 1;
    } bits_24hr;
    struct {
        unsigned char value     : 6;
        unsigned char           : 2;
    } bcd_24hr;
} max3133x_alm_hrs_reg_t;

/**
 * @brief ALM_DAY_DATE Register
 */
typedef union {
    unsigned char raw;
    struct {
        unsigned char day_date      : 4;
        unsigned char date_10       : 2;
        unsigned char dy_dt_match   : 1;
        unsigned char am4           : 1;
    } bits;
    struct {
        unsigned char value         : 3;
        unsigned char               : 5;
    } bcd_day;
    struct {
        unsigned char value         : 6;
        unsigned char               : 2;
    } bcd_date;
} max3133x_alm_day_date_reg_t;

/**
 * @brief ALM_MON Register
 */
typedef union {
    unsigned char raw;
    struct {
        unsigned char month     : 4;
        unsigned char month_10  : 1;
        unsigned char           : 1;
        unsigned char am6       : 1;
        unsigned char am5       : 1;
    } bits;
    struct {
        unsigned char value     : 5;
        unsigned char           : 3;
    } bcd;
} max3133x_alm_mon_reg_t;

/**
 * @brief ALM_YEAR Register
 */
typedef union {
    unsigned char raw;
    struct {
        unsigned char year      : 4;
        unsigned char year_10   : 4;
    } bits;
    struct {
        unsigned char value     : 8;
    } bcd;
} max3133x_alm_year_reg_t;

/**
 * @brief PWR_MGMT Register
 */
typedef union {
    unsigned char raw;
    struct {
        unsigned char manual_sel        : 1;
        unsigned char vback_sel         : 1;
        unsigned char                   : 1;
        unsigned char en_vbat_detect    : 1;
        unsigned char                   : 4;
    } bits;
} max3133x_pwr_mgmt_reg_t;

/**
 * @brief TRICKLE_REG Register
 */
typedef union {
    unsigned char raw;
    struct {
        unsigned char en_trickle    : 1;
        unsigned char trickle       : 3;
        unsigned char               : 4;
    } bits;
} max3133x_trickle_reg_reg_t;

/**
 * @brief OFFSET_HIGH Register
 */
typedef union {
    unsigned char raw;
    struct {
        unsigned char compword;
    } bits;
} max3133x_offset_high_reg_t;

/**
 * @brief OFFSET_LOW Register
 */
typedef union {
    unsigned char raw;
    struct {
        unsigned char compword;
    } bits;
} max3133x_offset_low_reg_t;

/**
 * @brief TS_FLAGS Register
 */
typedef union {
    unsigned char raw;
    struct {
        unsigned char dinf      : 1;
        unsigned char vccf      : 1;
        unsigned char vbatf     : 1;
        unsigned char vlowf     : 1;
        unsigned char           : 4;
    } bits;
} max3133x_ts_flags_reg_t;

typedef struct {
    max3133x_seconds_1_128_reg_t    seconds_1_128_reg;
    max3133x_seconds_reg_t          seconds_reg;
    max3133x_minutes_reg_t          minutes_reg;
    max3133x_hours_reg_t            hours_reg;
    max3133x_day_reg_t              day_reg;
    max3133x_date_reg_t             date_reg;
    max3133x_month_reg_t            month_reg;
    max3133x_year_reg_t             year_reg;
} max3133x_rtc_time_regs_t;

typedef struct{
    max3133x_alm_sec_reg_t      sec;
    max3133x_alm_min_reg_t      min;
    max3133x_alm_hrs_reg_t      hrs;
    max3133x_alm_day_date_reg_t day_date;
    max3133x_alm_mon_reg_t      mon;
    max3133x_alm_year_reg_t     year;
}max3133x_alarm_regs_t;

typedef struct {
    max3133x_seconds_1_128_reg_t    ts_sec_1_128_reg;
    max3133x_seconds_reg_t          ts_sec_reg;
    max3133x_minutes_reg_t          ts_min_reg;
    max3133x_hours_reg_t            ts_hour_reg;
    max3133x_date_reg_t             ts_date_reg;
    max3133x_month_reg_t            ts_month_reg;
    max3133x_year_reg_t             ts_year_reg;
    max3133x_ts_flags_reg_t         ts_flags_reg;
}max3133x_ts_regs_t;

enum max31331_register_address{
    /*RTC REG*/
    MAX31331_STATUS             = 0x00,
    MAX31331_INT_EN             = 0x01,
    MAX31331_RTC_RESET          = 0x02,
    MAX31331_RTC_CONFIG1        = 0x03,
    MAX31331_RTC_CONFIG2        = 0x04,
    MAX31331_TIMESTAMP_CONFIG   = 0x05,
    MAX31331_TIMER_CONFIG       = 0x06,
    MAX31331_SECONDS_1_128      = 0x07,
    MAX31331_SECONDS            = 0x08,
    MAX31331_MINUTES            = 0x09,
    MAX31331_HOURS              = 0x0A,
    MAX31331_DAY                = 0x0B,
    MAX31331_DATE               = 0x0C,
    MAX31331_MONTH              = 0x0D,
    MAX31331_YEAR               = 0x0E,
    MAX31331_ALM1_SEC           = 0x0F,
    MAX31331_ALM1_MIN           = 0x10,
    MAX31331_ALM1_HRS           = 0x11,
    MAX31331_ALM1_DAY_DATE      = 0x12,
    MAX31331_ALM1_MON           = 0x13,
    MAX31331_ALM1_YEAR          = 0x14,
    MAX31331_ALM2_MIN           = 0x15,
    MAX31331_ALM2_HRS           = 0x16,
    MAX31331_ALM2_DAY_DATE      = 0x17,
    MAX31331_TIMER_COUNT        = 0x18,
    MAX31331_TIMER_INIT         = 0x19,
    MAX31331_PWR_MGMT           = 0x1A,
    MAX31331_TRICKLE_REG        = 0x1B,
    MAX31331_OFFSET_HIGH        = 0x1D,
    MAX31331_OFFSET_LOW         = 0x1E,
    /*TS_RAM_REG*/
    MAX31331_TS0_SEC_1_128      = 0x20,
    MAX31331_TS0_SEC            = 0x21,
    MAX31331_TS0_MIN            = 0x22,
    MAX31331_TS0_HOUR           = 0x23,
    MAX31331_TS0_DATE           = 0x24,
    MAX31331_TS0_MONTH          = 0x25,
    MAX31331_TS0_YEAR           = 0x26,
    MAX31331_TS0_FLAGS          = 0x27,
    MAX31331_TS1_SEC_1_128      = 0x28,
    MAX31331_TS1_SEC            = 0x29,
    MAX31331_TS1_MIN            = 0x2A,
    MAX31331_TS1_HOUR           = 0x2B,
    MAX31331_TS1_DATE           = 0x2C,
    MAX31331_TS1_MONTH          = 0x2D,
    MAX31331_TS1_YEAR           = 0x2E,
    MAX31331_TS1_FLAGS          = 0x2F,
    MAX31331_TS2_SEC_1_128      = 0x30,
    MAX31331_TS2_SEC            = 0x31,
    MAX31331_TS2_MIN            = 0x32,
    MAX31331_TS2_HOUR           = 0x33,
    MAX31331_TS2_DATE           = 0x34,
    MAX31331_TS2_MONTH          = 0x35,
    MAX31331_TS2_YEAR           = 0x36,
    MAX31331_TS2_FLAGS          = 0x37,
    MAX31331_TS3_SEC_1_128      = 0x38,
    MAX31331_TS3_SEC            = 0x39,
    MAX31331_TS3_MIN            = 0x3A,
    MAX31331_TS3_HOUR           = 0x3B,
    MAX31331_TS3_DATE           = 0x3C,
    MAX31331_TS3_MONTH          = 0x3D,
    MAX31331_TS3_YEAR           = 0x3E,
    MAX31331_TS3_FLAGS          = 0x3F,
    MAX31331_END
};

enum max31334_register_address{
    /*RTC REG*/
    MAX31334_STATUS             = 0x00,
    MAX31334_INT_EN             = 0x01,
    MAX31334_RTC_RESET          = 0x02,
    MAX31334_RTC_CONFIG1        = 0x03,
    MAX31334_RTC_CONFIG2        = 0x04,
    MAX31334_TIMESTAMP_CONFIG   = 0x05,
    MAX31334_TIMER_CONFIG       = 0x06,
    MAX31334_SLEEP_CONFIG       = 0x07,
    MAX31334_SECONDS_1_128      = 0x08,
    MAX31334_SECONDS            = 0x09,
    MAX31334_MINUTES            = 0x0A,
    MAX31334_HOURS              = 0x0B,
    MAX31334_DAY                = 0x0C,
    MAX31334_DATE               = 0x0D,
    MAX31334_MONTH              = 0x0E,
    MAX31334_YEAR               = 0x0F,
    MAX31334_ALM1_SEC           = 0x10,
    MAX31334_ALM1_MIN           = 0x11,
    MAX31334_ALM1_HRS           = 0x12,
    MAX31334_ALM1_DAY_DATE      = 0x13,
    MAX31334_ALM1_MON           = 0x14,
    MAX31334_ALM1_YEAR          = 0x15,
    MAX31334_ALM2_MIN           = 0x16,
    MAX31334_ALM2_HRS           = 0x17,
    MAX31334_ALM2_DAY_DATE      = 0x18,
    MAX31334_TIMER_COUNT2       = 0x19,
    MAX31334_TIMER_COUNT1       = 0x1A,
    MAX31334_TIMER_INIT2        = 0x1B,
    MAX31334_TIMER_INIT1        = 0x1C,
    MAX31334_PWR_MGMT           = 0x1D,
    MAX31334_TRICKLE_REG        = 0x1E,
    MAX31334_OFFSET_HIGH        = 0x20,
    MAX31334_OFFSET_LOW         = 0x21,
    /*TS_RAM_REG*/
    MAX31334_TS0_SEC_1_128      = 0x30,
    MAX31334_TS0_SEC            = 0x31,
    MAX31334_TS0_MIN            = 0x32,
    MAX31334_TS0_HOUR           = 0x33,
    MAX31334_TS0_DATE           = 0x34,
    MAX31334_TS0_MONTH          = 0x35,
    MAX31334_TS0_YEAR           = 0x36,
    MAX31334_TS0_FLAGS          = 0x37,
    MAX31334_TS1_SEC_1_128      = 0x38,
    MAX31334_TS1_SEC            = 0x39,
    MAX31334_TS1_MIN            = 0x3A,
    MAX31334_TS1_HOUR           = 0x3B,
    MAX31334_TS1_DATE           = 0x3C,
    MAX31334_TS1_MONTH          = 0x3D,
    MAX31334_TS1_YEAR           = 0x3E,
    MAX31334_TS1_FLAGS          = 0x3F,
    MAX31334_TS2_SEC_1_128      = 0x40,
    MAX31334_TS2_SEC            = 0x41,
    MAX31334_TS2_MIN            = 0x42,
    MAX31334_TS2_HOUR           = 0x43,
    MAX31334_TS2_DATE           = 0x44,
    MAX31334_TS2_MONTH          = 0x45,
    MAX31334_TS2_YEAR           = 0x46,
    MAX31334_TS2_FLAGS          = 0x47,
    MAX31334_TS3_SEC_1_128      = 0x48,
    MAX31334_TS3_SEC            = 0x49,
    MAX31334_TS3_MIN            = 0x4A,
    MAX31334_TS3_HOUR           = 0x4B,
    MAX31334_TS3_DATE           = 0x4C,
    MAX31334_TS3_MONTH          = 0x4D,
    MAX31334_TS3_YEAR           = 0x4E,
    MAX31334_TS3_FLAGS          = 0x4F,
    MAX31334_END
};

#endif /* MAX3133X_REGS_HPP_ */
