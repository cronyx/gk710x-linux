/*!
*****************************************************************************
** \file        arch/arm/mach-gk/include/mach/rct.h
**
** \version
**
** \brief
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2015 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef __MACH_RCT_H
#define __MACH_RCT_H

/* ==============================================*/
#if defined(CONFIG_ARCH_GK720X_FPGA)
#define GK_APB_FREQ             25000000
#define GK_UART_FREQ            (25000000/2)
#define GK_SD_FREQ              25000000
#else
#define GK_APB_FREQ             54000000    //20MHz
#define GK_UART_FREQ            24000000
#define GK_SD_FREQ              50000000    //this is fake value, the real is GK_APB_FREQ
#endif

u32  get_apb_bus_freq_hz(void);
u32  get_uart_freq_hz(void);
u32  get_sd_freq_hz(void);
u32  get_ssi0_freq_hz(void);
u32  get_ssi1_freq_hz(void);

void set_sd_rct(u32 freq);


#endif

