/*
 * Copyright (C) 2017 Goke Corporation
 *
 * Backlight driver using goke PWM peripheral.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/fb.h>
#include <linux/clk.h>
#include <linux/gpio.h>
#include <linux/backlight.h>
#include <linux/gk-pwm-bl.h>
#include <linux/slab.h>

#include <mach/pwm.h>

struct gk_pwm_bl {
    const struct gk_pwm_bl_platform_data  *pdata;
    struct backlight_device        *bldev;
    struct platform_device         *pdev;
    int                             current_intensity;
};

static int gk_pwm_bl_set_intensity(struct backlight_device *bd)
{
    struct gk_pwm_bl *pwmbl = bl_get_data(bd);
    int intensity = bd->props.brightness;
    int pwm_duty;
    pwm_channel_t pwm_ch;
    int retval;

    if (bd->props.power != FB_BLANK_UNBLANK)
        intensity = 0;
    if (bd->props.fb_blank != FB_BLANK_UNBLANK)
        intensity = 0;

    pwm_duty = intensity;//bd->props.brightness;
    if (pwm_duty > pwmbl->pdata->pwm_duty_max)
        pwm_duty = pwmbl->pdata->pwm_duty_max;
    if (pwm_duty < pwmbl->pdata->pwm_duty_min)
        pwm_duty = pwmbl->pdata->pwm_duty_min;

    if (pwmbl->current_intensity != bd->props.brightness) {
        pwm_ch.channel = pwmbl->pdata->pwm_channel;
        pwm_ch.chEnable= 1;
        pwm_ch.freqHz  = pwmbl->pdata->pwm_frequency;
        pwm_ch.duty    = pwm_duty;
        pwm_ch.mode    = 0;
        retval = gk_pwm_set_channel(&pwm_ch);
        if(retval < 0) {
            return retval;
        }

        pwmbl->current_intensity = bd->props.brightness;
    }

//	if (bd->props.fb_blank != pwmbl->powermode)
//		omapbl_set_power(bd, bd->props.fb_blank);
    return 0;
}

static int gk_pwm_bl_get_intensity(struct backlight_device *bd)
{
    struct gk_pwm_bl *pwmbl = bl_get_data(bd);
    return pwmbl->current_intensity;

}


static const struct backlight_ops gk_pwm_bl_ops = {
    .get_brightness = gk_pwm_bl_get_intensity,
    .update_status  = gk_pwm_bl_set_intensity,
};

static int gk_pwm_bl_probe(struct platform_device *pdev)
{
    struct backlight_properties props;
    const struct gk_pwm_bl_platform_data *pdata;
    struct backlight_device *bldev;
    struct gk_pwm_bl *pwmbl;
    int retval;

    pwmbl = kzalloc(sizeof(struct gk_pwm_bl), GFP_KERNEL);
    if (!pwmbl)
        return -ENOMEM;

    pwmbl->pdev = pdev;

    pdata = pdev->dev.platform_data;
    if (!pdata) {
        retval = -ENODEV;
        goto gk_err_free_mem;
    }

    if (pdata->pwm_duty_min > pdata->pwm_duty_max ||
            pdata->pwm_frequency == 0) {
        retval = -EINVAL;
        goto gk_err_free_mem;
    }

    pwmbl->pdata = pdata;

    memset(&props, 0, sizeof(struct backlight_properties));
    props.type = BACKLIGHT_RAW;
    props.max_brightness = pdata->pwm_duty_max - pdata->pwm_duty_min;
    bldev = backlight_device_register("goke-pwm-bl", &pdev->dev, pwmbl,
                      &gk_pwm_bl_ops, &props);
    if (IS_ERR(bldev)) {
        retval = PTR_ERR(bldev);
        goto gk_err_free_mem;
    }

    pwmbl->bldev = bldev;

    platform_set_drvdata(pdev, pwmbl);

    /* Power up the backlight by default at middle intesity. */
    bldev->props.power = FB_BLANK_UNBLANK;
    bldev->props.brightness = 0;//bldev->props.max_brightness / 2;//half of max_brightness
    gk_pwm_bl_set_intensity(bldev);

    return 0;

    platform_set_drvdata(pdev, NULL);
    backlight_device_unregister(bldev);
gk_err_free_mem:
    kfree(pwmbl);
    return retval;
}

static int __exit gk_pwm_bl_remove(struct platform_device *pdev)
{
    struct gk_pwm_bl *pwmbl = platform_get_drvdata(pdev);

    backlight_device_unregister(pwmbl->bldev);
    platform_set_drvdata(pdev, NULL);
    kfree(pwmbl);

    return 0;
}

static struct platform_driver gk_pwm_bl_driver = {
    .driver = {
        .name = "goke-pwm-bl",
    },
    /* REVISIT add suspend() and resume() */
    .remove = __exit_p(gk_pwm_bl_remove),
};

static int __init gk_pwm_bl_init(void)
{
    return platform_driver_probe(&gk_pwm_bl_driver, gk_pwm_bl_probe);
}
module_init(gk_pwm_bl_init);

static void __exit gk_pwm_bl_exit(void)
{
    platform_driver_unregister(&gk_pwm_bl_driver);
}
module_exit(gk_pwm_bl_exit);

MODULE_AUTHOR("Bao Chao <baochao@gokemicro.com>");
MODULE_DESCRIPTION("Goke PWM backlight driver");
MODULE_LICENSE("GPL");
