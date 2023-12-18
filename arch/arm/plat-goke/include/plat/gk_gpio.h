/*!
*****************************************************************************
** \file        arch/arm/plat-goke/include/plat/gk_gpio.h
**
** \version     $Id: gk_gpio.h 12382 2017-07-18 03:43:18Z yulindeng $
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
#ifndef _GK_GPIO_H_
#define _GK_GPIO_H_
#include <asm-generic/gpio.h>
#include <mach/irqs.h>



//*****************************************************************************
//*****************************************************************************
//** Defines and Macros
//*****************************************************************************
//*****************************************************************************
#define GK_GPIO(n)          (n)
#define GPIO_BANK_SIZE      64

#ifdef CONFIG_ARCH_GK7102C
#define GPIO_REG_LL_N       21
#else
#define GPIO_REG_LL_N       32
#endif

/* SW definitions */
#define GPIO_HIGH           1
#define GPIO_LOW            0

#define GK_GPIO_IO_MODULE_PARAM_CALL(name_prefix, arg, perm) \
    module_param_cb(name_prefix##gpio_id, &param_ops_int, &(arg.gpio_id), perm); \
    module_param_cb(name_prefix##active_level, &param_ops_int, &(arg.active_level), perm); \
    module_param_cb(name_prefix##active_delay, &param_ops_int, &(arg.active_delay), perm)
#define GK_GPIO_RESET_MODULE_PARAM_CALL(name_prefix, arg, perm) \
    module_param_cb(name_prefix##gpio_id, &param_ops_int, &(arg.gpio_id), perm); \
    module_param_cb(name_prefix##active_level, &param_ops_int, &(arg.active_level), perm); \
    module_param_cb(name_prefix##active_delay, &param_ops_int, &(arg.active_delay), perm)

/* GPIO function selection */
/* Select SW or HW control and input/output direction of S/W function */
#define GPIO_FUNC_SW_INPUT      0
#define GPIO_FUNC_SW_OUTPUT     1
#define GPIO_FUNC_HW            2

//*****************************************************************************
//*****************************************************************************
//** Enumerated types
//*****************************************************************************
//*****************************************************************************
/* GPIO function selection */
typedef enum
{
    GPIO_FUNC_IN     = 1,
    GPIO_FUNC_OUT    = 2,
    GPIO_FUNC_INOUT  = 3,
}GPIO_FUNC_TYPE_E;

typedef enum
{
    GK_GPIO_UNDEFINED       = 0x0000,
    GK_GPIO_SF_START        = 0x0001,               // 0x0001 = 1
    GK_GPIO_SF_CS0          = GK_GPIO_SF_START,     // 0x0001 = 1
    GK_GPIO_SF_CS1,
    GK_GPIO_SF_HOLD,
    GK_GPIO_SF_WP,
    GK_GPIO_SF_REV2,
    GK_GPIO_SF_REV3,
    GK_GPIO_SF_REV4,
    GK_GPIO_SF_REV5,
    GK_GPIO_SF_REV6,
    GK_GPIO_SF_REV7,
    GK_GPIO_SF_REV8,
    GK_GPIO_SF_REV9,
    GK_GPIO_SF_REV10,
    GK_GPIO_SF_REV11,
    GK_GPIO_SF_REV12,
    GK_GPIO_SF_REV13,
    GK_GPIO_SF_REV14,
    GK_GPIO_SF_REV15,
    GK_GPIO_SF_END,                                 // 0x0013 = 19
    GK_GPIO_SPI_START       = GK_GPIO_SF_END,       // 0x0013 = 19
    GK_GPIO_SPI0_SCLK       = GK_GPIO_SPI_START,    // 0x0013 = 19
    GK_GPIO_SPI0_SI,
    GK_GPIO_SPI0_SO,
    GK_GPIO_SPI0_HOLD,
    GK_GPIO_SPI0_WP,
    GK_GPIO_SPI0_CS0,
    GK_GPIO_SPI0_CS1,
    GK_GPIO_SPI0_CS2,
    GK_GPIO_SPI0_CS3,
    GK_GPIO_SPI0_CS4,
    GK_GPIO_SPI0_CS5,
    GK_GPIO_SPI0_CS6,
    GK_GPIO_SPI0_CS7,
    GK_GPIO_SPI1_SCLK,
    GK_GPIO_SPI1_SI,
    GK_GPIO_SPI1_SO,
    GK_GPIO_SPI1_HOLD,
    GK_GPIO_SPI1_WP,
    GK_GPIO_SPI1_CS0,
    GK_GPIO_SPI1_CS1,
    GK_GPIO_SPI1_CS2,
    GK_GPIO_SPI1_CS3,
    GK_GPIO_SPI1_CS4,
    GK_GPIO_SPI1_CS5,
    GK_GPIO_SPI1_CS6,
    GK_GPIO_SPI1_CS7,
    GK_GPIO_SPI_REV0,
    GK_GPIO_SPI_REV1,
    GK_GPIO_SPI_REV2,
    GK_GPIO_SPI_REV3,
    GK_GPIO_SPI_REV4,
    GK_GPIO_SPI_REV5,
    GK_GPIO_SPI_REV6,
    GK_GPIO_SPI_REV7,
    GK_GPIO_SPI_REV8,
    GK_GPIO_SPI_REV9,
    GK_GPIO_SPI_REV10,
    GK_GPIO_SPI_REV11,
    GK_GPIO_SPI_REV12,
    GK_GPIO_SPI_REV13,
    GK_GPIO_SPI_REV14,
    GK_GPIO_SPI_REV15,
    GK_GPIO_SPI_END,                                // 0x003D = 61
    GK_GPIO_UART_START      = GK_GPIO_SPI_END,      // 0x003D = 61
    GK_GPIO_UART0_TX        = GK_GPIO_UART_START,   // 0x003D = 61
    GK_GPIO_UART0_RX,
    GK_GPIO_UART0_RTS_N,
    GK_GPIO_UART0_DTR_N,
    GK_GPIO_UART1_TX,
    GK_GPIO_UART1_RX,
    GK_GPIO_UART1_RTS_N,
    GK_GPIO_UART1_DTR_N,
    GK_GPIO_UART2_TX,
    GK_GPIO_UART2_RX,
    GK_GPIO_UART2_RTS_N,
    GK_GPIO_UART2_DTR_N,
    GK_GPIO_UART_REV0,
    GK_GPIO_UART_REV1,
    GK_GPIO_UART_REV2,
    GK_GPIO_UART_REV3,
    GK_GPIO_UART_REV4,
    GK_GPIO_UART_REV5,
    GK_GPIO_UART_REV6,
    GK_GPIO_UART_REV7,
    GK_GPIO_UART_REV8,
    GK_GPIO_UART_REV9,
    GK_GPIO_UART_REV10,
    GK_GPIO_UART_REV11,
    GK_GPIO_UART_REV12,
    GK_GPIO_UART_REV13,
    GK_GPIO_UART_REV14,
    GK_GPIO_UART_REV15,
    GK_GPIO_UART_END,                               // 0x0059 = 89
    GK_GPIO_SDIO_START      = GK_GPIO_UART_END,     // 0x0059 = 89
    GK_GPIO_SDIO0_CLK       = GK_GPIO_SDIO_START,   // 0x0059 = 89
    GK_GPIO_SDIO0_CMD,
    GK_GPIO_SDIO0_WP_N,
    GK_GPIO_SDIO0_CD_N,
    GK_GPIO_SDIO0_DATA_0,
    GK_GPIO_SDIO0_DATA_1,
    GK_GPIO_SDIO0_DATA_2,
    GK_GPIO_SDIO0_DATA_3,
    GK_GPIO_SDIO0_DATA_4,
    GK_GPIO_SDIO0_DATA_5,
    GK_GPIO_SDIO0_DATA_6,
    GK_GPIO_SDIO0_DATA_7,
    GK_GPIO_SDIO1_CLK,
    GK_GPIO_SDIO1_CMD,
    GK_GPIO_SDIO1_WP_N,
    GK_GPIO_SDIO1_CD_N,
    GK_GPIO_SDIO1_DATA_0,
    GK_GPIO_SDIO1_DATA_1,
    GK_GPIO_SDIO1_DATA_2,
    GK_GPIO_SDIO1_DATA_3,
    GK_GPIO_SDIO1_DATA_4,
    GK_GPIO_SDIO1_DATA_5,
    GK_GPIO_SDIO1_DATA_6,
    GK_GPIO_SDIO1_DATA_7,
    GK_GPIO_SDIO_REV0,
    GK_GPIO_SDIO_REV1,
    GK_GPIO_SDIO_REV2,
    GK_GPIO_SDIO_REV3,
    GK_GPIO_SDIO_REV4,
    GK_GPIO_SDIO_REV5,
    GK_GPIO_SDIO_REV6,
    GK_GPIO_SDIO_REV7,
    GK_GPIO_SDIO_REV8,
    GK_GPIO_SDIO_REV9,
    GK_GPIO_SDIO_REV10,
    GK_GPIO_SDIO_REV11,
    GK_GPIO_SDIO_REV12,
    GK_GPIO_SDIO_REV13,
    GK_GPIO_SDIO_REV14,
    GK_GPIO_SDIO_REV15,
    GK_GPIO_SDIO_END,                               // 0x0081 = 129
    GK_GPIO_ENET_START      = GK_GPIO_SDIO_END,     // 0x0081 = 129
    GK_GPIO_ENET_PHY_TXD_0  = GK_GPIO_ENET_START,   // 0x0081 = 129
    GK_GPIO_ENET_PHY_TXD_1,
    GK_GPIO_ENET_PHY_TXD_2,
    GK_GPIO_ENET_PHY_TXD_3,
    GK_GPIO_ENET_PHY_TXD_4,
    GK_GPIO_ENET_PHY_TXD_5,
    GK_GPIO_ENET_PHY_TXD_6,
    GK_GPIO_ENET_PHY_TXD_7,
    GK_GPIO_ENET_PHY_RXD_0,
    GK_GPIO_ENET_PHY_RXD_1,
    GK_GPIO_ENET_PHY_RXD_2,
    GK_GPIO_ENET_PHY_RXD_3,
    GK_GPIO_ENET_PHY_RXD_4,
    GK_GPIO_ENET_PHY_RXD_5,
    GK_GPIO_ENET_PHY_RXD_6,
    GK_GPIO_ENET_PHY_RXD_7,
    GK_GPIO_ENET_PHY_CLK_RX,
    GK_GPIO_ENET_PHY_CLK_TX,
    GK_GPIO_ENET_PHY_MDC,
    GK_GPIO_ENET_PHY_MDIO,
    GK_GPIO_ENET_PHY_COL,
    GK_GPIO_ENET_PHY_CRS,
    GK_GPIO_ENET_PHY_RXER,
    GK_GPIO_ENET_PHY_RXDV,
    GK_GPIO_ENET_PHY_TXER,
    GK_GPIO_ENET_PHY_TXEN,
    GK_GPIO_EPHY_LED_0,
    GK_GPIO_EPHY_LED_1,
    GK_GPIO_EPHY_LED_2,
    GK_GPIO_EPHY_LED_3,
    GK_GPIO_EPHY_LED_4,
    GK_GPIO_EPHY_LED_5,
    GK_GPIO_EPHY_LED_6,
    GK_GPIO_EPHY_LED_7,
    GK_GPIO_ENET_REV0,
    GK_GPIO_ENET_REV1,
    GK_GPIO_ENET_REV2,
    GK_GPIO_ENET_REV3,
    GK_GPIO_ENET_REV4,
    GK_GPIO_ENET_REV5,
    GK_GPIO_ENET_REV6,
    GK_GPIO_ENET_REV7,
    GK_GPIO_ENET_REV8,
    GK_GPIO_ENET_REV9,
    GK_GPIO_ENET_REV10,
    GK_GPIO_ENET_REV11,
    GK_GPIO_ENET_REV12,
    GK_GPIO_ENET_REV13,
    GK_GPIO_ENET_REV14,
    GK_GPIO_ENET_REV15,
    GK_GPIO_ENET_END,                               // 0x00B3 = 179
    GK_GPIO_PWM_START       = GK_GPIO_ENET_END,     // 0x00B3 = 179
    GK_GPIO_PWM_0           = GK_GPIO_PWM_START,    // 0x00B3 = 179
    GK_GPIO_PWM_1,
    GK_GPIO_PWM_2,
    GK_GPIO_PWM_3,
    GK_GPIO_PWM_4,
    GK_GPIO_PWM_5,
    GK_GPIO_PWM_6,
    GK_GPIO_PWM_7,
    GK_GPIO_PWM_8,
    GK_GPIO_PWM_9,
    GK_GPIO_PWM_10,
    GK_GPIO_PWM_11,
    GK_GPIO_PWM_12,
    GK_GPIO_PWM_13,
    GK_GPIO_PWM_14,
    GK_GPIO_PWM_15,
    GK_GPIO_PWM_REV0,
    GK_GPIO_PWM_REV1,
    GK_GPIO_PWM_REV2,
    GK_GPIO_PWM_REV3,
    GK_GPIO_PWM_REV4,
    GK_GPIO_PWM_REV5,
    GK_GPIO_PWM_REV6,
    GK_GPIO_PWM_REV7,
    GK_GPIO_PWM_REV8,
    GK_GPIO_PWM_REV9,
    GK_GPIO_PWM_REV10,
    GK_GPIO_PWM_REV11,
    GK_GPIO_PWM_REV12,
    GK_GPIO_PWM_REV13,
    GK_GPIO_PWM_REV14,
    GK_GPIO_PWM_REV15,
    GK_GPIO_PWM_END,                                // 0x00D3 = 211
    GK_GPIO_VD_START        = GK_GPIO_PWM_END,      // 0x00D3 = 211
    GK_GPIO_VD_CLOCK        = GK_GPIO_VD_START,     // 0x00D3 = 211
    GK_GPIO_VD_VSYNC,
    GK_GPIO_VD_HSYNC,
    GK_GPIO_VD_HVLD,
    GK_GPIO_VD_DATA0,
    GK_GPIO_VD_DATA1,
    GK_GPIO_VD_DATA2,
    GK_GPIO_VD_DATA3,
    GK_GPIO_VD_DATA4,
    GK_GPIO_VD_DATA5,
    GK_GPIO_VD_DATA6,
    GK_GPIO_VD_DATA7,
    GK_GPIO_VD_DATA8,
    GK_GPIO_VD_DATA9,
    GK_GPIO_VD_DATA10,
    GK_GPIO_VD_DATA11,
    GK_GPIO_VD_DATA12,
    GK_GPIO_VD_DATA13,
    GK_GPIO_VD_DATA14,
    GK_GPIO_VD_DATA15,
    GK_GPIO_VD_DATA16,
    GK_GPIO_VD_DATA17,
    GK_GPIO_VD_DATA18,
    GK_GPIO_VD_DATA19,
    GK_GPIO_VD_DATA20,
    GK_GPIO_VD_DATA21,
    GK_GPIO_VD_DATA22,
    GK_GPIO_VD_DATA23,
    GK_GPIO_VD_REV0,
    GK_GPIO_VD_REV1,
    GK_GPIO_VD_REV2,
    GK_GPIO_VD_REV3,
    GK_GPIO_VD_REV4,
    GK_GPIO_VD_REV5,
    GK_GPIO_VD_REV6,
    GK_GPIO_VD_REV7,
    GK_GPIO_VD_REV8,
    GK_GPIO_VD_REV9,
    GK_GPIO_VD_REV10,
    GK_GPIO_VD_REV11,
    GK_GPIO_VD_REV12,
    GK_GPIO_VD_REV13,
    GK_GPIO_VD_REV14,
    GK_GPIO_VD_REV15,
    GK_GPIO_VD_END,                                 // 0x00FF = 255
    GK_GPIO_I80_START       = GK_GPIO_VD_END,       // 0x00FF = 255
    GK_GPIO_I80_LCD_RST     = GK_GPIO_I80_START,    // 0x00FF = 255
    GK_GPIO_I80_RDN,
    GK_GPIO_I80_WRN,
    GK_GPIO_I80_DCX,
    GK_GPIO_I80_CSN,
    GK_GPIO_I80_DATA0,
    GK_GPIO_I80_DATA1,
    GK_GPIO_I80_DATA2,
    GK_GPIO_I80_DATA3,
    GK_GPIO_I80_DATA4,
    GK_GPIO_I80_DATA5,
    GK_GPIO_I80_DATA6,
    GK_GPIO_I80_DATA7,
    GK_GPIO_I80_DATA8,
    GK_GPIO_I80_OUTPUT_DATA0,
    GK_GPIO_I80_OUTPUT_DATA1,
    GK_GPIO_I80_OUTPUT_DATA2,
    GK_GPIO_I80_OUTPUT_DATA3,
    GK_GPIO_I80_OUTPUT_DATA4,
    GK_GPIO_I80_OUTPUT_DATA5,
    GK_GPIO_I80_OUTPUT_DATA6,
    GK_GPIO_I80_OUTPUT_DATA7,
    GK_GPIO_I80_OUTPUT_DATA8,
    GK_GPIO_I80_INPUT_DATA0,
    GK_GPIO_I80_INPUT_DATA1,
    GK_GPIO_I80_INPUT_DATA2,
    GK_GPIO_I80_INPUT_DATA3,
    GK_GPIO_I80_INPUT_DATA4,
    GK_GPIO_I80_INPUT_DATA5,
    GK_GPIO_I80_INPUT_DATA6,
    GK_GPIO_I80_INPUT_DATA7,
    GK_GPIO_I80_INPUT_DATA8,
    GK_GPIO_I80_REV0,
    GK_GPIO_I80_REV1,
    GK_GPIO_I80_REV2,
    GK_GPIO_I80_REV3,
    GK_GPIO_I80_REV4,
    GK_GPIO_I80_REV5,
    GK_GPIO_I80_REV6,
    GK_GPIO_I80_REV7,
    GK_GPIO_I80_REV8,
    GK_GPIO_I80_REV9,
    GK_GPIO_I80_REV10,
    GK_GPIO_I80_REV11,
    GK_GPIO_I80_REV12,
    GK_GPIO_I80_REV13,
    GK_GPIO_I80_REV14,
    GK_GPIO_I80_REV15,
    GK_GPIO_I80_END,                                //  0x012F = 303
    GK_GPIO_I2C_START       = GK_GPIO_I80_END,      //  0x012F = 303
    GK_GPIO_I2C0_CLK        = GK_GPIO_I2C_START,    //  0x012F = 303
    GK_GPIO_I2C0_DATA,
    GK_GPIO_I2C1_CLK,
    GK_GPIO_I2C1_DATA,
    GK_GPIO_I2C2_CLK,
    GK_GPIO_I2C2_DATA,
    GK_GPIO_I2C_REV0,
    GK_GPIO_I2C_REV1,
    GK_GPIO_I2C_REV2,
    GK_GPIO_I2C_REV3,
    GK_GPIO_I2C_REV4,
    GK_GPIO_I2C_REV5,
    GK_GPIO_I2C_REV6,
    GK_GPIO_I2C_REV7,
    GK_GPIO_I2C_REV8,
    GK_GPIO_I2C_REV9,
    GK_GPIO_I2C_REV10,
    GK_GPIO_I2C_REV11,
    GK_GPIO_I2C_REV12,
    GK_GPIO_I2C_REV13,
    GK_GPIO_I2C_REV14,
    GK_GPIO_I2C_REV15,
    GK_GPIO_I2C_END,                                // 0x0145 = 325
    GK_GPIO_AU_START        = GK_GPIO_I2C_END,      // 0x0145 = 325
    GK_GPIO_AO0_MCLK        = GK_GPIO_AU_START,     // 0x0145 = 325
    GK_GPIO_AO0_BCLK,
    GK_GPIO_AO0_LRCLK,
    GK_GPIO_AO0_DATA,
    GK_GPIO_AO1_MCLK,
    GK_GPIO_AO1_BCLK,
    GK_GPIO_AO1_LRCLK,
    GK_GPIO_AO1_DATA,
    GK_GPIO_AI0_MCLK,
    GK_GPIO_AI0_BCLK,
    GK_GPIO_AI0_LRCLK,
    GK_GPIO_AI0_DATA,
    GK_GPIO_AI1_MCLK,
    GK_GPIO_AI1_BCLK,
    GK_GPIO_AI1_LRCLK,
    GK_GPIO_AI1_DATA,
    GK_GPIO_AU_REV0,
    GK_GPIO_AU_REV1,
    GK_GPIO_AU_REV2,
    GK_GPIO_AU_REV3,
    GK_GPIO_AU_REV4,
    GK_GPIO_AU_REV5,
    GK_GPIO_AU_REV6,
    GK_GPIO_AU_REV7,
    GK_GPIO_AU_REV8,
    GK_GPIO_AU_REV9,
    GK_GPIO_AU_REV10,
    GK_GPIO_AU_REV11,
    GK_GPIO_AU_REV12,
    GK_GPIO_AU_REV13,
    GK_GPIO_AU_REV14,
    GK_GPIO_AU_REV15,
    GK_GPIO_AU_END,                                 // 0x0165 = 357
    GK_GPIO_MISC_START      = GK_GPIO_AU_END,       // 0x0165 = 357
    GK_GPIO_OUTPUT_0        = GK_GPIO_MISC_START,   // 0x0165 = 357
    GK_GPIO_OUTPUT_1,
    GK_GPIO_INPUT_0,
    GK_GPIO_INPUT_1,
    GK_GPIO_INPUT,
    GK_GPIO_SENSOR_POWER,
    GK_GPIO_SENSOR_RESET,
    GK_GPIO_PHY_RESET,
    GK_GPIO_PHY_SPEED_LED,
    GK_GPIO_PHY_LINK_LED,
    GK_GPIO_PHY_DATA_LED,
    GK_GPIO_IR_LED_CTRL,
    GK_GPIO_IR_DETECT,
    GK_GPIO_IR_CUT1,
    GK_GPIO_IR_CUT2,
    GK_GPIO_ALARM_IN,
    GK_GPIO_ALARM_OUT,
    GK_GPIO_USB_HOST,
    GK_GPIO_USB_OTG,
    GK_GPIO_SDIO0_POWER,
    GK_GPIO_SDIO1_POWER,
    GK_GPIO_PMU_CTL,
    GK_GPIO_JTAG_TRSTN,
    GK_GPIO_JTAG_TCK,
    GK_GPIO_JTAG_TMS,
    GK_GPIO_JTAG_TDI,
    GK_GPIO_JTAG_TDO,
    GK_GPIO_TIMER1_CLK,
    GK_GPIO_TIMER2_CLK,
    GK_GPIO_TIMER3_CLK,
    GK_GPIO_RCT_CLK_OUT1,
    GK_GPIO_RCT_CLK_OUT2,
    GK_GPIO_MISC_END,
    GK_GPIO_FUN_ALL         = GK_GPIO_MISC_END,
} GK_GPIO_FUNC_E;


//*****************************************************************************
//*****************************************************************************
//** Data Structures
//*****************************************************************************
//*****************************************************************************
struct gk_gpio_irq_info
{
    int pin;
    int type;
    int val;    // when is input, the value will be transfered to handler
    irq_handler_t handler;
};

struct gk_gpio_io_info
{
    int gpio_id;
    int active_level;
    int active_delay;       //ms
};

typedef struct
{
    u32     pin;
    u32     type;
}GPIO_XREF_S;

struct gk_gpio_bank
{
    struct gpio_chip    chip;
    spinlock_t          lock;
    u32                 base_reg;   // for is/ibe/iev/ie/ic/ris/mis/din/
    u32                 io_reg;     // PLL_IOCTRL
    u32                 index;      // instance no
    struct
    {
        u32 isl_reg;
        u32 ish_reg;
        u32 ibel_reg;
        u32 ibeh_reg;
        u32 ievl_reg;
        u32 ievh_reg;
        u32 iel_reg;
        u32 ieh_reg;
    }pm_info;
};

struct gk_gpio_inst
{
    u32                     bank_num;
    struct  gk_gpio_bank*   gpio_bank;
    u32                     output_cfg[CONFIG_ARCH_NR_GPIO];            // CONFIG_GK_GPIO_MAX_OUTPUT_TYPE != CONFIG_ARCH_NR_GPIO
    u32                     input_cfg[CONFIG_GK_GPIO_MAX_INPUT_TYPE];   // CONFIG_GK_GPIO_MAX_INPUT_TYPE  != CONFIG_ARCH_NR_GPIO
    u32                     irq_no;
    struct gk_gpio_irq_info irq_info[CONFIG_ARCH_NR_GPIO];
    u32                     irq_now;
    u32                     gpio_valid[BITS_TO_LONGS(CONFIG_ARCH_NR_GPIO)];
    u32                     gpio_freeflag[BITS_TO_LONGS(CONFIG_ARCH_NR_GPIO)];
    u32                     irq_flag[BITS_TO_LONGS(CONFIG_ARCH_NR_GPIO)];
    u32                     base_bus;   // for sel/in
    u32                     per_sel_reg;
};

#define GK_GPIO_BANK(name, reg_base, io_base, base_gpio, gpio_num, bank)    \
{                                                       \
    .chip =                                             \
    {                                                   \
        .label              = name,                     \
        .owner              = THIS_MODULE,              \
        .request            = gk_gpio_request,          \
        .free               = gk_gpio_free,             \
        .direction_input    = gk_gpio_direction_input,  \
        .get                = gk_gpio_get_ex,           \
        .direction_output   = gk_gpio_direction_output, \
        .set                = gk_gpio_set,              \
        .to_irq             = gk_gpio_to_irq,           \
        .dbg_show           = gk_gpio_dbg_show,         \
        .base               = base_gpio,                \
        .ngpio              = gpio_num,                 \
        .can_sleep          = 0,                        \
        .exported           = 0,                        \
    },                                                  \
    .base_reg               = reg_base,                 \
    .io_reg                 = io_base,                  \
    .index                  = bank,                     \
    .pm_info                = {0},                      \
}


//*****************************************************************************
//*****************************************************************************
//** Global Data
//*****************************************************************************
//*****************************************************************************
extern char gk_gpio_name_table[GK_GPIO_FUN_ALL][20];



//*****************************************************************************
//*****************************************************************************
//** API Functions
//*****************************************************************************
//*****************************************************************************
#ifdef CONFIG_GK_GPIO_V1_00
#include <plat/gk_gpio_v1_00.h>
#endif
#ifdef CONFIG_GK_GPIO_V1_10
#include <plat/gk_gpio_v1_10.h>
#endif
#ifdef CONFIG_GK_GPIO_V1_20
#include <plat/gk_gpio_v1_20.h>
#endif
#ifdef CONFIG_GK_GPIO_V1_30
#include <plat/gk_gpio_v1_30.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif
extern int __init gk_init_gpio(void);
extern int gk_gpio_set_type(struct gk_gpio_bank* bank, u32 pin, u32 type); // GPIO_TYPE_E
int gk_gpio_request(struct gpio_chip *chip, u32 pin);
void gk_gpio_free(struct gpio_chip *chip, u32 pin);
int gk_gpio_direction_input(struct gpio_chip *chip, u32 pin, int val);
int gk_gpio_get(unsigned offset);
int gk_gpio_get_ex(struct gpio_chip *chip, unsigned offset);
int gk_gpio_direction_output(struct gpio_chip *chip, unsigned offset, int val);
void gk_gpio_set(struct gpio_chip *chip, unsigned offset, int val);
int gk_gpio_to_irq(struct gpio_chip *chip, unsigned offset);
void gk_gpio_dbg_show(struct seq_file *s, struct gpio_chip *chip);
int gk_is_valid_gpio_irq(struct gk_gpio_irq_info *pinfo);
extern void gk_gpio_set_out(u32 id, u32 value);

int gk_gpio_func_config(u32 pin, u32 func);

extern int gk_set_gpio_output(struct gk_gpio_io_info *pinfo, u32 on);
extern u32 gk_get_gpio_input(struct gk_gpio_io_info *pinfo);
extern int gk_set_gpio_reset(struct gk_gpio_io_info *pinfo);
extern int gk_set_gpio_output_can_sleep(struct gk_gpio_io_info *pinfo, u32 on, int can_sleep);
extern u32 gk_get_gpio_input_can_sleep(struct gk_gpio_io_info *pinfo, int can_sleep);
extern int gk_set_gpio_reset_can_sleep(struct gk_gpio_io_info *pinfo, int can_sleep);
extern void gk_gpio_config(u32 pin, u32 func);


#define GK_GPIO_IRQ_MODULE_PARAM_CALL(name_prefix, arg, perm) \
    module_param_cb(name_prefix##pin, &param_ops_int, &(arg.pin), perm); \
    module_param_cb(name_prefix##type, &param_ops_int, &(arg.type), perm); \
    module_param_cb(name_prefix##val, &param_ops_int, &(arg.val), perm);

#define GK_IRQ_MODULE_PARAM_CALL(name_prefix, arg, perm) \
    module_param_cb(name_prefix##irq_gpio, &param_ops_int, &(arg.irq_gpio), perm); \
    module_param_cb(name_prefix##irq_line, &param_ops_int, &(arg.irq_line), perm); \
    module_param_cb(name_prefix##irq_type, &param_ops_int, &(arg.irq_type), perm); \
    module_param_cb(name_prefix##irq_gpio_val, &param_ops_int, &(arg.irq_gpio_val), perm); \
    module_param_cb(name_prefix##irq_gpio_mode, &param_ops_int, &(arg.irq_gpio_mode), perm)
extern int gk_is_valid_gpio_irq(struct gk_gpio_irq_info *pgpio_irq);
extern int gk_gpio_request_irq(struct gk_gpio_irq_info *pinfo);
extern int gk_gpio_release_irq(u32 pin);
int __init goke_init_gpio(void);
void GH_GPIO_set_INPUT_CFG_in_sel(u8 index, u8 data);

#ifdef __cplusplus
}
#endif



#endif /* _GK_GPIO_H_ */

