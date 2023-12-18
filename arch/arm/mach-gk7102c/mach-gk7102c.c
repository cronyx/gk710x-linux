/*
 * linux/arch/arm/mach-gk/mach-gk.c
 *
 * Author: Steven Yu, <yulindeng@gokemicro.com>
 * Copyright (C) 2012-2015, goke, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */


#include CONFIG_GK_CHIP_INCLUDE_FILE

#include <linux/init.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/amba/bus.h>
#include <linux/usb/musb.h>
#include <linux/memory_hotplug.h>
#include <linux/clkdev.h>
#include <linux/clk-provider.h>
#include <linux/mtd/partitions.h>
#include <linux/pm.h>
#include <linux/moduleparam.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/spi/spi.h>
#include <linux/spi/spidev.h>
#include <linux/delay.h>
#include <linux/random.h>
#include <linux/ctype.h>
#include <linux/gk-pwm-bl.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/time.h>
#include <asm/mach/flash.h>
#include <asm/setup.h>
#include <asm/hardware/vic.h>

#include <mach/hardware.h>
#include <mach/flash.h>
#include <mach/irqs.h>
#include <mach/timer.h>
#include <mach/uart.h>
#include <mach/gpio.h>
#include <mach/eth.h>
#include <mach/idc.h>
#include <mach/rct.h>
#include <mach/sd.h>
#include <mach/ir.h>
#include <plat/audio.h>
#include <mach/i2s.h>
#include <plat/dma.h>
#include <mach/spi.h>
#include <mach/io.h>
#include <mach/audio_codec.h>
#include <mach/wdt.h>

extern struct platform_device gk_fb0;
extern struct platform_device gk_fb1;
extern int gk_init_fb(void);

extern struct platform_device gk_sd0;

extern struct i2c_board_info gk_board_vi_infos[2];
extern struct i2c_board_info gk_board_hdmi_infos[3];

//u64 gk_dmamask = DMA_BIT_MASK(32);
//EXPORT_SYMBOL(gk_dmamask);

/* ==========================================================================*/
#if defined(CONFIG_MTD_SFLASH_GOKE) || defined(CONFIG_MTD_SFLASH_GOKE_MODULE)
// channel 0 NOR
static struct mtd_partition gk_flash_parts_nor_0[]=
{
    //256K
    [0] = {
        .name   = "uboot",
        .offset = 0x00000000,
        .size   = 0x00040000,
    },
    //64K
    [1] = {
        .name   = "ubootenv",
        .offset = MTDPART_OFS_NXTBLK,
        .size   = 0x00010000,
    },
    //2M
    [2] = {
        .name   = "kernel",
        .offset = MTDPART_OFS_NXTBLK,
        .size   = 0x00200000,
    },
    //4M
    [3] = {
        .name   = "rootfs",
        .offset = MTDPART_OFS_NXTBLK,
        .size   = MTDPART_SIZ_FULL,
    },
    //ALL
    [4] = {
        .name   = "all",
        .offset = 0x00000000,
        .size   = MTDPART_SIZ_FULL,
    },
};

// channel 1 NOR
static struct mtd_partition gk_flash_parts_nor_1[]=
{
    //256K
    [0] = {
        .name   = "uboot",
        .offset = 0x00000000,
        .size   = 0x00040000,
    },
    //64K
    [1] = {
        .name   = "ubootenv",
        .offset = MTDPART_OFS_NXTBLK,
        .size   = 0x00010000,
    },
    //2M
    [2] = {
        .name   = "kernel",
        .offset = MTDPART_OFS_NXTBLK,
        .size   = 0x00200000,
    },
    //4M
    [3] = {
        .name   = "rootfs",
        .offset = MTDPART_OFS_NXTBLK,
        .size   = MTDPART_SIZ_FULL,
    },
    //ALL
    [4] = {
        .name   = "all",
        .offset = 0x00000000,
        .size   = MTDPART_SIZ_FULL,
    },
};

static struct sflash_platform_data flash_platform_data_nor_0 =
{
    .speed_mode = (uint32_t)GOKE_SFLASH_FREQ_DIV2,
    .channel    = 0,
    .nr_parts   = ARRAY_SIZE(gk_flash_parts_nor_0),
    .parts      = gk_flash_parts_nor_0,
};

static struct sflash_platform_data flash_platform_data_nor_1 =
{
    .speed_mode = (uint32_t)GOKE_SFLASH_FREQ_DIV2,
    .channel    = 1,
    .nr_parts   = ARRAY_SIZE(gk_flash_parts_nor_1),
    .parts      = gk_flash_parts_nor_1,
};

static struct platform_device gk_flash_device_nor_0 =
{
    .name = "gk_flash",
    .id   = 0,
    .dev =
    {
        .platform_data = &flash_platform_data_nor_0,
    },
};

static struct platform_device gk_flash_device_nor_1 =
{
    .name = "gk_flash",
    .id   = 1,
    .dev =
    {
        .platform_data = &flash_platform_data_nor_1,
    },
};
#endif

#if defined(CONFIG_MTD_SPINAND_GOKE) || defined(CONFIG_MTD_SPINAND_GOKE_MODULE)
// channel 0 NAND
static struct mtd_partition gk_flash_parts_nand_0[]=
{
    //256K
    [0] = {
        .name   = "uboot",
        .offset = 0x00000000,
        .size   = 0x00040000,
    },
    //128K page size
    [1] = {
        .name   = "ubootenv",
        .offset = MTDPART_OFS_NXTBLK,
        .size   = 0x00040000,
    },
    // 2M
    [2] = {
        .name   = "kernel",
        .offset = MTDPART_OFS_NXTBLK,
        .size   = 0x00200000,
    },
    [3] = {
        .name   = "rootfs1",
        .offset = MTDPART_OFS_NXTBLK,
        .size   = 0x02000000,//MTDPART_SIZ_FULL,
    },
    //ALL
    [4] = {
        .name   = "all",
        .offset = 0x00000000,
        .size   = MTDPART_SIZ_FULL,
    },
};

// channel 1 NAND
static struct mtd_partition gk_flash_parts_nand_1[]=
{
    //256K
    [0] = {
        .name   = "uboot",
        .offset = 0x00000000,
        .size   = 0x00040000,
    },
    //128K page size
    [1] = {
        .name   = "ubootenv",
        .offset = MTDPART_OFS_NXTBLK,
        .size   = 0x00040000,
    },
    // 2M
    [2] = {
        .name   = "kernel",
        .offset = MTDPART_OFS_NXTBLK,
        .size   = 0x00200000,
    },
    [3] = {
        .name   = "rootfs",
        .offset = MTDPART_OFS_NXTBLK,
        .size   = MTDPART_SIZ_FULL,
    },
    //ALL
    [4] = {
        .name   = "all",
        .offset = 0x00000000,
        .size   = MTDPART_SIZ_FULL,
    },
};

static struct sflash_platform_data flash_platform_data_nand_0 =
{
    .speed_mode = (uint32_t)GOKE_SFLASH_FREQ_DIV2,
    .channel    = 0,
    .nr_parts   = ARRAY_SIZE(gk_flash_parts_nand_0),
    .parts      = gk_flash_parts_nand_0,
};

static struct sflash_platform_data flash_platform_data_nand_1 =
{
    .speed_mode = (uint32_t)GOKE_SFLASH_FREQ_DIV2,
    .channel    = 1,
    .nr_parts   = ARRAY_SIZE(gk_flash_parts_nand_1),
    .parts      = gk_flash_parts_nand_1,
};

static struct platform_device gk_flash_device_nand_0 =
{
    .name = "gk_spi_nand",
    .id   = 0,
    .dev =
    {
        .platform_data = &flash_platform_data_nand_0,
    },
};

static struct platform_device gk_flash_device_nand_1 =
{
    .name = "gk_spi_nand",
    .id   = 1,
    .dev =
    {
        .platform_data = &flash_platform_data_nand_1,
    },
};
#endif

/* ================     Watch Dog Timer     ======================*/
#ifdef CONFIG_WATCHDOG_GOKE
static struct resource gk_wdt_resource[] = {
    [0] = DEFINE_RES_MEM(GK_VA_WDT, SZ_4K),
    [1] = DEFINE_RES_IRQ(WDT_IRQ),
};

struct platform_device gk_wdt = {
    .name       = "gk-wdt",
    .id         = -1,
    .resource   = gk_wdt_resource,
    .num_resources  = ARRAY_SIZE(gk_wdt_resource),
    .dev        = {
        .dma_mask       = &gk_dmamask,
        .coherent_dma_mask  = DMA_BIT_MASK(32),
    }
};
#endif /* CONFIG_WATCHDOG_GOKE */
/* =================               End            ======================*/

/* ================     Inter Integrated Circuit     ===================*/

#ifdef CONFIG_I2C_GOKE
static struct resource gk_idc_resources[] = {
    [0] = DEFINE_RES_MEM(GK_VA_IDC, SZ_4K),
    [1] = DEFINE_RES_IRQ(IDC_IRQ),
};

struct gk_platform_i2c gk_idc_platform_info = {
    .clk_limit      = 400000,
    .bulk_write_num = 60,
    .get_clock      = get_apb_bus_freq_hz,
};
IDC_PARAM_CALL(0, gk_idc_platform_info, 0644);

struct platform_device gk_idc = {
    .name           = "i2c",
    .id             = 0,
    .resource       = gk_idc_resources,
    .num_resources  = ARRAY_SIZE(gk_idc_resources),
    .dev            = {
        .platform_data      = &gk_idc_platform_info,
        .dma_mask           = &gk_dmamask,
        .coherent_dma_mask  = DMA_BIT_MASK(32),
    }
};

#if (IDC_INSTANCES >= 2)
static struct resource gk_idc_hdmi_resources[] = {
    [0] = DEFINE_RES_MEM(GK_VA_IDC2, SZ_4K),
    [1] = DEFINE_RES_IRQ(IDC_HDMI_IRQ),
};

struct gk_platform_i2c gk_idc_hdmi_platform_info = {
    .clk_limit      = 400000,
    .bulk_write_num = 60,
    .i2c_class      = I2C_CLASS_DDC,
    .get_clock      = get_apb_bus_freq_hz,
};
IDC_PARAM_CALL(1, gk_idc_hdmi_platform_info, 0644);

struct platform_device gk_idc_hdmi = {
    .name           = "i2c",
    .id             = 1,
    .resource       = gk_idc_hdmi_resources,
    .num_resources  = ARRAY_SIZE(gk_idc_hdmi_resources),
    .dev            = {
        .platform_data      = &gk_idc_hdmi_platform_info,
        .dma_mask           = &gk_dmamask,
        .coherent_dma_mask  = DMA_BIT_MASK(32),
    }
};
#endif
#endif

#ifdef CONFIG_SPI_GOKE
/* ==========================================================================*/
void gk_spi_cs_activate(struct gk_spi_cs_config *cs_config)
{
    u8            cs_pin;

    if (cs_config->bus_id >= SPI_MASTER_INSTANCES ||
        cs_config->cs_id >= cs_config->cs_num)
        return;
    cs_pin = cs_config->cs_pins[cs_config->cs_id];
    gk_gpio_set_out(cs_pin, 0);
}

void gk_spi_cs_deactivate(struct gk_spi_cs_config *cs_config)
{
    u8            cs_pin;

    if (cs_config->bus_id >= SPI_MASTER_INSTANCES ||
        cs_config->cs_id >= cs_config->cs_num)
        return;

    cs_pin = cs_config->cs_pins[cs_config->cs_id];
    gk_gpio_set_out(cs_pin, 1);
}

struct resource gk_spi0_resources[] = {
    [0] = {
        .start  = GK_VA_SSI1,
        .end    = GK_VA_SSI1 + 0x0FFF,
        .flags  = IORESOURCE_MEM,
    },
    [1] = {
        .start  = SSI_IRQ,
        .end    = SSI_IRQ,
        .flags  = IORESOURCE_IRQ,
    },
};

int gk_spi0_cs_pins[] = {-1, -1, -1, -1, -1, -1, -1, -1};
GK_SPI_PARAM_CALL(0, gk_spi0_cs_pins, 0644);
struct gk_spi_platform_info gk_spi0_platform_info = {
    .support_dma        = 0,
#if (SPI_MASTER_INSTANCES == 5 )
    .fifo_entries        = 64,
#else
    .fifo_entries        = 16,
#endif
    .cs_num             = ARRAY_SIZE(gk_spi0_cs_pins),
    .cs_pins            = gk_spi0_cs_pins,
    .cs_activate        = gk_spi_cs_activate,
    .cs_deactivate      = gk_spi_cs_deactivate,
//    .rct_set_ssi_pll    = rct_set_ssi_pll,
    .get_ssi_freq_hz    = get_ssi0_freq_hz,
};

struct platform_device gk_spi0 = {
    .name       = "spi",
    .id         = 0,
    .resource   = gk_spi0_resources,
    .num_resources    = ARRAY_SIZE(gk_spi0_resources),
    .dev        = {
        .platform_data        = &gk_spi0_platform_info,
        //.dma_mask        = &gk_dmamask,
        //.coherent_dma_mask    = DMA_BIT_MASK(32),
    }
};

#if (SPI_MASTER_INSTANCES >= 2 )
struct resource gk_spi1_resources[] = {
    [0] = {
        .start  = GK_VA_SSI2,
        .end    = GK_VA_SSI2 + 0x0FFF,
        .flags  = IORESOURCE_MEM,
    },
    [1] = {
        .start  = SSI2_IRQ,
        .end    = SSI2_IRQ,
        .flags  = IORESOURCE_IRQ,
    },
};

int gk_spi1_cs_pins[] = {-1, -1, -1, -1, -1, -1, -1, -1};
GK_SPI_PARAM_CALL(1, gk_spi1_cs_pins, 0644);
struct gk_spi_platform_info gk_spi1_platform_info = {
    .support_dma        = 0,
    .fifo_entries       = 16,
    .cs_num             = ARRAY_SIZE(gk_spi1_cs_pins),
    .cs_pins            = gk_spi1_cs_pins,
    .cs_activate        = gk_spi_cs_activate,
    .cs_deactivate      = gk_spi_cs_deactivate,
    .get_ssi_freq_hz    = get_ssi1_freq_hz,
};

struct platform_device gk_spi1 = {
    .name       = "spi",
    .id         = 1,
    .resource   = gk_spi1_resources,
    .num_resources    = ARRAY_SIZE(gk_spi1_resources),
    .dev        = {
        .platform_data      = &gk_spi1_platform_info,
        //.dma_mask           = &gk_dmamask,
        //.coherent_dma_mask  = DMA_BIT_MASK(32),
    }
};
#endif
#endif
static struct spi_board_info gk_spi_devices[] = {
    {
#ifdef CONFIG_MMC_SPI
        .modalias       = "mmc_spi",
        .max_speed_hz   = 69000000/4,     /* max spi clock (SCK) speed in HZ */
        .bus_num        = 0,
        .mode = SPI_MODE_3,
        //.platform_data = &bfin_mmc_spi_pdata,
        //.controller_data = &mmc_spi_chip_info,
#else
        .modalias       = "spidev",
#endif
        .bus_num        = 0,
        .chip_select    = 0,
    },
#if (SPI_INSTANCES >= 2)
    {
        .modalias       = "spidev",
        .bus_num        = 1,
        .chip_select    = 0,
    }
#endif
};

/* =================               rtc            ======================*/
struct platform_device gk_rtc = {
    .name           = "gk-rtc",
    .id             = -1,
};
/* =================               End            ======================*/

/* =================               ir            ======================*/

struct gk_ir_controller gk_platform_ir_controller0 = {

    .protocol       = GK_IR_PROTOCOL_NEC,
    .debug          = 0,
};
GK_IR_PARAM_CALL(gk_platform_ir_controller0, 0644);

struct platform_device gk_ir = {
    .name       = "ir",
    .id         = 1,
    .dev        = {
        .platform_data      = &gk_platform_ir_controller0,
        .dma_mask           = &gk_dmamask,
        .coherent_dma_mask  = DMA_BIT_MASK(32),
    }
};


/* =================               pcm/i2s            ======================*/

struct platform_device gk_pcm0 = {
    .name       = "gk-pcm-audio",
    .id         = -1,
};

static struct resource gk_i2s0_resources[] = {
    [0] = {
        .start  = I2S_BASE,
        .end    = I2S_BASE + 0x0FFF,
        .flags  = IORESOURCE_MEM,
    },
    [1] = {
        .start  = I2STX_IRQ,
        .end    = I2SRX_IRQ,
        .flags  = IORESOURCE_IRQ,
    },
};

static struct gk_i2s_controller gk_i2s_controller0 = {
    .aucodec_digitalio_0    = NULL,
    .aucodec_digitalio_1    = NULL,
    .aucodec_digitalio_2    = NULL,
    .channel_select         = NULL,
    .set_audio_pll          = NULL,
};


struct platform_device gk_i2s0 = {
    .name       = "gk-i2s",
    .id         = 0,
    .resource   = gk_i2s0_resources,
    .num_resources    = ARRAY_SIZE(gk_i2s0_resources),
    .dev        = {
        .platform_data      = &gk_i2s_controller0,
        .dma_mask           = &gk_dmamask,
        .coherent_dma_mask  = DMA_BIT_MASK(32),
    }
};
/* =================     musb   ====================*/
static struct musb_hdrc_config gk_musb_config = {
    .multipoint = 1,
    .dyn_fifo   = 0,
    .big_endian = 0,
    .dma        = 0,
    .num_eps    = 8,
    .ram_bits   = 12,
};

static struct musb_hdrc_platform_data gk_musb_data = {
#if defined(CONFIG_GK_USB_HOST_MODE)
    .mode                                              = MUSB_HOST,
#elif defined(CONFIG_GK_USB_SLAVE_MODE)
    .mode                                              = MUSB_PERIPHERAL,
#elif defined(CONFIG_GK_USB_OTG_MODE)
    .mode                                              = MUSB_HOST,
#endif
    .min_power  = 25,     /* x2 = 50 mA drawn from VBUS as peripheral */
    .config     = &gk_musb_config,
};

static struct resource gk_musb_resources[] = {
        /* Order is significant!  The start/end fields
         * are updated during setup..
         */
    [0] = {
        .start  = GK_VA_USB,
        .end    = GK_VA_USB + 0x2000,
        .flags  = IORESOURCE_MEM,
    },
/*
    [1] = {
        .start  = GK_VA_USB_PHY,
        .end    = GK_VA_USB_PHY + 0xa00,
        .flags  = IORESOURCE_MEM,
    },
*/
    [1] = {
        .start  = USBC_IRQ,
        .end    = USBC_IRQ,
        .flags  = IORESOURCE_IRQ,
        .name   = "mc",
    },
};


static struct platform_device gk_musb = {
        .name           = "musb-gk",
        .id             = -1,
        .dev = {
            .platform_data      = &gk_musb_data,
            .dma_mask           = &gk_dmamask,
            .coherent_dma_mask  = DMA_BIT_MASK(32),
        },
        .num_resources  = ARRAY_SIZE(gk_musb_resources),
        .resource       = gk_musb_resources,
};

/* ========================== lcd backlight  ===========================*/
#ifdef CONFIG_BACKLIGHT_GK_PWM
static struct gk_pwm_bl_platform_data gk_backlight_config = {
        .pwm_channel   = 3,
        .pwm_frequency = 1000,
        .pwm_duty_max  = 1000,
        .pwm_duty_min  = 0,
        .pwm_mode      = 0,
};

static struct platform_device gk_backlight = {
	.name		= "goke-pwm-bl",
	.id		= -1,
	.dev		= {
		.platform_data	= &gk_backlight_config,
	},
};

#endif
/* =================               End            ======================*/
unsigned int gk_aud_get_dma_offset(void)
{
    return AUDIO_DMA_REG_OFFSET;
}
EXPORT_SYMBOL(gk_aud_get_dma_offset);


static struct platform_device *gk_devices[] __initdata = {

#ifdef CONFIG_WATCHDOG_GOKE
    &gk_wdt,
#endif
#ifdef CONFIG_MMC_GOKE
#ifndef CONFIG_PHY_USE_SD_CLK
    &gk_sd0,
#ifdef CONFIG_SDIO2
    &gk_sd1,
#endif
#endif
#endif
#ifdef CONFIG_I2C_GOKE
    &gk_idc,
#if (IDC_INSTANCES >= 2)
    &gk_idc_hdmi,
#endif
#endif

#ifdef CONFIG_FB_GOKE
    &gk_fb0,
    &gk_fb1,
#endif
#ifdef CONFIG_SPI_GOKE
    &gk_spi0,
#if (SPI_MASTER_INSTANCES >= 2 )
    &gk_spi1,
#endif
#endif
#ifdef CONFIG_RTC_DRV_GOKE
    &gk_rtc,
#endif
#ifdef CONFIG_INPUT_GOKE_IR
    &gk_ir,
#endif

#ifdef CONFIG_ETH_GOKE
    &gk_eth0,
#endif
    //&gk_pcm0,
    //&gk_i2s0,
    &gk_musb,

#ifdef CONFIG_BACKLIGHT_GK_PWM
    &gk_backlight,
#endif

};

//louis add
/* ==========================================================================*/

unsigned int gk_debug_level = GK_DEBUG_MEDIA | GK_DEBUG_DSP | GK_DEBUG_VI | GK_DEBUG_VO | GK_DEBUG_AAA;
EXPORT_SYMBOL(gk_debug_level);

unsigned int gk_debug_info = 0;
EXPORT_SYMBOL(gk_debug_info);

unsigned int gk_boot_splash_logo = 0;
EXPORT_SYMBOL(gk_boot_splash_logo);

unsigned long gk_debug_lookup_name(const char *name)
{
    return module_kallsyms_lookup_name(name);
}
EXPORT_SYMBOL(gk_debug_lookup_name);
//end

void sensor_init(u8 active_level)
{
    struct gk_gpio_io_info  sensor_reset;
    sensor_reset.gpio_id = gk_all_gpio_cfg.sensor_reset;
    sensor_reset.active_level = active_level;
    sensor_reset.active_delay = 1;

    printk("sensor board reset...\n");
    gk_set_gpio_output(&sensor_reset, 1);
    mdelay(50);//100ms
    gk_set_gpio_output(&sensor_reset, 0);
    mdelay(200);//100ms
}
EXPORT_SYMBOL(sensor_init);

void sensor_power(u8 power)
{
}
EXPORT_SYMBOL(sensor_power);

/* ==========================================================================*/
static u8 cmdline_mac[6];
static int __init init_setup_mac(char *str)
{
    int count, i, val;

    for (count = 0; count < 6 && *str; count++, str += 3) {
        if (!isxdigit(str[0]) || !isxdigit(str[1]))
            return 0;
        if (str[2] != ((count < 5) ? ':' : '\0'))
            return 0;

        for (i = 0, val = 0; i < 2; i++) {
            val = val << 4;
            val |= isdigit(str[i]) ?
                str[i] - '0' : toupper(str[i]) - 'A' + 10;
        }
        cmdline_mac[count] = val;
    }
    return 1;

}
__setup("mac=", init_setup_mac);

u8 cmdline_phytype;
static int __init init_setup_phytype(char *str)
{
    if (!isxdigit(str[0]))
        return 0;

    cmdline_phytype = str[0] - '0';
    return 1;
}
__setup("phytype=", init_setup_phytype);
EXPORT_SYMBOL(cmdline_phytype);

int __init gk_init_board(char *board_name)
{
#ifdef CONFIG_ETH_GOKE
    unsigned char   mac_addr[6] = {00,0x11,0x22,0xa3,0xa0,00};
#endif
    int             retval = 0;

    //pr_info("Goke %s:\n", board_name);
    //pr_info("\tboard revision:\t\t%d\n", 1);

    //retval = gk_init_pll();
    //BUG_ON(retval != 0);
    //
#if 1 //wangbin add for gk7101s
//    gk_writel(0xf3170208,0x24);//RGB i80 CVBS
//    gk_writel(0xf3170208,0x20);//BT1120
#endif
    printk("init timer...\n");
    retval = gk_init_timer();
    BUG_ON(retval != 0);

    printk("init gpio...\n");
    retval = gk_init_gpio();
    BUG_ON(retval != 0);

    gk_set_sd_detect_pin(gk_all_gpio_cfg.sd_detect);
#ifdef CONFIG_SDIO2
    gk_set_sd1_detect_pin(gk_all_gpio_cfg.sd1_detect);
#endif
#ifdef CONFIG_SPI_GOKE
    gk_spi0_cs_pins[0] = gk_all_gpio_cfg.spi0_en0;
#if (SPI_MASTER_INSTANCES >= 2 )
    gk_spi1_cs_pins[0] = gk_all_gpio_cfg.spi1_en0;
#endif
#endif


#ifdef CONFIG_ETH_GOKE
    gk_set_phy_reset_pin(gk_all_gpio_cfg.phy_reset);
  #if (ETH_INSTANCES >= 1)
      if (cmdline_mac[0])
          memcpy(mac_addr, cmdline_mac, 6);
    mac_addr[0] &= 0xfe;    /* clear multicast bit */
    mac_addr[0] |= 0x02;    /* set local assignment bit (IEEE802) */
    retval = gk_init_eth0(mac_addr);
    BUG_ON(retval != 0);
  #endif
#endif

    retval = gk_init_dma();
    BUG_ON(retval != 0);

    //config video DAC for CVBS
    gk_vout_writel( GK_VA_VEDIO_DAC + 0x00, 0x01);
    gk_vout_writel( GK_VA_VEDIO_DAC + 0x04, 0x01);
    gk_vout_writel( GK_VA_VEDIO_DAC + 0x08, 0x00);
    gk_vout_writel( GK_VA_VEDIO_DAC + 0x10, 0x01);
    gk_vout_writel( GK_VA_VEDIO_DAC + 0x14, 0x01);
    gk_vout_writel( GK_VA_VEDIO_DAC + 0x18, 0x3F);

    gk_init_sd();

#ifdef CONFIG_PMU_ALWAYS_RUNNING
    // get pmu controller
    gk_gpio_func_config(gk_all_gpio_cfg.pmu_ctl, GPIO_TYPE_OUTPUT_1);
    msleep(1);
    gk_gpio_func_config(gk_all_gpio_cfg.pmu_ctl, GPIO_TYPE_OUTPUT_0);
    msleep(1);
    gk_load_51mcu_code(PMU_ALWAYS_RUNNING);
#endif

    return retval;

}

static void __init gk_init_machine(void)
{
    int i;

    //clkdev_init();
    gk_init_board("GK_chip");

#if defined(CONFIG_MTD_SFLASH_GOKE) || defined(CONFIG_MTD_SFLASH_GOKE_MODULE)
    platform_device_register(&gk_flash_device_nor_0);
    platform_device_register(&gk_flash_device_nor_1);
#endif
#if defined(CONFIG_MTD_SPINAND_GOKE) || defined(CONFIG_MTD_SPINAND_GOKE_MODULE)
    platform_device_register(&gk_flash_device_nand_0);
    platform_device_register(&gk_flash_device_nand_1);
#endif

    gk_create_proc_dir();

#ifdef CONFIG_FB_GOKE
    gk_init_fb();
#endif

    printk("gk register devices %d\n", ARRAY_SIZE(gk_devices));

    /* Register devices */
    platform_add_devices(gk_devices, ARRAY_SIZE(gk_devices));
    for (i = 0; i < ARRAY_SIZE(gk_devices); i++) {
        device_set_wakeup_capable(&gk_devices[i]->dev, 1);
        device_set_wakeup_enable(&gk_devices[i]->dev, 0);
    }

    printk("gk register I2C\n");
    //i2c_register_board_info(0, &gk_ak4642_board_info, 1);
    i2c_register_board_info(0, gk_board_vi_infos, ARRAY_SIZE(gk_board_vi_infos));
    i2c_register_board_info(1, gk_board_hdmi_infos, ARRAY_SIZE(gk_board_hdmi_infos));

    spi_register_board_info(gk_spi_devices,
        ARRAY_SIZE(gk_spi_devices));

    pm_power_off = gk_power_off;

}

MACHINE_START(GOKE_IPC, "Goke IPC Board")
    .map_io         = gk_map_io,
    .init_irq       = gk_init_irq,
    .handle_irq     = gk_vic_handle_irq,
    .timer          = &gk_sys_timer,
    .init_machine   = gk_init_machine,
    .restart        = gk_restart,
MACHINE_END


