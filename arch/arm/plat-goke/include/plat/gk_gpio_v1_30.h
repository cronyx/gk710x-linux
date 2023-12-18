/*!
*****************************************************************************
** \file        arch/arm/plat-goke/include/plat/gk_gpio_v1_30.h
**
** \version     $Id$
**
** \brief
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2016 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
#ifndef _GK_GPIO_V1_30_H_
#define _GK_GPIO_V1_30_H_



//*****************************************************************************
//*****************************************************************************
//** Defines and Macros
//*****************************************************************************
//*****************************************************************************

#define SENSOR_GPIO_USED
#define SENSOR_GPIO10   31
#define SENSOR_GPIO11   32

/************************/
/* GPIO pins definition */
/************************/
#define GPIO_SET_OUT_SEL(n)     ((n)&0x7F)          // 7bits
#define GPIO_SET_IN_SEL(n)      (((n)&0x7F)<<7)     // 7bits
#define GPIO_SET_OEN_SEL(n)     (((n)&0x7F)<<14)    // 7bits
#define GPIO_SET_OUT_INVERT(n)  (((n)&0x1)<<22)     // 1bits
#define GPIO_SET_OEN_INVERT(n)  (((n)&0x1)<<23)     // 1bits
#define GPIO_SET_IOCTRL(n)      (((n)&0x3F)<<24)    // 6bits
#define GPIO_SET_FUNC(n)        (((n)&0x3)<<30)

#define GPIO_GET_OUT_SEL(n)     (((n)&0x0000007F))
#define GPIO_GET_IN_SEL(n)      (((n)&0x00003F80)>>7)
#define GPIO_GET_OEN_SEL(n)     (((n)&0x001FC000)>>14)
#define GPIO_GET_OUT_INVERT(n)  (((n)&0x00400000)>>22)
#define GPIO_GET_OEN_INVERT(n)  (((n)&0x00800000)>>23)
#define GPIO_GET_IOCTRL(n)      (((n)&0x3F000000)>>24)
#define GPIO_GET_FUNC(n)        (((n)&0xC0000000)>>30)

#define IOCTRL_NORMAL           0x00    //!< Hi-z
#define IOCTRL_PULL_UP          0x10    //!< PULL_UP
#define IOCTRL_PULL_DOWN        0x20    //!< PULL_DOWN
#define IOCTRL_REPEAT           0x30    //!< REPEAT

#define IOCTRL_2MA              0x00    //!< 2mA
#define IOCTRL_4MA              0x01    //!< 4mA
#define IOCTRL_8MA              0x02    //!< 8mA
#define IOCTRL_12MA             0x03    //!< 12mA



//*****************************************************************************
//*****************************************************************************
//** Enumerated types
//*****************************************************************************
//*****************************************************************************
typedef enum
{
    /* ----------------------------------- GPIO output function define ----------------------------------- */
    GPIO_TYPE_OUTPUT_0               = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL( 0),   //!< Output type: value = 0
    GPIO_TYPE_OUTPUT_1               = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL( 1),   //!< Output type: value = 1
    GPIO_TYPE_OUTPUT_SPI1_SO         = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 2) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL( 2),   //!< Output type: tssi_txd
    GPIO_TYPE_OUTPUT_SPI1_CS0        = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL( 3),   //!< Output type: tssi_cs0_n
    GPIO_TYPE_OUTPUT_SPI1_SCLK       = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL( 4),   //!< Output type: tssi_sclk_out
    GPIO_TYPE_OUTPUT_UART2_RTS_N     = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL( 5),   //!< Output type: uart2_rts_n
    GPIO_TYPE_OUTPUT_UART2_DTR_N     = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL( 6),   //!< Output type: uart2_dtr_n
    GPIO_TYPE_OUTPUT_UART2_TX        = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_UP|IOCTRL_2MA)     | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL( 7),   //!< Output type: uart2_tx
    GPIO_TYPE_OUTPUT_UART1_TX        = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_UP|IOCTRL_2MA)     | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL( 8),   //!< Output type: uart1_tx
    GPIO_TYPE_OUTPUT_UART0_TX        = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_UP|IOCTRL_2MA)     | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL( 9),   //!< Output type: uart0_tx
    GPIO_TYPE_OUTPUT_PWM3_OUT        = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(10),   //!< Output type: pwm3_out
    GPIO_TYPE_OUTPUT_PWM2_OUT        = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(11),   //!< Output type: pwm2_out
    GPIO_TYPE_OUTPUT_PWM1_OUT        = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(12),   //!< Output type: pwm1_out
    GPIO_TYPE_OUTPUT_PWM0_OUT        = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(13),   //!< Output type: pwm0_out
    GPIO_TYPE_OUTPUT_SPI0_SO         = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 7) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(14),   //!< Output type: ssi_txd
    GPIO_TYPE_OUTPUT_SPI0_CS1        = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(15),   //!< Output type: ssi_cs1_n
    GPIO_TYPE_OUTPUT_SPI0_CS0        = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(16),   //!< Output type: ssi_cs0_n
    GPIO_TYPE_OUTPUT_SPI0_SCLK       = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(17),   //!< Output type: ssi_sclk_out
    //GPIO_TYPE_INOUT_SD0_DATA_0                                                                                                                                         GPIO_SET_OEN_SEL( 8)                           GPIO_SET_OUT_SEL(18)    //!< Input/Output type: sd0_data_out[0]
    //GPIO_TYPE_INOUT_SD0_DATA_1                                                                                                                                         GPIO_SET_OEN_SEL( 9)                           GPIO_SET_OUT_SEL(19)    //!< Input/Output type: sd0_data_out[1]
    //GPIO_TYPE_INOUT_SD0_DATA_2                                                                                                                                         GPIO_SET_OEN_SEL(10)                           GPIO_SET_OUT_SEL(20)    //!< Input/Output type: sd0_data_out[2]
    //GPIO_TYPE_INOUT_SD0_DATA_3                                                                                                                                         GPIO_SET_OEN_SEL(11)                           GPIO_SET_OUT_SEL(21)    //!< Input/Output type: sd0_data_out[3]
    //GPIO_TYPE_INOUT_SD0_CMD                                                                                                                                            GPIO_SET_OEN_SEL(12)                           GPIO_SET_OUT_SEL(22)    //!< Input/Output type: sd0_cmd
    GPIO_TYPE_OUTPUT_SD0_CLK         = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_UP|IOCTRL_2MA)     | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(23),   //!< Output type: sd0_clk_sdcard
    GPIO_TYPE_OUTPUT_SF_CS0          = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(24),   //!< Output type: sf_cs0_n
    GPIO_TYPE_OUTPUT_SF_CS1          = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(25),   //!< Output type: sf_cs1_n
    //GPIO_TYPE_INOUT_SD1_DATA_0                                                                                                                                         GPIO_SET_OEN_SEL(13)                           GPIO_SET_OUT_SEL(26)    //!< Input/Output type: sd1_data_out[0]
    //GPIO_TYPE_INOUT_SD1_DATA_1                                                                                                                                         GPIO_SET_OEN_SEL(14)                           GPIO_SET_OUT_SEL(27)    //!< Input/Output type: sd1_data_out[1]
    //GPIO_TYPE_INOUT_SD1_DATA_2                                                                                                                                         GPIO_SET_OEN_SEL(15)                           GPIO_SET_OUT_SEL(28)    //!< Input/Output type: sd1_data_out[2]
    //GPIO_TYPE_INOUT_SD1_DATA_3                                                                                                                                         GPIO_SET_OEN_SEL(16)                           GPIO_SET_OUT_SEL(29)    //!< Input/Output type: sd1_data_out[3]
    //GPIO_TYPE_INOUT_SD1_CMD                                                                                                                                            GPIO_SET_OEN_SEL(17)                           GPIO_SET_OUT_SEL(30)    //!< Input/Output type: sd1_cmd
    GPIO_TYPE_OUTPUT_SD1_CLK         = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_UP|IOCTRL_2MA)     | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(31),   //!< Output type: sd1_clk_sdcard
    GPIO_TYPE_OUTPUT_JTAG_TDO        = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(32),   //!< Output type: jtag_tdout
    GPIO_TYPE_OUTPUT_VD_VSYNC        = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(33),   //!< Output type: lcd_vsync
    GPIO_TYPE_OUTPUT_VD_HSYNC        = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(34),   //!< Output type: lcd_hsync
    GPIO_TYPE_OUTPUT_VD_CLOCK        = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(35),   //!< Output type: lcd_dclk
    GPIO_TYPE_OUTPUT_VD_HVLD         = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(36),   //!< Output type: lcd_hvld
    GPIO_TYPE_OUTPUT_VD_DATA0        = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(37),   //!< Output type: lcd_data0
    GPIO_TYPE_OUTPUT_VD_DATA1        = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(38),   //!< Output type: lcd_data1
    GPIO_TYPE_OUTPUT_VD_DATA2        = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(39),   //!< Output type: lcd_data2
    GPIO_TYPE_OUTPUT_VD_DATA3        = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(40),   //!< Output type: lcd_data3
    GPIO_TYPE_OUTPUT_VD_DATA4        = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(41),   //!< Output type: lcd_data4
    GPIO_TYPE_OUTPUT_VD_DATA5        = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(42),   //!< Output type: lcd_data5
    GPIO_TYPE_OUTPUT_VD_DATA6        = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(43),   //!< Output type: lcd_data6
    GPIO_TYPE_OUTPUT_VD_DATA7        = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(44),   //!< Output type: lcd_data7
    GPIO_TYPE_OUTPUT_VD_DATA8        = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(45),   //!< Output type: lcd_data8
    GPIO_TYPE_OUTPUT_VD_DATA9        = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(46),   //!< Output type: lcd_data9
    GPIO_TYPE_OUTPUT_VD_DATA10       = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(47),   //!< Output type: lcd_data10
    GPIO_TYPE_OUTPUT_VD_DATA11       = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(48),   //!< Output type: lcd_data11
    GPIO_TYPE_OUTPUT_VD_DATA12       = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(49),   //!< Output type: lcd_data12
    GPIO_TYPE_OUTPUT_VD_DATA13       = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(50),   //!< Output type: lcd_data13
    GPIO_TYPE_OUTPUT_VD_DATA14       = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(51),   //!< Output type: lcd_data14
    GPIO_TYPE_OUTPUT_VD_DATA15       = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(52),   //!< Output type: lcd_data15
    GPIO_TYPE_OUTPUT_RCT_CLK_OUT2    = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(53),   //!< Output type: rct_clk_out2
    GPIO_TYPE_OUTPUT_RCT_CLK_OUT1    = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(54),   //!< Output type: rct_clk_out1
    //GPIO_TYPE_INOUT_SF_WP                                                                                                                                              GPIO_SET_OEN_SEL(18)                           GPIO_SET_OUT_SEL(55),   //!< Output type: sf_wp
    //GPIO_TYPE_INOUT_SF_HOLD                                                                                                                                            GPIO_SET_OEN_SEL(19)                           GPIO_SET_OUT_SEL(56),   //!< Output type: sf_hold
    GPIO_TYPE_OUTPUT_RCT_XOSC        = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(57),   //!< Output type: rct_xosc
    GPIO_TYPE_OUTPUT_EPHY_LED_0      = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(58),   //!< Output type: ephy_led[0] hcd ok
    GPIO_TYPE_OUTPUT_EPHY_LED_1      = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(59),   //!< Output type: ephy_led[1] duplex
    GPIO_TYPE_OUTPUT_EPHY_LED_2      = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(60),   //!< Output type: ephy_led[2] 10M CRS out
    GPIO_TYPE_OUTPUT_EPHY_LED_3      = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(61),   //!< Output type: ephy_led[3] 100M CRS out
    GPIO_TYPE_OUTPUT_EPHY_LED_4      = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(62),   //!< Output type: ephy_led[4] clo gs
    GPIO_TYPE_OUTPUT_ENET_PHY_TXD_0  = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(63),   //!< Output type: enet_phy_txd[0]
    GPIO_TYPE_OUTPUT_ENET_PHY_TXD_1  = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(64),   //!< Output type: enet_phy_txd[1]
    GPIO_TYPE_OUTPUT_ENET_PHY_TXD_2  = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(65),   //!< Output type: enet_phy_txd[2]
    GPIO_TYPE_OUTPUT_ENET_PHY_TXD_3  = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(66),   //!< Output type: enet_phy_txd[3]
    GPIO_TYPE_OUTPUT_ENET_PHY_TXER   = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(67),   //!< Output type: enet_phy_txer
    GPIO_TYPE_OUTPUT_ENET_PHY_TXEN   = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(68),   //!< Output type: enet_phy_txen
    //GPIO_TYPE_INOUT_ETH_MDIO                                                                                                                                           GPIO_SET_OEN_SEL(20)                           GPIO_SET_OUT_SEL(69)    //!< Input/Output type: enet_gmii_mdi/enet_gmii_mod_o
    GPIO_TYPE_OUTPUT_ENET_GMII_MDC_O = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(70),   //!< Output type: enet_gmii_mdc_o
    GPIO_TYPE_OUTPUT_PWM7_OUT        = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(71),   //!< Output type: pwm7_out
    GPIO_TYPE_OUTPUT_PWM6_OUT        = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(72),   //!< Output type: pwm6_out
    GPIO_TYPE_OUTPUT_PWM5_OUT        = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(73),   //!< Output type: pwm5_out
    GPIO_TYPE_OUTPUT_PWM4_OUT        = GPIO_SET_FUNC(GPIO_FUNC_OUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 0) | GPIO_SET_IN_SEL( 0)   | GPIO_SET_OUT_SEL(74),   //!< Output type: pwm4_out

    /* ----------------------------------- GPIO input function define ----------------------------------- */
    GPIO_TYPE_INPUT                  = GPIO_SET_FUNC(GPIO_FUNC_IN)  | GPIO_SET_IOCTRL(IOCTRL_NORMAL|IOCTRL_2MA)      | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 1) | GPIO_SET_IN_SEL(   0) | GPIO_SET_OUT_SEL( 0),   //!< Input type: normal input
    GPIO_TYPE_INPUT_0                = GPIO_SET_FUNC(GPIO_FUNC_IN)  | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 1) | GPIO_SET_IN_SEL(   0) | GPIO_SET_OUT_SEL( 0),   //!< Input type: normal input
    GPIO_TYPE_INPUT_1                = GPIO_SET_FUNC(GPIO_FUNC_IN)  | GPIO_SET_IOCTRL(IOCTRL_PULL_UP|IOCTRL_2MA)     | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 1) | GPIO_SET_IN_SEL(   1) | GPIO_SET_OUT_SEL( 0),   //!< Input type: normal input
    GPIO_TYPE_INPUT_SPI1_SI          = GPIO_SET_FUNC(GPIO_FUNC_IN)  | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 1) | GPIO_SET_IN_SEL(2+ 0) | GPIO_SET_OUT_SEL( 0),   //!< Input type: tssi_rxd
    //GPIO_TYPE_INOUT_I2C0_DATA                                                                                                                                                                 GPIO_SET_IN_SEL(2+ 1)                           //!< Input/Output type: i2c0_sda
    //GPIO_TYPE_INOUT_I2C0_CLK                                                                                                                                                                  GPIO_SET_IN_SEL(2+ 2)                           //!< Input/Output type: i2c0_scl
    //GPIO_TYPE_INOUT_I2C1_DATA                                                                                                                                                                 GPIO_SET_IN_SEL(2+ 3)                           //!< Input/Output type: i2c1_sda
    //GPIO_TYPE_INOUT_I2C1_CLK                                                                                                                                                                  GPIO_SET_IN_SEL(2+ 4)                           //!< Input/Output type: i2c1_scl
    GPIO_TYPE_INPUT_UART2_RX         = GPIO_SET_FUNC(GPIO_FUNC_IN)  | GPIO_SET_IOCTRL(IOCTRL_PULL_UP|IOCTRL_2MA)     | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 1) | GPIO_SET_IN_SEL(2+ 5) | GPIO_SET_OUT_SEL( 0),   //!< Input type: uart2_rx
    GPIO_TYPE_INPUT_UART1_RX         = GPIO_SET_FUNC(GPIO_FUNC_IN)  | GPIO_SET_IOCTRL(IOCTRL_PULL_UP|IOCTRL_2MA)     | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 1) | GPIO_SET_IN_SEL(2+ 6) | GPIO_SET_OUT_SEL( 0),   //!< Input type: uart1_rx
    GPIO_TYPE_INPUT_UART0_RX         = GPIO_SET_FUNC(GPIO_FUNC_IN)  | GPIO_SET_IOCTRL(IOCTRL_PULL_UP|IOCTRL_2MA)     | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 1) | GPIO_SET_IN_SEL(2+ 7) | GPIO_SET_OUT_SEL( 0),   //!< Input type: uart0_rx
    GPIO_TYPE_INPUT_SPI0_SI          = GPIO_SET_FUNC(GPIO_FUNC_IN)  | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 1) | GPIO_SET_IN_SEL(2+ 8) | GPIO_SET_OUT_SEL( 0),   //!< Input type: ssi_rxd
    GPIO_TYPE_INPUT_SD0_WP_N         = GPIO_SET_FUNC(GPIO_FUNC_IN)  | GPIO_SET_IOCTRL(IOCTRL_PULL_UP|IOCTRL_2MA)     | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 1) | GPIO_SET_IN_SEL(2+ 9) | GPIO_SET_OUT_SEL( 0),   //!< Input type: sd0_wp_n
    GPIO_TYPE_INPUT_SD0_CD_N         = GPIO_SET_FUNC(GPIO_FUNC_IN)  | GPIO_SET_IOCTRL(IOCTRL_PULL_UP|IOCTRL_2MA)     | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 1) | GPIO_SET_IN_SEL(2+10) | GPIO_SET_OUT_SEL( 0),   //!< Input type: sd0_cd_n
    //GPIO_TYPE_INOUT_SD0_DATA_0                                                                                                                                                                GPIO_SET_IN_SEL(2+11)                           //!< Input/Output type: sd0_data_out[0]
    //GPIO_TYPE_INOUT_SD0_DATA_1                                                                                                                                                                GPIO_SET_IN_SEL(2+12)                           //!< Input/Output type: sd0_data_out[1]
    //GPIO_TYPE_INOUT_SD0_DATA_2                                                                                                                                                                GPIO_SET_IN_SEL(2+13)                           //!< Input/Output type: sd0_data_out[2]
    //GPIO_TYPE_INOUT_SD0_DATA_3                                                                                                                                                                GPIO_SET_IN_SEL(2+14)                           //!< Input/Output type: sd0_data_out[3]
    //GPIO_TYPE_INOUT_SD0_CMD                                                                                                                                                                   GPIO_SET_IN_SEL(2+15)                           //!< Input/Output type: sd0_cmd
    GPIO_TYPE_INPUT_SD1_WP_N         = GPIO_SET_FUNC(GPIO_FUNC_IN)  | GPIO_SET_IOCTRL(IOCTRL_PULL_UP|IOCTRL_2MA)     | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 1) | GPIO_SET_IN_SEL(2+16) | GPIO_SET_OUT_SEL( 0),   //!< Input type: sd1_wp_n
    GPIO_TYPE_INPUT_SD1_CD_N         = GPIO_SET_FUNC(GPIO_FUNC_IN)  | GPIO_SET_IOCTRL(IOCTRL_PULL_UP|IOCTRL_2MA)     | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 1) | GPIO_SET_IN_SEL(2+17) | GPIO_SET_OUT_SEL( 0),   //!< Input type: sd1_cd_n
    //GPIO_TYPE_INOUT_SD1_DATA_0                                                                                                                                                                GPIO_SET_IN_SEL(2+18)                           //!< Input/Output type: sd1_data_out[0]
    //GPIO_TYPE_INOUT_SD1_DATA_1                                                                                                                                                                GPIO_SET_IN_SEL(2+19)                           //!< Input/Output type: sd1_data_out[1]
    //GPIO_TYPE_INOUT_SD1_DATA_2                                                                                                                                                                GPIO_SET_IN_SEL(2+20)                           //!< Input/Output type: sd1_data_out[2]
    //GPIO_TYPE_INOUT_SD1_DATA_3                                                                                                                                                                GPIO_SET_IN_SEL(2+21)                           //!< Input/Output type: sd1_data_out[3]
    //GPIO_TYPE_INOUT_SD1_CMD                                                                                                                                                                   GPIO_SET_IN_SEL(2+22)                           //!< Input/Output type: sd1_cmd
    //GPIO_TYPE_INOUT_SF_HOLD                                                                                                                                            GPIO_SET_OEN_SEL(18)   GPIO_SET_IN_SEL(2+23)                           //!< Input/Output type: sf_hold
    //GPIO_TYPE_INOUT_SF_WP                                                                                                                                              GPIO_SET_OEN_SEL(19)   GPIO_SET_IN_SEL(2+24)                           //!< Input/Output type: sf_wp
    GPIO_TYPE_INPUT_JTAG_TRSTN       = GPIO_SET_FUNC(GPIO_FUNC_IN) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)    | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 1) | GPIO_SET_IN_SEL(2+26) | GPIO_SET_OUT_SEL( 0),   //!< Input type: jtag_trstn
    GPIO_TYPE_INPUT_JTAG_TCK         = GPIO_SET_FUNC(GPIO_FUNC_IN) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)    | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 1) | GPIO_SET_IN_SEL(2+27) | GPIO_SET_OUT_SEL( 0),   //!< Input type: jtag_tck
    GPIO_TYPE_INPUT_JTAG_TMS         = GPIO_SET_FUNC(GPIO_FUNC_IN) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)    | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 1) | GPIO_SET_IN_SEL(2+28) | GPIO_SET_OUT_SEL( 0),   //!< Input type: jtag_tms
    GPIO_TYPE_INPUT_JTAG_TDI         = GPIO_SET_FUNC(GPIO_FUNC_IN) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)    | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 1) | GPIO_SET_IN_SEL(2+29) | GPIO_SET_OUT_SEL( 0),   //!< Input type: jtag_tdi
    GPIO_TYPE_INPUT_SENSOR_IDSP      = GPIO_SET_FUNC(GPIO_FUNC_IN) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)    | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 1) | GPIO_SET_IN_SEL(2+30) | GPIO_SET_OUT_SEL( 0),   //!< Input type: sensor_idsp_field
    GPIO_TYPE_INPUT_ENET_PHY_RXD_0   = GPIO_SET_FUNC(GPIO_FUNC_IN) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)    | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 1) | GPIO_SET_IN_SEL(2+31) | GPIO_SET_OUT_SEL( 0),   //!< Input type: enet_phy_rxd[0]
    GPIO_TYPE_INPUT_ENET_PHY_RXD_1   = GPIO_SET_FUNC(GPIO_FUNC_IN) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)    | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 1) | GPIO_SET_IN_SEL(2+32) | GPIO_SET_OUT_SEL( 0),   //!< Input type: enet_phy_rxd[1]
    GPIO_TYPE_INPUT_ENET_PHY_RXD_2   = GPIO_SET_FUNC(GPIO_FUNC_IN) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)    | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 1) | GPIO_SET_IN_SEL(2+33) | GPIO_SET_OUT_SEL( 0),   //!< Input type: enet_phy_rxd[2]
    GPIO_TYPE_INPUT_ENET_PHY_RXD_3   = GPIO_SET_FUNC(GPIO_FUNC_IN) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)    | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 1) | GPIO_SET_IN_SEL(2+34) | GPIO_SET_OUT_SEL( 0),   //!< Input type: enet_phy_rxd[3]
    GPIO_TYPE_INPUT_ENET_PHY_COL     = GPIO_SET_FUNC(GPIO_FUNC_IN) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)    | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 1) | GPIO_SET_IN_SEL(2+35) | GPIO_SET_OUT_SEL( 0),   //!< Input type: enet_phy_col
    GPIO_TYPE_INPUT_ENET_PHY_CRS     = GPIO_SET_FUNC(GPIO_FUNC_IN) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)    | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 1) | GPIO_SET_IN_SEL(2+36) | GPIO_SET_OUT_SEL( 0),   //!< Input type: enet_phy_crs
    GPIO_TYPE_INPUT_ENET_PHY_RXER    = GPIO_SET_FUNC(GPIO_FUNC_IN) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)    | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 1) | GPIO_SET_IN_SEL(2+37) | GPIO_SET_OUT_SEL( 0),   //!< Input type: enet_phy_rxer
    GPIO_TYPE_INPUT_ENET_PHY_RXDV    = GPIO_SET_FUNC(GPIO_FUNC_IN) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)    | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 1) | GPIO_SET_IN_SEL(2+38) | GPIO_SET_OUT_SEL( 0),   //!< Input type: enet_phy_rxdv
    //GPIO_TYPE_INOUT_ETH_MDIO                                                                                                                                           GPIO_SET_OEN_SEL(20)   GPIO_SET_IN_SEL(2+39)                           //!< Input/Output type: enet_gmii_mdi/enet_gmii_mod_o
    GPIO_TYPE_INPUT_ENET_CLK_RX      = GPIO_SET_FUNC(GPIO_FUNC_IN) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)    | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 1) | GPIO_SET_IN_SEL(2+40) | GPIO_SET_OUT_SEL( 0),   //!< Input type: enet_clk_rx
    GPIO_TYPE_INPUT_ENET_CLK_TX      = GPIO_SET_FUNC(GPIO_FUNC_IN) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA)    | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 1) | GPIO_SET_IN_SEL(2+41) | GPIO_SET_OUT_SEL( 0),   //!< Input type: enet_clk_tx

    /* ----------------------------------- GPIO input&&output function define ----------------------------------- */
    GPIO_TYPE_INOUT_I2C0_DATA        = GPIO_SET_FUNC(GPIO_FUNC_INOUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_UP|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 3) | GPIO_SET_IN_SEL(2+ 1) | GPIO_SET_OUT_SEL( 0),   //!< Input/Output type: i2c0_sda
    GPIO_TYPE_INOUT_I2C0_CLK         = GPIO_SET_FUNC(GPIO_FUNC_INOUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_UP|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 4) | GPIO_SET_IN_SEL(2+ 2) | GPIO_SET_OUT_SEL( 0),   //!< Input/Output type: i2c0_scl
    GPIO_TYPE_INOUT_I2C1_DATA        = GPIO_SET_FUNC(GPIO_FUNC_INOUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_UP|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 5) | GPIO_SET_IN_SEL(2+ 3) | GPIO_SET_OUT_SEL( 0),   //!< Input/Output type: i2c1_sda
    GPIO_TYPE_INOUT_I2C1_CLK         = GPIO_SET_FUNC(GPIO_FUNC_INOUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_UP|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 6) | GPIO_SET_IN_SEL(2+ 4) | GPIO_SET_OUT_SEL( 0),   //!< Input/Output type: i2c1_scl
    GPIO_TYPE_INOUT_SD0_DATA_0       = GPIO_SET_FUNC(GPIO_FUNC_INOUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_UP|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(1) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 8) | GPIO_SET_IN_SEL(2+11) | GPIO_SET_OUT_SEL(18),   //!< Input/Output type: sd0_data_out[0]
    GPIO_TYPE_INOUT_SD0_DATA_1       = GPIO_SET_FUNC(GPIO_FUNC_INOUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_UP|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(1) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL( 9) | GPIO_SET_IN_SEL(2+12) | GPIO_SET_OUT_SEL(19),   //!< Input/Output type: sd0_data_out[1]
    GPIO_TYPE_INOUT_SD0_DATA_2       = GPIO_SET_FUNC(GPIO_FUNC_INOUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_UP|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(1) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL(10) | GPIO_SET_IN_SEL(2+13) | GPIO_SET_OUT_SEL(20),   //!< Input/Output type: sd0_data_out[2]
    GPIO_TYPE_INOUT_SD0_DATA_3       = GPIO_SET_FUNC(GPIO_FUNC_INOUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_UP|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(1) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL(11) | GPIO_SET_IN_SEL(2+14) | GPIO_SET_OUT_SEL(21),   //!< Input/Output type: sd0_data_out[3]
    GPIO_TYPE_INOUT_SD0_CMD          = GPIO_SET_FUNC(GPIO_FUNC_INOUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_UP|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(1) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL(12) | GPIO_SET_IN_SEL(2+15) | GPIO_SET_OUT_SEL(22),   //!< Input/Output type: sd_0cmd
    GPIO_TYPE_INOUT_SD1_DATA_0       = GPIO_SET_FUNC(GPIO_FUNC_INOUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_UP|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(1) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL(13) | GPIO_SET_IN_SEL(2+18) | GPIO_SET_OUT_SEL(26),   //!< Input/Output type: sd1_data_out[0]
    GPIO_TYPE_INOUT_SD1_DATA_1       = GPIO_SET_FUNC(GPIO_FUNC_INOUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_UP|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(1) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL(14) | GPIO_SET_IN_SEL(2+19) | GPIO_SET_OUT_SEL(27),   //!< Input/Output type: sd1_data_out[1]
    GPIO_TYPE_INOUT_SD1_DATA_2       = GPIO_SET_FUNC(GPIO_FUNC_INOUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_UP|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(1) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL(15) | GPIO_SET_IN_SEL(2+20) | GPIO_SET_OUT_SEL(28),   //!< Input/Output type: sd1_data_out[2]
    GPIO_TYPE_INOUT_SD1_DATA_3       = GPIO_SET_FUNC(GPIO_FUNC_INOUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_UP|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(1) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL(16) | GPIO_SET_IN_SEL(2+21) | GPIO_SET_OUT_SEL(29),   //!< Input/Output type: sd1_data_out[3]
    GPIO_TYPE_INOUT_SD1_CMD          = GPIO_SET_FUNC(GPIO_FUNC_INOUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_UP|IOCTRL_2MA)   | GPIO_SET_OEN_INVERT(1) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL(17) | GPIO_SET_IN_SEL(2+22) | GPIO_SET_OUT_SEL(30),   //!< Input/Output type: sd1_cmd
    GPIO_TYPE_INOUT_SF_HOLD          = GPIO_SET_FUNC(GPIO_FUNC_INOUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA) | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL(18) | GPIO_SET_IN_SEL(2+23) | GPIO_SET_OUT_SEL(56),   //!< Input/Output type: sf_hold
    GPIO_TYPE_INOUT_SF_WP            = GPIO_SET_FUNC(GPIO_FUNC_INOUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA) | GPIO_SET_OEN_INVERT(0) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL(19) | GPIO_SET_IN_SEL(2+24) | GPIO_SET_OUT_SEL(55),   //!< Input/Output type: sf_wp
    GPIO_TYPE_INOUT_ETH_MDIO         = GPIO_SET_FUNC(GPIO_FUNC_INOUT) | GPIO_SET_IOCTRL(IOCTRL_PULL_DOWN|IOCTRL_2MA) | GPIO_SET_OEN_INVERT(1) | GPIO_SET_OUT_INVERT(0) | GPIO_SET_OEN_SEL(20) | GPIO_SET_IN_SEL(2+39) | GPIO_SET_OUT_SEL(69),   //!< Input/Output type: enet_gmii_mdi/enet_gmii_mod_o

    GPIO_TYPE_UNDEFINED              = 0,
} GPIO_TYPE_E;



//*****************************************************************************
//*****************************************************************************
//** Data Structures
//*****************************************************************************
//*****************************************************************************
typedef union { /* PLL_IOCTRL_GPIO */
    u32 all;
    struct {
        u32 io0                         : 6;
        u32                             : 2;
        u32 io1                         : 6;
        u32                             : 2;
        u32 io2                         : 6;
        u32                             : 2;
        u32 io3                         : 6;
        u32                             : 2;
    } bitc;
} GH_PLL_IOCTRL_GPIO_S;

typedef union { /* GPIO_OUTPUT_CFG */
    u32 all;
    struct {
        u32 out_sel                     : 7;
        u32                             : 1;
        u32 oen_sel                     : 6;
        u32 out_invert                  : 1;
        u32 oen_invert                  : 1;
        u32                             : 16;
    } bitc;
} GH_GPIO_OUTPUT_CFG_S;

typedef union { /* GPIO_INPUT_CFG */
    u32 all;
    struct {
        u32 in_sel                      : 6;
        u32                             : 26;
    } bitc;
} GH_GPIO_INPUT_CFG_S;

typedef union { /* GPIO_INT_EN */
    u32 all;
    struct {
        u32 int_en                      : 1;
        u32                             : 31;
    } bitc;
} GH_GPIO_INT_EN_S;

typedef union { /* SFLASH_array offset */
    u32 all;
    struct {
        u32 channel0_offset                              : 14;
        u32 channel0_use_flag                            : 1;
        u32 channel0_nor_flag                            : 1;
        u32 channel1_offset                              : 14;
        u32 channel1_use_flag                            : 1;
        u32 channel1_nor_flag                            : 1;
    } bitc;
} goke_sflash_array_offset_s;
typedef struct gpio_cfg
{
    //---------------------------------------
    // do not add any element
    u32 gpio_count;
    GPIO_XREF_S gpio_chip[64];
    u32 extphy_gpio_count;
    GPIO_XREF_S ext_phy_gpio[16];
    u32 intphy_gpio_count;
    GPIO_XREF_S int_phy_gpio[16];

    u32 reserve[17];    // use for v2.0.0

    //board info
    u32 soc_type;
    s8  board_type[32];
    u32 board_version;
    u32 reserve1;

    //extra device info
    u32 ext_phy_clk;
    u32 reserve2;

    // Add by Steven Yu:for pmu
    u32 pmu_ctl;
	//flash array offset
    goke_sflash_array_offset_s  flash_array_offset;//[31:16] 1 CHANNEL   [15:0] 0 CHANNEL  BIT:31 and 15 =0 NOR  =1 NAND
    // do not add any element
    //---------------------------------------
    u32 first;

    // you can modify
    u32 ir_led_ctl;
    u32 ir_cut1;
    u32 ir_cut2;

    u32 sensor_reset;

    u32 phy_reset;
    u32 phy_speed_led;

    u32 spi0_en0;
    u32 spi1_en0;

    u32 pwm0;
    u32 pwm1;
    u32 pwm2;
    u32 pwm3;
    u32 pwm4;
    u32 pwm5;
    u32 pwm6;
    u32 pwm7;

    u32 usb_host;

    u32 sd_detect;
    u32 sd_power;
    u32 sd1_detect;
    u32 sd1_power;

}gpio_cfg_t;



//*****************************************************************************
//*****************************************************************************
//** Global Data
//*****************************************************************************
//*****************************************************************************
extern gpio_cfg_t gk_all_gpio_cfg;
extern u8 cmdline_phytype;



//*****************************************************************************
//*****************************************************************************
//** API Functions
//*****************************************************************************
//*****************************************************************************



#ifdef __cplusplus
extern "C" {
#endif



#ifdef __cplusplus
}
#endif



#endif /* _GK_GPIO_V1_30_H_ */

