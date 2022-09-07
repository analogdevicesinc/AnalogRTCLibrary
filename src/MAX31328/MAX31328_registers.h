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


#ifndef _MAX31328_REGISTERS_H_
#define _MAX31328_REGISTERS_H_


#define MAX3128_I2C_ADDRESS             0x68

#define MAX31328_R_SECONDS             ( 0x00 )
#define MAX31328_R_MINUTES             ( 0x01 )
#define MAX31328_R_HOURS               ( 0x02 )
#define MAX31328_R_DAY                 ( 0x03 )
#define MAX31328_R_DATE                ( 0x04 )
#define MAX31328_R_MONTH               ( 0x05 )
#define MAX31328_R_YEAR                ( 0x06 )
#define MAX31328_R_ALRM1_SECONDS       ( 0x07 )
#define MAX31328_R_ALRM1_MINUTES       ( 0x08 )
#define MAX31328_R_ALRM1_HOURS         ( 0x09 )
#define MAX31328_R_ALRM1_DAY_DATE      ( 0x0A )
#define MAX31328_R_ALRM2_MINUTES       ( 0x0B )
#define MAX31328_R_ALRM2_HOURS         ( 0x0C )
#define MAX31328_R_ALRM2_DAY_DATE      ( 0x0D )
#define MAX31328_R_CONTROL             ( 0x0E )
#define MAX31328_R_STATUS              ( 0x0F )
#define MAX31328_R_AGING_OFFSET        ( 0x10 )
#define MAX31328_R_MSB_TEMP            ( 0x11 )
#define MAX31328_R_LSB_TEMP            ( 0x12 )


//control register bit masks
#define MAX31328_F_CTRL_A1IE_POS        0    
#define MAX31328_F_CTRL_A1IE            (1<<MAX31328_F_CTRL_A1IE_POS)
#define MAX31328_F_CTRL_A2IE_POS        1    
#define MAX31328_F_CTRL_A2IE            (1<<MAX31328_F_CTRL_A2IE_POS)
#define MAX31328_F_CTRL_INTCN_POS       2  
#define MAX31328_F_CTRL_INTCN           (1<<MAX31328_F_CTRL_INTCN_POS)
#define MAX31328_F_CTRL_RS_POS          3    
#define MAX31328_F_CTRL_RS              (3<<MAX31328_F_CTRL_RS_POS)
#define MAX31328_F_CTRL_CONV_POS        5    
#define MAX31328_F_CTRL_CONV            (1<<MAX31328_F_CTRL_CONV_POS)
#define MAX31328_F_CTRL_BBSQW_POS       6    
#define MAX31328_F_CTRL_BBSQW           (1<<MAX31328_F_CTRL_BBSQW_POS)
#define MAX31328_F_CTRL_EOSC_POS        7    
#define MAX31328_F_CTRL_EOSC            (1<<MAX31328_F_CTRL_EOSC_POS)

//status register bit masks
#define MAX31328_F_STATUS_A1F_POS       0
#define MAX31328_F_STATUS_A1F           (1<<MAX31328_F_STATUS_A1F_POS)
#define MAX31328_F_STATUS_A2F_POS       1
#define MAX31328_F_STATUS_A2F           (1<<MAX31328_F_STATUS_A2F_POS)
#define MAX31328_F_STATUS_BSY_POS       2
#define MAX31328_F_STATUS_BSY           (1<<MAX31328_F_STATUS_BSY_POS)
#define MAX31328_F_STATUS_EN32KHZ_POS   3
#define MAX31328_F_STATUS_EN32KHZ       (1<<MAX31328_F_STATUS_EN32KHZ_POS)
#define MAX31328_F_STATUS_OSF_POS       7
#define MAX31328_F_STATUS_OSF           (1<<MAX31328_F_STATUS_OSF_POS)


// HOURS
#define MAX31328_F_HOURS_AM_PM_POS      5
#define MAX31328_F_HOURS_AM_PM          (1<<MAX31328_F_HOURS_AM_PM_POS)
#define MAX31328_F_HOURS_F24_12_POS     6
#define MAX31328_F_HOURS_F24_12         (1<<MAX31328_F_HOURS_F24_12_POS)

// Alarm registers
#define MAX31328_F_ALRMX_DY_DT_POS       6
#define MAX31328_F_ALRMX_DY_DT           (1<<MAX31328_F_ALRMX_DY_DT_POS)
#define MAX31328_F_ALRMX_ALRM_MASK_POS   7
#define MAX31328_F_ALRMX_ALRM_MASK       (1<<MAX31328_F_ALRMX_ALRM_MASK_POS)


#endif /* _MAX31328_REGISTERS_H_ */
