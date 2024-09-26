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

#ifndef _MAX31329_REGISTERS_H_
#define _MAX31329_REGISTERS_H_

#define MAX31329_I2C_ADDRESS                  	  0x68

#define MAX31329_R_STATUS	            	    ( 0x00 )
#define MAX31329_R_INT_EN		                ( 0x01 )
#define MAX31329_R_RTC_RESET			        ( 0x02 )
#define MAX31329_R_CFG1		                    ( 0x03 )
#define MAX31329_R_CFG2		                    ( 0x04 )
#define MAX31329_R_TIMER_CONFIG		            ( 0x05 )
#define MAX31329_R_SECONDS			            ( 0x06 )
#define MAX31329_R_MINUTES			            ( 0x07 )
#define MAX31329_R_HOURS				        ( 0x08 )
#define MAX31329_R_DAY				            ( 0x09 )
#define MAX31329_R_DATE				            ( 0x0A )
#define MAX31329_R_MONTH				        ( 0x0B )
#define MAX31329_R_YEAR				            ( 0x0C )
#define MAX31329_R_ALM1_SEC			            ( 0x0D )
#define MAX31329_R_ALM1_MIN			            ( 0x0E )
#define MAX31329_R_ALM1_HRS			            ( 0x0F )
#define MAX31329_R_ALM1DAY_DATE		            ( 0x10 )
#define MAX31329_R_ALM1_MON			            ( 0x11 )
#define MAX31329_R_ALM1_YEAR			        ( 0x12 )
#define MAX31329_R_ALM2_MIN			            ( 0x13 )
#define MAX31329_R_ALM2_HRS			            ( 0x14 )
#define MAX31329_R_ALM2DAY_DATE		            ( 0x15 )
#define MAX31329_R_TIMER_COUNT		            ( 0x16 )
#define MAX31329_R_TIMER_INIT		            ( 0x17 )
#define MAX31329_R_PWR_MGMT		                ( 0x18 )
#define MAX31329_R_TRICKLE		                ( 0x19 )
#define MAX31329_R_RAM_REG_START			    ( 0x22 )
#define MAX31329_R_RAM_REG_END			        ( 0x61 )


//status register bits
#define MAX31329_F_STATUS_A1F_POS       	    0
#define MAX31329_F_STATUS_A1F           	    (1<<MAX31329_F_STATUS_A1F_POS)
#define MAX31329_F_STATUS_A2F_POS       	    1
#define MAX31329_F_STATUS_A2F           	    (1<<MAX31329_F_STATUS_A2F_POS)
#define MAX31329_F_STATUS_TIF_POS       	    2
#define MAX31329_F_STATUS_TIF           	    (1<<MAX31329_F_STATUS_TIF_POS)
#define MAX31329_F_STATUS_DIF_POS       	    3
#define MAX31329_F_STATUS_DIF           	    (1<<MAX31329_F_STATUS_DIF_POS)
#define MAX31329_F_STATUS_PFAIL_POS     	    5
#define MAX31329_F_STATUS_PFAIL         	    (1<<MAX31329_F_STATUS_PFAIL_POS)
#define MAX31329_F_STATUS_OSF_POS       	    6
#define MAX31329_F_STATUS_OSF           	    (1<<MAX31329_F_STATUS_OSF_POS)
#define MAX31329_F_STATUS_PSDECT_POS    	    7
#define MAX31329_F_STATUS_PSDECT        	    (1<<MAX31329_F_STATUS_PSDECT_POS)
                                                
//Interrupt enable register bits                
#define MAX31329_F_INT_EN_A1IE_POS       	    0
#define MAX31329_F_INT_EN_A1IE           	    (1<<MAX31329_F_INT_EN_A1IE_POS)
#define MAX31329_F_INT_EN_A2IE_POS       	    1
#define MAX31329_F_INT_EN_A2IE           	    (1<<MAX31329_F_INT_EN_A2IE_POS)
#define MAX31329_F_INT_EN_TIE_POS       	    2
#define MAX31329_F_INT_EN_TIE           	    (1<<MAX31329_F_INT_EN_TIE_POS)
#define MAX31329_F_INT_EN_DIE_POS       	    3
#define MAX31329_F_INT_EN_DIE           	    (1<<MAX31329_F_INT_EN_DIE_POS)
#define MAX31329_F_INT_EN_PFAILE_POS     	    5
#define MAX31329_F_INT_EN_PFAILE         	    (1<<MAX31329_F_INT_EN_PFAILE_POS)
#define MAX31329_F_INT_EN_DOSF_POS       	    6
#define MAX31329_F_INT_EN_DOSF           	    (1<<MAX31329_F_INT_EN_DOSF_POS)
                                                
// RTC reset register bits                      
#define MAX31329_F_RTC_RESET_SWRST_POS		    0
#define MAX31329_F_RTC_RESET_SWRST			    (1<<MAX31329_F_RTC_RESET_SWRST_POS)
                                                
// Config 1 register bits                       
#define MAX31329_F_CFG1_ENOSC_POS       	    0
#define MAX31329_F_CFG1_ENOSC          	        (1<<MAX31329_F_CFG1_ENOSC_POS)
#define MAX31329_F_CFG1_I2C_TIMEOUT_POS         1
#define MAX31329_F_CFG1_I2C_TIMEOUT             (1<<MAX31329_F_CFG1_I2C_TIMEOUT_POS)
#define MAX31329_F_CFG1_DATA_RET_POS       	    2
#define MAX31329_F_CFG1_DATA_RET          	    (1<<MAX31329_F_CFG1_DATA_RET_POS)
#define MAX31329_F_CFG1_ENIO_POS       	        3
#define MAX31329_F_CFG1_ENIO          	        (1<<MAX31329_F_CFG1_ENIO_POS)
                                                
// Config 2 register bits                       
#define MAX31329_F_CFG2_CLKIN_HZ_POS       	    0
#define MAX31329_F_CFG2_CLKIN_HZ          	    (0x03<<MAX31329_F_CFG2_CLKIN_HZ_POS)
#define MAX31329_F_CFG2_ENCLKIN_POS       	    2
#define MAX31329_F_CFG2_ENCLKIN          	    (1<<MAX31329_F_CFG2_ENCLKIN_POS)
#define MAX31329_F_CFG2_DIP_POS       	        3
#define MAX31329_F_CFG2_DIP          	        (1<<MAX31329_F_CFG2_DIP_POS)
#define MAX31329_F_CFG2_CLKO_HZ_POS       	    5
#define MAX31329_F_CFG2_CLKO_HZ          	    (0x3<<MAX31329_F_CFG2_CLKO_HZ_POS)
#define MAX31329_F_CFG2_ENCLKO_POS       	    7
#define MAX31329_F_CFG2_ENCLKO      	        (1<<MAX31329_F_CFG2_ENCLKO_POS)


// Timer config register bits
#define MAX31329_F_TIMER_CONFIG_TFS_POS       	0
#define MAX31329_F_TIMER_CONFIG_TFS          	(3<<MAX31329_F_TIMER_CONFIG_TFS_POS)
#define MAX31329_F_TIMER_CONFIG_TRPT_POS        2
#define MAX31329_F_TIMER_CONFIG_TRPT            (1<<MAX31329_F_TIMER_CONFIG_TRPT_POS)
#define MAX31329_F_TIMER_CONFIG_TPAUSE_POS      3
#define MAX31329_F_TIMER_CONFIG_TPAUSE          (1<<MAX31329_F_TIMER_CONFIG_TPAUSE_POS)
#define MAX31329_F_TIMER_CONFIG_TE_POS       	4
#define MAX31329_F_TIMER_CONFIG_TE          	(1<<MAX31329_F_TIMER_CONFIG_TE_POS)

// Power management register bits
#define MAX31329_F_PWR_MGMT_DMAN_SEL_POS       	0
#define MAX31329_F_PWR_MGMT_DMAN_SEL          	(1<<MAX31329_F_PWR_MGMT_DMAN_SEL_POS)
#define MAX31329_F_PWR_MGMT_D_VBACK_SEL_POS     1
#define MAX31329_F_PWR_MGMT_D_VBACK_SEL         (1<<MAX31329_F_PWR_MGMT_D_VBACK_SEL_POS)
#define MAX31329_F_PWR_MGMT_PFVT_POS            2
#define MAX31329_F_PWR_MGMT_PFVT                (3<<MAX31329_F_PWR_MGMT_PFVT_POS)

// Trickle register bits
#define MAX31329_F_TRICKLE_D_TRICKLE_POS       	0
#define MAX31329_F_TRICKLE_D_TRICKLE          	(0x0f<<MAX31329_F_TRICKLE_D_TRICKLE_POS)
#define MAX31329_F_TRICKLE_D_TRKCHG_EN_POS      7
#define MAX31329_F_TRICKLE_D_TRKCHG_EN          (1<<MAX31329_F_TRICKLE_D_TRKCHG_EN_POS)

#endif /* _MAX31329_REGISTERS_H_ */
