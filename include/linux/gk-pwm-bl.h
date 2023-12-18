/*
 * Copyright (C) 2017 Goke Corporation
 *
 * Driver for the backlight
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#ifndef __INCLUDE_GK_PWM_BL_H
#define __INCLUDE_GK_PWM_BL_H

/**
 * struct gk_pwm_bl_platform_data
 * @pwm_channel: which PWM channel in the PWM module to use.
 * @pwm_frequency: PWM frequency to generate, the driver will try to be as
 *	close as the prescaler allows.
 * @pwm_duty_max: maximum duty cycle value, must be less than or equal to
 *	pwm_compare_max.
 * @pwm_duty_min: minimum duty cycle value, must be less than pwm_duty_max.
 * This struct must be added to the platform device in the board code. It is
 * used by the goke-pwm-bl driver to setup the GPIO to control on/off and the
 * PWM device.
 */
struct gk_pwm_bl_platform_data {
	unsigned int pwm_channel;
	unsigned int pwm_frequency;
	unsigned int pwm_duty_max;
	unsigned int pwm_duty_min;
    unsigned int pwm_mode;
};

#endif /* __INCLUDE_GK_PWM_BL_H */
