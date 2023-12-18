/*
 * drivers/drivers/misc/gk_pwm_v1_00.c
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


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/poll.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#include <mach/hardware.h>

#include <linux/clk.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/miscdevice.h>


#define DEVICE_NAME     "pwm" //�豸��, major 10
#define NODE_NAME       "gk_pwm"
#if defined(CONFIG_ARCH_GK710X) || defined(CONFIG_ARCH_GK710X_FPGA)
#define MAX_CHANNEL     4
#else
#define MAX_CHANNEL     8
#endif

//��������,���ں���� ioctl �еĿ�������
#define GK_PWM_IOCTL_MAGIC          'k'

#define _PWM_IO(IOCTL)              _IO(GK_PWM_IOCTL_MAGIC, IOCTL)
#define _PWM_IOR(IOCTL, param)      _IOR(GK_PWM_IOCTL_MAGIC, IOCTL, param)
#define _PWM_IOW(IOCTL, param)      _IOW(GK_PWM_IOCTL_MAGIC, IOCTL, param)
#define _PWM_IOWR(IOCTL, param)     _IOWR(GK_PWM_IOCTL_MAGIC, IOCTL, param)

extern u32 get_apb_bus_freq_hz(void);

typedef struct pwm_status_s
{
    u8 channel;
    u8 enable;
}pwm_status_t;

typedef struct pwm_mode_s
{
    u8   channel;    /* 0 ~ 3*/
    u8   mode;       /* 0: normal speed  1: sync speed */
}pwm_mode_t;

typedef struct pwm_speed_s
{
    u8  channel; /* 0 ~ 3 */
    u32 speed;  /* 0x01 ~ 0x3ff*/
}pwm_speed_t;

typedef struct
{
    u8   channel;    /* 0 ~ 3 */
    u16  xon;        /* 0x00 ~ 0xffff */
    u16  xoff;       /* 0x00 ~ 0xffff */
}pwm_duty_t;

typedef struct{
    u8  channel;
    u8  chEnable;
    u32 freqHz;
    u16 duty;
    u8  mode;    
}pwm_channel_t;

enum {
    IOCTL_ENABLE = 0x10,
    IOCTL_DISABLE,
    IOCTL_SET_DUTY,
    IOCTL_GET_DUTY,
    IOCTL_SET_MODE,
    IOCTL_GET_MODE,
    IOCTL_SET_SPEED,
    IOCTL_GET_SPEED,
    IOCTL_GET_STATUS,
};

#define GK_PWM_IOCTL_ENABLE     _PWM_IO(IOCTL_ENABLE)
#define GK_PWM_IOCTL_DISABLE    _PWM_IO(IOCTL_DISABLE)
#define GK_PWM_IOCTL_SET_DUTY   _PWM_IOW(IOCTL_SET_DUTY, pwm_duty_t*)
#define GK_PWM_IOCTL_GET_DUTY   _PWM_IOR(IOCTL_GET_DUTY, pwm_duty_t*)
#define GK_PWM_IOCTL_SET_MODE   _PWM_IOW(IOCTL_SET_MODE, pwm_mode_t*)
#define GK_PWM_IOCTL_GET_MODE   _PWM_IOR(IOCTL_GET_MODE, pwm_mode_t*)
#define GK_PWM_IOCTL_SET_SPEED  _PWM_IOW(IOCTL_SET_SPEED, pwm_speed_t*)
#define GK_PWM_IOCTL_GET_SPEED  _PWM_IOR(IOCTL_GET_SPEED, pwm_speed_t*)
#define GK_PWM_IOCTL_GET_STATUS _PWM_IOR(IOCTL_GET_STATUS, pwm_status_t*)
/*----------------------------------------------------------------------------*/
/* registers                                                                  */
/*----------------------------------------------------------------------------*/
#define PWM_VA_BASE                 GK_VA_PWM0
#define REG_PWM_ENABLE              (PWM_VA_BASE + 0x0000) /* read/write */
#define REG_PWM_CONTROL             (PWM_VA_BASE + 0x0004) /* read/write */
#define REG_PWM_CONTROL1            (PWM_VA_BASE + 0x0008) /* read/write */
#define REG_PWM_MODE                (PWM_VA_BASE + 0x000C) /* read/write */

typedef enum
{
    GD_NORMAL_SPEED,
    GD_SYNC_SPEED,
}GD_PWM_MODE_E;

static struct semaphore lock;

typedef union { /* PWM_Mode */
    u32 all;
    struct {
        u32 divider     : 10;
        u32 mode        : 1;
        u32             : 21;
    } bitc;
} GH_PWM_MODE_S;

// 69000000 / 200 / 172500Hz = 345000 / 172500 =     2
// 69000000 / 200 /   5000Hz = 345000 /   5000 =    69
// 69000000 / 200 /     10Hz = 345000 /     10 = 34500
// 69000000 / 200 /      6Hz = 345000 /      6 = 57500
// 69000000 / 200 /      5Hz = 345000 /      5 = 69000 > 65536
static u32 speed_ctl[5][2] =
{
//   speed     div     clk            div             max:xon+1+xoff=1
    {20000,  200}, // 69000000 /     1 = 69000000 / 2000 = 34500
    { 2000,  200}, // 69000000 /    10 =  6900000 /  200 = 34500
    {  200,  200}, // 69000000 /   100 =   690000 /   20 = 34500
    {   20,  200}, // 69000000 /  1000 =    69000 /    2 = 34500
    {    1,  200}, // 69000000 / 10000 =     6900 /    1 =  6900
};

#if defined(CONFIG_ARCH_GK710X) || defined(CONFIG_ARCH_GK710X_FPGA)
static u32  g_duty[MAX_CHANNEL]     = {1000, 1000, 1000, 1000};
static u32  g_dutyset[MAX_CHANNEL]  = { 499,  499,  499,  499};
#else
static u32  g_duty[MAX_CHANNEL]     = {1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000};
static u32  g_dutyset[MAX_CHANNEL]  = { 499,  499,  499,  499, 499,  499,  499,  499};
#endif
static void gk_pwm_set_duty(pwm_duty_t *pwm_duty);
static void gk_pwm_get_duty(pwm_duty_t *pwm_duty);

/* pwm enable */
static inline void GH_PWM_set_Enable(u8 index, u32 data)
{
    gk_pwm_writel(REG_PWM_ENABLE + index * (0x00001000), data);
}

/* get pwm enable status */
static inline u32 GH_PWM_get_Enable(u8 index)
{
    return gk_pwm_readl(REG_PWM_ENABLE + index * (0x00001000));
}

/* get pwm speed mode */
static inline u32 GH_PWM_get_Mode(u8 index)
{
    GH_PWM_MODE_S d;
    d.all = gk_pwm_readl(REG_PWM_MODE + index * (0x00001000));
    return d.bitc.mode;

}

/* set pwm speed mode */
static inline void GH_PWM_set_Mode(u8 index, u32 data)
{
    GH_PWM_MODE_S d;

    d.all = gk_pwm_readl(REG_PWM_MODE + index * (0x00001000));
    if(data)
        d.bitc.mode = 1;
    else
        d.bitc.mode = 0;
    gk_pwm_writel(REG_PWM_MODE + index * (0x00001000), d.all);
}

/* set pwm divider params */
static inline void GH_PWM_set_Mode_divider(u8 index, u16 data)
{
    GH_PWM_MODE_S d;
    d.all = gk_pwm_readl(REG_PWM_MODE + index * (0x00001000));
    d.bitc.divider = data;
    gk_pwm_writel(REG_PWM_MODE + index * (0x00001000), d.all);
}

/* get pwm divider params */
static inline u16 GH_PWM_get_Mode_divider(u8 index)
{
    GH_PWM_MODE_S d;
    d.all = gk_pwm_readl(REG_PWM_MODE + index * (0x00001000));
    return d.bitc.divider;
}

/* set value of control0 */
static inline void GH_PWM_set_Control(u8 index, u32 data)
{
    gk_pwm_writel(REG_PWM_CONTROL + index * (0x00001000), data);
}

/* get value of control0 */
static inline u32 GH_PWM_get_Control(u8 index)
{
    return gk_pwm_readl(REG_PWM_CONTROL + index * (0x00001000));
}

/* set value of control1 */
static inline void GH_PWM_set_Control1(u8 index, u32 data)
{
    gk_pwm_writel(REG_PWM_CONTROL1 + index * (0x00001000), data);
}

/* get value of control1 */
static inline u32 GH_PWM_get_Control1(u8 index)
{
    return gk_pwm_readl(REG_PWM_CONTROL1 + index * (0x00001000));
}

static inline s32 GH_PWM_get_freq(u8 channel)
{
    u32 frq;
    if(GH_PWM_get_Mode(channel))
    {
        return -EINVAL;// because we can not get the vi pclk
    }
    else
    {
        frq = get_apb_bus_freq_hz();
        //printk("frq=%d\n", frq);
    }

    return frq;

}

/* set pwm speed */
static int gk_pwm_set_speed(u8 channel, u32 speed)
{
    s32 frq = 0;
    u16 fdiv = 0;
    u16 i = 0;
    pwm_duty_t pwm_duty;
    pwm_duty.channel = channel;

    frq = GH_PWM_get_freq(channel);
    if(frq <= 0) return -EINVAL;

    fdiv = 1000;
    for(i=0;i<5;i++)
    {
        if(speed >= speed_ctl[i][0])
        {
            fdiv = speed_ctl[i][1];
            g_duty[channel] = frq / fdiv / speed;
            if(g_duty[channel] == 0)
            {
                g_duty[channel] = 1;
            }
            break;
        }
    }
    if(fdiv == 0) return -EINVAL;

    //printk(KERN_INFO "\npwm: freq:%d.%03dKHz, pwm freq:%dHz\n", frq/1000, frq%1000, frq/fdiv);
    if(GH_PWM_get_Mode(channel))
    {
        return -EINVAL;// because we can not get the vi pclk
    }
    else
    {
        gk_pwm_writel( GK_VA_RCT + 0x060, fdiv);
    }
    pwm_duty.xoff = g_dutyset[pwm_duty.channel];
    pwm_duty.xon  = 998 - g_dutyset[pwm_duty.channel];
    // reset duty
    gk_pwm_set_duty(&pwm_duty);

    return 0;
}

/* get pwm speed */
static long gk_pwm_get_speed(u8 channel, u32 *speed)
{
    s32 frq = 0;
    u32 fdiv = 0;

    if(GH_PWM_get_Mode(channel))
    {
        //fdiv = GH_PWM_get_Mode_divider(channel);
        //fdiv += 1;
        return -EINVAL;// because we can not get the vi pclk
    }
    else
    {
        fdiv = gk_pwm_readl( GK_VA_RCT + 0x060);
    }

    frq = GH_PWM_get_freq(channel);
    if(frq <= 0) return -EINVAL;

    *speed = frq / fdiv / g_duty[channel];

    return 0;
}

/* set pwm duty rate */
static void gk_pwm_set_duty(pwm_duty_t *pwm_duty)
{
    u32 mode;

    g_dutyset[pwm_duty->channel] = pwm_duty->xoff;
    pwm_duty->xoff = (pwm_duty->xoff + 1) * g_duty[pwm_duty->channel] / 1000;
    pwm_duty->xon  = (pwm_duty->xon  + 1) * g_duty[pwm_duty->channel] / 1000;
    if(pwm_duty->xoff)
        pwm_duty->xoff --;
    if(pwm_duty->xon)
        pwm_duty->xon --;

    mode = GH_PWM_get_Mode(pwm_duty->channel);
    if(mode == GD_NORMAL_SPEED){
        GH_PWM_set_Control(pwm_duty->channel,(pwm_duty->xoff << 16| pwm_duty->xon));
    }
    else if(mode == GD_SYNC_SPEED){
        GH_PWM_set_Control1(pwm_duty->channel,(pwm_duty->xoff << 16| pwm_duty->xon));
    }
}

/* get pwm duty rate */
static void gk_pwm_get_duty(pwm_duty_t *pwm_duty)
{
    u32 mode = 0;
    u32 duty_data = 0;

    mode = GH_PWM_get_Mode(pwm_duty->channel);
    if(mode == GD_NORMAL_SPEED){
        duty_data = GH_PWM_get_Control(pwm_duty->channel);
    }
    else if(mode == GD_SYNC_SPEED){
        duty_data = GH_PWM_get_Control1(pwm_duty->channel);
    }
    pwm_duty->xoff = (duty_data >> 16) & 0xffff;
    pwm_duty->xon  = duty_data & 0xffff;
    // fix to pwm_duty->xoff + 1 + pwm_duty->xon + 1 = 1000 (adi)
    pwm_duty->xoff = (pwm_duty->xoff + 1) * 1000 / g_duty[pwm_duty->channel];
    pwm_duty->xon  = (pwm_duty->xon  + 1) * 1000 / g_duty[pwm_duty->channel];
    if(pwm_duty->xoff)
        pwm_duty->xoff --;
    if(pwm_duty->xon)
        pwm_duty->xon --;
}

int gk_pwm_set_channel(pwm_channel_t *pwm_channel)
{
    pwm_duty_t pwm_duty;
    bool curr_enable;
    int ret = 0;
    u16 curr_duty;
    curr_duty = pwm_channel->duty;
    
    if(pwm_channel->channel >= MAX_CHANNEL || curr_duty > 1000)
    {
        return -EINVAL;
    }
    
    curr_enable = (GH_PWM_get_Enable(pwm_channel->channel) > 0) ? 1 : 0;
    if(curr_enable)
    {
        GH_PWM_set_Enable(pwm_channel->channel, 0);
    }

    GH_PWM_set_Mode(pwm_channel->channel, pwm_channel->mode);
    
    ret = gk_pwm_set_speed(pwm_channel->channel, pwm_channel->freqHz);
    if(ret < 0)
    {
        return -EINVAL;
    }

    if(curr_duty == 0)
    {
        switch(pwm_channel->channel) //duty=1/1000
        {
        case 0:
            gk_gpio_config(gk_all_gpio_cfg.pwm0, GPIO_TYPE_OUTPUT_0);
            break;
        case 1:
            gk_gpio_config(gk_all_gpio_cfg.pwm1, GPIO_TYPE_OUTPUT_0);
            break;
        case 2:
            gk_gpio_config(gk_all_gpio_cfg.pwm2, GPIO_TYPE_OUTPUT_0);
            break;
        case 3:
            gk_gpio_config(gk_all_gpio_cfg.pwm3, GPIO_TYPE_OUTPUT_0);
            break;
#if !(defined(CONFIG_ARCH_GK710X) || defined(CONFIG_ARCH_GK710X_FPGA))			
        case 4:
            gk_gpio_config(gk_all_gpio_cfg.pwm4, GPIO_TYPE_OUTPUT_0);
            break;
        case 5:
            gk_gpio_config(gk_all_gpio_cfg.pwm5, GPIO_TYPE_OUTPUT_0);
            break;
        case 6:
            gk_gpio_config(gk_all_gpio_cfg.pwm6, GPIO_TYPE_OUTPUT_0);
            break;
        case 7:
            gk_gpio_config(gk_all_gpio_cfg.pwm7, GPIO_TYPE_OUTPUT_0);
            break;			
#endif			
        default:
            return -EINVAL;
            break;
        }
    }
    else if(curr_duty == 1000 && pwm_channel->chEnable) //duty=1000/1000
    {
        switch(pwm_channel->channel)
        {
        case 0:
            gk_gpio_config(gk_all_gpio_cfg.pwm0, GPIO_TYPE_OUTPUT_1);
            break;
        case 1:
            gk_gpio_config(gk_all_gpio_cfg.pwm1, GPIO_TYPE_OUTPUT_1);
            break;
        case 2:
            gk_gpio_config(gk_all_gpio_cfg.pwm2, GPIO_TYPE_OUTPUT_1);
            break;
        case 3:
            gk_gpio_config(gk_all_gpio_cfg.pwm3, GPIO_TYPE_OUTPUT_1);
            break;
#if !(defined(CONFIG_ARCH_GK710X) || defined(CONFIG_ARCH_GK710X_FPGA))			
        case 4:
            gk_gpio_config(gk_all_gpio_cfg.pwm4, GPIO_TYPE_OUTPUT_1);
            break;
        case 5:
            gk_gpio_config(gk_all_gpio_cfg.pwm5, GPIO_TYPE_OUTPUT_1);
            break;
        case 6:
            gk_gpio_config(gk_all_gpio_cfg.pwm6, GPIO_TYPE_OUTPUT_1);
            break;
        case 7:
            gk_gpio_config(gk_all_gpio_cfg.pwm7, GPIO_TYPE_OUTPUT_1);
            break;			
#endif			
        default:
            return -EINVAL;
            break;
        }
    }
    else if(curr_duty < 1000)
    {
        switch(pwm_channel->channel)
        {
        case 0:
            gk_gpio_config(gk_all_gpio_cfg.pwm0, GPIO_TYPE_OUTPUT_PWM0_OUT);
            break;
        case 1:
            gk_gpio_config(gk_all_gpio_cfg.pwm1, GPIO_TYPE_OUTPUT_PWM1_OUT);
            break;
        case 2:
            gk_gpio_config(gk_all_gpio_cfg.pwm2, GPIO_TYPE_OUTPUT_PWM2_OUT);
            break;
        case 3:
            gk_gpio_config(gk_all_gpio_cfg.pwm3, GPIO_TYPE_OUTPUT_PWM3_OUT);         
            break;
#if !(defined(CONFIG_ARCH_GK710X) || defined(CONFIG_ARCH_GK710X_FPGA))			
        case 4:
            gk_gpio_config(gk_all_gpio_cfg.pwm4, GPIO_TYPE_OUTPUT_PWM4_OUT);
            break;
        case 5:
            gk_gpio_config(gk_all_gpio_cfg.pwm5, GPIO_TYPE_OUTPUT_PWM5_OUT);
            break;
        case 6:
            gk_gpio_config(gk_all_gpio_cfg.pwm6, GPIO_TYPE_OUTPUT_PWM6_OUT);
            break;
        case 7:
            gk_gpio_config(gk_all_gpio_cfg.pwm7, GPIO_TYPE_OUTPUT_PWM7_OUT);         
            break;
#endif				
        default:
            return -EINVAL;
            break;
        }
        
        pwm_duty.channel = pwm_channel->channel;
        pwm_duty.xon = curr_duty -1;//1-999
        pwm_duty.xoff= (1000-curr_duty)-1;
        gk_pwm_set_duty(&pwm_duty);
    }

    if(pwm_channel->chEnable)
        GH_PWM_set_Enable(pwm_channel->channel, 1); 

 //   up(&lock);
    return ret;
}
EXPORT_SYMBOL(gk_pwm_set_channel);


int gk_pwm_enable_channel(u8 channel, u16 duty)
{
    if(channel >= MAX_CHANNEL || duty > 1000)
    {
        return -EINVAL;
    }
    
    if(duty == 0)
     {
         switch(channel) //duty=1/1000
         {
         case 0:
             gk_gpio_config(gk_all_gpio_cfg.pwm0, GPIO_TYPE_OUTPUT_0);
             break;
         case 1:
             gk_gpio_config(gk_all_gpio_cfg.pwm1, GPIO_TYPE_OUTPUT_0);
             break;
         case 2:
             gk_gpio_config(gk_all_gpio_cfg.pwm2, GPIO_TYPE_OUTPUT_0);
             break;
         case 3:
             gk_gpio_config(gk_all_gpio_cfg.pwm3, GPIO_TYPE_OUTPUT_0);
             break;
#if !(defined(CONFIG_ARCH_GK710X) || defined(CONFIG_ARCH_GK710X_FPGA))			 
         case 4:
             gk_gpio_config(gk_all_gpio_cfg.pwm4, GPIO_TYPE_OUTPUT_0);
             break;
         case 5:
             gk_gpio_config(gk_all_gpio_cfg.pwm5, GPIO_TYPE_OUTPUT_0);
             break;
         case 6:
             gk_gpio_config(gk_all_gpio_cfg.pwm6, GPIO_TYPE_OUTPUT_0);
             break;
         case 7:
             gk_gpio_config(gk_all_gpio_cfg.pwm7, GPIO_TYPE_OUTPUT_0);
             break;
#endif			 
         default:
             return -EINVAL;
             break;
         }
     }
     else if(duty == 1000) //duty=1000/1000
     {
         switch(channel)
         {
         case 0:
             gk_gpio_config(gk_all_gpio_cfg.pwm0, GPIO_TYPE_OUTPUT_1);
             break;
         case 1:
             gk_gpio_config(gk_all_gpio_cfg.pwm1, GPIO_TYPE_OUTPUT_1);
             break;
         case 2:
             gk_gpio_config(gk_all_gpio_cfg.pwm2, GPIO_TYPE_OUTPUT_1);
             break;
         case 3:
             gk_gpio_config(gk_all_gpio_cfg.pwm3, GPIO_TYPE_OUTPUT_1);
             break;
#if !(defined(CONFIG_ARCH_GK710X) || defined(CONFIG_ARCH_GK710X_FPGA))				 
         case 4:
             gk_gpio_config(gk_all_gpio_cfg.pwm4, GPIO_TYPE_OUTPUT_1);
             break;
         case 5:
             gk_gpio_config(gk_all_gpio_cfg.pwm5, GPIO_TYPE_OUTPUT_1);
             break;
         case 6:
             gk_gpio_config(gk_all_gpio_cfg.pwm6, GPIO_TYPE_OUTPUT_1);
             break;
         case 7:
             gk_gpio_config(gk_all_gpio_cfg.pwm7, GPIO_TYPE_OUTPUT_1);
             break;
#endif			 
         default:
             return -EINVAL;
             break;
         }
     }

    GH_PWM_set_Enable(channel, 1);
    return 0;
}
EXPORT_SYMBOL(gk_pwm_enable_channel);

int gk_pwm_disable_channel(u8 channel)
{
    GH_PWM_set_Enable(channel, 0);
    return 0;
}
EXPORT_SYMBOL(gk_pwm_disable_channel);


int gk_pwm_close_channel(u8 channel)
{
    if(channel >= MAX_CHANNEL)
    {
        return -EINVAL;
    }

//    if (down_trylock(&lock))
//    {
//        return -EBUSY;
//    }

    GH_PWM_set_Enable(channel,0);

    // logout PWM function.
    switch(channel)
    {
    case 0:
        gk_gpio_config(gk_all_gpio_cfg.pwm0, GPIO_TYPE_UNDEFINED);
        break;
    case 1:
        gk_gpio_config(gk_all_gpio_cfg.pwm1, GPIO_TYPE_UNDEFINED);
        break;
    case 2:
        gk_gpio_config(gk_all_gpio_cfg.pwm2, GPIO_TYPE_UNDEFINED);
        break;
    case 3:
        gk_gpio_config(gk_all_gpio_cfg.pwm3, GPIO_TYPE_UNDEFINED);
        break;
#if !(defined(CONFIG_ARCH_GK710X) || defined(CONFIG_ARCH_GK710X_FPGA))			
    case 4:
        gk_gpio_config(gk_all_gpio_cfg.pwm4, GPIO_TYPE_UNDEFINED);
        break;
    case 5:
        gk_gpio_config(gk_all_gpio_cfg.pwm5, GPIO_TYPE_UNDEFINED);
        break;
    case 6:
        gk_gpio_config(gk_all_gpio_cfg.pwm6, GPIO_TYPE_UNDEFINED);
        break;
    case 7:
        gk_gpio_config(gk_all_gpio_cfg.pwm7, GPIO_TYPE_UNDEFINED);
        break;
#endif		
    default:
        return -EINVAL;
        break;
    }

//    up(&lock);

    return 0;
}
EXPORT_SYMBOL(gk_pwm_close_channel);


/* open file description of pwm */
static int gk_pwm_open(struct inode *inode, struct file *file)
{
    if (!down_trylock(&lock)) //�Ƿ����ź���,�� down_trylock(&lock)=0,����� 0
    {
        //configure PWM function.
#if defined(CONFIG_ARCH_GK710X) || defined(CONFIG_ARCH_GK710X_FPGA)
        gk_gpio_config(gk_all_gpio_cfg.pwm0, GPIO_TYPE_OUTPUT_PWM0_OUT);
        gk_gpio_config(gk_all_gpio_cfg.pwm1, GPIO_TYPE_OUTPUT_PWM1_OUT);
        gk_gpio_config(gk_all_gpio_cfg.pwm2, GPIO_TYPE_OUTPUT_PWM2_OUT);
        gk_gpio_config(gk_all_gpio_cfg.pwm3, GPIO_TYPE_OUTPUT_PWM3_OUT);
#else
        gk_gpio_config(gk_all_gpio_cfg.pwm0, GPIO_TYPE_OUTPUT_PWM0_OUT);
        gk_gpio_config(gk_all_gpio_cfg.pwm1, GPIO_TYPE_OUTPUT_PWM1_OUT);
        gk_gpio_config(gk_all_gpio_cfg.pwm2, GPIO_TYPE_OUTPUT_PWM2_OUT);
        gk_gpio_config(gk_all_gpio_cfg.pwm3, GPIO_TYPE_OUTPUT_PWM3_OUT);
        gk_gpio_config(gk_all_gpio_cfg.pwm4, GPIO_TYPE_OUTPUT_PWM4_OUT);
        gk_gpio_config(gk_all_gpio_cfg.pwm5, GPIO_TYPE_OUTPUT_PWM5_OUT);
        gk_gpio_config(gk_all_gpio_cfg.pwm6, GPIO_TYPE_OUTPUT_PWM6_OUT);
        gk_gpio_config(gk_all_gpio_cfg.pwm7, GPIO_TYPE_OUTPUT_PWM7_OUT);
#endif
        return 0;
    }
    else
        return -EBUSY; //���ش�����Ϣ:�������Դ������
}
/* close pwm */
static int gk_pwm_close(struct inode *inode, struct file *file)
{
#if defined(CONFIG_ARCH_GK710X) || defined(CONFIG_ARCH_GK710X_FPGA)
    GH_PWM_set_Enable(0,0);
    GH_PWM_set_Enable(1,0);
    GH_PWM_set_Enable(2,0);
    GH_PWM_set_Enable(3,0);
    // logout PWM function.
    gk_gpio_config(gk_all_gpio_cfg.pwm0, GPIO_TYPE_UNDEFINED);
    gk_gpio_config(gk_all_gpio_cfg.pwm1, GPIO_TYPE_UNDEFINED);
    gk_gpio_config(gk_all_gpio_cfg.pwm2, GPIO_TYPE_UNDEFINED);
    gk_gpio_config(gk_all_gpio_cfg.pwm3, GPIO_TYPE_UNDEFINED);
#else
    GH_PWM_set_Enable(0,0);
    GH_PWM_set_Enable(1,0);
    GH_PWM_set_Enable(2,0);
    GH_PWM_set_Enable(3,0);
    GH_PWM_set_Enable(4,0);
    GH_PWM_set_Enable(5,0);
    GH_PWM_set_Enable(6,0);
    GH_PWM_set_Enable(7,0);
    // logout PWM function.
    gk_gpio_config(gk_all_gpio_cfg.pwm0, GPIO_TYPE_UNDEFINED);
    gk_gpio_config(gk_all_gpio_cfg.pwm1, GPIO_TYPE_UNDEFINED);
    gk_gpio_config(gk_all_gpio_cfg.pwm2, GPIO_TYPE_UNDEFINED);
    gk_gpio_config(gk_all_gpio_cfg.pwm3, GPIO_TYPE_UNDEFINED);
    gk_gpio_config(gk_all_gpio_cfg.pwm4, GPIO_TYPE_UNDEFINED);
    gk_gpio_config(gk_all_gpio_cfg.pwm5, GPIO_TYPE_UNDEFINED);
    gk_gpio_config(gk_all_gpio_cfg.pwm6, GPIO_TYPE_UNDEFINED);
    gk_gpio_config(gk_all_gpio_cfg.pwm7, GPIO_TYPE_UNDEFINED);
#endif
    up(&lock); //�ͷ��ź��� lock

    return 0;
}

/* pwm ioctl */
static long gk_pwm_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
//    pwm_ctrl_t  pwm_ctrl;
    long error = 0;
    pwm_mode_t  pwm_mode;
    pwm_duty_t pwm_duty;
    pwm_speed_t pwm_speed;
    pwm_status_t pwm_status;
    switch (cmd)
    {
        case GK_PWM_IOCTL_DISABLE: //disabel channel
            if(arg >= MAX_CHANNEL) return -EINVAL;

            GH_PWM_set_Enable(arg, 0);
            break;

        case GK_PWM_IOCTL_ENABLE: //enabel channel
            if(arg >= MAX_CHANNEL) return -EINVAL;

            GH_PWM_set_Enable(arg, 1);
            break;


        case GK_PWM_IOCTL_SET_MODE: //config pwm mode.
            if (arg == 0)
                return -EINVAL;

            if (copy_from_user(&pwm_mode, (void __user *)arg, sizeof(pwm_mode_t)))
                return -EFAULT;

            //printk("set mode channel:%d mode:%d\n", pwm_mode.channel, pwm_mode.mode);
            GH_PWM_set_Mode(pwm_mode.channel, pwm_mode.mode);

            break;

        case GK_PWM_IOCTL_GET_MODE: //get pwm mode.
            if (arg == 0)
                return -EINVAL;

            if (copy_from_user(&pwm_mode, (void __user *)arg, sizeof(pwm_mode_t)))
                return -EFAULT;

            pwm_mode.mode = GH_PWM_get_Mode(pwm_mode.channel);

            if (copy_to_user((void __user *)arg, &pwm_mode, sizeof(pwm_mode_t)))
                return -EFAULT;

            break;

        case GK_PWM_IOCTL_SET_SPEED://set pwm speed.
            if (arg == 0)
                return -EINVAL;

            if (copy_from_user(&pwm_speed, (void __user *)arg, sizeof(pwm_speed_t)))
                return -EFAULT;

            //printk("set speed channel:%d speed:%d\n", pwm_speed.channel, pwm_speed.speed);
            return gk_pwm_set_speed(pwm_speed.channel, pwm_speed.speed);

            break;

        case GK_PWM_IOCTL_GET_SPEED://get pwm speed.
            if (arg == 0)
                return -EINVAL;

            if (copy_from_user(&pwm_speed, (void __user *)arg, sizeof(pwm_speed_t)))
                return -EFAULT;

            error = gk_pwm_get_speed(pwm_speed.channel, &pwm_speed.speed);
            if(error != 0)
                return error;

            if (copy_to_user((void __user *)arg, &pwm_speed, sizeof(pwm_speed_t)))
                return -EFAULT;

            break;

        case GK_PWM_IOCTL_SET_DUTY://set pwm duty.
            if (arg == 0)
                return -EINVAL; //���ش�����Ϣ,��ʾ�������������Ч�Ĳ���

            if (copy_from_user(&pwm_duty, (void __user *)arg, sizeof(pwm_duty_t)))
                return -EFAULT;
            gk_pwm_set_duty(&pwm_duty);

            break;

        case GK_PWM_IOCTL_GET_DUTY://get pwm duty.
            if (arg == 0)
                return -EINVAL; //���ش�����Ϣ,��ʾ�������������Ч�Ĳ���

            if (copy_from_user(&pwm_duty, (void __user *)arg, sizeof(pwm_duty_t)))
                return -EFAULT;

            gk_pwm_get_duty(&pwm_duty);

            if (copy_to_user((void __user *)arg, &pwm_duty, sizeof(pwm_duty_t)))
                return -EFAULT;

            break;

        case GK_PWM_IOCTL_GET_STATUS:
            if (arg == 0)
                return -EINVAL;

            if (copy_from_user(&pwm_status, (void __user *)arg, sizeof(pwm_status_t)))
                return -EFAULT;

            pwm_status.enable = (GH_PWM_get_Enable(pwm_status.channel) > 0) ? 1 : 0;

            if (copy_to_user((void __user *)arg, &pwm_status, sizeof(pwm_status_t)))
                return -EFAULT;

            break;

        default:
            printk(KERN_ERR "\npwm:bad params\n");
            return -EINVAL;
            break;
    }
    return 0; //normal return
}


/*��ʼ���豸���ļ������Ľṹ��*/
static struct file_operations dev_fops = {
    .owner      = THIS_MODULE,
    .open       = gk_pwm_open,
    .release    = gk_pwm_close,
    .unlocked_ioctl      = gk_pwm_ioctl,
};

static struct miscdevice misc = {
    .minor  = 10, /*MISC_DYNAMIC_MINOR,*/
    .name   = DEVICE_NAME,
    .nodename = NODE_NAME,
    .fops   = &dev_fops,
};



static int __init dev_init(void)
{
    int ret;
    sema_init(&lock, 1); //��ʼ��һ��������
    ret = misc_register(&misc); //ע��һ�� misc �豸
    printk (DEVICE_NAME" initialized (10:10)\n");
    return ret;
}

static void __exit dev_exit(void)
{
    misc_deregister(&misc); //ע���豸
}

module_init(dev_init);
module_exit(dev_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Goke Microelectronics Inc.");
MODULE_DESCRIPTION("GK PWM Driver");
