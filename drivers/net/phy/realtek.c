/*
 * drivers/net/phy/realtek.c
 *
 * Driver for Realtek PHYs
 *
 * Author: Johnson Leung <r58129@freescale.com>
 *
 * Copyright (c) 2004 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */
#include <linux/phy.h>
#include <linux/module.h>

#define REALTEK_PHY_ID_RTL8201EL    0x001cc815
#define REALTEK_PHY_ID_RTL8211B     0x001cc912

#define RTL821x_PHYSR		0x11
#define RTL821x_PHYSR_DUPLEX	0x2000
#define RTL821x_PHYSR_SPEED	0xc000
#define RTL821x_INER		0x12
#define RTL821x_INER_INIT	0x6400
#define RTL821x_INSR		0x13

MODULE_DESCRIPTION("Realtek PHY driver");
MODULE_AUTHOR("Johnson Leung");
MODULE_LICENSE("GPL");

static int rtl821x_ack_interrupt(struct phy_device *phydev)
{
	int err;

	err = phy_read(phydev, RTL821x_INSR);

	return (err < 0) ? err : 0;
}

static int rtl821x_config_intr(struct phy_device *phydev)
{
	int err;

	if (phydev->interrupts == PHY_INTERRUPT_ENABLED)
		err = phy_write(phydev, RTL821x_INER,
				RTL821x_INER_INIT);
	else
		err = phy_write(phydev, RTL821x_INER, 0);

	return err;
}

static int rtl820x_config_init(struct phy_device *phydev)
{
	int val;
	int val_need;
	u32 features;

    val = phy_read(phydev, 0x19);
    if (val < 0)
        return val;
    do
    {
        if(phydev->interface == PHY_INTERFACE_MODE_RMII)
        {
            val |= (1<<10);
            val &= ~(1<<11);
        }
        else
        {
            val &= ~(1<<10);
            val &= ~(1<<11);
        }
        phy_write(phydev, 0x19, val);
        val_need = val;
        val = phy_read(phydev, 0x19);
    	if (val < 0)
    		return val;
    }while(val_need != val);

	/* For now, I'll claim that the generic driver supports
	 * all possible port types */
	features = (SUPPORTED_TP | SUPPORTED_MII);

	/* Do we support autonegotiation? */
	val = phy_read(phydev, MII_BMCR);

	if (val < 0)
		return val;
	if (val & BMCR_ANENABLE)
		features |= SUPPORTED_Autoneg;

	val = phy_read(phydev, MII_BMSR);

	if (val < 0)
		return val;

	if (val & BMSR_100FULL)
		features |= SUPPORTED_100baseT_Full;
	if (val & BMSR_100HALF)
		features |= SUPPORTED_100baseT_Half;
	if (val & BMSR_10FULL)
		features |= SUPPORTED_10baseT_Full;
	if (val & BMSR_10HALF)
		features |= SUPPORTED_10baseT_Half;

	phydev->supported = features;
	phydev->advertising = features;

	return 0;
}

static struct phy_driver realtek_driver[] = {
    /* RTL8201EL */
	{
        .phy_id		= REALTEK_PHY_ID_RTL8201EL,
    	.name		= "REALTEK rtl8201e(l)",
    	.phy_id_mask	= 0x001fffff,
    	.features	= PHY_BASIC_FEATURES,
    	.flags		= PHY_POLL,
    	.config_init	= &rtl820x_config_init,
        .config_aneg    = &genphy_config_aneg,
    	.read_status	= &genphy_read_status,
    	.driver		= { .owner = THIS_MODULE,},
    },
    /* RTL8211B */
	{
        .phy_id		= REALTEK_PHY_ID_RTL8211B,
    	.name		= "REALTEK Gigabit Ethernet",
    	.phy_id_mask	= 0x001fffff,
    	.features	= PHY_GBIT_FEATURES,
    	.flags		= PHY_HAS_INTERRUPT,
    	.config_aneg	= &genphy_config_aneg,
    	.read_status	= &genphy_read_status,
    	.ack_interrupt	= &rtl821x_ack_interrupt,
    	.config_intr	= &rtl821x_config_intr,
    	.driver		= { .owner = THIS_MODULE,},
    },
};

static int __init realtek_init(void)
{
	int ret;
	int i;

	for (i = 0; i < ARRAY_SIZE(realtek_driver); i++) {
		ret = phy_driver_register(&realtek_driver[i]);

		if (ret) {
			while (i-- > 0)
				phy_driver_unregister(&realtek_driver[i]);
			return ret;
		}
	}

	return ret;
}

static void __exit realtek_exit(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(realtek_driver); i++)
		phy_driver_unregister(&realtek_driver[i]);
}

module_init(realtek_init);
module_exit(realtek_exit);

static struct mdio_device_id __maybe_unused realtek_tbl[] = {
	{ REALTEK_PHY_ID_RTL8201EL, 0x001fffff },
    { REALTEK_PHY_ID_RTL8211B,  0x001fffff },
	{ }
};

MODULE_DEVICE_TABLE(mdio, realtek_tbl);
