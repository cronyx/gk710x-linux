/*
 * arch/arm/plat-goke/gk_gpio.c
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
#include <mach/io.h>

#if defined(CONFIG_SYSTEM_USE_NETLINK_MSG_GPIO)
#include <plat/gk_net_link.h>
#endif

char gk_gpio_name_table[GK_GPIO_FUN_ALL][20] =
{
    [(GK_GPIO_UNDEFINED)]           = "UNDEFINED",
    [(GK_GPIO_SF_CS0)]              = "SF_CS0",
    [(GK_GPIO_SF_CS1)]              = "SF_CS1",
    [(GK_GPIO_SF_HOLD)]             = "SF_HOLD",
    [(GK_GPIO_SF_WP)]               = "SF_WP",
    [(GK_GPIO_SPI0_SCLK)]           = "SPI0_SCLK",
    [(GK_GPIO_SPI0_SI)]             = "SPI0_SI",
    [(GK_GPIO_SPI0_SO)]             = "SPI0_SO",
    [(GK_GPIO_SPI0_HOLD)]           = "SPI0_HOLD",
    [(GK_GPIO_SPI0_WP)]             = "SPI0_WP",
    [(GK_GPIO_SPI0_CS0)]            = "SPI0_CS0",
    [(GK_GPIO_SPI0_CS1)]            = "SPI0_CS1",
    [(GK_GPIO_SPI0_CS2)]            = "SPI0_CS2",
    [(GK_GPIO_SPI0_CS3)]            = "SPI0_CS3",
    [(GK_GPIO_SPI0_CS4)]            = "SPI0_CS4",
    [(GK_GPIO_SPI0_CS5)]            = "SPI0_CS5",
    [(GK_GPIO_SPI0_CS6)]            = "SPI0_CS6",
    [(GK_GPIO_SPI0_CS7)]            = "SPI0_CS7",
    [(GK_GPIO_SPI1_SCLK)]           = "SPI1_SCLK",
    [(GK_GPIO_SPI1_SI)]             = "SPI1_SI",
    [(GK_GPIO_SPI1_SO)]             = "SPI1_SO",
    [(GK_GPIO_SPI1_HOLD)]           = "SPI1_HOLD",
    [(GK_GPIO_SPI1_WP)]             = "SPI1_WP",
    [(GK_GPIO_SPI1_CS0)]            = "SPI1_CS0",
    [(GK_GPIO_SPI1_CS1)]            = "SPI1_CS1",
    [(GK_GPIO_SPI1_CS2)]            = "SPI1_CS2",
    [(GK_GPIO_SPI1_CS3)]            = "SPI1_CS3",
    [(GK_GPIO_SPI1_CS4)]            = "SPI1_CS4",
    [(GK_GPIO_SPI1_CS5)]            = "SPI1_CS5",
    [(GK_GPIO_SPI1_CS6)]            = "SPI1_CS6",
    [(GK_GPIO_SPI1_CS7)]            = "SPI1_CS7",
    [(GK_GPIO_UART0_TX)]            = "UART0_TX",
    [(GK_GPIO_UART0_RX)]            = "UART0_RX",
    [(GK_GPIO_UART0_RTS_N)]         = "UART0_RTS_N",
    [(GK_GPIO_UART0_DTR_N)]         = "UART0_DTR_N",
    [(GK_GPIO_UART1_TX)]            = "UART1_TX",
    [(GK_GPIO_UART1_RX)]            = "UART1_RX",
    [(GK_GPIO_UART1_RTS_N)]         = "UART1_RTS_N",
    [(GK_GPIO_UART1_DTR_N)]         = "UART1_DTR_N",
    [(GK_GPIO_UART2_TX)]            = "UART2_TX",
    [(GK_GPIO_UART2_RX)]            = "UART2_RX",
    [(GK_GPIO_UART2_RTS_N)]         = "UART2_RTS_N",
    [(GK_GPIO_UART2_DTR_N)]         = "UART2_DTR_N",
    [(GK_GPIO_SDIO0_CLK)]           = "SDIO0_CLK",
    [(GK_GPIO_SDIO0_CMD)]           = "SDIO0_CMD",
    [(GK_GPIO_SDIO0_WP_N)]          = "SDIO0_WP_N",
    [(GK_GPIO_SDIO0_CD_N)]          = "SDIO0_CD_N",
    [(GK_GPIO_SDIO0_DATA_0)]        = "SDIO0_DATA_0",
    [(GK_GPIO_SDIO0_DATA_1)]        = "SDIO0_DATA_1",
    [(GK_GPIO_SDIO0_DATA_2)]        = "SDIO0_DATA_2",
    [(GK_GPIO_SDIO0_DATA_3)]        = "SDIO0_DATA_3",
    [(GK_GPIO_SDIO0_DATA_4)]        = "SDIO0_DATA_4",
    [(GK_GPIO_SDIO0_DATA_5)]        = "SDIO0_DATA_5",
    [(GK_GPIO_SDIO0_DATA_6)]        = "SDIO0_DATA_6",
    [(GK_GPIO_SDIO0_DATA_7)]        = "SDIO0_DATA_7",
    [(GK_GPIO_SDIO1_CLK)]           = "SDIO1_CLK",
    [(GK_GPIO_SDIO1_CMD)]           = "SDIO1_CMD",
    [(GK_GPIO_SDIO1_WP_N)]          = "SDIO1_WP_N",
    [(GK_GPIO_SDIO1_CD_N)]          = "SDIO1_CD_N",
    [(GK_GPIO_SDIO1_DATA_0)]        = "SDIO1_DATA_0",
    [(GK_GPIO_SDIO1_DATA_1)]        = "SDIO1_DATA_1",
    [(GK_GPIO_SDIO1_DATA_2)]        = "SDIO1_DATA_2",
    [(GK_GPIO_SDIO1_DATA_3)]        = "SDIO1_DATA_3",
    [(GK_GPIO_SDIO1_DATA_4)]        = "SDIO1_DATA_4",
    [(GK_GPIO_SDIO1_DATA_5)]        = "SDIO1_DATA_5",
    [(GK_GPIO_SDIO1_DATA_6)]        = "SDIO1_DATA_6",
    [(GK_GPIO_SDIO1_DATA_7)]        = "SDIO1_DATA_7",
    [(GK_GPIO_ENET_PHY_TXD_0)]      = "ENET_PHY_TXD_0",
    [(GK_GPIO_ENET_PHY_TXD_1)]      = "ENET_PHY_TXD_1",
    [(GK_GPIO_ENET_PHY_TXD_2)]      = "ENET_PHY_TXD_2",
    [(GK_GPIO_ENET_PHY_TXD_3)]      = "ENET_PHY_TXD_3",
    [(GK_GPIO_ENET_PHY_TXD_4)]      = "ENET_PHY_TXD_4",
    [(GK_GPIO_ENET_PHY_TXD_5)]      = "ENET_PHY_TXD_5",
    [(GK_GPIO_ENET_PHY_TXD_6)]      = "ENET_PHY_TXD_6",
    [(GK_GPIO_ENET_PHY_TXD_7)]      = "ENET_PHY_TXD_7",
    [(GK_GPIO_ENET_PHY_RXD_0)]      = "ENET_PHY_RXD_0",
    [(GK_GPIO_ENET_PHY_RXD_1)]      = "ENET_PHY_RXD_1",
    [(GK_GPIO_ENET_PHY_RXD_2)]      = "ENET_PHY_RXD_2",
    [(GK_GPIO_ENET_PHY_RXD_3)]      = "ENET_PHY_RXD_3",
    [(GK_GPIO_ENET_PHY_RXD_4)]      = "ENET_PHY_RXD_4",
    [(GK_GPIO_ENET_PHY_RXD_5)]      = "ENET_PHY_RXD_5",
    [(GK_GPIO_ENET_PHY_RXD_6)]      = "ENET_PHY_RXD_6",
    [(GK_GPIO_ENET_PHY_RXD_7)]      = "ENET_PHY_RXD_7",
    [(GK_GPIO_ENET_PHY_CLK_RX)]     = "ENET_PHY_CLK_RX",
    [(GK_GPIO_ENET_PHY_CLK_TX)]     = "ENET_PHY_CLK_TX",
    [(GK_GPIO_ENET_PHY_MDC)]        = "ENET_PHY_MDC",
    [(GK_GPIO_ENET_PHY_MDIO)]       = "ENET_PHY_MDIO",
    [(GK_GPIO_ENET_PHY_COL)]        = "ENET_PHY_COL",
    [(GK_GPIO_ENET_PHY_CRS)]        = "ENET_PHY_CRS",
    [(GK_GPIO_ENET_PHY_RXER)]       = "ENET_PHY_RXER",
    [(GK_GPIO_ENET_PHY_RXDV)]       = "ENET_PHY_RXDV",
    [(GK_GPIO_ENET_PHY_TXER)]       = "ENET_PHY_TXER",
    [(GK_GPIO_ENET_PHY_TXEN)]       = "ENET_PHY_TXEN",
    [(GK_GPIO_EPHY_LED_0)]          = "EPHY_LED_0",
    [(GK_GPIO_EPHY_LED_1)]          = "EPHY_LED_1",
    [(GK_GPIO_EPHY_LED_2)]          = "EPHY_LED_2",
    [(GK_GPIO_EPHY_LED_3)]          = "EPHY_LED_3",
    [(GK_GPIO_EPHY_LED_4)]          = "EPHY_LED_4",
    [(GK_GPIO_EPHY_LED_5)]          = "EPHY_LED_5",
    [(GK_GPIO_EPHY_LED_6)]          = "EPHY_LED_6",
    [(GK_GPIO_EPHY_LED_7)]          = "EPHY_LED_7",
    [(GK_GPIO_PWM_0)]               = "PWM_0",
    [(GK_GPIO_PWM_1)]               = "PWM_1",
    [(GK_GPIO_PWM_2)]               = "PWM_2",
    [(GK_GPIO_PWM_3)]               = "PWM_3",
    [(GK_GPIO_PWM_4)]               = "PWM_4",
    [(GK_GPIO_PWM_5)]               = "PWM_5",
    [(GK_GPIO_PWM_6)]               = "PWM_6",
    [(GK_GPIO_PWM_7)]               = "PWM_7",
    [(GK_GPIO_PWM_8)]               = "PWM_8",
    [(GK_GPIO_PWM_9)]               = "PWM_9",
    [(GK_GPIO_PWM_10)]              = "PWM_10",
    [(GK_GPIO_PWM_11)]              = "PWM_11",
    [(GK_GPIO_PWM_12)]              = "PWM_12",
    [(GK_GPIO_PWM_13)]              = "PWM_13",
    [(GK_GPIO_PWM_14)]              = "PWM_14",
    [(GK_GPIO_PWM_15)]              = "PWM_15",
    [(GK_GPIO_VD_CLOCK)]            = "VD_CLOCK",
    [(GK_GPIO_VD_VSYNC)]            = "VD_VSYNC",
    [(GK_GPIO_VD_HSYNC)]            = "VD_HSYNC",
    [(GK_GPIO_VD_HVLD)]             = "VD_HVLD",
    [(GK_GPIO_VD_DATA0)]            = "VD_DATA0",
    [(GK_GPIO_VD_DATA1)]            = "VD_DATA1",
    [(GK_GPIO_VD_DATA2)]            = "VD_DATA2",
    [(GK_GPIO_VD_DATA3)]            = "VD_DATA3",
    [(GK_GPIO_VD_DATA4)]            = "VD_DATA4",
    [(GK_GPIO_VD_DATA5)]            = "VD_DATA5",
    [(GK_GPIO_VD_DATA6)]            = "VD_DATA6",
    [(GK_GPIO_VD_DATA7)]            = "VD_DATA7",
    [(GK_GPIO_VD_DATA8)]            = "VD_DATA8",
    [(GK_GPIO_VD_DATA9)]            = "VD_DATA9",
    [(GK_GPIO_VD_DATA10)]           = "VD_DATA10",
    [(GK_GPIO_VD_DATA11)]           = "VD_DATA11",
    [(GK_GPIO_VD_DATA12)]           = "VD_DATA12",
    [(GK_GPIO_VD_DATA13)]           = "VD_DATA13",
    [(GK_GPIO_VD_DATA14)]           = "VD_DATA14",
    [(GK_GPIO_VD_DATA15)]           = "VD_DATA15",
    [(GK_GPIO_VD_DATA16)]           = "VD_DATA16",
    [(GK_GPIO_VD_DATA17)]           = "VD_DATA17",
    [(GK_GPIO_VD_DATA18)]           = "VD_DATA18",
    [(GK_GPIO_VD_DATA19)]           = "VD_DATA19",
    [(GK_GPIO_VD_DATA20)]           = "VD_DATA20",
    [(GK_GPIO_VD_DATA21)]           = "VD_DATA21",
    [(GK_GPIO_VD_DATA22)]           = "VD_DATA22",
    [(GK_GPIO_VD_DATA23)]           = "VD_DATA23",
    [(GK_GPIO_I80_LCD_RST)]         = "I80_LCD_RST",
    [(GK_GPIO_I80_RDN)]             = "I80_RDN",
    [(GK_GPIO_I80_WRN)]             = "I80_WRN",
    [(GK_GPIO_I80_DCX)]             = "I80_DCX",
    [(GK_GPIO_I80_CSN)]             = "I80_CSN",
    [(GK_GPIO_I80_DATA0)]           = "I80_DATA0",
    [(GK_GPIO_I80_DATA1)]           = "I80_DATA1",
    [(GK_GPIO_I80_DATA2)]           = "I80_DATA2",
    [(GK_GPIO_I80_DATA3)]           = "I80_DATA3",
    [(GK_GPIO_I80_DATA4)]           = "I80_DATA4",
    [(GK_GPIO_I80_DATA5)]           = "I80_DATA5",
    [(GK_GPIO_I80_DATA6)]           = "I80_DATA6",
    [(GK_GPIO_I80_DATA7)]           = "I80_DATA7",
    [(GK_GPIO_I80_DATA8)]           = "I80_DATA8",
    [(GK_GPIO_I80_OUTPUT_DATA0)]    = "I80_OUTPUT_DATA0",
    [(GK_GPIO_I80_OUTPUT_DATA1)]    = "I80_OUTPUT_DATA1",
    [(GK_GPIO_I80_OUTPUT_DATA2)]    = "I80_OUTPUT_DATA2",
    [(GK_GPIO_I80_OUTPUT_DATA3)]    = "I80_OUTPUT_DATA3",
    [(GK_GPIO_I80_OUTPUT_DATA4)]    = "I80_OUTPUT_DATA4",
    [(GK_GPIO_I80_OUTPUT_DATA5)]    = "I80_OUTPUT_DATA5",
    [(GK_GPIO_I80_OUTPUT_DATA6)]    = "I80_OUTPUT_DATA6",
    [(GK_GPIO_I80_OUTPUT_DATA7)]    = "I80_OUTPUT_DATA7",
    [(GK_GPIO_I80_OUTPUT_DATA8)]    = "I80_OUTPUT_DATA8",
    [(GK_GPIO_I80_INPUT_DATA0)]     = "I80_INPUT_DATA0",
    [(GK_GPIO_I80_INPUT_DATA1)]     = "I80_INPUT_DATA1",
    [(GK_GPIO_I80_INPUT_DATA2)]     = "I80_INPUT_DATA2",
    [(GK_GPIO_I80_INPUT_DATA3)]     = "I80_INPUT_DATA3",
    [(GK_GPIO_I80_INPUT_DATA4)]     = "I80_INPUT_DATA4",
    [(GK_GPIO_I80_INPUT_DATA5)]     = "I80_INPUT_DATA5",
    [(GK_GPIO_I80_INPUT_DATA6)]     = "I80_INPUT_DATA6",
    [(GK_GPIO_I80_INPUT_DATA7)]     = "I80_INPUT_DATA7",
    [(GK_GPIO_I80_INPUT_DATA8)]     = "I80_INPUT_DATA8",
    [(GK_GPIO_I2C0_CLK)]            = "I2C0_CLK",
    [(GK_GPIO_I2C0_DATA)]           = "I2C0_DATA",
    [(GK_GPIO_I2C1_CLK)]            = "I2C1_CLK",
    [(GK_GPIO_I2C1_DATA)]           = "I2C1_DATA",
    [(GK_GPIO_I2C2_CLK)]            = "I2C2_CLK",
    [(GK_GPIO_I2C2_DATA)]           = "I2C2_DATA",
    [(GK_GPIO_AO0_MCLK)]            = "AO0_MCLK",
    [(GK_GPIO_AO0_BCLK)]            = "AO0_BCLK",
    [(GK_GPIO_AO0_LRCLK)]           = "AO0_LRCLK",
    [(GK_GPIO_AO0_DATA)]            = "AO0_DATA",
    [(GK_GPIO_AO1_MCLK)]            = "AO1_MCLK",
    [(GK_GPIO_AO1_BCLK)]            = "AO1_BCLK",
    [(GK_GPIO_AO1_LRCLK)]           = "AO1_LRCLK",
    [(GK_GPIO_AO1_DATA)]            = "AO1_DATA",
    [(GK_GPIO_AI0_MCLK)]            = "AI0_MCLK",
    [(GK_GPIO_AI0_BCLK)]            = "AI0_BCLK",
    [(GK_GPIO_AI0_LRCLK)]           = "AI0_LRCLK",
    [(GK_GPIO_AI0_DATA)]            = "AI0_DATA",
    [(GK_GPIO_AI1_MCLK)]            = "AI1_MCLK",
    [(GK_GPIO_AI1_BCLK)]            = "AI1_BCLK",
    [(GK_GPIO_AI1_LRCLK)]           = "AI1_LRCLK",
    [(GK_GPIO_AI1_DATA)]            = "AI1_DATA",
    [(GK_GPIO_OUTPUT_0)]            = "OUTPUT_0",
    [(GK_GPIO_OUTPUT_1)]            = "OUTPUT_1",
    [(GK_GPIO_INPUT_0)]             = "INPUT_0",
    [(GK_GPIO_INPUT_1)]             = "INPUT_1",
    [(GK_GPIO_INPUT)]               = "INPUT",
    [(GK_GPIO_SENSOR_POWER)]        = "SENSOR_POWER",
    [(GK_GPIO_SENSOR_RESET)]        = "SENSOR_RESET",
    [(GK_GPIO_PHY_RESET)]           = "PHY_RESET",
    [(GK_GPIO_PHY_SPEED_LED)]       = "PHY_SPEED_LED",
    [(GK_GPIO_PHY_LINK_LED)]        = "PHY_LINK_LED",
    [(GK_GPIO_PHY_DATA_LED)]        = "PHY_DATA_LED",
    [(GK_GPIO_IR_LED_CTRL)]         = "IR_LED_CTRL",
    [(GK_GPIO_IR_DETECT)]           = "IR_DETECT",
    [(GK_GPIO_IR_CUT1)]             = "IR_CUT1",
    [(GK_GPIO_IR_CUT2)]             = "IR_CUT2",
    [(GK_GPIO_ALARM_IN)]            = "ALARM_IN",
    [(GK_GPIO_ALARM_OUT)]           = "ALARM_OUT",
    [(GK_GPIO_USB_HOST)]            = "USB_HOST",
    [(GK_GPIO_USB_OTG)]             = "USB_OTG",
    [(GK_GPIO_SDIO0_POWER)]         = "SDIO0_POWER",
    [(GK_GPIO_SDIO1_POWER)]         = "SDIO1_POWER",
    [(GK_GPIO_PMU_CTL)]             = "PMU_CTL",
    [(GK_GPIO_JTAG_TRSTN)]          = "JTAG_TRSTN",
    [(GK_GPIO_JTAG_TCK)]            = "JTAG_TCK",
    [(GK_GPIO_JTAG_TMS)]            = "JTAG_TMS",
    [(GK_GPIO_JTAG_TDI)]            = "JTAG_TDI",
    [(GK_GPIO_JTAG_TDO)]            = "JTAG_TDO",
    [(GK_GPIO_TIMER1_CLK)]          = "TIMER1_CLK",
    [(GK_GPIO_TIMER2_CLK)]          = "TIMER2_CLK",
    [(GK_GPIO_TIMER3_CLK)]          = "TIMER3_CLK",
    [(GK_GPIO_RCT_CLK_OUT1)]        = "RCT_CLK_OUT1",
    [(GK_GPIO_RCT_CLK_OUT2)]        = "RCT_CLK_OUT2",
};

/*
    m instances per chip
    n banks per instance
    max 64 pins per bank
    pin: used as pin number at chip
    offset: used as offset at it's instance
    pin = it's instance base number + offset
*/

static DEFINE_MUTEX(gk_gpio_mtx);
extern struct gk_gpio_inst gk_gpio_insts[CONFIG_GK_GPIO_INSTANCES];
extern GPIO_TYPE_E gk_gpio_table[GK_GPIO_FUN_ALL];

void GH_GPIO_set_INPUT_CFG_in_sel(u8 index, u8 data)
{
    GH_GPIO_INPUT_CFG_S  in_data;
    in_data.all = gk_gpio_insts[0].input_cfg[index];
    in_data.bitc.in_sel = data;
    gk_gpio_insts[0].input_cfg[index] = in_data.all;
    gk_gpio_writel(gk_gpio_insts[0].gpio_bank[0].base_reg + REG_GPIO_INPUT_CFG_OFFSET + (index * 0x4), in_data.all);
}

/* ==========================================================================*/
#define to_gk_gpio_chip(c) \
    container_of(c, struct gk_gpio_bank, chip)

static struct gk_gpio_inst* gk_gpio_id_to_inst(u32 pin)
{
    u32 i;
    u32 j;
    if (pin < 0)
    {
        return NULL;
    }
    else
    {
        for(i=0; i<CONFIG_GK_GPIO_INSTANCES; i++)
        {
            for(j=0; j<gk_gpio_insts[i].bank_num; j++)
            {
                if((pin >= gk_gpio_insts[i].gpio_bank[j].chip.base) &&
                   (pin < (gk_gpio_insts[i].gpio_bank[j].chip.base +
                          gk_gpio_insts[i].gpio_bank[j].chip.ngpio)))
                {
                    return &gk_gpio_insts[i];
                }
            }
        }
    }
    return NULL;
}

static struct gk_gpio_bank* gk_gpio_id_to_bank(u32 pin)
{
    u32 i;
    u32 j;
    if (pin< 0)
    {
        return NULL;
    }
    else
    {
        for(i=0; i<CONFIG_GK_GPIO_INSTANCES; i++)
        {
            for(j=0; j<gk_gpio_insts[i].bank_num; j++)
            {
                if((pin >= gk_gpio_insts[i].gpio_bank[j].chip.base) &&
                   (pin < (gk_gpio_insts[i].gpio_bank[j].chip.base +
                          gk_gpio_insts[i].gpio_bank[j].chip.ngpio)))
                {
                    return &gk_gpio_insts[i].gpio_bank[j];
                }
            }
        }
    }
    return NULL;
}



int gk_gpio_request(struct gpio_chip *chip, u32 offset)
{
    int ret = 0;
    struct gk_gpio_bank *bank;
    u32 pin;

    bank = to_gk_gpio_chip(chip);
    mutex_lock(&gk_gpio_mtx);

    pin = offset + chip->base;
    if (test_bit(pin, (const volatile unsigned long *)
        gk_gpio_insts[bank->index].gpio_valid))
    {
        if (test_bit(pin, (const volatile unsigned long *)
            gk_gpio_insts[bank->index].gpio_freeflag))
        {
            __clear_bit(pin, (volatile unsigned long *)
                gk_gpio_insts[bank->index].gpio_freeflag);
        }
        else
        {
            ret = -EACCES;
        }
    }
    else
    {
        ret = -EPERM;
    }

    mutex_unlock(&gk_gpio_mtx);

    return ret;
}

void gk_gpio_free(struct gpio_chip *chip, u32 offset)
{
    u32 pin;
    struct gk_gpio_bank *bank;

    mutex_lock(&gk_gpio_mtx);

    pin = offset + chip->base;
    bank = to_gk_gpio_chip(chip);

    __set_bit(pin, (volatile unsigned long *)
        gk_gpio_insts[bank->index].gpio_freeflag);

    mutex_unlock(&gk_gpio_mtx);
}

int gk_gpio_func_config(u32 pin, u32 func)
{
    int                 ret = 0;
    unsigned long       flags;
    struct gk_gpio_bank *bank;

    bank = gk_gpio_id_to_bank(pin);
    if (bank == NULL)
    {
        pr_err("%s: invalid GPIO %d for func %d.\n", __func__, pin, func);
        return -EINVAL;
    }
    spin_lock_irqsave(&bank->lock, flags);

    if(GPIO_GET_FUNC(func) > GPIO_FUNC_INOUT)
    {
        pr_err("%s: invalid GPIO func %d for GPIO:%d.\n", __func__, func, pin);
        spin_unlock_irqrestore(&bank->lock, flags);
        return -EINVAL;
    }

    //printk("gpio==============[%s %d]\n", __func__, __LINE__);
    ret =  gk_gpio_set_type(bank, pin, func);
    spin_unlock_irqrestore(&bank->lock, flags);

    return ret;

}

int gk_gpio_direction_input(struct gpio_chip *chip, u32 offset, int val)
{
    int ret = 0;
    u32 pin;

    pin = offset + chip->base;
    if (val == 0)
        ret = gk_gpio_func_config(pin, GPIO_TYPE_INPUT_0);
    else
        ret = gk_gpio_func_config(pin, GPIO_TYPE_INPUT_1);
    return ret;
}

static inline int gk_gpio_inline_get(struct gk_gpio_bank *bank, u32 pin)
{
    unsigned long   flags;
    u32 val = 0;

    spin_lock_irqsave(&bank->lock, flags);

    if((pin - bank->chip.base) < GPIO_REG_LL_N)
    {
        val = gk_gpio_readl(bank->base_reg + REG_GPIO_DIN_LOW_OFFSET);
        val = (val >> pin) & 0x1;
    }
    else if((pin - bank->chip.base) < 2*GPIO_REG_LL_N)
    {
        val = gk_gpio_readl(bank->base_reg + REG_GPIO_DIN_HIGH_OFFSET);
        val = (val >> (pin - GPIO_REG_LL_N)) & 0x1;
    }
#ifdef REG_GPIO_DIN_HIGH_H_OFFSET
    else
    {
        val = gk_gpio_readl(bank->base_reg + REG_GPIO_DIN_HIGH_H_OFFSET);
        val = (val >> (pin - 2*GPIO_REG_LL_N)) & 0x1;
    }
#endif
    spin_unlock_irqrestore(&bank->lock, flags);

    return (val ? GPIO_HIGH : GPIO_LOW);

}

int gk_gpio_get(u32 pin)
{
    struct gk_gpio_bank *bank;

    bank = gk_gpio_id_to_bank(pin);
    if (bank == NULL)
    {
        pr_err("%s: invalid GPIO %d.\n", __func__, pin);
        return 0;
    }

    return gk_gpio_inline_get(bank, (u32)pin);
}
EXPORT_SYMBOL(gk_gpio_get);

int gk_gpio_get_ex(struct gpio_chip *chip, unsigned offset)
{
    u32                 pin;

    pin = offset + chip->base;
    return gk_gpio_get(pin);
}
EXPORT_SYMBOL(gk_gpio_get_ex);

void gk_gpio_set_out(u32 pin, u32 value)
{
    struct gk_gpio_bank* bank;
    GH_GPIO_OUTPUT_CFG_S data;
    bank = gk_gpio_id_to_bank(pin);
    if(bank)
    {
        data.all = gk_gpio_insts[bank->index].output_cfg[pin];
        data.bitc.out_sel = 1;
        if (value == GPIO_LOW)
        {
            data.bitc.out_sel &= ~0x01;
        }
        else
        {
            data.bitc.out_sel |= 0x01;
        }
        gk_gpio_writel(bank->base_reg + REG_GPIO_OUTPUT_CFG_OFFSET + ((pin - bank->chip.base) * 0x4),
            data.all);
        gk_gpio_insts[bank->index].output_cfg[pin] = data.all;
    }
}
EXPORT_SYMBOL(gk_gpio_set_out);

static inline void gk_gpio_inline_set(struct gk_gpio_bank *bank, u32 pin, int value)
{
    unsigned long               flags;

    spin_lock_irqsave(&bank->lock, flags);
    gk_gpio_set_out(pin, value);
    spin_unlock_irqrestore(&bank->lock, flags);
}

int gk_gpio_direction_output(struct gpio_chip *chip, unsigned offset, int val)
{
    int                 ret = 0;
    u32                 pin;
    struct gk_gpio_bank *bank;

    pin = offset + chip->base;

    bank = gk_gpio_id_to_bank(pin);
    ret = gk_gpio_func_config(pin, val ? GPIO_TYPE_OUTPUT_1 : GPIO_TYPE_OUTPUT_0);
    gk_gpio_inline_set(bank, pin, val);

    return ret;
}

void gk_gpio_set(struct gpio_chip *chip, unsigned offset, int val)
{
    struct gk_gpio_bank *bank;
    u32                 pin;

    pin = offset + chip->base;

    bank = gk_gpio_id_to_bank(pin);
    if (bank == NULL)
    {
        pr_err("%s: invalid GPIO %d.\n", __func__, pin);
        return;
    }
    gk_gpio_inline_set(bank, pin, val);
}
EXPORT_SYMBOL(gk_gpio_set);

int gk_gpio_to_irq(struct gpio_chip *chip, unsigned offset)
{
    struct gk_gpio_bank *bank;
    u32                 pin;
    pin = offset + chip->base;

    bank = to_gk_gpio_chip(chip);
    return gk_gpio_insts[bank->index].irq_no;
}

void gk_gpio_dbg_show(struct seq_file *s, struct gpio_chip *chip)
{
    int                 i;
    struct gk_gpio_bank *bank;
    u32                 afsel;
    u32                 lmask;
    u32                 data;
    u32                 hmask;
    unsigned long       flags;

    bank = to_gk_gpio_chip(chip);

    spin_lock_irqsave(&bank->lock, flags);
    afsel = gk_gpio_readl(gk_gpio_insts[bank->index].base_bus + REG_GPIO_PER_SEL_OFFSET);
    lmask = gk_gpio_readl(bank->base_reg + REG_GPIO_IE_LOW_OFFSET);
    hmask = gk_gpio_readl(bank->base_reg + REG_GPIO_IE_HIGH_OFFSET);
    data = gk_gpio_readl(gk_gpio_insts[bank->index].base_bus + REG_GPIO_INT_EN_OFFSET);
    spin_unlock_irqrestore(&bank->lock, flags);

    seq_printf(s, "GPIO_BASE:\t0x%08X\n", bank->base_reg);
    seq_printf(s, "GPIO_PSEL:\t0x%08X\n", afsel);
    seq_printf(s, "GPIO_MASK:\t0x%08X:0x%08X\n", hmask, lmask);
    seq_printf(s, "GPIO_GPEN:\t0x%08X\n", data);

    for (i = 0; i < chip->ngpio; i++)
    {
        seq_printf(s, "GPIO %d: HW\n", (chip->base + i));
    }
}

int gk_gpio_set_type(struct gk_gpio_bank* bank, u32 pin, u32 func)
{
    GH_GPIO_OUTPUT_CFG_S out_data;
    GH_GPIO_INPUT_CFG_S  in_data;
    GH_PLL_IOCTRL_GPIO_S io_data;
    u32 type;

    out_data.all    = gk_gpio_insts[bank->index].output_cfg[pin];
    if(func < GK_GPIO_FUN_ALL)
    {
        type = gk_gpio_table[func];
        if(type == GPIO_TYPE_UNDEFINED)
        {
            return 0;
        }
    }
    else
    {
        type = func;
    }
    switch(GPIO_GET_FUNC(type))
    {
    case GPIO_FUNC_OUT:     // out
        out_data.bitc.out_sel = GPIO_GET_OUT_SEL(type);
        out_data.bitc.oen_sel = GPIO_GET_OEN_SEL(type);
        break;
    case GPIO_FUNC_IN:     // in
        if(GPIO_GET_IN_SEL(type) >= 2)
        {
            out_data.bitc.out_sel   = GPIO_GET_OUT_SEL(type);
            out_data.bitc.oen_sel   = GPIO_GET_OEN_SEL(type);
            in_data.all             = gk_gpio_insts[bank->index].input_cfg[GPIO_GET_IN_SEL(type) - 2];
            in_data.bitc.in_sel     = pin;
            gk_gpio_writel(bank->base_reg + REG_GPIO_INPUT_CFG_OFFSET + ((GPIO_GET_IN_SEL(type) - 2) * 0x4),
                in_data.all);
            gk_gpio_insts[bank->index].input_cfg[GPIO_GET_IN_SEL(type) - 2] = in_data.all;
        }
        else
        {
            out_data.bitc.out_sel = GPIO_GET_OUT_SEL(type);
            out_data.bitc.oen_sel = GPIO_GET_OEN_SEL(type);
        }
        break;
    case GPIO_FUNC_INOUT:     // in+out
        // don't change, otherwise if out_sel at first might output a 0, then change to 1
        in_data.all             = gk_gpio_insts[bank->index].input_cfg[GPIO_GET_IN_SEL(type) - 2];
        in_data.bitc.in_sel     = pin;
        gk_gpio_writel(bank->base_reg + REG_GPIO_INPUT_CFG_OFFSET + ((GPIO_GET_IN_SEL(type) - 2) * 0x4),
            in_data.all);
        gk_gpio_insts[bank->index].input_cfg[GPIO_GET_IN_SEL(type) - 2] = in_data.all;
        out_data.bitc.oen_sel = GPIO_GET_OEN_SEL(type);
        out_data.bitc.out_sel = GPIO_GET_OUT_SEL(type);
        break;
    default:
        return -EINVAL;
    }
    out_data.bitc.oen_invert = GPIO_GET_OEN_INVERT(type);
    out_data.bitc.out_invert = GPIO_GET_OUT_INVERT(type);
    gk_gpio_writel(bank->base_reg + REG_GPIO_OUTPUT_CFG_OFFSET + ((pin - bank->chip.base) * 0x4),
        out_data.all);
    gk_gpio_insts[bank->index].output_cfg[pin] = out_data.all;
    // Pull up/down & 2mA......
#if defined(CONFIG_ARCH_GK710X)
    io_data.all = gk_gpio_readl((bank->io_reg - (((pin-bank->chip.base)/0x04) * 0x04)));
    switch(pin%4)
    {
    case 0:
        io_data.bitc.io0 = GPIO_GET_IOCTRL(type);
        break;
    case 1:
        io_data.bitc.io1 = GPIO_GET_IOCTRL(type);
        break;
    case 2:
        io_data.bitc.io2 = GPIO_GET_IOCTRL(type);
        break;
    case 3:
        io_data.bitc.io3 = GPIO_GET_IOCTRL(type);
        break;
    }
    gk_gpio_writel((bank->io_reg - (((pin-bank->chip.base)/0x04) * 0x04)), io_data.all);
#elif defined(CONFIG_ARCH_GK710XS)
    if(pin<4)//gpio0-3
    {
        io_data.all = gk_gpio_readl((bank->io_reg + ((((55-pin)-bank->chip.base)/0x04) * 0x04)));
        switch(pin%4)
        {
        case 0:
            io_data.bitc.io2 = GPIO_GET_IOCTRL(type);
            break;
        case 1:
            io_data.bitc.io0 = GPIO_GET_IOCTRL(type);
            break;
        case 2:
            io_data.bitc.io1 = GPIO_GET_IOCTRL(type);
            break;
        case 3:
            io_data.bitc.io3 = GPIO_GET_IOCTRL(type);
            break;
        }
        gk_gpio_writel((bank->io_reg + ((((55-pin)-bank->chip.base)/0x04) * 0x04)), io_data.all);
    }
    else if(pin<52)//gpio4-51
    {
        io_data.all = gk_gpio_readl((bank->io_reg + ((((55-pin)-bank->chip.base)/0x04) * 0x04)));
        switch(pin%4)
        {
        case 0:
            io_data.bitc.io1 = GPIO_GET_IOCTRL(type);
            break;
        case 1:
            io_data.bitc.io2 = GPIO_GET_IOCTRL(type);
            break;
        case 2:
            io_data.bitc.io0 = GPIO_GET_IOCTRL(type);
            break;
        case 3:
            io_data.bitc.io3 = GPIO_GET_IOCTRL(type);
            break;
        }
        gk_gpio_writel((bank->io_reg + ((((55-pin)-bank->chip.base)/0x04) * 0x04)), io_data.all);

    }
    else if(pin<56)//gpio52-55
    {
        io_data.all = gk_gpio_readl((bank->io_reg + ((((55-pin)-bank->chip.base)/0x04) * 0x04)));
        switch(pin%4)
        {
        case 0:
            io_data.bitc.io1 = GPIO_GET_IOCTRL(type);
            break;
        case 1:
            io_data.bitc.io0 = GPIO_GET_IOCTRL(type);
            break;
        case 2:
            io_data.bitc.io2 = GPIO_GET_IOCTRL(type);
            break;
        case 3:
            io_data.bitc.io3 = GPIO_GET_IOCTRL(type);
            break;
        }
        gk_gpio_writel((bank->io_reg + ((((55-pin)-bank->chip.base)/0x04) * 0x04)), io_data.all);
    }
    else//gpio56-62
    {
        io_data.all = gk_gpio_readl((bank->io_reg + (((pin-bank->chip.base)/0x04) * 0x04)));
        switch(pin%4)
        {
        case 0:
            io_data.bitc.io0 = GPIO_GET_IOCTRL(type);
            break;
        case 1:
            io_data.bitc.io1 = GPIO_GET_IOCTRL(type);
            break;
        case 2:
            io_data.bitc.io2 = GPIO_GET_IOCTRL(type);
            break;
        case 3:
            io_data.bitc.io3 = GPIO_GET_IOCTRL(type);
            break;
        }
        gk_gpio_writel((bank->io_reg + (((pin-bank->chip.base)/0x04) * 0x04)), io_data.all);
    }
#else//gk7102c
    io_data.all = gk_gpio_readl((bank->io_reg + (((pin-bank->chip.base)/0x04 + 3) * 0x04)));
    if(pin<40)
    {
        switch(pin%4)
        {
        case 0:
            io_data.bitc.io3 = GPIO_GET_IOCTRL(type);
            break;
        case 1:
            io_data.bitc.io0 = GPIO_GET_IOCTRL(type);
            break;
        case 2:
            io_data.bitc.io2 = GPIO_GET_IOCTRL(type);
            break;
        case 3:
            io_data.bitc.io1 = GPIO_GET_IOCTRL(type);
            break;
        }
    }
    else if(pin<44)
    {
        switch(pin%4)
        {
        case 0:
            io_data.bitc.io3 = GPIO_GET_IOCTRL(type);
            break;
        case 1:
            io_data.bitc.io1 = GPIO_GET_IOCTRL(type);
            break;
        case 2:
            io_data.bitc.io0 = GPIO_GET_IOCTRL(type);
            break;
        case 3:
            io_data.bitc.io2 = GPIO_GET_IOCTRL(type);
            break;
        }
    }
    gk_gpio_writel((bank->io_reg + (((pin-bank->chip.base)/0x04 + 3) * 0x04)), io_data.all);
#endif
    return(0);
}

void gk_gpio_config(u32 pin, u32 func)
{
    if(gk_gpio_func_config((u32)pin, func))
    {
        pr_err("%s: failed to configure GPIO %d for func %d.\n", __func__, pin, func);
    }
}
EXPORT_SYMBOL(gk_gpio_config);

void gk_gpio_raw_lock(u32 pin, unsigned long *pflags)
{
    struct gk_gpio_bank *bank;
    bank = gk_gpio_id_to_bank(pin);
    if (bank == NULL)
    {
        pr_err("%s: invalid GPIO %d.\n", __func__, pin);
        return;
    }
    spin_lock_irqsave(&bank->lock, *pflags);
}

void gk_gpio_raw_unlock(u32 pin, unsigned long *pflags)
{
    struct gk_gpio_bank *bank;
    bank = gk_gpio_id_to_bank(pin);
    if (bank == NULL)
    {
        pr_err("%s: invalid GPIO %d.\n", __func__, pin);
        return;
    }
    spin_unlock_irqrestore(&bank->lock, *pflags);
}

u32 gk_gpio_suspend(u32 level)
{
    u32             i;
    u32             j;
    unsigned long   flags;

    for (i = 0; i < CONFIG_GK_GPIO_INSTANCES; i++)
    {
        for (j = 0; j < gk_gpio_insts[i].bank_num; j++)
        {
            spin_lock_irqsave(&gk_gpio_insts[i].gpio_bank[j].lock, flags);
            gk_gpio_insts[i].gpio_bank[j].pm_info.isl_reg  =
                gk_gpio_readl(gk_gpio_insts[i].gpio_bank[j].base_reg + REG_GPIO_IS_LOW_OFFSET);
            gk_gpio_insts[i].gpio_bank[j].pm_info.ish_reg  =
                gk_gpio_readl(gk_gpio_insts[i].gpio_bank[j].base_reg + REG_GPIO_IS_HIGH_OFFSET);
            gk_gpio_insts[i].gpio_bank[j].pm_info.ibel_reg =
                gk_gpio_readl(gk_gpio_insts[i].gpio_bank[j].base_reg + REG_GPIO_IBE_LOW_OFFSET);
            gk_gpio_insts[i].gpio_bank[j].pm_info.ibeh_reg =
                gk_gpio_readl(gk_gpio_insts[i].gpio_bank[j].base_reg + REG_GPIO_IBE_HIGH_OFFSET);
            gk_gpio_insts[i].gpio_bank[j].pm_info.ievl_reg =
                gk_gpio_readl(gk_gpio_insts[i].gpio_bank[j].base_reg + REG_GPIO_IEV_LOW_OFFSET);
            gk_gpio_insts[i].gpio_bank[j].pm_info.ievh_reg =
                gk_gpio_readl(gk_gpio_insts[i].gpio_bank[j].base_reg + REG_GPIO_IEV_HIGH_OFFSET);
            gk_gpio_insts[i].gpio_bank[j].pm_info.iel_reg  =
                gk_gpio_readl(gk_gpio_insts[i].gpio_bank[j].base_reg + REG_GPIO_IE_LOW_OFFSET);
            gk_gpio_insts[i].gpio_bank[j].pm_info.ieh_reg  =
                gk_gpio_readl(gk_gpio_insts[i].gpio_bank[j].base_reg + REG_GPIO_IE_HIGH_OFFSET);
            spin_unlock_irqrestore(&gk_gpio_insts[i].gpio_bank[j].lock, flags);
        }
        gk_gpio_insts[i].per_sel_reg      =
            gk_gpio_readl(gk_gpio_insts[i].base_bus + REG_GPIO_PER_SEL_OFFSET);
        gk_gpio_writel(gk_gpio_insts[i].base_bus + REG_GPIO_INT_EN_OFFSET, 0x00000000);
    }

    return 0;
}

u32 gk_gpio_resume(u32 level)
{
    u32             i;
    u32             j;
    unsigned long   flags;

    for (i = 0; i < CONFIG_GK_GPIO_INSTANCES; i++)
    {
        for (j = 0; j < gk_gpio_insts[i].bank_num; j++)
        {
            spin_lock_irqsave(&gk_gpio_insts[i].gpio_bank[j].lock, flags);
            gk_gpio_writel(gk_gpio_insts[i].gpio_bank[j].base_reg + REG_GPIO_IS_LOW_OFFSET,
                gk_gpio_insts[i].gpio_bank[j].pm_info.isl_reg);
            gk_gpio_writel(gk_gpio_insts[i].gpio_bank[j].base_reg + REG_GPIO_IS_HIGH_OFFSET,
                gk_gpio_insts[i].gpio_bank[j].pm_info.ish_reg);
            gk_gpio_writel(gk_gpio_insts[i].gpio_bank[j].base_reg + REG_GPIO_IBE_LOW_OFFSET,
                gk_gpio_insts[i].gpio_bank[j].pm_info.ibel_reg);
            gk_gpio_writel(gk_gpio_insts[i].gpio_bank[j].base_reg + REG_GPIO_IBE_HIGH_OFFSET,
                gk_gpio_insts[i].gpio_bank[j].pm_info.ibeh_reg);
            gk_gpio_writel(gk_gpio_insts[i].gpio_bank[j].base_reg + REG_GPIO_IEV_LOW_OFFSET,
                gk_gpio_insts[i].gpio_bank[j].pm_info.ievl_reg);
            gk_gpio_writel(gk_gpio_insts[i].gpio_bank[j].base_reg + REG_GPIO_IEV_HIGH_OFFSET,
                gk_gpio_insts[i].gpio_bank[j].pm_info.ievh_reg);
            gk_gpio_writel(gk_gpio_insts[i].gpio_bank[j].base_reg + REG_GPIO_IE_LOW_OFFSET,
                gk_gpio_insts[i].gpio_bank[j].pm_info.iel_reg);
            gk_gpio_writel(gk_gpio_insts[i].gpio_bank[j].base_reg + REG_GPIO_IE_HIGH_OFFSET,
                gk_gpio_insts[i].gpio_bank[j].pm_info.ieh_reg);
            spin_unlock_irqrestore(&gk_gpio_insts[i].gpio_bank[j].lock, flags);
        }
        gk_gpio_writel(gk_gpio_insts[i].base_bus + REG_GPIO_PER_SEL_OFFSET,
            gk_gpio_insts[i].per_sel_reg);
        gk_gpio_writel(gk_gpio_insts[i].base_bus + REG_GPIO_INT_EN_OFFSET, 0x00000001);
    }
    return 0;
}

int gk_set_gpio_output_can_sleep(struct gk_gpio_io_info *pinfo, u32 on, int can_sleep)
{
    if (pinfo == NULL)
    {
        pr_err("%s: pinfo is NULL.\n", __func__);
        return -1;
    }
    if (pinfo->gpio_id < 0 )
    {
        pr_debug("%s: wrong gpio id %d.\n", __func__, pinfo->gpio_id);
        return -1;
    }

    pr_debug("%s: Gpio[%d] %s, level[%s], delay[%dms].\n", __func__,
        pinfo->gpio_id, on ? "ON" : "OFF",
        pinfo->active_level ? "HIGH" : "LOW",
        pinfo->active_delay);
    if (pinfo->gpio_id >= CONFIG_ARCH_NR_GPIO)
    {
        pr_debug("%s: try expander gpio id %d.\n",
            __func__, pinfo->gpio_id);
        return -1;
    }
    else
    {
        if (on)
        {
            gk_gpio_config(pinfo->gpio_id, pinfo->active_level ? GPIO_TYPE_OUTPUT_1 : GPIO_TYPE_OUTPUT_0);
            gk_gpio_set_out(pinfo->gpio_id, pinfo->active_level);
        }
        else
        {
            gk_gpio_config(pinfo->gpio_id, pinfo->active_level ? GPIO_TYPE_OUTPUT_0 : GPIO_TYPE_OUTPUT_1);
            gk_gpio_set_out(pinfo->gpio_id, !pinfo->active_level);
        }
    }
    if (can_sleep)
    {
        msleep(pinfo->active_delay);
    }
    else
    {
        mdelay(pinfo->active_delay);
    }

    return 0;
}
EXPORT_SYMBOL(gk_set_gpio_output_can_sleep);

u32 gk_get_gpio_input_can_sleep(struct gk_gpio_io_info *pinfo, int can_sleep)
{
    u32                 gpio_value = 0;

    if (pinfo == NULL)
    {
        pr_err("%s: pinfo is NULL.\n", __func__);
        goto gk_get_gpio_input_can_sleep_exit;
    }
    if (pinfo->gpio_id < 0 )
    {
        pr_debug("%s: wrong gpio id %d.\n", __func__, pinfo->gpio_id);
        goto gk_get_gpio_input_can_sleep_exit;
    }

    if (pinfo->gpio_id >= CONFIG_ARCH_NR_GPIO)
    {
        pr_debug("%s: try expander gpio id %d.\n",
            __func__, pinfo->gpio_id);
        goto gk_get_gpio_input_can_sleep_exit;
    }
    else
    {
        gk_gpio_config(pinfo->gpio_id, pinfo->active_level ? GPIO_TYPE_INPUT_1 : GPIO_TYPE_INPUT_0);
        if (can_sleep)
        {
            msleep(pinfo->active_delay);
        }
        else
        {
            mdelay(pinfo->active_delay);
        }
        gpio_value = gk_gpio_get(pinfo->gpio_id);
    }

    pr_debug("%s: {gpio[%d], level[%s], delay[%dms]} get[%d].\n",
        __func__, pinfo->gpio_id,
        pinfo->active_level ? "HIGH" : "LOW",
        pinfo->active_delay, gpio_value);

gk_get_gpio_input_can_sleep_exit:
    return (gpio_value == pinfo->active_level) ? 1 : 0;
}
EXPORT_SYMBOL(gk_get_gpio_input_can_sleep);

int gk_set_gpio_reset_can_sleep(struct gk_gpio_io_info *pinfo, int can_sleep)
{
    int                 ret = 0;

    if (pinfo == NULL)
    {
        pr_err("%s: pinfo is NULL.\n", __func__);
        ret = -1;
        goto gk_set_gpio_reset_can_sleep_exit;
    }
    if (pinfo->gpio_id < 0 )
    {
        pr_debug("%s: wrong gpio id %d.\n", __func__, pinfo->gpio_id);
        ret = -1;
        goto gk_set_gpio_reset_can_sleep_exit;
    }

    pr_debug("%s: Reset gpio[%d], level[%s], delay[%dms].\n",
        __func__, pinfo->gpio_id,
        pinfo->active_level ? "HIGH" : "LOW",
        pinfo->active_delay);
    if (pinfo->gpio_id >= CONFIG_ARCH_NR_GPIO)
    {
        pr_debug("%s: try expander gpio id %d.\n",
            __func__, pinfo->gpio_id);
        ret = -1;
        goto gk_set_gpio_reset_can_sleep_exit;
    }
    else
    {
        gk_gpio_config(pinfo->gpio_id, GPIO_TYPE_OUTPUT_0);
        gk_gpio_set_out(pinfo->gpio_id, pinfo->active_level);
        if (can_sleep)
        {
            msleep(pinfo->active_delay);
        }
        else
        {
            mdelay(pinfo->active_delay);
        }
        gk_gpio_set_out(pinfo->gpio_id, !pinfo->active_level);
        if (can_sleep)
        {
            msleep(pinfo->active_delay);
        }
        else
        {
            mdelay(pinfo->active_delay);
        }
    }

gk_set_gpio_reset_can_sleep_exit:
    return ret;
}
EXPORT_SYMBOL(gk_set_gpio_reset_can_sleep);

int gk_set_gpio_output(struct gk_gpio_io_info *pinfo, u32 on)
{
    return gk_set_gpio_output_can_sleep(pinfo, on, 0);
}
EXPORT_SYMBOL(gk_set_gpio_output);

u32 gk_get_gpio_input(struct gk_gpio_io_info *pinfo)
{
    return gk_get_gpio_input_can_sleep(pinfo, 0);
}
EXPORT_SYMBOL(gk_get_gpio_input);

int gk_set_gpio_reset(struct gk_gpio_io_info *pinfo)
{
    return gk_set_gpio_reset_can_sleep(pinfo, 0);
}
EXPORT_SYMBOL(gk_set_gpio_reset);

int gk_is_valid_gpio_irq(struct gk_gpio_irq_info *pinfo)
{
    struct gk_gpio_inst     *gpio_inst;
    struct gk_gpio_bank     *gpio_bank;
    if (pinfo == NULL)
    {
        printk("%s: pinfo is NULL.\n", __func__);
        return 0;
    }

    gpio_inst = gk_gpio_id_to_inst(pinfo->pin);
    gpio_bank = gk_gpio_id_to_bank(pinfo->pin);
    if ((gpio_bank == NULL) || (gpio_inst == NULL))
    {
        pr_err("%s: invalid GPIO %d.\n", __func__, pinfo->pin);
        return 0;
    }

    if ((pinfo->type != IRQ_TYPE_EDGE_RISING) &&
        (pinfo->type != IRQ_TYPE_EDGE_FALLING) &&
        (pinfo->type != IRQ_TYPE_EDGE_BOTH) &&
        (pinfo->type != IRQ_TYPE_LEVEL_HIGH) &&
        (pinfo->type != IRQ_TYPE_LEVEL_LOW))
        return 0;

    return 1;
}
EXPORT_SYMBOL(gk_is_valid_gpio_irq);

static irqreturn_t gk_gpio_irq(int irq, void *dev)
{
    struct gk_gpio_inst     *gpio_inst;
    struct gk_gpio_bank     *gpio_bank;
    struct gk_gpio_irq_info *pinfo = (struct gk_gpio_irq_info*)dev;
    unsigned long   flags;
    u32 misl_reg;
    u32 mish_reg;
    u32 index;
    u32 mask = 1;
    u32 i = 1;
    mdelay(20);

    if (pinfo == NULL)
    {
        pr_err("%s: pinfo is NULL.\n", __func__);
        return IRQ_NONE;
    }
    gpio_inst = gk_gpio_id_to_inst(pinfo->pin);
    if (gpio_inst == NULL)
    {
        pr_err("%s: invalid GPIO %d.\n", __func__, pinfo->pin);
        return IRQ_NONE;
    }
    for(i=0;i<gpio_inst->bank_num;i++)
    {
        gpio_bank = &gpio_inst->gpio_bank[i];
        spin_lock_irqsave(&gpio_bank->lock, flags);
        misl_reg    = gk_gpio_readl(gpio_bank->base_reg + REG_GPIO_MIS_LOW_OFFSET);
        mish_reg    = gk_gpio_readl(gpio_bank->base_reg + REG_GPIO_MIS_HIGH_OFFSET);
        gk_gpio_writel(gpio_bank->base_reg + REG_GPIO_IC_LOW_OFFSET, misl_reg);
        gk_gpio_writel(gpio_bank->base_reg + REG_GPIO_IC_HIGH_OFFSET, mish_reg);
        spin_unlock_irqrestore(&gpio_bank->lock, flags);

        for(index=0; index < 32; index++)
        {
            if(misl_reg & mask)
            {
                pinfo[index+gpio_bank->chip.base].val = gk_gpio_inline_get(gpio_bank, index+gpio_bank->chip.base);
#if defined(CONFIG_SYSTEM_USE_NETLINK_MSG_GPIO)
                GK_NET_LINK_MSG_S msg;
                msg.module = GK_NL_GPIO;
                msg.data.gk_nl_gpio_data.id = index+gpio_bank->chip.base;
                msg.data.gk_nl_gpio_data.val = pinfo[index+gpio_bank->chip.base].val;
                msg.len = GK_NL_GPIO_MSG_LEN;
                gk_nl_send_message(&msg);
#endif
                if (pinfo[index+gpio_bank->chip.base].handler)
                {
                    pinfo[index+gpio_bank->chip.base].handler(irq, &pinfo[index+gpio_bank->chip.base]);
                }
            }
            if(mish_reg & mask)
            {
                pinfo[index + gpio_bank->chip.base + GPIO_REG_LL_N].val = gk_gpio_inline_get(gpio_bank, index+gpio_bank->chip.base + 32);
#if defined(CONFIG_SYSTEM_USE_NETLINK_MSG_GPIO)
                GK_NET_LINK_MSG_S msg;
                msg.module = GK_NL_GPIO;
                msg.data.gk_nl_gpio_data.id = index+gpio_bank->chip.base + GPIO_REG_LL_N;
                msg.data.gk_nl_gpio_data.val = pinfo[index+gpio_bank->chip.base + GPIO_REG_LL_N].val;
                msg.len = GK_NL_GPIO_MSG_LEN;
                gk_nl_send_message(&msg);
#endif
                if (pinfo[index + gpio_bank->chip.base + GPIO_REG_LL_N].handler)
                {
                    pinfo[index + gpio_bank->chip.base + GPIO_REG_LL_N].handler(irq, &pinfo[index + gpio_bank->chip.base + 32]);
                }
            }
            mask <<= 1;
        }
    }
    return IRQ_HANDLED;
}

int gk_gpio_request_irq(struct gk_gpio_irq_info *pinfo)
{
    struct gk_gpio_inst     *gpio_inst;
    struct gk_gpio_bank     *gpio_bank;
    u32 mask;
    u32 offset;
    unsigned long   flags;
    if (pinfo == NULL)
    {
        pr_err("%s: pinfo is NULL.\n", __func__);
        return -1;
    }

    gpio_inst = gk_gpio_id_to_inst(pinfo->pin);
    gpio_bank = gk_gpio_id_to_bank(pinfo->pin);
    if ((gpio_bank == NULL) || (gpio_inst == NULL))
    {
        pr_err("%s: invalid GPIO %d.\n", __func__, pinfo->pin);
        return -1;
    }
    spin_lock_irqsave(&gpio_bank->lock, flags);
    memcpy(&gpio_inst->irq_info[pinfo->pin], pinfo, sizeof(struct gk_gpio_irq_info));
    gk_gpio_writel(gpio_inst->base_bus + REG_GPIO_INT_EN_OFFSET, 0x00);
    if(pinfo->pin >= GPIO_REG_LL_N)
    {
        mask = 0x01<<(pinfo->pin - GPIO_REG_LL_N);
    }
    else
    {
        mask = 0x01<<pinfo->pin;
    }
    offset = pinfo->pin - gpio_bank->chip.base;
    if(offset >= GPIO_REG_LL_N)
    {
        gpio_bank->pm_info.ish_reg        = gk_gpio_readl(gpio_bank->base_reg + REG_GPIO_IS_HIGH_OFFSET);
        gpio_bank->pm_info.ibeh_reg       = gk_gpio_readl(gpio_bank->base_reg + REG_GPIO_IBE_HIGH_OFFSET);
        gpio_bank->pm_info.ievh_reg       = gk_gpio_readl(gpio_bank->base_reg + REG_GPIO_IEV_HIGH_OFFSET);
        gpio_bank->pm_info.ieh_reg        = gk_gpio_readl(gpio_bank->base_reg + REG_GPIO_IE_HIGH_OFFSET);

        gpio_bank->pm_info.ieh_reg       |= mask;
        switch(pinfo->type)
        {
        case IRQ_TYPE_LEVEL_LOW:
            gpio_bank->pm_info.ish_reg        |= mask;
            gpio_bank->pm_info.ibeh_reg       &= ~mask;
            gpio_bank->pm_info.ievh_reg       &= ~mask;
            gk_gpio_set_type(gpio_bank, pinfo->pin, GPIO_TYPE_INPUT_1);
            break;
        case IRQ_TYPE_LEVEL_HIGH:
            gpio_bank->pm_info.ish_reg        |= mask;
            gpio_bank->pm_info.ibeh_reg       &= ~mask;
            gpio_bank->pm_info.ievh_reg       |= mask;
            gk_gpio_set_type(gpio_bank, pinfo->pin, GPIO_TYPE_INPUT_0);
            break;
        case IRQ_TYPE_EDGE_FALLING:
            gpio_bank->pm_info.ish_reg        &= ~mask;
            gpio_bank->pm_info.ibeh_reg       &= ~mask;
            gpio_bank->pm_info.ievh_reg       &= ~mask;
            gk_gpio_set_type(gpio_bank, pinfo->pin, GPIO_TYPE_INPUT_1);
            break;
        case IRQ_TYPE_EDGE_RISING:
            gpio_bank->pm_info.ish_reg        &= ~mask;
            gpio_bank->pm_info.ibeh_reg       &= ~mask;
            gpio_bank->pm_info.ievh_reg       |= mask;
            gk_gpio_set_type(gpio_bank, pinfo->pin, GPIO_TYPE_INPUT_0);
            break;
        case IRQ_TYPE_EDGE_BOTH:
            gpio_bank->pm_info.ish_reg        &= ~mask;
            gpio_bank->pm_info.ibeh_reg       |= mask;
            gk_gpio_set_type(gpio_bank, pinfo->pin, GPIO_TYPE_INPUT_1);
            break;
        }
        gk_gpio_writel(gpio_bank->base_reg + REG_GPIO_IS_HIGH_OFFSET, gpio_bank->pm_info.ish_reg);
        gk_gpio_writel(gpio_bank->base_reg + REG_GPIO_IBE_HIGH_OFFSET, gpio_bank->pm_info.ibeh_reg);
        gk_gpio_writel(gpio_bank->base_reg + REG_GPIO_IEV_HIGH_OFFSET, gpio_bank->pm_info.ievh_reg);
        gk_gpio_writel(gpio_bank->base_reg + REG_GPIO_IE_HIGH_OFFSET, gpio_bank->pm_info.ieh_reg);
    }
    else
    {
        gpio_bank->pm_info.isl_reg        = gk_gpio_readl(gpio_bank->base_reg + REG_GPIO_IS_LOW_OFFSET);
        gpio_bank->pm_info.ibel_reg       = gk_gpio_readl(gpio_bank->base_reg + REG_GPIO_IBE_LOW_OFFSET);
        gpio_bank->pm_info.ievl_reg       = gk_gpio_readl(gpio_bank->base_reg + REG_GPIO_IEV_LOW_OFFSET);
        gpio_bank->pm_info.iel_reg        = gk_gpio_readl(gpio_bank->base_reg + REG_GPIO_IE_LOW_OFFSET);

        gpio_bank->pm_info.iel_reg       |= mask;
        switch(pinfo->type)
        {
        case IRQ_TYPE_LEVEL_LOW:
            gpio_bank->pm_info.isl_reg        |= mask;
            gpio_bank->pm_info.ibel_reg       &= ~mask;
            gpio_bank->pm_info.ievl_reg       &= ~mask;
            gk_gpio_set_type(gpio_bank, pinfo->pin, GPIO_TYPE_INPUT_1);
            break;
        case IRQ_TYPE_LEVEL_HIGH:
            gpio_bank->pm_info.isl_reg        |= mask;
            gpio_bank->pm_info.ibel_reg       &= ~mask;
            gpio_bank->pm_info.ievl_reg       |= mask;
            gk_gpio_set_type(gpio_bank, pinfo->pin, GPIO_TYPE_INPUT_0);
            break;
        case IRQ_TYPE_EDGE_FALLING:
            gpio_bank->pm_info.isl_reg        &= ~mask;
            gpio_bank->pm_info.ibel_reg       &= ~mask;
            gpio_bank->pm_info.ievl_reg       &= ~mask;
            gk_gpio_set_type(gpio_bank, pinfo->pin, GPIO_TYPE_INPUT_1);
            break;
        case IRQ_TYPE_EDGE_RISING:
            gpio_bank->pm_info.isl_reg        &= ~mask;
            gpio_bank->pm_info.ibel_reg       &= ~mask;
            gpio_bank->pm_info.ievl_reg       |= mask;
            gk_gpio_set_type(gpio_bank, pinfo->pin, GPIO_TYPE_INPUT_0);
            break;
        case IRQ_TYPE_EDGE_BOTH:
            gpio_bank->pm_info.isl_reg        &= ~mask;
            gpio_bank->pm_info.ibel_reg       |= mask;
            gk_gpio_set_type(gpio_bank, pinfo->pin, GPIO_TYPE_INPUT_1);
            break;
        }
        gk_gpio_writel(gpio_bank->base_reg + REG_GPIO_IS_LOW_OFFSET, gpio_bank->pm_info.isl_reg);
        gk_gpio_writel(gpio_bank->base_reg + REG_GPIO_IBE_LOW_OFFSET, gpio_bank->pm_info.ibel_reg);
        gk_gpio_writel(gpio_bank->base_reg + REG_GPIO_IEV_LOW_OFFSET, gpio_bank->pm_info.ievl_reg);
        gk_gpio_writel(gpio_bank->base_reg + REG_GPIO_IE_LOW_OFFSET, gpio_bank->pm_info.iel_reg);
    }
    __set_bit(offset, (volatile unsigned long *)gpio_inst->irq_flag);
    if(gpio_inst->irq_now == 0x00)
    {
        request_irq(gpio_inst->irq_no, gk_gpio_irq, IRQF_TRIGGER_HIGH, "gpio_irq", (void*)(&gpio_inst->irq_info[0]));
        gpio_inst->irq_now = 0x01;
        gk_gpio_writel(gpio_inst->base_bus + REG_GPIO_INT_EN_OFFSET, 0x01);
    }
    spin_unlock_irqrestore(&gpio_bank->lock, flags);
    return 0;
}
EXPORT_SYMBOL(gk_gpio_request_irq);

int gk_gpio_release_irq(u32 pin)
{
    struct gk_gpio_inst     *gpio_inst;
    struct gk_gpio_bank     *gpio_bank;
    u32 mask, i;
    u32 offset;
    unsigned long   flags;

    gpio_inst = gk_gpio_id_to_inst(pin);
    gpio_bank = gk_gpio_id_to_bank(pin);
    if ((gpio_bank == NULL) || (gpio_inst == NULL))
    {
        pr_err("%s: invalid GPIO %d.\n", __func__, pin);
        return -1;
    }
    spin_lock_irqsave(&gpio_bank->lock, flags);
    memset(&gpio_inst->irq_info[pin], 0x00, sizeof(struct gk_gpio_irq_info));

    offset = pin - gpio_bank->chip.base;
    if(offset >= GPIO_REG_LL_N)
    {
        mask = 0x01<<(offset - GPIO_REG_LL_N);
    }
    else
    {
        mask = 0x01<<offset;
    }

    if(offset >= GPIO_REG_LL_N)
    {
        gpio_bank->pm_info.ieh_reg        = gk_gpio_readl(gpio_bank->base_reg + REG_GPIO_IE_HIGH_OFFSET);
        gpio_bank->pm_info.ieh_reg       &= ~mask;
        gk_gpio_writel(gpio_bank->base_reg + REG_GPIO_IE_HIGH_OFFSET, gpio_bank->pm_info.ieh_reg);
    }
    else
    {
        gpio_bank->pm_info.iel_reg        = gk_gpio_readl(gpio_bank->base_reg + REG_GPIO_IE_LOW_OFFSET);
        gpio_bank->pm_info.iel_reg       &= ~mask;
        gk_gpio_writel(gpio_bank->base_reg + REG_GPIO_IE_LOW_OFFSET, gpio_bank->pm_info.iel_reg);
    }
    __clear_bit(offset, (volatile unsigned long *)gpio_inst->irq_flag);
    offset = 0;
    for (i = 0; i < (sizeof(gpio_inst->irq_flag) /sizeof(gpio_inst->irq_flag[0])); i++)
    {
        if(gpio_inst->irq_flag[i] != 0)
        {
            offset = 1;
            break;
        }
    }
    if(!offset)
    {
        gk_gpio_writel(gpio_bank->base_reg + REG_GPIO_INT_EN_OFFSET, 0x00);
    }
    spin_unlock_irqrestore(&gpio_bank->lock, flags);
    return 0;
}
EXPORT_SYMBOL(gk_gpio_release_irq);

int __init goke_init_gpio(void)
{
    u32 i,j,ret=0;
    for(i=0;i<CONFIG_GK_GPIO_INSTANCES;i++)
    {
        mutex_lock(&gk_gpio_mtx);
        memset(gk_gpio_insts[i].gpio_valid, 0xff, sizeof(gk_gpio_insts[i].gpio_valid));
        memset(gk_gpio_insts[i].gpio_freeflag, 0xff, sizeof(gk_gpio_insts[i].gpio_freeflag));
        /* clear unused gpio */
        for (j = CONFIG_ARCH_NR_GPIO; j < (BITS_TO_LONGS(CONFIG_ARCH_NR_GPIO) * 0x20); j++)
        {
            __clear_bit(j, (volatile unsigned long *)gk_gpio_insts[i].gpio_valid);
            __clear_bit(j, (volatile unsigned long *)gk_gpio_insts[i].gpio_freeflag);
        }
        mutex_unlock(&gk_gpio_mtx);
        for (j = 0; j < gk_gpio_insts[i].bank_num; j++)
        {
            spin_lock_init(&gk_gpio_insts[i].gpio_bank[j].lock);
            ret = gpiochip_add(&gk_gpio_insts[i].gpio_bank[j].chip);
            if (ret)
            {
                pr_err("%s: gpiochip_add %s fail %d.\n", __func__,
                    gk_gpio_insts[i].gpio_bank[j].chip.label, ret);
                break;
            }
        }
    }
    return ret;
}

int gk_set_ircut(u32 mode)
{
    // A=SYSTEM_GPIO_IR_CUT2
    // B=SYSTEM_GPIO_IR_CUT1
    if(mode==0) // clear A clear B
    {
        gk_gpio_set_out(gk_all_gpio_cfg.ir_cut1, 0);
        gk_gpio_set_out(gk_all_gpio_cfg.ir_cut2, 0);
    }
    else if(mode==1) // night set A clear B
    {
        gk_gpio_set_out(gk_all_gpio_cfg.ir_cut1, 0);
        gk_gpio_set_out(gk_all_gpio_cfg.ir_cut2, 1);
    }
    else if(mode==2) // day clear A set B
    {
        gk_gpio_set_out(gk_all_gpio_cfg.ir_cut1, 1);
        gk_gpio_set_out(gk_all_gpio_cfg.ir_cut2, 0);
    }
    else
    {
        return -1;
    }
    return 0;
}
EXPORT_SYMBOL(gk_set_ircut);

int gk_set_sensor_gpio(u32 mode)
{
    if(mode & 0x1)
    {
 #ifdef SENSOR_GPIO_USED
        gk_gpio_config(SENSOR_GPIO10, GK_GPIO_INPUT_0);
        gk_gpio_config(SENSOR_GPIO11, GK_GPIO_INPUT_0);
 #endif
    }
    return 0;
}
EXPORT_SYMBOL(gk_set_sensor_gpio);

unsigned int gk_board_version(void)
{
    return gk_all_gpio_cfg.board_version;
}
EXPORT_SYMBOL(gk_board_version);

const char *gk_board_type(void)
{
    return (const char *)gk_all_gpio_cfg.board_type;
}
EXPORT_SYMBOL(gk_board_type);


