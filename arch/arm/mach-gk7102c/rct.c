/*
 * arch/arm/mach-gk/rct.c
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
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>

#include <mach/hardware.h>
#include <mach/io.h>
#include <mach/rct.h>

u32 get_apb_bus_freq_hz(void)
{
#if !defined (CONFIG_ARCH_GK7102C_FPGA)
    typedef union {
        u32 all;
        struct {
            u32 fbdiv                       : 12;
            u32 pstdiv1                     : 3;
            u32                             : 1;
            u32 pstdiv2                     : 3;
            u32                             : 1;
            u32 refdiv                      : 6;
            u32                             : 6;
        } bitc;
    } GH_PLL_CORE_CTRL_S;
    GH_PLL_CORE_CTRL_S arm_pll;
    u32 div0118;
    u32 apb_freq;
    arm_pll.all = gk_rct_readl(GK_VA_RCT + 0x014);
    div0118 = gk_rct_readl(GK_VA_RCT + 0x0118);
    if(div0118 * arm_pll.bitc.fbdiv * arm_pll.bitc.refdiv * arm_pll.bitc.pstdiv1 * arm_pll.bitc.pstdiv2)
    {
        apb_freq = (24 * arm_pll.bitc.fbdiv * 1000000)/(div0118 * 4 * arm_pll.bitc.refdiv * arm_pll.bitc.pstdiv1 * arm_pll.bitc.pstdiv2);
    }
    else
    {
        apb_freq = GK_APB_FREQ;
    }

    return apb_freq;
#else
    return GK_APB_FREQ;
#endif
}
EXPORT_SYMBOL(get_apb_bus_freq_hz);

u32 get_uart_freq_hz(void)
{
    return (u32)GK_UART_FREQ;
}
EXPORT_SYMBOL(get_uart_freq_hz);

u32 get_ssi0_freq_hz(void)
{
#if !defined (CONFIG_ARCH_GK7102C_FPGA)
    u32 div = gk_rct_readl(GK_VA_RCT + 0x0030) & 0xFF;
    if(div == 0)
    {
        div = 8;
    }
    return (get_apb_bus_freq_hz()/div);
#else
	return 3000000;
#endif
}
EXPORT_SYMBOL(get_ssi0_freq_hz);

u32 get_ssi1_freq_hz(void)
{
#if !defined (CONFIG_ARCH_GK7102C_FPGA)
    u32 div = gk_rct_readl(GK_VA_RCT + 0x00A0) & 0xFF;
    if(div == 0)
    {
        div = 8;
    }
    return (get_apb_bus_freq_hz()/div);
#else
    return 3000000;
#endif
}
EXPORT_SYMBOL(get_ssi1_freq_hz);

u32 get_sd_freq_hz(void)
{
    typedef union {
        u32 all;
        struct {
            u32 fbdiv                       : 12;
            u32 pstdiv1                     : 3;
            u32                             : 1;
            u32 pstdiv2                     : 3;
            u32                             : 1;
            u32 refdiv                      : 6;
            u32                             : 6;
        } bitc;
    } GH_PLL_CORE_CTRL_S;
    GH_PLL_CORE_CTRL_S arm_pll;
    u32 div0018;
    u32 sd_freq;
#if !defined (CONFIG_ARCH_GK7102C_FPGA)
    arm_pll.all = gk_rct_readl(GK_VA_RCT + 0x014);
    div0018 = gk_rct_readl(GK_VA_RCT + 0x0018);
    if(div0018 * arm_pll.bitc.fbdiv * arm_pll.bitc.refdiv * arm_pll.bitc.pstdiv1 * arm_pll.bitc.pstdiv2)
    {
        sd_freq = (24 * arm_pll.bitc.fbdiv * 1000000)/(div0018 * arm_pll.bitc.refdiv * arm_pll.bitc.pstdiv1 * arm_pll.bitc.pstdiv2);
    }
    else
    {
        sd_freq = GK_SD_FREQ;
    }
#else
	sd_freq = GK_SD_FREQ;
#endif
    return sd_freq;
}
EXPORT_SYMBOL(get_sd_freq_hz);

void set_sd_rct(u32 freq)
{
#if !defined(CONFIG_ARCH_GK7102C_FPGA)
    typedef union {
        u32 all;
        struct {
            u32 fbdiv                       : 12;
            u32 pstdiv1                     : 3;
            u32                             : 1;
            u32 pstdiv2                     : 3;
            u32                             : 1;
            u32 refdiv                      : 6;
            u32                             : 6;
        } bitc;
    } GH_PLL_CORE_CTRL_S;
    GH_PLL_CORE_CTRL_S arm_pll;
    u32 div0018;
    arm_pll.all = gk_rct_readl(GK_VA_RCT + 0x014);
    div0018 = gk_rct_readl(GK_VA_RCT + 0x0018);
    if(div0018 * arm_pll.bitc.fbdiv * arm_pll.bitc.refdiv * arm_pll.bitc.pstdiv1 * arm_pll.bitc.pstdiv2)
    {
        div0018 = (24 * arm_pll.bitc.fbdiv * 1000000)/(arm_pll.bitc.refdiv * arm_pll.bitc.pstdiv1 * arm_pll.bitc.pstdiv2);
        div0018 /= freq;
        gk_rct_writel(GK_VA_RCT + 0x0018, div0018);
    }
#endif
}
EXPORT_SYMBOL(set_sd_rct);

u32 get_audio_clk_freq(void)
{
#if !defined (CONFIG_ARCH_GK7102C_FPGA)
    u32 aud_freq;

    aud_freq=get_apb_bus_freq_hz()/4;
    printk("audio freq is %d\n",aud_freq);

    return aud_freq;
#else
    return 26000000;
#endif
}
EXPORT_SYMBOL(get_audio_clk_freq);

