#ifndef __MACH_GK_PWM_H
#define __MACH_GK_PWM_H

#include <mach/hardware.h>
#include CONFIG_GK_CHIP_INCLUDE_FILE

#ifndef __ASSEMBLER__

typedef struct{
    u8  channel;
    u8  chEnable;
    u32 freqHz;
    u16 duty;
    u8  mode;    
}pwm_channel_t;

extern int gk_pwm_close_channel(u8 channel);
extern int gk_pwm_enable_channel(u8 channel, u16 duty);
extern int gk_pwm_disable_channel(u8 channel);
extern int gk_pwm_set_channel(pwm_channel_t *pwm_channel);

#endif /* __ASSEMBLER__ */
/* ==========================================================================*/

#endif