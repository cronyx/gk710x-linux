/*!
*****************************************************************************
** \file        arch/arm/mach-gk/include/mach/audio_codec.h
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
#ifndef __HAL_AUDIO_CODEC_H__
#define __HAL_AUDIO_CODEC_H__

#include <mach/hardware.h>

#define AUDIO_DMA_REG_OFFSET                (0x00000000)


/*AHB genneral */
#define AHB_GREG_BASE                       (GK_VA_AHB_GREG)
#define AHB_GREG_BASE_PHYS                  (GK_PA_AHB_GREG)
#define AHB_GREG_REG(x)                     (AHB_GREG_BASE + (x))
#define AHB_GREG_REG_PHYS(x)                (AHB_GREG_BASE_PHYS + (x))

#define AHB_GREG_BASE_extern                AHB_GREG_BASE

/*digital model*/
#define AUDIO_CODEC_DIGITAL_BASE            (GK_VA_AUDIO_CODEC_DIGITAL)
#define AUDIO_CODEC_DIGITAL_BASE_PHYS       (GK_PA_AUDIO_CODEC_DIGITAL)
#define AUDIO_CODEC_DIGITAL_REG(x)          (AUDIO_CODEC_DIGITAL_BASE + (x))
#define AUDIO_CODEC_DIGITAL_REG_PHYS(x)     (AUDIO_CODEC_DIGITAL_BASE_PHYS + (x))

#define AUDIO_CODEC_DIGITAL_BASE_extern     AUDIO_CODEC_DIGITAL_BASE

/*analog model*/
#define AUDIO_CODEC_ANALOG_BASE             (GK_VA_AUDIO_CODEC_ANALOG)
#define AUDIO_CODEC_ANALOG_BASE_PHYS        (GK_PA_AUDIO_CODEC_ANALOG)
#define AUDIO_CODEC_ANALOG_REG(x)           (AUDIO_CODEC_ANALOG_BASE + (x))
#define AUDIO_CODEC_ANALOG_REG_PHYS(x)      (AUDIO_CODEC_ANALOG_BASE_PHYS + (x))

#define AUDIO_CODEC_ANALOG_BASE_extern      AUDIO_CODEC_ANALOG_BASE

/*AHB genneral reg*/
#define AHB_GENNERNAL0_REG                  AHB_GREG_REG(0x00)
#define AHB_GENNERNAL1_REG                  AHB_GREG_REG(0x04)

/*digital model reg */
#define AUDC_DIGITAL_SYS_RST_CTRL0_REG      AUDIO_CODEC_DIGITAL_REG(0x00)
#define AUDC_DIGITAL_CKG_CTRL0_REG          AUDIO_CODEC_DIGITAL_REG(0x04)
#define AUDC_DIGITAL_AUDIOBAND_CTRL2_REG    AUDIO_CODEC_DIGITAL_REG(0x18)
#define AUDC_DIGITAL_TIMING_CTRL0_REG       AUDIO_CODEC_DIGITAL_REG(0x08)
#define AUDC_DIGITAL_AUDIOBAND_CTRL0_REG    AUDIO_CODEC_DIGITAL_REG(0x10)
#define AUDC_DIGITAL_AUDIOBAND_STS_REG      AUDIO_CODEC_DIGITAL_REG(0x1c)
#define AUDC_DIGITAL_SDM_CTRL0_REG          AUDIO_CODEC_DIGITAL_REG(0x34)
#define AUDC_DIGITAL_SDM_CTRL1_REG          AUDIO_CODEC_DIGITAL_REG(0x38)
#define AUDC_DIGITAL_NF_SYNTH_1_NF_H_REG    AUDIO_CODEC_DIGITAL_REG(0x3c)
#define AUDC_DIGITAL_NF_SYNTH_1_NF_L_REG    AUDIO_CODEC_DIGITAL_REG(0x40)
#define AUDC_DIGITAL_NF_SYNTH_2_NF_H_REG    AUDIO_CODEC_DIGITAL_REG(0x44)
#define AUDC_DIGITAL_NF_SYNTH_2_NF_L_REG    AUDIO_CODEC_DIGITAL_REG(0x48)
#define AUDC_DIGITAL_DIG_MIC_CTRL_REG       AUDIO_CODEC_DIGITAL_REG(0x4c)
#define AUDC_DIGITAL_AUDIOBAND_STS2_REG     AUDIO_CODEC_DIGITAL_REG(0x20)
#define AUDC_DIGITAL_SDM_DWA_DATAIN_L_REG   AUDIO_CODEC_DIGITAL_REG(0x54)
#define AUDC_DIGITAL_SDM_DWA_DATAIN_R_REG   AUDIO_CODEC_DIGITAL_REG(0x58)
#define AUDC_DIGITAL_VALID_SIGNALS_REG      AUDIO_CODEC_DIGITAL_REG(0x5c)
#define AUDC_DIGITAL_PGA1_DPGA_CFG1_REG     AUDIO_CODEC_DIGITAL_REG(0x68)
#define AUDC_DIGITAL_MMP1_DPGA_CFG1_REG     AUDIO_CODEC_DIGITAL_REG(0x60)
#define AUDC_DIGITAL_MMP1_DPGA_CFG2_REG     AUDIO_CODEC_DIGITAL_REG(0x64)
#define AUDC_DIGITAL_MIX_CTRL0_REG          AUDIO_CODEC_DIGITAL_REG(0x50)

#define AUDC_DIGITAL_MMP2_DPGA_CFG1_REG     AUDIO_CODEC_DIGITAL_REG(0x6c)
#define AUDC_DIGITAL_MMP2_DPGA_CFG2_REG     AUDIO_CODEC_DIGITAL_REG(0x70)
#define AUDC_DIGITAL_INT1_DOUT_REG          AUDIO_CODEC_DIGITAL_REG(0x74)
#define AUDC_DIGITAL_FIFO_TH_CTRL0_REG      AUDIO_CODEC_DIGITAL_REG(0x7c)
#define AUDC_DIGITAL_INT2_DOUT_REG          AUDIO_CODEC_DIGITAL_REG(0x78)

#define AUDC_DIGITAL_TIMING_CTRL1_REG       AUDIO_CODEC_DIGITAL_REG(0x0c)
#define AUDC_DIGITAL_AUDIOBAND_CTRL1_REG    AUDIO_CODEC_DIGITAL_REG(0x14)
#define AUDC_DIGITAL_FIFO_CTRL_REG          AUDIO_CODEC_DIGITAL_REG(0x80)
#define AUDC_DIGITAL_FIFO_STS_REG           AUDIO_CODEC_DIGITAL_REG(0x84)
#define AUDC_DIGITAL_NF_SYNTH_5_NF_H_REG    AUDIO_CODEC_DIGITAL_REG(0x88)
#define AUDC_DIGITAL_NF_SYNTH_5_NF_L_REG    AUDIO_CODEC_DIGITAL_REG(0x8c)
#define AUDC_DIGITAL_INT_CTRL_REG           AUDIO_CODEC_DIGITAL_REG(0x90)

#define AUDC_DIGITAL_SINE_GEN_CTRL0_REG     AUDIO_CODEC_DIGITAL_REG(0x24)
#define AUDC_DIGITAL_SINE_GEN_CTRL1_REG     AUDIO_CODEC_DIGITAL_REG(0x28)
#define AUDC_DIGITAL_TEST_CTRL0_REG         AUDIO_CODEC_DIGITAL_REG(0x30)

/*analog model reg */
#define AUDC_ANALOG_CTRL00_REG              AUDIO_CODEC_ANALOG_REG(0x00)
#define AUDC_ANALOG_CTRL01_REG              AUDIO_CODEC_ANALOG_REG(0x04)
#define AUDC_ANALOG_CTRL02_REG              AUDIO_CODEC_ANALOG_REG(0x08)
#define AUDC_ANALOG_CTRL03_REG              AUDIO_CODEC_ANALOG_REG(0x0c)
#define AUDC_ANALOG_CTRL04_REG              AUDIO_CODEC_ANALOG_REG(0x10)
#define AUDC_ANALOG_CTRL05_REG              AUDIO_CODEC_ANALOG_REG(0x14)
#define AUDC_ANALOG_CTRL06_REG              AUDIO_CODEC_ANALOG_REG(0x18)
#define AUDC_ANALOG_CTRL07_REG              AUDIO_CODEC_ANALOG_REG(0x1c)
#define AUDC_ANALOG_CTRL08_REG              AUDIO_CODEC_ANALOG_REG(0x20)
#define AUDC_ANALOG_CTRL09_REG              AUDIO_CODEC_ANALOG_REG(0x24)
#define AUDC_ANALOG_CTRL10_REG              AUDIO_CODEC_ANALOG_REG(0x28)
#define AUDC_ANALOG_CTRL11_REG              AUDIO_CODEC_ANALOG_REG(0x2c)
#define AUDC_ANALOG_CTRL12_REG              AUDIO_CODEC_ANALOG_REG(0x30)
#define AUDC_ANALOG_CTRL13_REG              AUDIO_CODEC_ANALOG_REG(0x34)
#define AUDC_ANALOG_CTRL14_REG              AUDIO_CODEC_ANALOG_REG(0x38)
#define AUDC_ANALOG_CTRL15_REG              AUDIO_CODEC_ANALOG_REG(0x3c)
#define AUDC_ANALOG_CTRL16_REG              AUDIO_CODEC_ANALOG_REG(0x40)
#define AUDC_ANALOG_CTRL17_REG              AUDIO_CODEC_ANALOG_REG(0x44)
#define AUDC_ANALOG_CTRL18_REG              AUDIO_CODEC_ANALOG_REG(0x48)

extern unsigned int gk_aud_get_dma_offset(void);
#endif /* __HAL_AUDIO_CODEC_H__ */