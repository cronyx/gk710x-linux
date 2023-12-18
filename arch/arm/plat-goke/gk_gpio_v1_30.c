/*
 * arch/arm/mach-gk7101/gpio.c
 *
 * Author: Steven Yu, <yulindeng@gokemicro.com>
 * Copyright (C) 2012-2015, goke, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/device.h>
#include <linux/seq_file.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>

#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#include <mach/hardware.h>
#include <mach/gpio.h>
#include <mach/io.h>

#include <plat/gk_gpio.h>


GPIO_TYPE_E gk_gpio_table[GK_GPIO_FUN_ALL] =
{
    [(GK_GPIO_UNDEFINED)]          = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_SF_CS0)]             = GPIO_TYPE_OUTPUT_SF_CS0,
    [(GK_GPIO_SF_CS1)]             = GPIO_TYPE_OUTPUT_SF_CS1,
    [(GK_GPIO_SF_HOLD)]            = GPIO_TYPE_INOUT_SF_HOLD,
    [(GK_GPIO_SF_WP)]              = GPIO_TYPE_INOUT_SF_WP,
    [(GK_GPIO_SPI0_SCLK)]          = GPIO_TYPE_OUTPUT_SPI0_SCLK,
    [(GK_GPIO_SPI0_SI)]            = GPIO_TYPE_INPUT_SPI0_SI,
    [(GK_GPIO_SPI0_SO)]            = GPIO_TYPE_OUTPUT_SPI0_SO,
    [(GK_GPIO_SPI0_HOLD)]          = GPIO_TYPE_OUTPUT_1,
    [(GK_GPIO_SPI0_WP)]            = GPIO_TYPE_OUTPUT_1,
    [(GK_GPIO_SPI0_CS0)]           = GPIO_TYPE_OUTPUT_SPI0_CS0,
    [(GK_GPIO_SPI0_CS1)]           = GPIO_TYPE_OUTPUT_SPI0_CS1,
    [(GK_GPIO_SPI0_CS2)]           = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_SPI0_CS3)]           = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_SPI0_CS4)]           = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_SPI0_CS5)]           = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_SPI0_CS6)]           = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_SPI0_CS7)]           = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_SPI1_SCLK)]          = GPIO_TYPE_OUTPUT_SPI1_SCLK,
    [(GK_GPIO_SPI1_SI)]            = GPIO_TYPE_INPUT_SPI1_SI,
    [(GK_GPIO_SPI1_SO)]            = GPIO_TYPE_OUTPUT_SPI1_SO,
    [(GK_GPIO_SPI1_HOLD)]          = GPIO_TYPE_OUTPUT_1,
    [(GK_GPIO_SPI1_WP)]            = GPIO_TYPE_OUTPUT_1,
    [(GK_GPIO_SPI1_CS0)]           = GPIO_TYPE_OUTPUT_SPI1_CS0,
    [(GK_GPIO_SPI1_CS1)]           = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_SPI1_CS2)]           = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_SPI1_CS3)]           = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_SPI1_CS4)]           = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_SPI1_CS5)]           = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_SPI1_CS6)]           = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_SPI1_CS7)]           = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_UART0_TX)]           = GPIO_TYPE_OUTPUT_UART0_TX,
    [(GK_GPIO_UART0_RX)]           = GPIO_TYPE_INPUT_UART0_RX,
    [(GK_GPIO_UART0_RTS_N)]        = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_UART0_DTR_N)]        = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_UART1_TX)]           = GPIO_TYPE_OUTPUT_UART1_TX,
    [(GK_GPIO_UART1_RX)]           = GPIO_TYPE_INPUT_UART1_RX,
    [(GK_GPIO_UART1_RTS_N)]        = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_UART1_DTR_N)]        = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_UART2_TX)]           = GPIO_TYPE_OUTPUT_UART2_TX,
    [(GK_GPIO_UART2_RX)]           = GPIO_TYPE_INPUT_UART2_RX,
    [(GK_GPIO_UART2_RTS_N)]        = GPIO_TYPE_OUTPUT_UART2_RTS_N,
    [(GK_GPIO_UART2_DTR_N)]        = GPIO_TYPE_OUTPUT_UART2_DTR_N,
    [(GK_GPIO_SDIO0_CLK)]          = GPIO_TYPE_OUTPUT_SD0_CLK,
    [(GK_GPIO_SDIO0_CMD)]          = GPIO_TYPE_INOUT_SD0_CMD,
    [(GK_GPIO_SDIO0_WP_N)]         = GPIO_TYPE_INPUT_SD0_WP_N,
    [(GK_GPIO_SDIO0_CD_N)]         = GPIO_TYPE_INPUT_SD0_CD_N,
    [(GK_GPIO_SDIO0_DATA_0)]       = GPIO_TYPE_INOUT_SD0_DATA_0,
    [(GK_GPIO_SDIO0_DATA_1)]       = GPIO_TYPE_INOUT_SD0_DATA_1,
    [(GK_GPIO_SDIO0_DATA_2)]       = GPIO_TYPE_INOUT_SD0_DATA_2,
    [(GK_GPIO_SDIO0_DATA_3)]       = GPIO_TYPE_INOUT_SD0_DATA_3,
    [(GK_GPIO_SDIO0_DATA_4)]       = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_SDIO0_DATA_5)]       = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_SDIO0_DATA_6)]       = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_SDIO0_DATA_7)]       = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_SDIO1_CLK)]          = GPIO_TYPE_OUTPUT_SD1_CLK,
    [(GK_GPIO_SDIO1_CMD)]          = GPIO_TYPE_INOUT_SD1_CMD,
    [(GK_GPIO_SDIO1_WP_N)]         = GPIO_TYPE_INPUT_SD1_WP_N,
    [(GK_GPIO_SDIO1_CD_N)]         = GPIO_TYPE_INPUT_SD1_CD_N,
    [(GK_GPIO_SDIO1_DATA_0)]       = GPIO_TYPE_INOUT_SD1_DATA_0,
    [(GK_GPIO_SDIO1_DATA_1)]       = GPIO_TYPE_INOUT_SD1_DATA_1,
    [(GK_GPIO_SDIO1_DATA_2)]       = GPIO_TYPE_INOUT_SD1_DATA_2,
    [(GK_GPIO_SDIO1_DATA_3)]       = GPIO_TYPE_INOUT_SD1_DATA_3,
    [(GK_GPIO_SDIO1_DATA_4)]       = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_SDIO1_DATA_5)]       = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_SDIO1_DATA_6)]       = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_SDIO1_DATA_7)]       = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_ENET_PHY_TXD_0)]     = GPIO_TYPE_OUTPUT_ENET_PHY_TXD_0,
    [(GK_GPIO_ENET_PHY_TXD_1)]     = GPIO_TYPE_OUTPUT_ENET_PHY_TXD_1,
    [(GK_GPIO_ENET_PHY_TXD_2)]     = GPIO_TYPE_OUTPUT_ENET_PHY_TXD_2,
    [(GK_GPIO_ENET_PHY_TXD_3)]     = GPIO_TYPE_OUTPUT_ENET_PHY_TXD_3,
    [(GK_GPIO_ENET_PHY_TXD_4)]     = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_ENET_PHY_TXD_5)]     = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_ENET_PHY_TXD_6)]     = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_ENET_PHY_TXD_7)]     = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_ENET_PHY_RXD_0)]     = GPIO_TYPE_INPUT_ENET_PHY_RXD_0,
    [(GK_GPIO_ENET_PHY_RXD_1)]     = GPIO_TYPE_INPUT_ENET_PHY_RXD_1,
    [(GK_GPIO_ENET_PHY_RXD_2)]     = GPIO_TYPE_INPUT_ENET_PHY_RXD_2,
    [(GK_GPIO_ENET_PHY_RXD_3)]     = GPIO_TYPE_INPUT_ENET_PHY_RXD_3,
    [(GK_GPIO_ENET_PHY_RXD_4)]     = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_ENET_PHY_RXD_5)]     = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_ENET_PHY_RXD_6)]     = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_ENET_PHY_RXD_7)]     = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_ENET_PHY_CLK_RX)]    = GPIO_TYPE_INPUT_ENET_CLK_RX,
    [(GK_GPIO_ENET_PHY_CLK_TX)]    = GPIO_TYPE_INPUT_ENET_CLK_TX,
    [(GK_GPIO_ENET_PHY_MDC)]       = GPIO_TYPE_OUTPUT_ENET_GMII_MDC_O,
    [(GK_GPIO_ENET_PHY_MDIO)]      = GPIO_TYPE_INOUT_ETH_MDIO,
    [(GK_GPIO_ENET_PHY_COL)]       = GPIO_TYPE_INPUT_ENET_PHY_COL,
    [(GK_GPIO_ENET_PHY_CRS)]       = GPIO_TYPE_INPUT_ENET_PHY_CRS,
    [(GK_GPIO_ENET_PHY_RXER)]      = GPIO_TYPE_INPUT_ENET_PHY_RXER,
    [(GK_GPIO_ENET_PHY_RXDV)]      = GPIO_TYPE_INPUT_ENET_PHY_RXDV,
    [(GK_GPIO_ENET_PHY_TXER)]      = GPIO_TYPE_OUTPUT_ENET_PHY_TXER,
    [(GK_GPIO_ENET_PHY_TXEN)]      = GPIO_TYPE_OUTPUT_ENET_PHY_TXEN,
    [(GK_GPIO_EPHY_LED_0)]         = GPIO_TYPE_OUTPUT_EPHY_LED_0,
    [(GK_GPIO_EPHY_LED_1)]         = GPIO_TYPE_OUTPUT_EPHY_LED_1,
    [(GK_GPIO_EPHY_LED_2)]         = GPIO_TYPE_OUTPUT_EPHY_LED_2,
    [(GK_GPIO_EPHY_LED_3)]         = GPIO_TYPE_OUTPUT_EPHY_LED_3,
    [(GK_GPIO_EPHY_LED_4)]         = GPIO_TYPE_OUTPUT_EPHY_LED_4,
    [(GK_GPIO_EPHY_LED_5)]         = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_EPHY_LED_6)]         = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_EPHY_LED_7)]         = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_PWM_0)]              = GPIO_TYPE_OUTPUT_PWM0_OUT,
    [(GK_GPIO_PWM_1)]              = GPIO_TYPE_OUTPUT_PWM1_OUT,
    [(GK_GPIO_PWM_2)]              = GPIO_TYPE_OUTPUT_PWM2_OUT,
    [(GK_GPIO_PWM_3)]              = GPIO_TYPE_OUTPUT_PWM3_OUT,
    [(GK_GPIO_PWM_4)]              = GPIO_TYPE_OUTPUT_PWM4_OUT,
    [(GK_GPIO_PWM_5)]              = GPIO_TYPE_OUTPUT_PWM5_OUT,
    [(GK_GPIO_PWM_6)]              = GPIO_TYPE_OUTPUT_PWM6_OUT,
    [(GK_GPIO_PWM_7)]              = GPIO_TYPE_OUTPUT_PWM7_OUT,
    [(GK_GPIO_PWM_8)]              = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_PWM_9)]              = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_PWM_10)]             = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_PWM_11)]             = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_PWM_12)]             = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_PWM_13)]             = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_PWM_14)]             = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_PWM_15)]             = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_VD_CLOCK)]           = GPIO_TYPE_OUTPUT_VD_CLOCK,
    [(GK_GPIO_VD_VSYNC)]           = GPIO_TYPE_OUTPUT_VD_VSYNC,
    [(GK_GPIO_VD_HSYNC)]           = GPIO_TYPE_OUTPUT_VD_HSYNC,
    [(GK_GPIO_VD_HVLD)]            = GPIO_TYPE_OUTPUT_VD_HVLD,
    [(GK_GPIO_VD_DATA0)]           = GPIO_TYPE_OUTPUT_VD_DATA0,
    [(GK_GPIO_VD_DATA1)]           = GPIO_TYPE_OUTPUT_VD_DATA1,
    [(GK_GPIO_VD_DATA2)]           = GPIO_TYPE_OUTPUT_VD_DATA2,
    [(GK_GPIO_VD_DATA3)]           = GPIO_TYPE_OUTPUT_VD_DATA3,
    [(GK_GPIO_VD_DATA4)]           = GPIO_TYPE_OUTPUT_VD_DATA4,
    [(GK_GPIO_VD_DATA5)]           = GPIO_TYPE_OUTPUT_VD_DATA5,
    [(GK_GPIO_VD_DATA6)]           = GPIO_TYPE_OUTPUT_VD_DATA6,
    [(GK_GPIO_VD_DATA7)]           = GPIO_TYPE_OUTPUT_VD_DATA7,
    [(GK_GPIO_VD_DATA8)]           = GPIO_TYPE_OUTPUT_VD_DATA8,
    [(GK_GPIO_VD_DATA9)]           = GPIO_TYPE_OUTPUT_VD_DATA9,
    [(GK_GPIO_VD_DATA10)]          = GPIO_TYPE_OUTPUT_VD_DATA10,
    [(GK_GPIO_VD_DATA11)]          = GPIO_TYPE_OUTPUT_VD_DATA11,
    [(GK_GPIO_VD_DATA12)]          = GPIO_TYPE_OUTPUT_VD_DATA12,
    [(GK_GPIO_VD_DATA13)]          = GPIO_TYPE_OUTPUT_VD_DATA13,
    [(GK_GPIO_VD_DATA14)]          = GPIO_TYPE_OUTPUT_VD_DATA14,
    [(GK_GPIO_VD_DATA15)]          = GPIO_TYPE_OUTPUT_VD_DATA15,
    [(GK_GPIO_VD_DATA16)]          = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_VD_DATA17)]          = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_VD_DATA18)]          = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_VD_DATA19)]          = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_VD_DATA20)]          = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_VD_DATA21)]          = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_VD_DATA22)]          = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_VD_DATA23)]          = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_I80_LCD_RST)]        = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_I80_RDN)]            = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_I80_WRN)]            = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_I80_DCX)]            = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_I80_CSN)]            = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_I80_DATA0)]          = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_I80_DATA1)]          = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_I80_DATA2)]          = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_I80_DATA3)]          = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_I80_DATA4)]          = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_I80_DATA5)]          = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_I80_DATA6)]          = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_I80_DATA7)]          = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_I80_DATA8)]          = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_I80_OUTPUT_DATA0)]   = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_I80_OUTPUT_DATA1)]   = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_I80_OUTPUT_DATA2)]   = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_I80_OUTPUT_DATA3)]   = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_I80_OUTPUT_DATA4)]   = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_I80_OUTPUT_DATA5)]   = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_I80_OUTPUT_DATA6)]   = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_I80_OUTPUT_DATA7)]   = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_I80_OUTPUT_DATA8)]   = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_I80_INPUT_DATA0)]    = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_I80_INPUT_DATA1)]    = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_I80_INPUT_DATA2)]    = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_I80_INPUT_DATA3)]    = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_I80_INPUT_DATA4)]    = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_I80_INPUT_DATA5)]    = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_I80_INPUT_DATA6)]    = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_I80_INPUT_DATA7)]    = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_I80_INPUT_DATA8)]    = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_I2C0_CLK)]           = GPIO_TYPE_INOUT_I2C0_CLK,
    [(GK_GPIO_I2C0_DATA)]          = GPIO_TYPE_INOUT_I2C0_DATA,
    [(GK_GPIO_I2C1_CLK)]           = GPIO_TYPE_INOUT_I2C1_CLK,
    [(GK_GPIO_I2C1_DATA)]          = GPIO_TYPE_INOUT_I2C1_DATA,
    [(GK_GPIO_I2C2_CLK)]           = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_I2C2_DATA)]          = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_AO0_MCLK)]           = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_AO0_BCLK)]           = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_AO0_LRCLK)]          = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_AO0_DATA)]           = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_AO1_MCLK)]           = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_AO1_BCLK)]           = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_AO1_LRCLK)]          = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_AO1_DATA)]           = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_AI0_MCLK)]           = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_AI0_BCLK)]           = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_AI0_LRCLK)]          = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_AI0_DATA)]           = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_AI1_MCLK)]           = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_AI1_BCLK)]           = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_AI1_LRCLK)]          = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_AI1_DATA)]           = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_OUTPUT_0)]           = GPIO_TYPE_OUTPUT_0,
    [(GK_GPIO_OUTPUT_1)]           = GPIO_TYPE_OUTPUT_1,
    [(GK_GPIO_INPUT_0)]            = GPIO_TYPE_INPUT_0,
    [(GK_GPIO_INPUT_1)]            = GPIO_TYPE_INPUT_1,
    [(GK_GPIO_INPUT)]              = GPIO_TYPE_INPUT,
    [(GK_GPIO_SENSOR_POWER)]       = GPIO_TYPE_OUTPUT_1,
    [(GK_GPIO_SENSOR_RESET)]       = GPIO_TYPE_OUTPUT_1,
    [(GK_GPIO_PHY_RESET)]          = GPIO_TYPE_OUTPUT_1,
    [(GK_GPIO_PHY_SPEED_LED)]      = GPIO_TYPE_OUTPUT_EPHY_LED_3,
    [(GK_GPIO_PHY_LINK_LED)]       = GPIO_TYPE_OUTPUT_EPHY_LED_0,
    [(GK_GPIO_PHY_DATA_LED)]       = GPIO_TYPE_OUTPUT_EPHY_LED_1,
    [(GK_GPIO_IR_LED_CTRL)]        = GPIO_TYPE_OUTPUT_1,
    [(GK_GPIO_IR_DETECT)]          = GPIO_TYPE_INPUT_0,
    [(GK_GPIO_IR_CUT1)]            = GPIO_TYPE_OUTPUT_0,
    [(GK_GPIO_IR_CUT2)]            = GPIO_TYPE_OUTPUT_0,
    [(GK_GPIO_ALARM_IN)]           = GPIO_TYPE_INPUT_0,
    [(GK_GPIO_ALARM_OUT)]          = GPIO_TYPE_OUTPUT_0,
    [(GK_GPIO_USB_HOST)]           = GPIO_TYPE_OUTPUT_1,
    [(GK_GPIO_USB_OTG)]            = GPIO_TYPE_INPUT_1,
    [(GK_GPIO_SDIO0_POWER)]        = GPIO_TYPE_OUTPUT_1,
    [(GK_GPIO_SDIO1_POWER)]        = GPIO_TYPE_OUTPUT_1,
    [(GK_GPIO_PMU_CTL)]            = GPIO_TYPE_OUTPUT_1,
    [(GK_GPIO_JTAG_TRSTN)]         = GPIO_TYPE_INPUT_JTAG_TRSTN,
    [(GK_GPIO_JTAG_TCK)]           = GPIO_TYPE_INPUT_JTAG_TCK,
    [(GK_GPIO_JTAG_TMS)]           = GPIO_TYPE_INPUT_JTAG_TMS,
    [(GK_GPIO_JTAG_TDI)]           = GPIO_TYPE_INPUT_JTAG_TDI,
    [(GK_GPIO_JTAG_TDO)]           = GPIO_TYPE_OUTPUT_JTAG_TDO,
    [(GK_GPIO_TIMER1_CLK)]         = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_TIMER2_CLK)]         = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_TIMER3_CLK)]         = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_RCT_CLK_OUT1)]       = GPIO_TYPE_UNDEFINED,
    [(GK_GPIO_RCT_CLK_OUT2)]       = GPIO_TYPE_UNDEFINED,
};

gpio_cfg_t  gk_all_gpio_cfg;

struct gk_gpio_bank gk_gpio0_banks[] =
{
    GK_GPIO_BANK("gk-gpio0", GPIO0_BANK0_BASE, GPIO0_BANK0_PLL_IOCTRL_BASE,
        GK_GPIO(0 * GPIO_BANK_SIZE), CONFIG_ARCH_NR_GPIO, 0),
};
// last one max = CONFIG_ARCH_NR_GPIO - base

struct gk_gpio_inst gk_gpio_insts[CONFIG_GK_GPIO_INSTANCES] =
{
    [0] =
    {
        .bank_num       = ARRAY_SIZE(gk_gpio0_banks),
        .gpio_bank      = gk_gpio0_banks,
        .output_cfg     = {0},
        .input_cfg      = {0},
        .irq_no         = GPIO0_IRQ,
        .irq_info       = {{0},},
        .irq_now        = 0,
        .gpio_valid     = {0},
        .gpio_freeflag  = {0},
        .irq_flag       = {0},
        .base_bus       = GPIO0_BASE,
        .per_sel_reg    = 0,
    },
};

int __init gk_init_gpio(void)
{
    int     retval = 0;
    int     i, index;
    int     gpio_count;

    const char chip_type[][8] =
    {
        {"GK7101"},
        {"GK7102"},
        {"GK7101S"},
        {"GK7102S"},
        {"GK7102C"},
        {"GK7202"},
    };

    /* use usr memory's GPIO setting by uboot pass*/
    u32 usrmemphyaddr=0;
    int intphy_gpio_count;
    int extphy_gpio_count;

    usrmemphyaddr=get_usrmem_virt();
    memcpy((u8*)&gk_all_gpio_cfg, (u8*)usrmemphyaddr, (u32)(&gk_all_gpio_cfg.first) - (u32)(&gk_all_gpio_cfg.gpio_count));

    gk_all_gpio_cfg.board_type[31] = '\0';
    /*get gpio configure infor*/
    gpio_count = gk_all_gpio_cfg.gpio_count;
    intphy_gpio_count = gk_all_gpio_cfg.intphy_gpio_count;
    extphy_gpio_count = gk_all_gpio_cfg.extphy_gpio_count;

    gk_all_gpio_cfg.ir_led_ctl      = gk_all_gpio_cfg.gpio_count;
    gk_all_gpio_cfg.ir_cut1         = gk_all_gpio_cfg.gpio_count;
    gk_all_gpio_cfg.ir_cut2         = gk_all_gpio_cfg.gpio_count;
    gk_all_gpio_cfg.sensor_reset    = gk_all_gpio_cfg.gpio_count;
    gk_all_gpio_cfg.phy_reset       = gk_all_gpio_cfg.gpio_count;
    gk_all_gpio_cfg.phy_speed_led   = gk_all_gpio_cfg.gpio_count;
    gk_all_gpio_cfg.spi0_en0        = gk_all_gpio_cfg.gpio_count;
    gk_all_gpio_cfg.spi1_en0        = gk_all_gpio_cfg.gpio_count;
    gk_all_gpio_cfg.pwm0            = gk_all_gpio_cfg.gpio_count;
    gk_all_gpio_cfg.pwm1            = gk_all_gpio_cfg.gpio_count;
    gk_all_gpio_cfg.pwm2            = gk_all_gpio_cfg.gpio_count;
    gk_all_gpio_cfg.pwm3            = gk_all_gpio_cfg.gpio_count;
    gk_all_gpio_cfg.pwm4            = gk_all_gpio_cfg.gpio_count;
    gk_all_gpio_cfg.pwm5            = gk_all_gpio_cfg.gpio_count;
    gk_all_gpio_cfg.pwm6            = gk_all_gpio_cfg.gpio_count;
    gk_all_gpio_cfg.pwm7            = gk_all_gpio_cfg.gpio_count;
    gk_all_gpio_cfg.usb_host        = gk_all_gpio_cfg.gpio_count;
    gk_all_gpio_cfg.sd_detect       = gk_all_gpio_cfg.gpio_count;
    gk_all_gpio_cfg.sd_power        = gk_all_gpio_cfg.gpio_count;
    gk_all_gpio_cfg.sd1_detect      = gk_all_gpio_cfg.gpio_count;
    gk_all_gpio_cfg.sd1_power       = gk_all_gpio_cfg.gpio_count;
    for(i = 0; i < gpio_count; i++)
    {
        if(gk_all_gpio_cfg.gpio_chip[i].type == GK_GPIO_IR_LED_CTRL)
            gk_all_gpio_cfg.ir_led_ctl  = gk_all_gpio_cfg.gpio_chip[i].pin;
        if(gk_all_gpio_cfg.gpio_chip[i].type == GK_GPIO_IR_CUT1)
            gk_all_gpio_cfg.ir_cut1     = gk_all_gpio_cfg.gpio_chip[i].pin;
        if(gk_all_gpio_cfg.gpio_chip[i].type == GK_GPIO_IR_CUT2)
            gk_all_gpio_cfg.ir_cut2     = gk_all_gpio_cfg.gpio_chip[i].pin;
        if(gk_all_gpio_cfg.gpio_chip[i].type == GK_GPIO_SENSOR_RESET)
            gk_all_gpio_cfg.sensor_reset= gk_all_gpio_cfg.gpio_chip[i].pin;
        if(gk_all_gpio_cfg.gpio_chip[i].type == GK_GPIO_PHY_RESET)
            gk_all_gpio_cfg.phy_reset   = gk_all_gpio_cfg.gpio_chip[i].pin;
        if(gk_all_gpio_cfg.gpio_chip[i].type == GK_GPIO_PHY_SPEED_LED)
            gk_all_gpio_cfg.phy_speed_led = gk_all_gpio_cfg.gpio_chip[i].pin;
        if(gk_all_gpio_cfg.gpio_chip[i].type == GK_GPIO_SPI0_CS0)
            gk_all_gpio_cfg.spi0_en0    = gk_all_gpio_cfg.gpio_chip[i].pin;
        if(gk_all_gpio_cfg.gpio_chip[i].type == GK_GPIO_SPI1_CS0)
            gk_all_gpio_cfg.spi1_en0    = gk_all_gpio_cfg.gpio_chip[i].pin;
        if(gk_all_gpio_cfg.gpio_chip[i].type == GK_GPIO_PWM_0)
            gk_all_gpio_cfg.pwm0        = gk_all_gpio_cfg.gpio_chip[i].pin;
        if(gk_all_gpio_cfg.gpio_chip[i].type == GK_GPIO_PWM_1)
            gk_all_gpio_cfg.pwm1        = gk_all_gpio_cfg.gpio_chip[i].pin;
        if(gk_all_gpio_cfg.gpio_chip[i].type == GK_GPIO_PWM_2)
            gk_all_gpio_cfg.pwm2        = gk_all_gpio_cfg.gpio_chip[i].pin;
        if(gk_all_gpio_cfg.gpio_chip[i].type == GK_GPIO_PWM_3)
            gk_all_gpio_cfg.pwm3        = gk_all_gpio_cfg.gpio_chip[i].pin;
        if(gk_all_gpio_cfg.gpio_chip[i].type == GK_GPIO_PWM_4)
            gk_all_gpio_cfg.pwm4        = gk_all_gpio_cfg.gpio_chip[i].pin;
        if(gk_all_gpio_cfg.gpio_chip[i].type == GK_GPIO_PWM_5)
            gk_all_gpio_cfg.pwm5        = gk_all_gpio_cfg.gpio_chip[i].pin;
        if(gk_all_gpio_cfg.gpio_chip[i].type == GK_GPIO_PWM_6)
            gk_all_gpio_cfg.pwm6        = gk_all_gpio_cfg.gpio_chip[i].pin;
        if(gk_all_gpio_cfg.gpio_chip[i].type == GK_GPIO_PWM_7)
            gk_all_gpio_cfg.pwm7        = gk_all_gpio_cfg.gpio_chip[i].pin;
        if(gk_all_gpio_cfg.gpio_chip[i].type == GK_GPIO_USB_HOST)
            gk_all_gpio_cfg.usb_host    = gk_all_gpio_cfg.gpio_chip[i].pin;
        if(gk_all_gpio_cfg.gpio_chip[i].type == GK_GPIO_SDIO0_CD_N)
            gk_all_gpio_cfg.sd_detect   = gk_all_gpio_cfg.gpio_chip[i].pin;
        if(gk_all_gpio_cfg.gpio_chip[i].type == GK_GPIO_SDIO0_POWER)
            gk_all_gpio_cfg.sd_power    = gk_all_gpio_cfg.gpio_chip[i].pin;
        if(gk_all_gpio_cfg.gpio_chip[i].type == GK_GPIO_SDIO1_CD_N)
            gk_all_gpio_cfg.sd1_detect  = gk_all_gpio_cfg.gpio_chip[i].pin;
        if(gk_all_gpio_cfg.gpio_chip[i].type == GK_GPIO_SDIO1_POWER)
            gk_all_gpio_cfg.sd1_power   = gk_all_gpio_cfg.gpio_chip[i].pin;
    }
#if 1
    printk("###################################\n");
    if(gk_all_gpio_cfg.soc_type >= sizeof(chip_type)/ sizeof(chip_type[0]))
    {
        printk("[BOOT VERSION] %s %s v%d.%d \n", "Unknown",gk_all_gpio_cfg.board_type,
            gk_all_gpio_cfg.board_version>> 16, gk_all_gpio_cfg.board_version & 0xffff);
    }
    else
    {
        printk("[BOOT VERSION] %s %s v%d.%d \n", chip_type[gk_all_gpio_cfg.soc_type],gk_all_gpio_cfg.board_type,
            gk_all_gpio_cfg.board_version >> 16, gk_all_gpio_cfg.board_version & 0xffff);
    }
    if (gk_all_gpio_cfg.ext_phy_clk == 0)
    printk("[NET  INT_CLK] Internal PHY clock \n");
    else
    printk("[NET  EXT_CLK] External PHY clock %dMHz \n", gk_all_gpio_cfg.ext_phy_clk);
    printk("[GPIO]#############################\n");
    printk("[GPIO] gpio map get from uboot\n");
    printk("[GPIO CFG] gpio   count = %d\n",gk_all_gpio_cfg.gpio_count);
    printk("[GPIO CFG] intphy count = %d\n",gk_all_gpio_cfg.intphy_gpio_count);
    printk("[GPIO CFG] extphy count = %d\n",gk_all_gpio_cfg.extphy_gpio_count);
    if(gk_all_gpio_cfg.ir_led_ctl == gk_all_gpio_cfg.gpio_count)
    {
        printk("[GPIO CFG] IR LED CTL    (undefined)\n");
    }
    else
    {
        printk("[GPIO CFG] IR LED CTL    (%d)\n",gk_all_gpio_cfg.ir_led_ctl);
    }
    if(gk_all_gpio_cfg.ir_cut1 == gk_all_gpio_cfg.gpio_count)
    {
        printk("[GPIO CFG] IR CUT1       (undefined)\n");
    }
    else
    {
        printk("[GPIO CFG] IR CUT1       (%d)\n",gk_all_gpio_cfg.ir_cut1);
    }
    if(gk_all_gpio_cfg.ir_cut2 == gk_all_gpio_cfg.gpio_count)
    {
        printk("[GPIO CFG] IR CUT2       (undefined)\n");
    }
    else
    {
        printk("[GPIO CFG] IR CUT2       (%d)\n",gk_all_gpio_cfg.ir_cut2);
    }
    if(gk_all_gpio_cfg.sensor_reset == gk_all_gpio_cfg.gpio_count)
    {
        printk("[GPIO CFG] SENSOR Reset  (undefined)\n");
    }
    else
    {
        printk("[GPIO CFG] SENSOR Reset  (%d)\n",gk_all_gpio_cfg.sensor_reset);
    }
    if(gk_all_gpio_cfg.phy_reset == gk_all_gpio_cfg.gpio_count)
    {
        printk("[GPIO CFG] PHY Reset     (undefined)\n");
    }
    else
    {
        printk("[GPIO CFG] PHY Reset     (%d)\n",gk_all_gpio_cfg.phy_reset);
    }
    if(gk_all_gpio_cfg.phy_speed_led == gk_all_gpio_cfg.gpio_count)
    {
        printk("[GPIO CFG] PHY Speed Led (undefined)\n");
    }
    else
    {
        printk("[GPIO CFG] PHY Speed Led (%d)\n",gk_all_gpio_cfg.phy_speed_led);
    }
    if(gk_all_gpio_cfg.spi0_en0 == gk_all_gpio_cfg.gpio_count)
    {
        printk("[GPIO CFG] SPI0 EN       (undefined)\n");
    }
    else
    {
        printk("[GPIO CFG] SPI0 EN       (%d)\n",gk_all_gpio_cfg.spi0_en0);
    }
    if(gk_all_gpio_cfg.spi1_en0 == gk_all_gpio_cfg.gpio_count)
    {
        printk("[GPIO CFG] SPI1 EN       (undefined)\n");
    }
    else
    {
        printk("[GPIO CFG] SPI1 EN       (%d)\n",gk_all_gpio_cfg.spi1_en0);
    }
    if(gk_all_gpio_cfg.usb_host == gk_all_gpio_cfg.gpio_count)
    {
        printk("[GPIO CFG] USB HOST      (undefined)\n");
    }
    else
    {
        printk("[GPIO CFG] USB HOST      (%d)\n",gk_all_gpio_cfg.usb_host);
    }
    if(gk_all_gpio_cfg.sd_detect == gk_all_gpio_cfg.gpio_count)
    {
        printk("[GPIO CFG] SD Detect     (undefined)\n");
    }
    else
    {
        printk("[GPIO CFG] SD Detect     (%d)\n",gk_all_gpio_cfg.sd_detect);
    }
    if(gk_all_gpio_cfg.sd_power == gk_all_gpio_cfg.gpio_count)
    {
        printk("[GPIO CFG] SD Power      (undefined)\n");
    }
    else
    {
        printk("[GPIO CFG] SD Power      (%d)\n",gk_all_gpio_cfg.sd_power);
    }
    if(gk_all_gpio_cfg.sd1_detect == gk_all_gpio_cfg.gpio_count)
    {
        printk("[GPIO CFG] SD1 Detect    (undefined)\n");
    }
    else
    {
        printk("[GPIO CFG] SD1 Detect    (%d)\n",gk_all_gpio_cfg.sd1_detect);
    }
    if(gk_all_gpio_cfg.sd1_power == gk_all_gpio_cfg.gpio_count)
    {
        printk("[GPIO CFG] SD1 Power     (undefined)\n");
    }
    else
    {
        printk("[GPIO CFG] SD1 Power     (%d)\n",gk_all_gpio_cfg.sd1_power);
    }
    if(gk_all_gpio_cfg.pwm0== gk_all_gpio_cfg.gpio_count)
    {
        printk("[GPIO CFG] pwm0          (undefined)\n");
    }
    else
    {
        printk("[GPIO CFG] pwm0          (%d)\n",gk_all_gpio_cfg.pwm0);
    }
    if(gk_all_gpio_cfg.pwm1== gk_all_gpio_cfg.gpio_count)
    {
        printk("[GPIO CFG] pwm1          (undefined)\n");
    }
    else
    {
        printk("[GPIO CFG] pwm1          (%d)\n",gk_all_gpio_cfg.pwm1);
    }
    if(gk_all_gpio_cfg.pwm2== gk_all_gpio_cfg.gpio_count)
    {
        printk("[GPIO CFG] pwm2          (undefined)\n");
    }
    else
    {
        printk("[GPIO CFG] pwm2          (%d)\n",gk_all_gpio_cfg.pwm2);
    }
    if(gk_all_gpio_cfg.pwm3== gk_all_gpio_cfg.gpio_count)
    {
        printk("[GPIO CFG] pwm3          (undefined)\n");
    }
    else
    {
        printk("[GPIO CFG] pwm3          (%d)\n",gk_all_gpio_cfg.pwm3);
    }
    if(gk_all_gpio_cfg.pwm4== gk_all_gpio_cfg.gpio_count)
    {
        printk("[GPIO CFG] pwm4          (undefined)\n");
    }
    else
    {
        printk("[GPIO CFG] pwm4          (%d)\n",gk_all_gpio_cfg.pwm4);
    }
    if(gk_all_gpio_cfg.pwm5== gk_all_gpio_cfg.gpio_count)
    {
        printk("[GPIO CFG] pwm5          (undefined)\n");
    }
    else
    {
        printk("[GPIO CFG] pwm5          (%d)\n",gk_all_gpio_cfg.pwm5);
    }
    if(gk_all_gpio_cfg.pwm6== gk_all_gpio_cfg.gpio_count)
    {
        printk("[GPIO CFG] pwm6          (undefined)\n");
    }
    else
    {
        printk("[GPIO CFG] pwm6          (%d)\n",gk_all_gpio_cfg.pwm6);
    }
    if(gk_all_gpio_cfg.pwm7== gk_all_gpio_cfg.gpio_count)
    {
        printk("[GPIO CFG] pwm7          (undefined)\n");
    }
    else
    {
        printk("[GPIO CFG] pwm7          (%d)\n",gk_all_gpio_cfg.pwm7);
    }
    printk("[GPIO]#############################\n");
#endif

    goke_init_gpio();

    for(index=0; index < gpio_count; index++)
    {
        if(gk_all_gpio_cfg.gpio_chip[index].type != GPIO_TYPE_UNDEFINED)
        {
            gk_gpio_func_config(gk_all_gpio_cfg.gpio_chip[index].pin, gk_all_gpio_cfg.gpio_chip[index].type);
            printk("[GPIO CFG] gpio_%02d=%s\n", gk_all_gpio_cfg.gpio_chip[index].pin,
                gk_gpio_name_table[gk_all_gpio_cfg.gpio_chip[index].type]);
        }
    }

    if(cmdline_phytype == 0)
    {
        gpio_count = intphy_gpio_count;
        for(index=0; index < gpio_count; index++)
        {
            if(gk_all_gpio_cfg.int_phy_gpio[index].type != GPIO_TYPE_UNDEFINED)
            {
                gk_gpio_func_config(gk_all_gpio_cfg.int_phy_gpio[index].pin, gk_all_gpio_cfg.int_phy_gpio[index].type);
            }
        }
    }
    else
    {
        gpio_count = extphy_gpio_count;
        for(index=0; index < gpio_count; index++)
        {
            if(gk_all_gpio_cfg.ext_phy_gpio[index].type != GPIO_TYPE_UNDEFINED)
            {
                gk_gpio_func_config(gk_all_gpio_cfg.ext_phy_gpio[index].pin, gk_all_gpio_cfg.ext_phy_gpio[index].type);
            }
        }
    }

    for(i=0;i<CONFIG_GK_GPIO_INSTANCES;i++)
    {
        gk_gpio_writel(gk_gpio_insts[i].base_bus + REG_GPIO_INT_EN_OFFSET, 0x00000001);
    }

#if 0 // for gpio irq test
    {
        struct gk_gpio_irq_info info;
        info.pin = 10;
        info.type = IRQ_TYPE_EDGE_BOTH;
        info.handler = NULL;

        gk_gpio_request_irq(&info);
        printk("init gk_gpio_request_irq...\n");
    }
#endif
    return retval;
}

