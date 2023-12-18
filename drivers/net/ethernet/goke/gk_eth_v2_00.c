/*
 * drivers/net/arm/gk_eth.c
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
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>
#include <linux/crc32.h>
#include <linux/time.h>
#include <linux/mii.h>
#include <linux/phy.h>
#include <linux/ethtool.h>
#include <linux/ip.h>
#include <linux/ipv6.h>

#include <asm/dma.h>

#include <mach/hardware.h>
#include <mach/io.h>
#include <mach/eth.h>
#include <mach/gpio.h>
#if defined(CONFIG_SYSTEM_USE_NETLINK_MSG_ETH)
#include <plat/gk_net_link.h>
#endif

#define GK_SUPPORT_TSO

/* ================================================================= */
//#define GKETH_MAX_TX_MSS    (1518)
#define GKETH_RX_COPYBREAK      (1518)
#define GKETH_PACKET_MAXFRAME   (1536)//0x600    //7969
#define GKETH_TCP_MSS           (1460)
//#define GK_MAC_LEN              (14)
#define GK_IPV6_LEN             (40)

#define GK_ETH_INTEN  (ETH_REG_INT_EN_RXOK | ETH_REG_INT_EN_RXER | ETH_REG_INT_EN_TXOK | \
    ETH_REG_INT_EN_TXER | ETH_REG_INT_EN_RXDU | ETH_REG_INT_EN_LKCHG | ETH_REG_INT_EN_RXFIFOOV | \
    ETH_REG_INT_EN_TXDU | ETH_REG_INT_EN_SWINT | ETH_REG_INT_EN_TMOUT)

#define GK_ETH_INT_STA  (ETH_REG_INT_STA_RXOK | ETH_REG_INT_STA_RXER | ETH_REG_INT_STA_TXOK | \
    ETH_REG_INT_STA_TXER | ETH_REG_INT_STA_RXDU | ETH_REG_INT_STA_LKCHG | ETH_REG_INT_STA_RXFIFOOV | \
    ETH_REG_INT_STA_TXDU | ETH_REG_INT_STA_SFINT | ETH_REG_INT_STA_RXDLLK | ETH_REG_INT_STA_EPHYDLINK | \
    ETH_REG_INT_STA_EPHYERR | ETH_REG_INT_STA_TIMEOUT | ETH_REG_INT_STA_SYSERR)

#define GK_ETH_TX_INT_STA  (ETH_REG_INT_STA_TXOK | ETH_REG_INT_STA_TXER | ETH_REG_INT_STA_TXDU)
#define GK_ETH_RX_INT_STA  (ETH_REG_INT_STA_RXOK | ETH_REG_INT_STA_RXER | ETH_REG_INT_STA_RXDU | \
    ETH_REG_INT_STA_RXFIFOOV | ETH_REG_INT_STA_RXDLLK)

#define PRINTK 0
/* ================================================================= */
struct GKETH_desc{
    u32     desc0;
    u32     desc1;
    u32     bufferl;
    u32     bufferh;
} __attribute((packed));

struct GKETH_rng_info {
    struct sk_buff          *skb;
    dma_addr_t              mapping;
    unsigned long           buf_size;
	unsigned int    		lastflag;
};

struct GKETH_tx_rngmng {
    unsigned int            cur_tx;
    unsigned int            dirty_tx;
    struct GKETH_rng_info   *rng_tx;
    struct GKETH_desc       *desc_tx;
};

struct GKETH_rx_rngmng {
    unsigned int            cur_rx;
    unsigned int            dirty_rx;
    struct GKETH_rng_info   *rng_rx;
    struct GKETH_desc       *desc_rx;
	unsigned short			intstatus;
};

struct GKETH_tally_info {
    u32        mapping;
    u32        mapping_size;
    u32        buffer;
    u32        buf_size;
};

struct GKETH_info {
    unsigned int            rx_count;
    struct GKETH_rx_rngmng  rx;
    unsigned int            tx_count;
    unsigned int            tx_irq_count;
    struct GKETH_tx_rngmng  tx;
    dma_addr_t              rx_dma_desc;
    dma_addr_t              tx_dma_desc;
    struct GKETH_tally_info tally;
    spinlock_t              lock;
    int                     oldspeed;
    int                     oldduplex;
    int                     oldlink;

    struct net_device_stats stats;
    struct napi_struct      napi;
    struct net_device       *ndev;
    struct mii_bus          new_bus;
    struct phy_device       *phydev;
    uint32_t                msg_enable;

    unsigned char __iomem   *regbase;
    struct gk_eth_platform_info *platform_info;
};
/* ================================================================= */
extern uint8_t cmdline_phytype;
static int msg_level = -1;
module_param (msg_level, int, 0);
MODULE_PARM_DESC (msg_level, "Override default message level");

typedef union { /* EPHY_SPEED */
    u16 all;
    struct {
        u16 ltp_f                       : 8;
        u16 isolate                     : 1;
        u16 rptr                        : 1;
        u16 duplex                      : 1;
        u16 speed                       : 1;
        u16 ane                         : 1;
        u16 ldps                        : 1;
        u16 disable_eee_force           : 1;
        u16                             : 1;
    } bitc;
} GH_EPHY_SPEED_S;

static void GK_EPHY_POWER_OFF(void)
{
    u16 dat;

    dat = gk_eth_readl(REG_EPHY_POWER);
    gk_eth_writel(REG_EPHY_POWER, 0x03ff);
}

static void GK_EPHY_POWER_ON(void)
{
    u16 dat;

    dat = gk_eth_readl(REG_EPHY_POWER);
    gk_eth_writel(REG_EPHY_POWER, 0);
}

static void GH_EPHY_set_SPEED_ane(u8 data)
{
    GH_EPHY_SPEED_S d;
	d.all = gk_eth_readl(REG_EPHY_SPEED);
    d.bitc.ane = data;
	gk_eth_writel(REG_EPHY_SPEED, d.all);
}

typedef union { /* EPHY_MII_RMII */
    u32 all;
    struct {
        u32 usb_tm1 : 1;
        u32 rmii    : 1;
        u32         : 30;
    } bitc;
} GH_EPHY_MII_RMII_S;
#define REG_EPHY_MII_RMII  GK_VA_AHB_GREG

void GH_EPHY_set_MII_RMII_rmii(u8 data)
{
    GH_EPHY_MII_RMII_S d;
	d.all = gk_eth_readl(REG_EPHY_MII_RMII);
    d.bitc.rmii = data;
	gk_eth_writel(REG_EPHY_MII_RMII, d.all);
}

u8 GH_EPHY_get_MII_RMII_rmii(void)
{
    GH_EPHY_MII_RMII_S d;
	d.all = gk_eth_readl(REG_EPHY_MII_RMII);
    return d.bitc.rmii;
}

static void MHal_EMAC_WritReg8( u32 bank, u32 reg, u8 val )
{
    u32 address = REG_EPHY_CONTROL + (bank-0x32)*0x100*2;
    address = address + (reg << 1) - (reg & 1);
	gk_eth_writeb(address, val);
}

static u8 MHal_EMAC_ReadReg8( u32 bank, u32 reg )
{
    u8 val;
    u32 address = REG_EPHY_CONTROL + (bank-0x32)*0x100*2;
    address = address + (reg << 1) - (reg & 1);
	val = gk_eth_readb(address);
    return val;
}

static inline phy_interface_t gkhw_get_interface(struct GKETH_info *lp)
{
    return GH_EPHY_get_MII_RMII_rmii() ? PHY_INTERFACE_MODE_RMII :
        PHY_INTERFACE_MODE_MII;
}

static int GKETH_read_eri(struct GKETH_info *lp,int addr)
{
    int  val = 0;
    int limit = 100;
    
    val = ETH_REG_ERI_AR_RD & (ETH_REG_ERI_AR_BYT_EN(0xF) | ETH_REG_ERI_AR_ADDR(addr));
    gk_eth_writel((unsigned int)(lp->regbase + ETH_REG_ERI_AR_OFFSET), val);
    
    for (; limit > 0; limit--) {
        if (gk_eth_tstbitsl((unsigned int)(lp->regbase + ETH_REG_ERI_AR_OFFSET),
            ETH_REG_ERI_AR_WR))
            break;
        udelay(100);
    }
    if (limit <= 0) {
        dev_err(&lp->ndev->dev, "eri read Error: Postread tmo!\n");
        val = 0;
        goto gk_eri_read_exit;
    }
    
    val = gk_eth_readl((unsigned int)(lp->regbase + ETH_REG_ERI_DR_OFFSET));
    
gk_eri_read_exit:

    return val;
}

static void GKETH_write_eri(struct GKETH_info *lp, int addr, int data)
{
    int val   = 0;
    int limit = 100;
    
    val = data;
    gk_eth_writel((unsigned int)(lp->regbase + ETH_REG_ERI_DR_OFFSET), val);
    
    val = ETH_REG_ERI_AR_WR | ETH_REG_ERI_AR_BYT_EN(0xF) | ETH_REG_ERI_AR_ADDR(addr);
    gk_eth_writel((unsigned int)(lp->regbase + ETH_REG_ERI_AR_OFFSET), val);
    
    for (; limit > 0; limit--) {
        if (!gk_eth_tstbitsl((unsigned int)(lp->regbase + ETH_REG_ERI_AR_OFFSET),
            ETH_REG_ERI_AR_WR))
            break;
        udelay(100);
    }
    if (limit <= 0) {
        dev_err(&lp->ndev->dev, "eri read Error: Postread tmo!\n");
    }
}

static void GKETH_write_ocp(struct GKETH_info *lp, u16 addr, int data)
{
    int val;
    
    val = ETH_REG_PATCH_DBG_WR | ETH_REG_PATCH_DBG_ADDR(addr) | ETH_REG_PATCH_DBG_DATA(data);
    gk_eth_writel((unsigned int)(lp->regbase + ETH_REG_PATCH_DBG_OFFSET), val);
}

static int GKETH_read_ocp(struct GKETH_info *lp, int addr)
{
    int val;
    
    val = ETH_REG_PATCH_DBG_RD & (ETH_REG_PATCH_DBG_ADDR(addr));
    gk_eth_writel((unsigned int)(lp->regbase + ETH_REG_PATCH_DBG_OFFSET), val);
    
    val = gk_eth_readl((unsigned int)(lp->regbase + ETH_REG_PATCH_DBG_OFFSET));

    return ETH_REG_PATCH_DBG_DATA(val);
}

static inline void GKETH_pre_tally(struct GKETH_info *lp)
{
    gk_eth_writel((unsigned int)(lp->regbase + ETH_REG_DTCCR_LO_OFFSET), \
        (lp->tally.buffer & 0xFFFFFFC0));
}
#if 0
static void GKETH_write_tally(struct GKETH_info *lp, int addr)
{
    int val;

    val = ETH_REG_DTCCR_LO_ADDR(addr);
    gk_eth_writel((unsigned int)(lp->regbase + ETH_REG_DTCCR_LO_OFFSET), val);
    gk_eth_writel((unsigned int)(lp->regbase + ETH_REG_DTCCR_HI_OFFSET), 0);
}

static void GKETH_read_tally(struct GKETH_info *lp)
{
    int val;
    val = gk_eth_readl((unsigned int)(lp->regbase + ETH_REG_DTCCR_LO_OFFSET));
    val = val | ETH_REG_DTCCR_CMD;
    gk_eth_writel((unsigned int)(lp->regbase + ETH_REG_DTCCR_LO_OFFSET), val);
}

static void GKETH_clean_tally(struct GKETH_info *lp)
{
    int val;
    
    val = gk_eth_readl((unsigned int)(lp->regbase + ETH_REG_DTCCR_LO_OFFSET));
    val = val| ETH_REG_DTCCR_TCCLR;
    gk_eth_writel((unsigned int)(lp->regbase + ETH_REG_DTCCR_LO_OFFSET), val);
}

static int GKETN_dump_tally(struct GKETH_info *lp, int addr)
{
    int val;
    u32 Taddr;
    int limit = 100;

    val = gk_eth_readl((unsigned int)(lp->regbase + ETH_REG_DTCCR_LO_OFFSET));
    Taddr = ETH_REG_DTCCR_LO_ADDR(val);
    val = val | ETH_REG_DTCCR_CMD;
    gk_eth_writel((unsigned int)(lp->regbase + ETH_REG_DTCCR_LO_OFFSET), val);

    for (; limit > 0; limit--) {
        if (!gk_eth_tstbitsl((unsigned int)(lp->regbase + ETH_REG_DTCCR_LO_OFFSET),
            ETH_REG_DTCCR_CMD))
            break;
        udelay(100);
    }
    if (limit <= 0) {
        dev_err(&lp->ndev->dev, "eri read Error: Postread tmo!\n");
        return 0;
    }

    return *((volatile int *)(Taddr + addr));
}
#endif
/* ================================================================= */
int gkhw_check_tx_busy(struct GKETH_info *lp, int tx_desc_num)
{
    u32 desc_w_start;
    u32 desc_w_end;
    int dirty_tx;
    
    desc_w_start = lp->tx.cur_tx % lp->tx_count;
    dirty_tx = lp->tx.dirty_tx;
    desc_w_end = dirty_tx % lp->tx_count;
    if(desc_w_end <= desc_w_start)
    {
        desc_w_end += lp->tx_count;
    }
    if((desc_w_end - desc_w_start) < (tx_desc_num ))
    {
        return NETDEV_TX_BUSY;
    }
    else
    {
        return NETDEV_TX_OK;
    }
}
int gkhw_tx_free_check(struct GKETH_info *lp, u32 entry)
{
    u32 fastentry;
    u32 status;
     //   printk("%s\n",__FUNCTION__);
    
    fastentry = (entry + lp->tx.rng_tx[entry].lastflag) % lp->tx_count;
    status = lp->tx.desc_tx[fastentry].desc0;
    
    if(status & ETH_TDES0_OWN)
    {
        return NETDEV_TX_BUSY;
    }
    return NETDEV_TX_OK;
}

int gkhw_n_desc(struct sk_buff *skb, struct GKETH_info *lp)
{
    u32 tx_flag0 = 0;
    u32 tx_flag1 = 0;
    dma_addr_t              mapping;
    u32 entry;
    struct iphdr *iph;
    u8 proto = 0;
    
    if(gkhw_check_tx_busy(lp, skb_shinfo(skb)->nr_frags + 1))
        return NETDEV_TX_BUSY;

    entry = lp->tx.cur_tx % lp->tx_count;
    mapping = dma_map_single(&lp->ndev->dev,
        skb->data, skb->len, DMA_TO_DEVICE);
    
    lp->tx.rng_tx[entry].skb = skb;
    lp->tx.rng_tx[entry].buf_size = skb->len;
    lp->tx.rng_tx[entry].mapping = mapping;
    lp->tx.desc_tx[entry].bufferl = mapping;
	lp->tx.rng_tx[entry].lastflag = 0;
    
    tx_flag0 = ETH_TDES0_FS | ETH_TDES0_LS;
	lp->tx.rng_tx[entry].lastflag = skb_shinfo(skb)->nr_frags;
    if (skb->protocol == __constant_htons(ETH_P_IP))
    {
        proto = ip_hdr(skb)->protocol;
        if(proto == IPPROTO_UDP)
        {
            tx_flag1 = ETH_TDES1_N_UDPCKS | ETH_TDES1_N_IPV4CS;
        }
        else if(proto == IPPROTO_TCP)
        {
            iph = ip_hdr(skb);
            tx_flag1 = ETH_TDES1_N_TCPCKS | ETH_TDES1_N_IPV4CS | \
            ETH_TDES1_N_TCPHO((iph->ihl * 4) + ETH_HLEN);
        }
    }
    else if (skb->protocol == __constant_htons(ETH_P_IPV6)) 
    {
        proto = ipv6_hdr(skb)->nexthdr;
        if(proto == IPPROTO_UDP)
        {
            tx_flag1 = ETH_TDES1_N_UDPCKS | ETH_TDES1_N_IPV6FG;
        }
        else if(proto == IPPROTO_TCP)
        {
            tx_flag1 = ETH_TDES1_N_TCPCKS | ETH_TDES1_N_IPV6FG | \
            ETH_TDES1_N_TCPHO(GK_IPV6_LEN + ETH_HLEN);
        }
    }
    else
    {
        tx_flag1 = 0;
    }
    lp->tx.desc_tx[entry].desc0 = ETH_TDES0_N_FML(skb->len) | tx_flag0;
    lp->tx.desc_tx[entry].desc1 = tx_flag1;
    if(entry == lp->tx_count - 1)
        lp->tx.desc_tx[entry].desc0 |= ETH_TDES0_EOR;
    lp->tx.desc_tx[entry].desc0 |= ETH_TDES0_OWN;
    lp->tx.cur_tx++;
    return NETDEV_TX_OK;
}

int gkhw_l_desc(struct sk_buff *skb, struct GKETH_info *lp)
{
    u32 tx_flag0 = 0;
    u32 tx_flag1 = 0;
    dma_addr_t              mapping;
    u32 entry;
    const skb_frag_t *frag;
    u8  i;
    
    if(gkhw_check_tx_busy(lp, skb_shinfo(skb)->nr_frags + 1))
        return NETDEV_TX_BUSY;
    
    entry = lp->tx.cur_tx % lp->tx_count;
	lp->tx.rng_tx[entry].lastflag = 0;
		
	if(skb_shinfo(skb)->nr_frags == 0)
	{
    	mapping = dma_map_single(&lp->ndev->dev,
        	skb->data, skb->len, DMA_TO_DEVICE);
    	lp->tx.rng_tx[entry].buf_size  = skb->len;
	}
	else		
    {
    	mapping = dma_map_single(&lp->ndev->dev,
        	skb->data, (skb->len - skb->data_len), DMA_TO_DEVICE);
    	lp->tx.rng_tx[entry].buf_size  = (skb->len - skb->data_len);
	}
    lp->tx.rng_tx[entry].skb = skb;
    lp->tx.rng_tx[entry].mapping = mapping;
    lp->tx.desc_tx[entry].bufferl = mapping;
    tx_flag0 = ETH_TDES0_FS | ETH_TDES0_LGSEN;
	if(skb_shinfo(skb)->nr_frags == 0)
	{
    	tx_flag0 |= ETH_TDES0_LS;
	}
    lp->tx.rng_tx[entry].lastflag = skb_shinfo(skb)->nr_frags;
    tx_flag0 |= ETH_TDES0_L_FML(lp->tx.rng_tx[entry].buf_size);
    tx_flag1 = ETH_TDES1_L_MSS(GKETH_TCP_MSS);
    
    lp->tx.desc_tx[entry].desc0 = tx_flag0;
    lp->tx.desc_tx[entry].desc1 = tx_flag1;
    if(entry == lp->tx_count - 1)
        lp->tx.desc_tx[entry].desc0 |= ETH_TDES0_EOR;
    lp->tx.desc_tx[entry].desc0 |= ETH_TDES0_OWN;
    lp->tx.cur_tx++;
    for(i=0; i < skb_shinfo(skb)->nr_frags; i++) 
	{
	    frag = &skb_shinfo(skb)->frags[i];
        entry = lp->tx.cur_tx % lp->tx_count;
		lp->tx.rng_tx[entry].lastflag = 0;
        
        mapping = skb_frag_dma_map(&lp->ndev->dev, frag, 0,
			   skb_frag_size(frag),DMA_TO_DEVICE);
            
        lp->tx.rng_tx[entry].skb = skb;
    	lp->tx.rng_tx[entry].buf_size = skb_frag_size(frag);
        lp->tx.rng_tx[entry].mapping = mapping;
        lp->tx.desc_tx[entry].bufferl = mapping;
        tx_flag0 = 0;
        if(i == (skb_shinfo(skb)->nr_frags - 1))
        {
            tx_flag0 |= ETH_TDES0_LS;
        }
        lp->tx.rng_tx[entry].lastflag = (skb_shinfo(skb)->nr_frags - 1 - i);
		tx_flag0 |= ETH_TDES0_LGSEN;
        tx_flag1 = ETH_TDES1_L_MSS(GKETH_TCP_MSS);
    
        lp->tx.desc_tx[entry].desc0 = ETH_TDES0_L_FML(lp->tx.rng_tx[entry].buf_size) | tx_flag0;
        lp->tx.desc_tx[entry].desc1 = tx_flag1;
        if(entry == lp->tx_count - 1)
            lp->tx.desc_tx[entry].desc0 |= ETH_TDES0_EOR;
        lp->tx.desc_tx[entry].desc0 |= ETH_TDES0_OWN;
        lp->tx.cur_tx++;
    }
    return NETDEV_TX_OK;
}
#if 0
int gkhw_g_desc(struct sk_buff *skb, struct GKETH_info *lp)
{
    u32 tx_flag0 = 0;
    u32 tx_flag1 = 0;
    dma_addr_t              mapping;
    u32 entry;
    const skb_frag_t *frag;
    u8  i;
    
    if(gkhw_check_tx_busy(lp, skb_shinfo(skb)->nr_frags))
        return NETDEV_TX_BUSY;
    
    entry = lp->tx.cur_tx % lp->tx_count;
	lp->tx.rng_tx[entry].lastflag = 0;
		
	if(skb_shinfo(skb)->nr_frags == 0)
	{
    	mapping = dma_map_single(&lp->ndev->dev,
        	skb->data, skb->len, DMA_TO_DEVICE);
    	lp->tx.rng_tx[entry].buf_size  = skb->len;
	}
	else		
    {
    	mapping = dma_map_single(&lp->ndev->dev,
        	skb->data, (skb->len - skb->data_len), DMA_TO_DEVICE);
    	lp->tx.rng_tx[entry].buf_size  = (skb->len - skb->data_len);
	}
    lp->tx.rng_tx[entry].skb = skb;
    lp->tx.rng_tx[entry].mapping = mapping;
    lp->tx.desc_tx[entry].bufferl = mapping;
    
    if (skb->protocol == __constant_htons(ETH_P_IPV6)) 
    {
        tx_flag0 = ETH_TDES0_FS | ETH_TDES0_GTSENV6;
    }
    else
    {
        tx_flag0 = ETH_TDES0_FS | ETH_TDES0_GTSENV4;
    }    
	if(skb_shinfo(skb)->nr_frags == 0)
	{
    	tx_flag0 |= ETH_TDES0_LS;
	}
    lp->tx.rng_tx[entry].lastflag = skb_shinfo(skb)->nr_frags;
    tx_flag0 |= ETH_TDES0_G_FML(lp->tx.rng_tx[entry].buf_size);
    tx_flag1 = ETH_TDES1_G_MSS(GKETH_TCP_MSS);
    
    lp->tx.desc_tx[entry].desc0 = tx_flag0;
    lp->tx.desc_tx[entry].desc1 = tx_flag1;
    if(entry == lp->tx_count - 1)
        lp->tx.desc_tx[entry].desc0 |= ETH_TDES0_EOR;
    lp->tx.desc_tx[entry].desc0 |= ETH_TDES0_OWN;
    lp->tx.cur_tx++;
    for(i=0; i < skb_shinfo(skb)->nr_frags; i++) 
	{
	    frag = &skb_shinfo(skb)->frags[i];
        entry = lp->tx.cur_tx % lp->tx_count;
		lp->tx.rng_tx[entry].lastflag = 0;
        
        mapping = skb_frag_dma_map(&lp->ndev->dev, frag, 0,
			   skb_frag_size(frag),DMA_TO_DEVICE);
            
        lp->tx.rng_tx[entry].skb = skb;
    	lp->tx.rng_tx[entry].buf_size = skb_frag_size(frag);
        lp->tx.rng_tx[entry].mapping = mapping;
        lp->tx.desc_tx[entry].bufferl = mapping;
        tx_flag0 = 0;
        if(i == skb_shinfo(skb)->nr_frags-1)
        {
            tx_flag0 |= ETH_TDES0_LS;
        }
        lp->tx.rng_tx[entry].lastflag = (skb_shinfo(skb)->nr_frags - 1 - i);
        if (skb->protocol == __constant_htons(ETH_P_IPV6)) 
        {
            tx_flag0 |= ETH_TDES0_GTSENV6;
        }
        else
        {
            tx_flag0 |= ETH_TDES0_GTSENV4;
        }
        tx_flag1 = ETH_TDES1_G_MSS(GKETH_TCP_MSS);
    
        lp->tx.desc_tx[entry].desc0 = ETH_TDES0_G_FML(lp->tx.rng_tx[entry].buf_size) | tx_flag0;
        lp->tx.desc_tx[entry].desc1 = tx_flag1;
        if(entry == lp->tx_count - 1)
            lp->tx.desc_tx[entry].desc0 |= ETH_TDES0_EOR;
        lp->tx.desc_tx[entry].desc0 |= ETH_TDES0_OWN;
        lp->tx.cur_tx++;
    }
    return NETDEV_TX_OK;
}
#endif
/* ================================================================= */

void gkhw_sf_reset(struct GKETH_info *lp)
{
    gk_rct_setbitsl((unsigned int)(lp->regbase + ETH_REG_CMD_OFFSET), \
        ETH_REG_CMD_STOP | ETH_REG_CMD_RST);
}

void gkhw_mac_reset(struct GKETH_info *lp)
{
    int counter = 0;
    
    gk_rct_setbitsl(ETH_RCT_PW, ETH_RCT_PW_ETH_GEN);
    mdelay(100);
    do {
        if (counter++ > 1000) {
            //printk("eth reset error\n");
            break;
        }
        mdelay(1);
    } while (!gk_eth_tstbitsb((unsigned int)(lp->regbase + ETH_REG_MAC_EEPRM_OFFSET), ETH_REG_MAC_EEPRM_ALTBD));

}

static inline void gkhw_dma_tx_poll(struct GKETH_info *lp)
{
    gk_eth_writeb((unsigned int)(lp->regbase + ETH_REG_TPPOLL_OFFSET), ETH_REG_TPPOLL_TX_NPQ);
}

static inline void gkhw_set_tx_desc(struct GKETH_info *lp)
{
    gk_eth_writel((unsigned int)(lp->regbase + ETH_REG_TX_NDS_LO_OFFSET),
        lp->tx_dma_desc);
    gk_eth_writel((unsigned int)(lp->regbase + ETH_REG_TX_NDS_HI_OFFSET), 0);
    gk_eth_writel((unsigned int)(lp->regbase + ETH_REG_TX_HDS_LO_OFFSET), 0);
    gk_eth_writel((unsigned int)(lp->regbase + ETH_REG_TX_HDS_HI_OFFSET), 0);
    GKETH_write_eri(lp, ETH_ERI_TX_NDS_L, (int)lp->tx_dma_desc);
    GKETH_write_eri(lp, ETH_ERI_TX_NDS_H, 0);
    GKETH_write_eri(lp, ETH_ERI_TX_HDS_L, 0);
    GKETH_write_eri(lp, ETH_ERI_TX_HDS_H, 0);
    gk_eth_writeb((unsigned int)(lp->regbase + ETH_REG_TX_DFN_OFFSET), ETH_REG_TX_DFN(1));
}

static inline void gkhw_set_rx_desc(struct GKETH_info *lp)
{
    gk_eth_writel((unsigned int)(lp->regbase + ETH_REG_RX_DS_LO_OFFSET),
        lp->rx_dma_desc);
    gk_eth_writel((unsigned int)(lp->regbase + ETH_REG_RX_DS_HI_OFFSET), 0);

}

static inline void gkhw_set_fpga_clk(struct GKETH_info *lp)
{
    int val;
    
    val = GKETH_read_eri(lp,0x2f8);
    val = (val & 0xFF00FC00) | 0x00080050;
    GKETH_write_eri(lp,0x2f8,val);
}

static inline int gkhw_reset(struct GKETH_info *lp)
{
    int                 errorCode = 0;
    
    if (errorCode && netif_msg_drv(lp))
        dev_err(&lp->ndev->dev, "DMA Error: Check PHY.\n");

    return errorCode;
}

static inline void gkhw_dma_rx_start(struct GKETH_info *lp)
{
    int val;
    int limit = 100;
    
    val = gk_eth_readl((unsigned int)(lp->regbase + ETH_REG_RX_CFG_OFFSET));
    val = (val & 0xFEFF8840) | ETH_REG_RX_CFG_RXEH(ETH_RX_CFG_RXEH_256) | ETH_REG_RX_CFG_APM | \
        ETH_REG_RX_CFG_AM | ETH_REG_RX_CFG_AB;
    gk_eth_writel((unsigned int)(lp->regbase + ETH_REG_RX_CFG_OFFSET), val);

    for (; limit > 0; limit--) {
        if (gk_eth_tstbitsl((unsigned int)(lp->regbase + ETH_REG_PHY_PWR_DUNC_OFFSET),
            0x02000000))
            break;
        udelay(100);
    }
    
    gk_eth_writeb((unsigned int)(lp->regbase + 0x0034),0x00);
    
    val = gk_eth_readb((unsigned int)(lp->regbase + ETH_REG_CMD_OFFSET));
    val = (val & ~ETH_REG_CMD_STOP) | ETH_REG_CMD_STOP | ETH_REG_CMD_RXEN;
    gk_eth_writeb((unsigned int)(lp->regbase + ETH_REG_CMD_OFFSET), (u8)val);

    gk_eth_clrbitsb((unsigned int)(lp->regbase + ETH_REG_CMD_OFFSET),ETH_REG_CMD_STOP);
}

static inline void gkhw_dma_rx_stop(struct GKETH_info *lp)
{
    gk_eth_setbitsb((unsigned int)(lp->regbase + ETH_REG_CMD_OFFSET),ETH_REG_CMD_STOP);
    gk_eth_writel((unsigned int)(lp->regbase + ETH_REG_RX_CFG_OFFSET), 0);
    gk_eth_clrbitsb((unsigned int)(lp->regbase + ETH_REG_CMD_OFFSET), \
        ETH_REG_CMD_STOP | ETH_REG_CMD_RXEN);
}

static inline void gkhw_dma_tx_start(struct GKETH_info *lp)
{
    int val;
    
    val = ETH_REG_TX_CFG_IFG_0 | ETH_REG_TX_CFG_MXDMA(ETH_TX_CFG_MXDMA_256) | 0x80;
    gk_eth_writel((unsigned int)(lp->regbase + ETH_REG_TX_CFG_OFFSET), val);
    
    val = gk_eth_readb((unsigned int)(lp->regbase + ETH_REG_CMD_OFFSET));
    val = (val & ~ETH_REG_CMD_STOP) | ETH_REG_CMD_STOP | ETH_REG_CMD_TXEN;
    gk_eth_writeb((unsigned int)(lp->regbase + ETH_REG_CMD_OFFSET), (u8)val);

    gk_eth_clrbitsb((unsigned int)(lp->regbase + ETH_REG_CMD_OFFSET),ETH_REG_CMD_STOP);

}

static inline void gkhw_dma_tx_stop(struct GKETH_info *lp)
{
    gk_eth_setbitsb((unsigned int)(lp->regbase + ETH_REG_CMD_OFFSET),ETH_REG_CMD_STOP);
    gk_eth_writel((unsigned int)(lp->regbase + ETH_REG_TX_CFG_OFFSET), 0);
    gk_eth_clrbitsb((unsigned int)(lp->regbase + ETH_REG_CMD_OFFSET), \
        ETH_REG_CMD_STOP | ETH_REG_CMD_TXEN);
}

static inline void gkhw_dma_tx_restart(struct GKETH_info *lp, u32 entry)
{
    gkhw_set_tx_desc(lp);    
    gkhw_dma_tx_start(lp);
}

static inline void gkhw_stop_tx_rx(struct GKETH_info *lp)
{
    int val;
    
    val = gk_eth_readb((unsigned int)(lp->regbase + ETH_REG_CMD_OFFSET));
    val = (val & 0x7F) | ETH_REG_CMD_STOP;
    gk_eth_writeb((unsigned int)(lp->regbase + ETH_REG_CMD_OFFSET), (u8)val);
    
    gk_eth_clrbitsb((unsigned int)(lp->regbase + ETH_REG_CMD_OFFSET),
                    (ETH_REG_CMD_STOP | ETH_REG_CMD_TXEN | ETH_REG_CMD_RXEN));
}

/*============================================================*/
static inline void gkhw_int_enable(struct GKETH_info *lp)
{
    gk_eth_writew((unsigned int)(lp->regbase + ETH_REG_INT_EN_OFFSET), GK_ETH_INTEN);
}

static inline void gkhw_int_disable(struct GKETH_info *lp)
{
    gk_eth_writew((unsigned int)(lp->regbase + ETH_REG_INT_EN_OFFSET), 0);
}

static inline void gkhw_set_link_mode_speed(struct GKETH_info *lp)
{
    u32                 val;

    GKETH_write_ocp(lp, ETH_OCP_FPGA_GPHY0,0x0);
    val =  GKETH_read_ocp(lp, ETH_OCP_FPGA_GPHY0);
    val = (val & 0xffc3) | (1<<2);
    
    switch (lp->oldspeed) {
    case SPEED_1000: 
        break;
    case SPEED_100:
        break;
    case SPEED_10:
    default:
        break;
    }
    val = val | ETH_OCP_FULL_DUMP | ETH_OCP_SPEED_100M;
    GKETH_write_ocp(lp, ETH_OCP_FPGA_GPHY0, val);
}

static inline void gkhw_set_hwaddr(struct GKETH_info *lp, u8 *hwaddr)
{
    u32                 val;

    val = (hwaddr[3] << 24) | (hwaddr[2] << 16) | (hwaddr[1] << 8) | (hwaddr[0]);
    gk_eth_writel((unsigned int)(lp->regbase + ETH_REG_MAC_LO_OFFSET), val);

    val = (hwaddr[5] << 8) | (hwaddr[4]);
    gk_eth_writel((unsigned int)(lp->regbase + ETH_REG_MAC_HI_OFFSET), val);    

    val = (hwaddr[1]<<24) | (hwaddr[0]<<16);
    GKETH_write_eri(lp, ETH_ERI_MACID_VLAN0_0, val);
    
    val = (hwaddr[5]<<24) | (hwaddr[4]<<16) | (hwaddr[3]<<8) | (hwaddr[2]);
    GKETH_write_eri(lp, ETH_ERI_MACID_VLAN0_2, val);
}

static inline void gkhw_get_hwaddr(struct GKETH_info *lp, u8 *hwaddr)
{
    u32                 hval;
    u32                 lval;

    hval = gk_eth_readl((unsigned int)(lp->regbase + ETH_REG_MAC_HI_OFFSET));
    lval = gk_eth_readl((unsigned int)(lp->regbase + ETH_REG_MAC_LO_OFFSET));
    hwaddr[5] = ((hval >> 8) & 0xff);
    hwaddr[4] = ((hval >> 0) & 0xff);
    hwaddr[3] = ((lval >> 24) & 0xff);
    hwaddr[2] = ((lval >> 16) & 0xff);
    hwaddr[1] = ((lval >> 8) & 0xff);
    hwaddr[0] = ((lval >> 0) & 0xff);
}

static inline void gkhw_set_desc(struct GKETH_info *lp)
{
    gkhw_set_tx_desc(lp);
    gkhw_set_rx_desc(lp);
}

static inline void gkhw_set_ccr(struct GKETH_info *lp)
{
    u16                 val;

    val = gk_eth_readw((unsigned int)(lp->regbase + ETH_REG_CCR_OFFSET));
    val = val | 0x2C00 | ETH_REG_CCR_TM_INT_1 | ETH_REG_CCR_RCVCKEN |  ETH_REG_CCR_RCVVLANEN;
    gk_eth_writew((unsigned int)(lp->regbase + ETH_REG_CCR_OFFSET), val);
}

static inline void gkhw_set_rxtx_ms(struct GKETH_info *lp)
{
    int                 val;
    
    val = GKETH_PACKET_MAXFRAME;
    gk_eth_writew((unsigned int)(lp->regbase + ETH_REG_RMS_OFFSET), val);

    val = gk_eth_readb((unsigned int)(lp->regbase + ETH_REG_MTPS_OFFSET));
    val = (val & 0x0F);
    gk_eth_writeb((unsigned int)(lp->regbase + ETH_REG_MTPS_OFFSET), (u8)val);
}

static inline void gkhw_set_fifo(struct GKETH_info *lp)
{
    int                 val;
    
    GKETH_write_eri(lp, ETH_ERI_FIFOFULL_TH, 0x00080002);
    GKETH_write_eri(lp, ETH_ERI_RFIFONFULL_TH, 0x02000038);
    GKETH_write_eri(lp, ETH_ERI_RFIFOEMPTY_TH, 0x00000048);
    GKETH_write_eri(lp, ETH_ERI_TFIFOFULL_TH, 0x00100006);

    val = GKETH_read_eri(lp, ETH_ERI_FTR_MCU_CTRL);
    val = (val & 0xFFFD0000) | 0x0000000D;
    GKETH_write_eri(lp, ETH_ERI_FTR_MCU_CTRL, val);
}

static inline int gkhw_enable(struct GKETH_info *lp)
{
    int errorCode = 0;
    int val = 0;
    
    errorCode = gkhw_reset(lp);
    if (errorCode)
        goto gkhw_init_exit;

    gk_eth_setbitsb((unsigned int)(lp->regbase + ETH_REG_MAC_EEPRM_OFFSET), ETH_REG_MAC_EEPRM_CRWEN);
    //gk_eth_setbitsb((unsigned int)(lp->regbase + ETH_REG_MAC_CFG0_OFFSET), ETH_REG_MAC_CFG0_P_SPICS);
    gk_eth_writeb((unsigned int)(lp->regbase + ETH_REG_MAC_CFG0_OFFSET), ETH_MAC_CFG0_BOOTROM_SIZE(ETH_CFG0_BOOTROM_64K));
    gk_eth_writeb((unsigned int)(lp->regbase + ETH_REG_MAC_CFG1_OFFSET), 0x00);
    val = ETH_REG_MAC_CFG2_SELSMB | ETH_REG_MAC_CFG2_LEDLPEN | \
        ETH_REG_MAC_CFG2_LWRST | ETH_REG_MAC_CFG2_AUXSTA | ETH_REG_MAC_CFG2_PMEEN;
    gk_eth_writeb((unsigned int)(lp->regbase + ETH_REG_MAC_CFG2_OFFSET), val);

    val = ETH_REG_MAC_CFG3_L23EN | ETH_REG_MAC_CFG3_JBEN | ETH_REG_MAC_CFG3_MAGIC;
    gk_eth_writeb((unsigned int)(lp->regbase + ETH_REG_MAC_CFG3_OFFSET), val);
    gk_eth_writeb((unsigned int)(lp->regbase + ETH_REG_MAC_CFG4_OFFSET), 0x00);
    gk_eth_writeb((unsigned int)(lp->regbase + ETH_REG_MAC_CFG5_OFFSET), ETH_REG_MAC_CFG5_LWAK);

    gk_eth_writel((unsigned int)(lp->regbase + ETH_REG_MAR_LO_OFFSET), 0xFFFFFFFF);
    gk_eth_writel((unsigned int)(lp->regbase + ETH_REG_MAR_HI_OFFSET), 0xFFFFFFFF);

    gkhw_set_hwaddr(lp, lp->ndev->dev_addr);
    
gkhw_init_exit:
    return errorCode;
}

static inline void gkhw_disable(struct GKETH_info *lp)
{
    gkhw_stop_tx_rx(lp);
    gkhw_int_disable(lp);
    //gk_set_gpio_output(&lp->platform_info->mii_power, 0);
    //gk_set_gpio_output(&lp->platform_info->mii_reset, 1);
}

static inline void gkhw_dump(struct GKETH_info *lp)
{
    u32                 i;
    
    dev_info(&lp->ndev->dev, "RX Info: cur_rx %d, dirty_rx %d.\n",
        lp->rx.cur_rx % lp->rx_count, lp->rx.dirty_rx % lp->rx_count);
    dev_info(&lp->ndev->dev, "RX Info: RX descriptor[%d] "
        "0x%08x 0x%08x 0x%08x 0x%08x.\n",lp->rx.dirty_rx % lp->rx_count,
        lp->rx.desc_rx[lp->rx.dirty_rx % lp->rx_count].desc0,
        lp->rx.desc_rx[lp->rx.dirty_rx % lp->rx_count].desc1,
        lp->rx.desc_rx[lp->rx.dirty_rx % lp->rx_count].bufferl,
        lp->rx.desc_rx[lp->rx.dirty_rx % lp->rx_count].bufferh);
    dev_info(&lp->ndev->dev, "TX Info: cur_tx %d, dirty_tx %d.\n",
        lp->tx.cur_tx % lp->tx_count, lp->tx.dirty_tx % lp->tx_count);
    for (i = lp->tx.dirty_tx; i < lp->tx.cur_tx; i++) {
        dev_info(&lp->ndev->dev, "TX Info: TX descriptor[%d] "
            "0x%08x 0x%08x 0x%08x 0x%08x.\n", i % lp->tx_count,
            lp->tx.desc_tx[i % lp->tx_count].desc0,
            lp->tx.desc_tx[i % lp->tx_count].desc1,
            lp->tx.desc_tx[i % lp->tx_count].bufferl,
            lp->tx.desc_tx[i % lp->tx_count].bufferh);
    }
    for (i = 0; i <= 63; i++) {
        dev_info(&lp->ndev->dev, "MAC[%d]: 0x%08x.\n", i * 4, \
        gk_eth_readl((unsigned int)(lp->regbase + ETH_REG_MAC_LO_OFFSET + (i << 2))));
    }
}

static int gkhw_mdio_read(struct mii_bus *bus,
    int mii_id, int regnum)
{
    struct GKETH_info   *lp;
    u32                 val = 0;
    int                 limit;
    int                 addr;
    
    if(cmdline_phytype != 0){
        lp = (struct GKETH_info *)bus->priv;

        for (limit = lp->platform_info->mii_retry_limit; limit > 0; limit--) {
            if (!gk_eth_tstbitsl((unsigned int)(lp->regbase + ETH_REG_PHY_ACCESS_OFFSET),
                ETH_REG_PHY_ACCESS_BUSY))
                break;
            udelay(lp->platform_info->mii_retry_tmo);
        }
        if ((limit <= 0) && netif_msg_hw(lp)) {
            dev_err(&lp->ndev->dev, "MII Error: Preread tmo!\n");
            val = 0xFFFFFFFF;
            goto gkhw_mdio_read_exit;
        }
        val = ETH_REG_PHY_ACCESS_REG(regnum) & ETH_REG_PHY_ACCESS_RD;
        gk_eth_writel((unsigned int)(lp->regbase + ETH_REG_PHY_ACCESS_OFFSET), val);

        for (limit = lp->platform_info->mii_retry_limit; limit > 0; limit--) {
            if (!gk_eth_tstbitsl((unsigned int)(lp->regbase + ETH_REG_PHY_ACCESS_OFFSET),
                ETH_REG_PHY_ACCESS_BUSY))
                break;
            udelay(lp->platform_info->mii_retry_tmo);
        }
        if ((limit <= 0) && netif_msg_hw(lp)) {
            dev_err(&lp->ndev->dev, "MII Error: Postread tmo!\n");
            val = 0xFFFFFFFF;
            goto gkhw_mdio_read_exit;
        }

        val = gk_eth_readl((unsigned int)(lp->regbase + ETH_REG_PHY_ACCESS_OFFSET)) & 0x0000FFFF;

    gkhw_mdio_read_exit:
        if (netif_msg_hw(lp))
        {
            printk("mdio read error\n");
            dev_info(&lp->ndev->dev,
                "MII Read: id[0x%02x], add[0x%02x], val[0x%04x].\n",
                mii_id, regnum, val);
        }
        
		if(val==0)
			val=0xFFFF;
        return val;
    } else {

        lp = (struct GKETH_info *)bus->priv;
        addr = REG_EPHY_CONTROL + regnum * 4;
        val = gk_eth_readl((unsigned int)(REG_EPHY_CONTROL + regnum * 4));
        if(regnum == 1){
            if((gk_eth_readl((unsigned int)(REG_EPHY_CONTROL + 4)) & (0x1 << 5)) != 0)
                val |= 0x1 << 2;
        }
        return val;
    }
}

static int gkhw_mdio_write(struct mii_bus *bus,
    int mii_id, int regnum, u16 value)
{
#define INTER_PHY_BASE 0xF0022000
    int                 errorCode = 0;
    struct GKETH_info   *lp;
    int                 val;
    int                 limit = 0;
    
    if(cmdline_phytype != 0){

        lp = (struct GKETH_info *)bus->priv;

        if (netif_msg_hw(lp))
            dev_info(&lp->ndev->dev,
                "MII Write: id[0x%02x], add[0x%02x], val[0x%04x].\n",
                mii_id, regnum, value);

        for (limit = lp->platform_info->mii_retry_limit; limit > 0; limit--) {
            if (!gk_eth_tstbitsl((unsigned int)(lp->regbase + ETH_REG_PHY_ACCESS_OFFSET),
                ETH_REG_PHY_ACCESS_BUSY))
                break;
            udelay(lp->platform_info->mii_retry_tmo);
        }
        if ((limit <= 0) && netif_msg_hw(lp)) {
            dev_err(&lp->ndev->dev, "MII Error: Prewrite tmo!\n");
            errorCode = -EIO;
            goto gkhw_mdio_write_exit;
        }

        val = ETH_REG_PHY_ACCESS_REG(regnum) | ETH_REG_PHY_ACCESS_DATA(value) | ETH_REG_PHY_ACCESS_WR;
        gk_eth_writel((unsigned int)(lp->regbase + ETH_REG_PHY_ACCESS_OFFSET), val);

        for (limit = lp->platform_info->mii_retry_limit; limit > 0; limit--) {
            if (!gk_eth_tstbitsl((unsigned int)(lp->regbase + ETH_REG_PHY_ACCESS_OFFSET),
                ETH_REG_PHY_ACCESS_BUSY))
                break;
            udelay(lp->platform_info->mii_retry_tmo);
        }
        if ((limit <= 0) && netif_msg_hw(lp)) {
            dev_err(&lp->ndev->dev, "MII Error: Postwrite tmo!\n");
            errorCode = -EIO;
            goto gkhw_mdio_write_exit;
        }

gkhw_mdio_write_exit:
        return errorCode;
    } else {

        lp = (struct GKETH_info *)bus->priv;
        gk_eth_writel((unsigned int)(INTER_PHY_BASE + regnum * 4), value);
        return 0;
    }
}

static int gkhw_mdio_reset(struct mii_bus *bus)
{
    int                 errorCode = 0;
    struct GKETH_info   *lp;
    
    lp = (struct GKETH_info *)bus->priv;
    if(cmdline_phytype != 0)
    {
        struct gk_eth_platform_info *pf;

        pf = lp->platform_info;
        if(pf != NULL)
        {
            // LindengYu: Add reset to here
            printk("###### PHY Reset.1.0.2\n");
            gk_gpio_set_out(pf->phy_reset.gpio_id, 0);
            mdelay(50);//100ms
            gk_gpio_set_out(pf->phy_reset.gpio_id, 1);
            // RTL8201 need 200ms
            mdelay(200);//100ms
        }
    }
    return errorCode;
}

/* ================================================================= */

/* ======================================================== */

int GKETH_change_mtu(struct net_device *dev, int new_mtu)
{
	if (new_mtu < 68 || new_mtu > GKETH_PACKET_MAXFRAME)
		return -EINVAL;
	dev->mtu = new_mtu;
	return 0;
}

static void GKETH_adjust_link(struct net_device *ndev)
{
    struct GKETH_info   *lp;
    unsigned long       flags;
    struct phy_device   *phydev;
    int                 need_update = 0;

    lp = (struct GKETH_info *)netdev_priv(ndev);

    spin_lock_irqsave(&lp->lock, flags);
    phydev = lp->phydev;
    if (phydev && phydev->link)
    {
        if (phydev->duplex != lp->oldduplex)
        {
            need_update = 1;
            lp->oldduplex = phydev->duplex;
        }
        if (phydev->speed != lp->oldspeed)
        {
            switch (phydev->speed)
            {
            case SPEED_1000:
            case SPEED_100:
            case SPEED_10:
                need_update = 1;
                lp->oldspeed = phydev->speed;
                break;
            default:
                if (netif_msg_link(lp))
                    dev_warn(&lp->ndev->dev,
                        "Unknown Speed(%d).\n",
                        phydev->speed);
                break;
            }
        }
        if (lp->oldlink != phydev->link)
        {
            need_update = 1;
            lp->oldlink = phydev->link;
        }
    }
    else if (lp->oldlink)
    {
        need_update = 1;
        lp->oldlink = PHY_DOWN;
        lp->oldspeed = 0;
        lp->oldduplex = -1;
    }

    if (need_update)
    {
        u16 rmii;
        gkhw_set_link_mode_speed(lp);
        rmii = gkhw_mdio_read(&lp->new_bus, lp->phydev->addr , 0x01);
        #if 0
        printk("#############-------------reg1 %04x--------\n", rmii);
        #endif
        if(rmii==1)
        {
            if (phydev && phydev->link)
	        {
	            switch (phydev->speed)
	            {
	            case SPEED_100:
	                if(cmdline_phytype == 0)
	                {
	                    gk_set_phy_speed_led(GPIO_TYPE_OUTPUT_EPHY_LED_3);
	                }
	                GH_GPIO_set_INPUT_CFG_in_sel(GPIO_GET_IN_SEL(GPIO_TYPE_INPUT_ENET_PHY_RXD_3) - 2, 0x01);
	                break;
	            case SPEED_10:
	                if(cmdline_phytype == 0)
	                {
	                    gk_set_phy_speed_led(GPIO_TYPE_OUTPUT_EPHY_LED_2);
	                }
	                GH_GPIO_set_INPUT_CFG_in_sel(GPIO_GET_IN_SEL(GPIO_TYPE_INPUT_ENET_PHY_RXD_3) - 2, 0x00);
	                break;
	            default:
	                break;
	            }
	        }
	        else
	        {
	            if(cmdline_phytype == 0)
	            {
	                gk_set_phy_speed_led(GPIO_TYPE_INPUT_0);
	            }
            }
        }
        if (phydev && netif_msg_link(lp))
            phy_print_status(phydev);
#if defined(CONFIG_SYSTEM_USE_NETLINK_MSG_ETH)
        GK_NET_LINK_MSG_S msg;
        msg.module = GK_NL_ETH;
        if(phydev != NULL)
        {
        msg.data.gk_nl_eth_data.link    = phydev->link;
        msg.data.gk_nl_eth_data.speed   = phydev->speed;
        msg.data.gk_nl_eth_data.duplex  = phydev->duplex;
        }
        else
            printk("phydev is NULL\n");
        msg.len = GK_NL_GPIO_MSG_LEN;
        gk_nl_send_message(&msg);
#endif
    }
    spin_unlock_irqrestore(&lp->lock, flags);
}

static int GKETH_phy_start(struct GKETH_info *lp)
{
    int                 errorCode = 0;
    struct phy_device   *phydev;
    phy_interface_t     interface;
    struct net_device   *ndev;
    int                 phy_addr;
    unsigned long       flags;
    
    spin_lock_irqsave(&lp->lock, flags);
    phydev = lp->phydev;
    spin_unlock_irqrestore(&lp->lock, flags);
    if (phydev)
        goto GKETH_init_phy_exit;

    ndev = lp->ndev;
    lp->oldlink = PHY_DOWN;
    lp->oldspeed = 0;
    lp->oldduplex = -1;

    if(cmdline_phytype != 0)
    {
        phy_addr = lp->platform_info->mii_id;
        if ((phy_addr >= 0) && (phy_addr < PHY_MAX_ADDR))
        {
            if (lp->new_bus.phy_map[phy_addr])
            {
                phydev = lp->new_bus.phy_map[phy_addr];
                if (phydev->phy_id == lp->platform_info->phy_id)
                {
                    goto GKETH_init_phy_default;
                }
            }
            dev_notice(&lp->ndev->dev,
                "Could not find default PHY in %d.\n", phy_addr);
        }
        goto GKETH_init_phy_scan;

    GKETH_init_phy_default:
        printk("GKETH_init_phy_default ...\n");
        if (netif_msg_hw(lp))
            dev_info(&lp->ndev->dev, "Find default PHY in %d!\n", phy_addr);
        goto GKETH_init_phy_connect;

    GKETH_init_phy_scan:
        for (phy_addr = 0; phy_addr < PHY_MAX_ADDR; phy_addr++) {
            if (lp->new_bus.phy_map[phy_addr]) {
                phydev = lp->new_bus.phy_map[phy_addr];
                if (phydev->phy_id == lp->platform_info->phy_id)
                    goto GKETH_init_phy_connect;
            }
        }
        if (!phydev) {
            if (netif_msg_drv(lp))
                dev_err(&lp->ndev->dev, "No PHY device.\n");
            errorCode = -ENODEV;
            goto GKETH_init_phy_exit;
        } else {
            if (netif_msg_drv(lp))
                dev_notice(&lp->ndev->dev,
                "Try PHY[%d] whose id is 0x%08x, whose name is %s!\n",
                phydev->addr, phydev->phy_id,
                phydev->drv?phydev->drv->name:"unkown,\n================================================\n\
please make menuconfig to select the phy device!\n================================================");
        }

    GKETH_init_phy_connect:
        printk("GKETH_init_phy_connect ...\n");
        interface = gkhw_get_interface(lp);

        phydev = phy_connect(ndev, dev_name(&phydev->dev),
            &GKETH_adjust_link, 0, interface);
        if (IS_ERR(phydev)) {
            if (netif_msg_drv(lp))
                dev_err(&lp->ndev->dev, "Could not attach to PHY!\n");
            errorCode = PTR_ERR(phydev);
            goto GKETH_init_phy_exit;
        }

        phydev->supported = lp->platform_info->phy_supported;
        phydev->advertising = phydev->supported;


        spin_lock_irqsave(&lp->lock, flags);
        lp->phydev = phydev;
        spin_unlock_irqrestore(&lp->lock, flags);

        dev_err(&lp->ndev->dev,"###### GKETH_phy_start_aneg...\n");

        errorCode = phy_start_aneg(phydev);

    GKETH_init_phy_exit:
        return errorCode;
    } else {

        interface = gkhw_get_interface(lp);
        phydev = lp->new_bus.phy_map[0];
        phydev = phy_connect(ndev, dev_name(&phydev->dev),
            &GKETH_adjust_link, 0, interface);
        if (IS_ERR(phydev)) {
            if (netif_msg_drv(lp))
                dev_err(&lp->ndev->dev, "Could not attach to PHY!\n");
            errorCode = PTR_ERR(phydev);
            return errorCode;
        }

        phydev->supported = lp->platform_info->phy_supported;
        phydev->advertising = phydev->supported;

        spin_lock_irqsave(&lp->lock, flags);
        lp->phydev = phydev;
        spin_unlock_irqrestore(&lp->lock, flags);

        dev_err(&lp->ndev->dev,"###### GKETH_phy_start_aneg...\n");

        errorCode = phy_start_aneg(phydev);
        return 0;
    }
}

static void GKETH_phy_stop(struct GKETH_info *lp)
{
    struct phy_device           *phydev;
    unsigned long               flags;
    
    spin_lock_irqsave(&lp->lock, flags);
    phydev = lp->phydev;
    dev_err(&lp->ndev->dev,"###### GKETH_phy_stop\n");
    lp->phydev = NULL;
    spin_unlock_irqrestore(&lp->lock, flags);
    if (phydev)
        phy_disconnect(phydev);
}

static inline int GKETH_rx_rngmng_check_skb(struct GKETH_info *lp, u32 entry)
{
    int                 errorCode = 0;
    dma_addr_t              mapping;
    struct sk_buff              *skb;
    
    if (lp->rx.rng_rx[entry].skb == NULL) {
        skb = dev_alloc_skb(GKETH_PACKET_MAXFRAME);
        if (skb == NULL) {
            if (netif_msg_drv(lp))
                dev_err(&lp->ndev->dev,
                "RX Error: dev_alloc_skb.\n");
            errorCode = -ENOMEM;
            goto GKETH_rx_rngmng_skb_exit;
        }

        skb->dev                      = lp->ndev;
        lp->rx.rng_rx[entry].buf_size = GKETH_PACKET_MAXFRAME;
        mapping                       = dma_map_single(&lp->ndev->dev,
                                                       skb->data,
                                                       lp->rx.rng_rx[entry].buf_size,
                                                       DMA_FROM_DEVICE);

        lp->rx.rng_rx[entry].skb = skb;
        lp->rx.rng_rx[entry].mapping = mapping;
        lp->rx.desc_rx[entry].bufferl = mapping;
    }

GKETH_rx_rngmng_skb_exit:
    return errorCode;
}

static inline void GKETH_rx_rngmng_init(struct GKETH_info *lp)
{
    int                 i;
    
    lp->rx.cur_rx = 0;
    lp->rx.dirty_rx = 0;
    for (i = 0; i < lp->rx_count; i++) {
        if (GKETH_rx_rngmng_check_skb(lp, i))
            break;
        if(i < lp->rx_count - 1)
            lp->rx.desc_rx[i].desc0 = ETH_RDES0_OWN | ETH_RDES0_FML(lp->rx.rng_rx[i].buf_size);
        else
            lp->rx.desc_rx[i].desc0 = ETH_RDES0_FML(lp->rx.rng_rx[i].buf_size);
        lp->rx.desc_rx[i].desc1 = 0;
    }
    lp->rx.desc_rx[lp->rx_count - 1].desc0 |= ETH_RDES0_OWN | ETH_RDES0_EOR;
}

static inline void GKETH_rx_rngmng_refill(struct GKETH_info *lp)
{
    u32                 entry;

    while (lp->rx.cur_rx > lp->rx.dirty_rx) {
        entry = lp->rx.dirty_rx % lp->rx_count;
        if (GKETH_rx_rngmng_check_skb(lp, entry))
            break;
        lp->rx.desc_rx[entry].desc0 = ETH_RDES0_FML(lp->rx.rng_rx[entry].buf_size);
        lp->rx.desc_rx[entry].desc1 = 0;
        if (entry == lp->rx_count - 1)
            lp->rx.desc_rx[entry].desc0 |= ETH_RDES0_EOR;
        lp->rx.desc_rx[entry].desc0 |= ETH_RDES0_OWN;
        lp->rx.dirty_rx++;
    }
}

static inline void GKETH_rx_rngmng_del(struct GKETH_info *lp)
{
    int                 i;
    dma_addr_t              mapping;
    struct sk_buff              *skb;
    
    for (i = 0; i < lp->rx_count; i++) {
        if (lp->rx.rng_rx) {
            skb = lp->rx.rng_rx[i].skb;
            mapping = lp->rx.rng_rx[i].mapping;
            lp->rx.rng_rx[i].skb = NULL;
            lp->rx.rng_rx[i].mapping = 0;
            if (skb) {
                dma_unmap_single(&lp->ndev->dev, mapping,
                    lp->rx.rng_rx[i].buf_size, DMA_FROM_DEVICE);
                dev_kfree_skb(skb);
            }
            lp->rx.rng_rx[i].buf_size = 0;
        }
        if (lp->rx.desc_rx) {
            lp->rx.desc_rx[i].desc0= 0;
            lp->rx.desc_rx[i].desc1= 0;
            lp->rx.desc_rx[i].bufferl = 0xBADF00D0;
            lp->rx.desc_rx[i].bufferh = 0xBADF00D0;
        }
    }
}

static inline void GKETH_tx_rngmng_init(struct GKETH_info *lp)
{
    int                 i;
    
    lp->tx.cur_tx = 0;
    lp->tx.dirty_tx = 0;
    for (i = 0; i < lp->tx_count; i++) {
        lp->tx.rng_tx[i].mapping = 0 ;
        lp->tx.desc_tx[i].desc0 = (ETH_TDES0_LS | ETH_TDES0_FS);
        lp->tx.desc_tx[i].desc1 = 0;
        lp->tx.desc_tx[i].bufferl = 0;
        lp->tx.desc_tx[i].bufferh = 0;
    }
    lp->tx.desc_tx[lp->tx_count - 1].desc0 |= ETH_TDES0_EOR;
}

static inline void GKETH_tx_rngmng_del(struct GKETH_info *lp)
{
    int                 i;
    dma_addr_t              mapping;
    struct sk_buff              *skb;
    unsigned int                dirty_tx;
    u32                 entry;
    u32                 status;
    
    for (dirty_tx = lp->tx.dirty_tx; lp->tx.cur_tx > dirty_tx; dirty_tx++) {
        entry = dirty_tx % lp->tx_count;
        if (lp->tx.desc_tx) {
            status = lp->tx.desc_tx[entry].desc0;
            if (status & ETH_TDES0_OWN)
                lp->stats.tx_dropped++;
        }
    }
    for (i = 0; i < lp->tx_count; i++) {
        if (lp->tx.rng_tx) {
            skb = lp->tx.rng_tx[i].skb;
            mapping = lp->tx.rng_tx[i].mapping;
            lp->tx.rng_tx[i].skb = NULL;
            lp->tx.rng_tx[i].mapping = 0;
            if (skb) {
                dma_unmap_single(&lp->ndev->dev, mapping,
                    skb->len, DMA_TO_DEVICE);
                dev_kfree_skb(skb);
            }
        }
        if (lp->tx.desc_tx) {
            lp->tx.desc_tx[i].desc0 = 0;
            lp->tx.desc_tx[i].desc1= 0;
            lp->tx.desc_tx[i].bufferl = 0xBADF00D0;
            lp->tx.desc_tx[i].bufferh = 0xBADF00D0;
        }
    }
}

static inline void GKETH_check_dma_error(struct GKETH_info *lp,
    u16 irq_status)
{
    if (unlikely(irq_status & GK_ETH_INT_STA)) {
        if (irq_status & ETH_REG_INT_STA_RXOK) {
//                dev_err(&lp->ndev->dev,
//                "DMA Error: Rx OK.\n");
        }
        if (irq_status & ETH_REG_INT_STA_RXER) {
            if (netif_msg_rx_err(lp))
                dev_err(&lp->ndev->dev,
                "DMA Error: Receive Watchdog Timeout.\n");
        }
        if (irq_status & ETH_REG_INT_STA_TXOK) {
//                dev_err(&lp->ndev->dev,
//                "DMA Error: Tx OK.\n");
        }
        if (irq_status & ETH_REG_INT_STA_TXER) {
            if (netif_msg_tx_err(lp))
                dev_err(&lp->ndev->dev,
                "DMA Error: TX ERROR.\n");
        }
        if (irq_status & ETH_REG_INT_STA_RXDU) {
            if (netif_msg_rx_err(lp))
                dev_err(&lp->ndev->dev,
                "DMA Error: RX Desc Un.\n");
        }
        if (irq_status & ETH_REG_INT_STA_LKCHG) {
            if (netif_msg_link(lp))
                dev_err(&lp->ndev->dev,
                "DMA Error: Link Change.\n");
        }
        if (irq_status & ETH_REG_INT_STA_RXFIFOOV) {
            if (netif_msg_rx_err(lp))
                dev_err(&lp->ndev->dev,
                "DMA Error: RX FIFO Overflow.\n");
        }
        if (irq_status & ETH_REG_INT_STA_TXDU) {
//                dev_err(&lp->ndev->dev,
//                "DMA Error: Tx Descriptor Succeed.\n");
        }
        if (irq_status & ETH_REG_INT_STA_SFINT) {
                dev_err(&lp->ndev->dev,
                "DMA Error: FSWInt.\n");
        }
        if (irq_status & ETH_REG_INT_STA_RXDLLK) {
            if (netif_msg_rx_err(lp))
                dev_err(&lp->ndev->dev,
                "DMA Error: RX Deadlock Overflow.\n");
        }
        if (irq_status & ETH_REG_INT_STA_EPHYDLINK) {
            if (netif_msg_ifdown(lp))
                dev_err(&lp->ndev->dev,
                "DMA Error: EPHY Desconnect.\n");
        }
        if (irq_status & ETH_REG_INT_STA_EPHYERR) {
            if (netif_msg_ifdown(lp))
                dev_err(&lp->ndev->dev,
                "DMA Error: EPHY Rx Error.\n");
        }
        if (irq_status & ETH_REG_INT_STA_TIMEOUT) {
            lp->stats.tx_errors++;
            if (netif_msg_timer(lp))
                dev_err(&lp->ndev->dev,
                "DMA Error: Transmit Jabber Timeout.\n");
        }
        if (irq_status & ETH_REG_INT_STA_SYSERR) {
                dev_err(&lp->ndev->dev,
                "DMA Error: SYSTEM Error.\n");
        }
        
        if (netif_msg_tx_err(lp) || netif_msg_rx_err(lp)) {
            dev_err(&lp->ndev->dev, "DMA Error: Abnormal: 0x%x.\n",
                irq_status);
            gkhw_dump(lp);
        }
    }
}

static inline void GKETH_interrupt_rx(struct GKETH_info *lp, u16 irq_status)
{
    if (irq_status & GK_ETH_RX_INT_STA) {
        dev_vdbg(&lp->ndev->dev, "RX IRQ[0x%x]!\n", irq_status);
        lp->rx.intstatus = (irq_status & GK_ETH_RX_INT_STA);
        gk_eth_clrbitsw((unsigned int)(lp->regbase + ETH_REG_INT_EN_OFFSET),
            ETH_REG_INT_EN_RXOK);
		if(irq_status & (ETH_REG_INT_STA_RXDU | ETH_REG_INT_STA_RXFIFOOV))
		{
			gkhw_dma_rx_stop(lp);
		}
        napi_schedule(&lp->napi);
    }
}

static inline void GKETH_interrupt_tx(struct GKETH_info *lp, u16 irq_status)
{
    unsigned int                dirty_tx;
    unsigned int                dirty_to_tx;
    u32                 entry;
    u32                 status;
    
    if (irq_status & GK_ETH_TX_INT_STA)
    {
        dev_vdbg(&lp->ndev->dev, "cur_tx[%d], dirty_tx[%d], 0x%x.\n",
            lp->tx.cur_tx, lp->tx.dirty_tx, irq_status);
        for (dirty_tx = lp->tx.dirty_tx; dirty_tx < lp->tx.cur_tx; dirty_tx++)
        {
            entry = dirty_tx % lp->tx_count;
            status = lp->tx.desc_tx[entry].desc0;

            if ((status & ETH_TDES0_OWN) | gkhw_tx_free_check(lp, entry))
            {
                break;
            }

            if (irq_status & ETH_REG_INT_STA_TXER)
            {
                gkhw_dma_tx_stop(lp);
                gkhw_dma_tx_restart(lp, entry);
                gkhw_dma_tx_poll(lp);
                lp->stats.tx_errors++;
            }
            else if (irq_status & ETH_REG_INT_STA_TXOK)
            {
                lp->stats.tx_bytes +=
                    lp->tx.rng_tx[entry].skb->len;
                lp->stats.tx_packets++;
            }

            dma_unmap_single(&lp->ndev->dev,
                lp->tx.rng_tx[entry].mapping,
                lp->tx.rng_tx[entry].buf_size,
                DMA_TO_DEVICE);
			if(lp->tx.rng_tx[entry].lastflag == 0)
	            dev_kfree_skb_irq(lp->tx.rng_tx[entry].skb);
            //lp->tx.rng_tx[entry].lastflag = 0;
            lp->tx.rng_tx[entry].skb = NULL;
            lp->tx.rng_tx[entry].mapping = 0;
        }

        dirty_to_tx = lp->tx.cur_tx - dirty_tx;
        if (unlikely(dirty_to_tx > lp->tx_count)) {
            netif_stop_queue(lp->ndev);
            if (netif_msg_drv(lp))
                dev_err(&lp->ndev->dev, "TX Error: TX OV.\n");
            gkhw_dump(lp);
            gkhw_dma_tx_stop(lp);
            GKETH_tx_rngmng_del(lp);
            GKETH_tx_rngmng_init(lp);
            dirty_tx = dirty_to_tx = 0;
        }
        if (likely(dirty_to_tx < (lp->tx_count / 2))) {
            dev_vdbg(&lp->ndev->dev, "TX Info: Now gap %d.\n",
                dirty_to_tx);
            netif_wake_queue(lp->ndev);
        }
        lp->tx.dirty_tx = dirty_tx;
    }
}

static irqreturn_t GKETH_interrupt(int irq, void *dev_id)
{
    struct net_device           *ndev;
    struct GKETH_info           *lp;
    u16                 irq_status;
    unsigned long               flags;
    
    ndev = (struct net_device *)dev_id;
    lp = netdev_priv(ndev);

    spin_lock_irqsave(&lp->lock, flags);
    irq_status = gk_eth_readw((unsigned int)(lp->regbase + ETH_REG_INT_STA_OFFSET));
    GKETH_check_dma_error(lp, irq_status);
    GKETH_interrupt_rx(lp, irq_status);
    GKETH_interrupt_tx(lp, irq_status);
    gk_eth_writew((unsigned int)(lp->regbase + ETH_REG_INT_STA_OFFSET), irq_status);
    spin_unlock_irqrestore(&lp->lock, flags);

    return IRQ_HANDLED;
}

static int GKETH_start_hw(struct net_device *ndev)
{
    int                 errorCode = 0;
    struct GKETH_info           *lp;
    unsigned long               flags;
    
    lp = (struct GKETH_info *)netdev_priv(ndev);
    dev_err(&lp->ndev->dev,"###### GKETH_start_hw\n");
    //while(1);
    
#if 1
        //kewell add
        //LindengYu: if reset, might change the phy address. So, reset the phy before at the get phy id.
        //gkhw_mdio_reset(lp);
        GKETH_phy_start(lp);
#else
        gkhw_phy_set_high(lp);
        gkhw_phy_reset(lp);
        GH_EPHY_set_MII_RMII_rmii(0x1);
#endif
    spin_lock_irqsave(&lp->lock, flags);
    errorCode = gkhw_enable(lp);
    spin_unlock_irqrestore(&lp->lock, flags);
    if (errorCode)
        goto GKETH_start_hw_exit;

    lp->rx.rng_rx = kmalloc((sizeof(struct GKETH_rng_info) *
        lp->rx_count), GFP_KERNEL);
    if (lp->rx.rng_rx == NULL) {
        if (netif_msg_drv(lp))
            dev_err(&lp->ndev->dev, "alloc rng_rx fail.\n");
        errorCode = -ENOMEM;
        goto GKETH_start_hw_exit;
    }
    lp->rx.desc_rx = dma_alloc_coherent(&lp->ndev->dev,
        (sizeof(struct GKETH_desc) * lp->rx_count),
        &lp->rx_dma_desc, GFP_KERNEL);
    if (lp->rx.desc_rx == NULL) {
        if (netif_msg_drv(lp))
            dev_err(&lp->ndev->dev,
            "dma_alloc_coherent desc_rx fail.\n");
        errorCode = -ENOMEM;
        goto GKETH_start_hw_exit;
    }
    memset(lp->rx.rng_rx, 0,
        (sizeof(struct GKETH_rng_info) * lp->rx_count));
    memset(lp->rx.desc_rx, 0,
        (sizeof(struct GKETH_desc) * lp->rx_count));
    GKETH_rx_rngmng_init(lp);

    lp->tx.rng_tx = kmalloc((sizeof(struct GKETH_rng_info) *
        lp->tx_count), GFP_KERNEL);
    if (lp->tx.rng_tx == NULL) {
        if (netif_msg_drv(lp))
            dev_err(&lp->ndev->dev, "alloc rng_tx fail.\n");
        errorCode = -ENOMEM;
        goto GKETH_start_hw_exit;
    }
    lp->tx.desc_tx = dma_alloc_coherent(&lp->ndev->dev,
        (sizeof(struct GKETH_desc) * lp->tx_count),
        &lp->tx_dma_desc, GFP_KERNEL);
    if (lp->tx.desc_tx == NULL) {
        if (netif_msg_drv(lp))
            dev_err(&lp->ndev->dev,
            "dma_alloc_coherent desc_tx fail.\n");
        errorCode = -ENOMEM;
        goto GKETH_start_hw_exit;
    }
    memset(lp->tx.rng_tx, 0,
        (sizeof(struct GKETH_rng_info) * lp->tx_count));
    memset(lp->tx.desc_tx, 0,
        (sizeof(struct GKETH_desc) * lp->tx_count));
    GKETH_tx_rngmng_init(lp);

    lp->tally.mapping_size = 128;
    lp->tally.mapping = (u32)kmalloc(lp->tally.mapping_size, GFP_KERNEL);
    if (lp->tx.rng_tx == NULL) {
        if (netif_msg_drv(lp))
            dev_err(&lp->ndev->dev, "alloc rng_tx fail.\n");
        errorCode = -ENOMEM;
        goto GKETH_start_hw_exit;
    }
    lp->tally.buf_size = 64;
    lp->tally.buffer   = (lp->tally.mapping + 63) & 0xFFFFFFC0;
    memset((void *)lp->tally.mapping, 0, lp->tally.mapping_size);

    spin_lock_irqsave(&lp->lock, flags);
    gkhw_set_desc(lp);
    gkhw_set_ccr(lp);
    gkhw_set_rxtx_ms(lp);
    gkhw_set_fifo(lp);
    gkhw_dma_tx_start(lp);
    gkhw_dma_rx_start(lp);
    GKETH_pre_tally(lp);
    gkhw_set_link_mode_speed(lp);
    gkhw_set_fpga_clk(lp);//use on fpga 100M 
    spin_unlock_irqrestore(&lp->lock, flags);

GKETH_start_hw_exit:
    return errorCode;
}

static void GKETH_stop_hw(struct net_device *ndev)
{
    struct GKETH_info           *lp;
    unsigned long               flags;
    
    lp = (struct GKETH_info *)netdev_priv(ndev);

    spin_lock_irqsave(&lp->lock, flags);
    gkhw_disable(lp);
    spin_unlock_irqrestore(&lp->lock, flags);

    GKETH_tx_rngmng_del(lp);
    if (lp->tx.desc_tx) {
        dma_free_coherent(&lp->ndev->dev,
            (sizeof(struct GKETH_desc) * lp->tx_count),
            lp->tx.desc_tx, lp->tx_dma_desc);
        lp->tx.desc_tx = NULL;
    }
    if (lp->tx.rng_tx) {
        kfree(lp->tx.rng_tx);
        lp->tx.rng_tx = NULL;
    }

    GKETH_rx_rngmng_del(lp);
    if (lp->rx.desc_rx) {
        dma_free_coherent(&lp->ndev->dev,
            (sizeof(struct GKETH_desc) * lp->rx_count),
            lp->rx.desc_rx, lp->rx_dma_desc);
        lp->rx.desc_rx = NULL;
    }
    if (lp->rx.rng_rx) {
        kfree(lp->rx.rng_rx);
        lp->rx.rng_rx = NULL;
    }
}

static int GKETH_open(struct net_device *ndev)
{
    int                 errorCode = 0;
    struct GKETH_info           *lp;
    
	if(!(cmdline_phytype != 0)) {
		GK_EPHY_POWER_ON();
		udelay(1000);
	}

    lp = (struct GKETH_info *)netdev_priv(ndev);

    errorCode = GKETH_start_hw(ndev);
    if (errorCode)
        goto GKETH_open_exit;

    errorCode = request_irq(ndev->irq, GKETH_interrupt,
        IRQF_SHARED | IRQF_TRIGGER_HIGH, ndev->name, ndev);
    if (errorCode) {
        if (netif_msg_drv(lp))
            dev_err(&lp->ndev->dev,
            "Request_irq[%d] fail.\n", ndev->irq);
        goto GKETH_open_exit;
    }

    napi_enable(&lp->napi);
    netif_start_queue(ndev);


    netif_carrier_off(ndev);


    gkhw_int_enable(lp);

    errorCode = GKETH_phy_start(lp);

GKETH_open_exit:
    if (errorCode) {
        printk("eth open error\n");
        GKETH_phy_stop(lp);
        GKETH_stop_hw(ndev);
    }

    return errorCode;
}

static int GKETH_stop(struct net_device *ndev)
{
    int                 errorCode = 0;
    struct GKETH_info           *lp;
    
    lp = (struct GKETH_info *)netdev_priv(ndev);

    netif_stop_queue(ndev);
    napi_disable(&lp->napi);
    flush_scheduled_work();
    GKETH_phy_stop(lp);
    GKETH_stop_hw(ndev);
    free_irq(ndev->irq, ndev);
	gk_phy_reset();
	//gkhw_sf_reset(lp);
	gkhw_mac_reset(lp);
	gk_phy_reset();

    //printk("GKETH_stop\n");
    GK_EPHY_POWER_OFF();

    return errorCode;
}

static void GKETH_timeout(struct net_device *ndev)
{
    struct GKETH_info           *lp;
    unsigned long               flags;
    u16                 irq_status;
    
    lp = (struct GKETH_info *)netdev_priv(ndev);

    dev_info(&lp->ndev->dev, "OOM Info:...\n");
    spin_lock_irqsave(&lp->lock, flags);
    irq_status = gk_eth_readw((unsigned int)(lp->regbase + ETH_REG_INT_STA_OFFSET));
    GKETH_interrupt_tx(lp, irq_status);
    gkhw_dump(lp);
    spin_unlock_irqrestore(&lp->lock, flags);

    netif_wake_queue(ndev);
}

static int GKETH_hard_start_xmit(struct sk_buff *skb, struct net_device *ndev)
{
    int                     errorCode = 0;
    struct GKETH_info      *lp;
    unsigned int            entry;
    unsigned int            dirty_to_tx;
    unsigned long           flags;
	unsigned char           proto = 0;
    
    lp = (struct GKETH_info *)netdev_priv(ndev);

    spin_lock_irqsave(&lp->lock, flags);
    dirty_to_tx = lp->tx.cur_tx - lp->tx.dirty_tx;
    if (dirty_to_tx >= lp->tx_count) {
        netif_stop_queue(ndev);
        errorCode = -ENOMEM;
        spin_unlock_irqrestore(&lp->lock, flags);
        goto GKETH_hard_start_xmit_exit;
    }
#ifdef GK_SUPPORT_TSO
    if (skb->protocol == __constant_htons(ETH_P_IP))
    {
        proto = ip_hdr(skb)->protocol;
        if (proto == IPPROTO_TCP)
        {
            errorCode = gkhw_l_desc(skb, lp);
            if(errorCode)
            {
                spin_unlock_irqrestore(&lp->lock, flags);
                goto GKETH_hard_start_xmit_exit;
            }
        }
        else
        {
            errorCode = gkhw_n_desc(skb, lp);
            if(errorCode)
            {
                spin_unlock_irqrestore(&lp->lock, flags);
                goto GKETH_hard_start_xmit_exit;
            }
        }
    }
    else
    {
        errorCode = gkhw_n_desc(skb, lp);
        if(errorCode)
        {
            spin_unlock_irqrestore(&lp->lock, flags);
            goto GKETH_hard_start_xmit_exit;
        }
    }
#else
    errorCode = gkhw_n_desc(skb, lp);
    if(errorCode)
    {
        spin_unlock_irqrestore(&lp->lock, flags);
        goto GKETH_hard_start_xmit_exit;
    }
#endif
    spin_unlock_irqrestore(&lp->lock, flags);
    gkhw_dma_tx_poll(lp);

    ndev->trans_start = jiffies;
    dev_vdbg(&lp->ndev->dev, "TX Info: cur_tx[%d], dirty_tx[%d], "
        "entry[%d], len[%d], data_len[%d], ip_summed[%d], "
        "csum_start[%d], csum_offset[%d].\n",
        lp->tx.cur_tx, lp->tx.dirty_tx, entry, skb->len, skb->data_len,
        skb->ip_summed, skb->csum_start, skb->csum_offset);
    
GKETH_hard_start_xmit_exit:
    return errorCode;
}

static struct net_device_stats *GKETH_get_stats(struct net_device *ndev)
{
    struct GKETH_info *lp = netdev_priv(ndev);
    
    return &lp->stats;
}

static inline void GKETH_check_rdes0_status(struct GKETH_info *lp,
    unsigned int status)
{
    if (status & ETH_RDES0_RES) {
        lp->stats.rx_errors++;
        if (netif_msg_rx_err(lp))
            dev_err(&lp->ndev->dev,
            "RX Error: Recive Error.\n");
    }
    if (status & (ETH_RDES0_RUNT | ETH_RDES0_RWT)) {
        lp->stats.rx_length_errors++;
        if (netif_msg_rx_err(lp))
            dev_err(&lp->ndev->dev,
            "RX Error: Frame Len Too Short.\n");
    }
    if (status & (ETH_RDES0_CRCER | ETH_RDES0_IPCKF | ETH_RDES0_UDPCKF | ETH_RDES0_TCPCKF)) {
        lp->stats.rx_crc_errors++;
        if (netif_msg_rx_err(lp))
            dev_err(&lp->ndev->dev,
            "RX Error: Recive CRC Error.\n");
    }
}

static inline void GKETH_napi_rx(struct GKETH_info *lp, u32 status, u32 entry)
{
    short                   pkt_len;
    struct sk_buff              *skb;
    dma_addr_t              mapping;
    struct net_device       *ndev;
	struct ethhdr *eth;

    ndev = lp->ndev;
    
    pkt_len = ETH_RDES0_FML(status);
	if(pkt_len == lp->rx.rng_rx[entry].buf_size)
	{
		mdelay(1);
    	pkt_len = ETH_RDES0_FML(lp->rx.desc_rx[entry].desc0);
	}
	
    if (unlikely(pkt_len > GKETH_RX_COPYBREAK)) {
        dev_warn(&lp->ndev->dev, "Goke Bogus packet size %d.\n", pkt_len);
        pkt_len = GKETH_RX_COPYBREAK;
        lp->stats.rx_length_errors++;
    }

    skb = lp->rx.rng_rx[entry].skb;
    mapping = lp->rx.rng_rx[entry].mapping;
    if (likely(skb && mapping)) {
        dma_unmap_single(&lp->ndev->dev, mapping,
            lp->rx.rng_rx[entry].buf_size, DMA_FROM_DEVICE);
        skb->len = 0;
        skb_put(skb, pkt_len);
#ifdef GK_SUPPORT_TSO
        if(ndev->features & NETIF_F_RXCSUM)
        {
            if (status & (ETH_RDES0_UDPTP | ETH_RDES0_TCPTP))
            {
                if(!(status & (ETH_RDES0_CRCER | ETH_RDES0_IPCKF | \
                    ETH_RDES0_UDPCKF | ETH_RDES0_TCPCKF)))
                    skb->ip_summed = CHECKSUM_UNNECESSARY;
            }
            else
                skb_checksum_none_assert(skb);
        }
        
        if(status & ETH_RDES0_MAR)
            skb->pkt_type = PACKET_BROADCAST;
        else if( status & ETH_RDES0_BAR)
            skb->pkt_type = PACKET_MULTICAST;
        else if(status & ETH_RDES0_PAM)
            skb->pkt_type = PACKET_HOST;
        
        skb_reset_mac_header(skb);
        skb_pull_inline(skb, ETH_HLEN);
        eth = eth_hdr(skb);
        if (ntohs(eth->h_proto) >= 1536)
            skb->protocol = eth->h_proto;
        else if (skb->len >= 2 && *(unsigned short *)(skb->data) == 0xFFFF)
            skb->protocol = htons(ETH_P_802_3);
        else
            skb->protocol = htons(ETH_P_802_2);
#else
        skb->protocol = eth_type_trans(skb, lp->ndev);
#endif
        netif_receive_skb(skb);
        lp->rx.rng_rx[entry].skb = NULL;
        lp->rx.rng_rx[entry].mapping = 0;
        lp->ndev->last_rx = jiffies;
        lp->stats.rx_packets++;
        lp->stats.rx_bytes += pkt_len;
    } else {
        if (netif_msg_drv(lp)) {
            dev_err(&lp->ndev->dev,
            "RX Error: %d skb[%p], map[0x%08X].\n",
            entry, skb, mapping);
            gkhw_dump(lp);
        }
    }
}

int GKETH_napi(struct napi_struct *napi, int budget)
{
    int                 rx_budget = budget;
    struct GKETH_info           *lp;
    u32                 entry;
    u32                 status;
    unsigned long               flags;
    
    lp = container_of(napi, struct GKETH_info, napi);

    if (unlikely(!netif_carrier_ok(lp->ndev)))
        goto GKETH_poll_complete;
    while (rx_budget > 0) {
        entry = lp->rx.cur_rx % lp->rx_count;
        status = lp->rx.desc_rx[entry].desc0;
        if (status & ETH_RDES0_OWN)
            break;
        if (unlikely((status & (ETH_RDES0_FS | ETH_RDES0_LS)) !=
            (ETH_RDES0_FS | ETH_RDES0_LS))) {
            if (netif_msg_probe(lp)) {
                dev_err(&lp->ndev->dev, "RX Error: Wrong FS/LS"
                " cur_rx[%d] status 0x%08x.\n",
                lp->rx.cur_rx, status);
                gkhw_dump(lp);
            }
            break;
        }
        if (likely((status & ETH_RDES0_RES) != ETH_RDES0_RES))
        {
            GKETH_napi_rx(lp, status, entry);
        }
        else
        {
            GKETH_check_rdes0_status(lp, status);
        }
        rx_budget--;
        lp->rx.cur_rx++;

        if ((lp->rx.cur_rx - lp->rx.dirty_rx) > (lp->rx_count / 4))
            GKETH_rx_rngmng_refill(lp);
    }

GKETH_poll_complete:
    if (rx_budget > 0) {
        GKETH_rx_rngmng_refill(lp);
        spin_lock_irqsave(&lp->lock, flags);
		if(lp->rx.intstatus & (ETH_REG_INT_STA_RXDU | ETH_REG_INT_STA_RXFIFOOV))
		{
			GKETH_rx_rngmng_init(lp);
			if(lp->rx.intstatus & (ETH_REG_INT_STA_RXFIFOOV))
			{
        		lp->rx.cur_rx	= 1;
        		lp->rx.dirty_rx = 1;
			}
		}
		lp->rx.intstatus = 0;
        napi_complete(&lp->napi);
        gkhw_dma_rx_start(lp);
        gkhw_int_enable(lp);
        spin_unlock_irqrestore(&lp->lock, flags);
    }

    return (budget - rx_budget);
}

static int GKETH_set_mac_address(struct net_device *ndev, void *addr)
{
    struct GKETH_info           *lp;
    unsigned long               flags;
    struct sockaddr             *saddr;
    
    lp = (struct GKETH_info *)netdev_priv(ndev);
    spin_lock_irqsave(&lp->lock, flags);
    saddr = (struct sockaddr *)(addr);

    if (!is_valid_ether_addr(saddr->sa_data))
        goto fail2;

    dev_dbg(&lp->ndev->dev, "MAC address[%pM].\n", saddr->sa_data);

    memcpy(ndev->dev_addr, saddr->sa_data, ndev->addr_len);
    gkhw_set_hwaddr(lp, ndev->dev_addr);
    gkhw_get_hwaddr(lp, ndev->dev_addr);
    memcpy(lp->platform_info->mac_addr, ndev->dev_addr, GKETH_MAC_SIZE);

    spin_unlock_irqrestore(&lp->lock, flags);

    return 0;

fail2:
    spin_unlock_irqrestore(&lp->lock, flags);
    return -EADDRNOTAVAIL;
}

static void GKETH_set_multicast_list(struct net_device *ndev)
{
}

#ifdef CONFIG_NET_POLL_CONTROLLER
static void GKETH_poll_controller(struct net_device *ndev)
{
    GKETH_interrupt(ndev->irq, ndev);
}
#endif

static int GKETH_get_settings(struct net_device *ndev,
    struct ethtool_cmd *ecmd)
{
    int                 errorCode = 0;
    struct GKETH_info           *lp;
    unsigned long               flags;

    if (!netif_running(ndev)) {
        errorCode = -EINVAL;
        goto GKETH_get_settings_exit;
    }

    lp = (struct GKETH_info *)netdev_priv(ndev);
    spin_lock_irqsave(&lp->lock, flags);
    if (lp->phydev) {
        errorCode = phy_ethtool_gset(lp->phydev, ecmd);
    } else {
        errorCode = -EINVAL;
    }
    spin_unlock_irqrestore(&lp->lock, flags);

GKETH_get_settings_exit:
    return errorCode;
}

static int GKETH_set_settings(struct net_device *ndev,
    struct ethtool_cmd *ecmd)
{
    int                 errorCode = 0;
    struct GKETH_info           *lp;
    unsigned long               flags;

    if (!netif_running(ndev)) {
        errorCode = -EINVAL;
        goto GKETH_get_settings_exit;
    }

    lp = (struct GKETH_info *)netdev_priv(ndev);
    spin_lock_irqsave(&lp->lock, flags);
    if (lp->phydev) {
        errorCode = phy_ethtool_sset(lp->phydev, ecmd);
    } else {
        errorCode = -EINVAL;
    }
    spin_unlock_irqrestore(&lp->lock, flags);

GKETH_get_settings_exit:
    return errorCode;
}

static int GKETH_ioctl(struct net_device *ndev, struct ifreq *ifr, int cmd)
{
    int                 errorCode = 0;
    struct GKETH_info           *lp;
    unsigned long               flags;

    if (!netif_running(ndev)) {
        errorCode = -EINVAL;
        goto GKETH_get_settings_exit;
    }

    lp = (struct GKETH_info *)netdev_priv(ndev);
    spin_lock_irqsave(&lp->lock, flags);
    if (lp->phydev) {
        errorCode = phy_mii_ioctl(lp->phydev, ifr, cmd);
    } else {
        errorCode = -EINVAL;
    }
    spin_unlock_irqrestore(&lp->lock, flags);

GKETH_get_settings_exit:
    return errorCode;
}

/* ================================================================= */

static const struct net_device_ops GKETH_netdev_ops = {
    .ndo_open       = GKETH_open,
    .ndo_stop       = GKETH_stop,
    .ndo_start_xmit     = GKETH_hard_start_xmit,
    .ndo_set_mac_address    = GKETH_set_mac_address,
    .ndo_validate_addr  = eth_validate_addr,
    .ndo_do_ioctl       = GKETH_ioctl,
    //.ndo_change_mtu     = eth_change_mtu,
    .ndo_change_mtu     = GKETH_change_mtu,
    .ndo_tx_timeout     = GKETH_timeout,
    .ndo_get_stats      = GKETH_get_stats,
    .ndo_set_rx_mode    = GKETH_set_multicast_list,
#ifdef CONFIG_NET_POLL_CONTROLLER
    .ndo_poll_controller    = GKETH_poll_controller,
#endif
};

static const struct ethtool_ops GKETH_ethtool_ops = {
    .get_settings   = GKETH_get_settings,
    .set_settings   = GKETH_set_settings,
    .get_link   = ethtool_op_get_link,
};

static int __devinit GKETH_drv_probe(struct platform_device *pdev)
{
    int                 errorCode = 0;
    struct net_device           *ndev;
    struct GKETH_info           *lp;
    struct gk_eth_platform_info *platform_info;
    struct resource             *reg_res;
    struct resource             *irq_res;

    void __iomem        *eth_base;
    int                 i;

    platform_info = (struct gk_eth_platform_info *)pdev->dev.platform_data;
    if (platform_info == NULL) {
        dev_err(&pdev->dev, "Can't get platform_data!\n");
        errorCode = - EPERM;
        goto GKETH_drv_probe_exit;
    }

    if (platform_info->is_enabled) {
        if (!platform_info->is_enabled()) {
            dev_err(&pdev->dev, "Not enabled, check HW config!\n");
            errorCode = -EPERM;
            goto GKETH_drv_probe_exit;
        }
    }

    reg_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (reg_res == NULL) {
        dev_err(&pdev->dev, "Get reg_res failed!\n");
        errorCode = -ENXIO;
        goto GKETH_drv_probe_exit;
    }

    irq_res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
    if (irq_res == NULL) {
        dev_err(&pdev->dev, "Get irq_res failed!\n");
        errorCode = -ENXIO;
        goto GKETH_drv_probe_exit;
    }

    ndev = alloc_etherdev(sizeof(struct GKETH_info));
    if (ndev == NULL) {
        dev_err(&pdev->dev, "alloc_etherdev fail.\n");
        errorCode = -ENOMEM;
        goto GKETH_drv_probe_exit;
    }
	eth_base = (void __iomem *)reg_res->start;

#define KE_DEBUG
#ifdef KE_DEBUG
    printk("[%s] eth_base = 0x%08x\n", __FUNCTION__, (u32)eth_base);
#endif

    SET_NETDEV_DEV(ndev, &pdev->dev);
    ndev->dev.dma_mask = pdev->dev.dma_mask;
    ndev->dev.coherent_dma_mask = pdev->dev.coherent_dma_mask;
    ndev->irq = irq_res->start;
#if defined(CONFIG_GK_ETH_SUPPORT_IPC)
    ndev->features = NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM;
#endif

#ifdef GK_SUPPORT_TSO
    ndev->features |= NETIF_F_SG | NETIF_F_HW_CSUM | NETIF_F_TSO | NETIF_F_RXCSUM;
	netif_set_gso_max_size(ndev,64512);
#endif
    lp = netdev_priv(ndev);
    spin_lock_init(&lp->lock);
    lp->ndev = ndev;
    lp->regbase = (unsigned char __iomem *)eth_base;
    lp->platform_info = platform_info;
    lp->rx_count = lp->platform_info->default_rx_ring_size;
    lp->tx_count = lp->platform_info->default_tx_ring_size;
    lp->tx_irq_count = (lp->tx_count * 2) / 3;
    lp->msg_enable = netif_msg_init(msg_level, NETIF_MSG_DRV | NETIF_MSG_LINK);

    lp->new_bus.name = "goke MII Bus",
    lp->new_bus.read = &gkhw_mdio_read;
    lp->new_bus.write = &gkhw_mdio_write;
    lp->new_bus.reset = &gkhw_mdio_reset;
    //pdev->id = 0x2;
    //lp->platform_info->mii_id = pdev->id;
    printk("mii id = %d \n", pdev->id);
    snprintf(lp->new_bus.id, MII_BUS_ID_SIZE, "%x", pdev->id);
    lp->new_bus.priv = lp;
    lp->new_bus.irq = kmalloc(sizeof(int)*PHY_MAX_ADDR, GFP_KERNEL);
    if (lp->new_bus.irq == NULL) {
        dev_err(&pdev->dev, "alloc new_bus.irq fail.\n");
        errorCode = -ENOMEM;
        goto GKETH_drv_probe_free_mii_gpio_irq;
    }
    for(i = 0; i < PHY_MAX_ADDR; ++i)
        lp->new_bus.irq[i] = PHY_POLL;
    lp->new_bus.parent = &pdev->dev;
    lp->new_bus.state = MDIOBUS_ALLOCATED;
//#ifndef CONFIG_SYSTEM_USE_EXTERN_ETHPHY
    if(cmdline_phytype == 0){
        lp->new_bus.phy_mask = 0xFFFFFFFE;
    }
//#endif

    //printk("mdiobus register ...\n");
    errorCode = mdiobus_register(&lp->new_bus);
    if (errorCode) {
        dev_err(&pdev->dev, "mdiobus_register fail%d.\n", errorCode);
        goto GKETH_drv_probe_kfree_mdiobus;
    }
    if(cmdline_phytype != 0)
    {
        GH_EPHY_set_MII_RMII_rmii(0x0);
        // RMII mode GPIO_TYPE_OUTPUT_ENET_PHY_TXD_3 used as 10/100M indicator
        gk_set_phy_speed_led(GPIO_TYPE_INPUT_0);
		//EPHY POWER OFF
		//printk("net probe ephy power off\n");
		GK_EPHY_POWER_OFF();
    }
    else
    {
        u8 uRegVal;
        //GH_EPHY_set_MII_RMII_rmii(0x0);
		gk_eth_writel(GK_VA_ETH_PHY + 0x0540, 0x00000000);
        gk_eth_writel(GK_VA_ETH_PHY + 0x00C8, 0x0000C400);    // reg_test_out(debug_bus_out)
        gk_eth_writel(GK_VA_ETH_PHY + 0x00E0, 0x0000810A);    // debug mode
        gk_eth_writel(GK_VA_ETH_PHY + 0x0588, 0x00000007);    // DAC 100M clk gate for 10M TX
        MHal_EMAC_WritReg8(0x0033, 0xde, 0x59);
        MHal_EMAC_WritReg8(0x0033, 0xf4, 0x21);
        MHal_EMAC_WritReg8(0x0032, 0x72, 0x80);
        MHal_EMAC_WritReg8(0x0033, 0xfc, 0x00);
        MHal_EMAC_WritReg8(0x0033, 0xfd, 0x00);
        MHal_EMAC_WritReg8(0x0033, 0xb7, 0x07);
        MHal_EMAC_WritReg8(0x0033, 0xcb, 0x11);
        MHal_EMAC_WritReg8(0x0033, 0xcc, 0x80);
        MHal_EMAC_WritReg8(0x0033, 0xcd, 0xd1);
        MHal_EMAC_WritReg8(0x0033, 0xd4, 0x00);
        MHal_EMAC_WritReg8(0x0033, 0xb9, 0x40);
        MHal_EMAC_WritReg8(0x0033, 0xbb, 0x05);
        MHal_EMAC_WritReg8(0x0033, 0xea, 0x46);
        MHal_EMAC_WritReg8(0x0033, 0xa1, 0x00);
        MHal_EMAC_WritReg8(0x0034, 0x3a, 0x03);
        MHal_EMAC_WritReg8(0x0034, 0x3b, 0x00);

        //gain shift
        MHal_EMAC_WritReg8(0x0033, 0xb4, 0x56);

        //det max
        MHal_EMAC_WritReg8(0x0033, 0x4f, 0x02);

        //det min
        MHal_EMAC_WritReg8(0x0033, 0x51, 0x01);

        //snr len (emc noise)
        MHal_EMAC_WritReg8(0x0033, 0x77, 0x18);

        //snr k value
        MHal_EMAC_WritReg8(0x0033, 0x43, 0x15);

        //100 gat
        MHal_EMAC_WritReg8(0x0034, 0xc5, 0x00);

        //200 gat
        MHal_EMAC_WritReg8(0x0034, 0x30, 0x43);

        //en_100t_phase
        MHal_EMAC_WritReg8(0x0034, 0x39, 0x41);

        //10T waveform
        uRegVal = MHal_EMAC_ReadReg8(0x0034, 0xe8);
        uRegVal &= 0xf8;
        uRegVal |= 0x06;
        MHal_EMAC_WritReg8(0x0034, 0xe8, uRegVal);
        MHal_EMAC_WritReg8(0x0032, 0x2b, 0x00);

        //analog
        MHal_EMAC_WritReg8(0x0033, 0xd8, 0xb0);
        MHal_EMAC_WritReg8(0x0033, 0xd9, 0x30);

        //disable EEE
        uRegVal = MHal_EMAC_ReadReg8(0x0032, 0x2d);
        uRegVal |= 0x40;
        MHal_EMAC_WritReg8(0x0032, 0x2d, uRegVal);

        GH_EPHY_set_SPEED_ane(0x01);
    }

    ether_setup(ndev);
    ndev->netdev_ops = &GKETH_netdev_ops;
    ndev->watchdog_timeo = lp->platform_info->watchdog_timeo;
    netif_napi_add(ndev, &lp->napi, GKETH_napi,
        lp->platform_info->napi_weight);
    if (!is_valid_ether_addr(lp->platform_info->mac_addr))
        random_ether_addr(lp->platform_info->mac_addr);
    memcpy(ndev->dev_addr, lp->platform_info->mac_addr, GKETH_MAC_SIZE);

    SET_ETHTOOL_OPS(ndev, &GKETH_ethtool_ops);
    errorCode = register_netdev(ndev);
    if (errorCode) {
        dev_err(&pdev->dev, " register_netdev fail%d.\n", errorCode);
        goto GKETH_drv_probe_netif_napi_del;
    }

    platform_set_drvdata(pdev, ndev);
    dev_notice(&pdev->dev, "MAC Address[%pM].\n", ndev->dev_addr);

    goto GKETH_drv_probe_exit;

GKETH_drv_probe_netif_napi_del:
    netif_napi_del(&lp->napi);
    mdiobus_unregister(&lp->new_bus);

GKETH_drv_probe_kfree_mdiobus:
    kfree(lp->new_bus.irq);

GKETH_drv_probe_free_mii_gpio_irq:
    //iounmap(eth_base);

//GKTH_drv_probe_free_res:
    //release_mem_region(reg_res->start, size);


//GKETH_drv_probe_free_netdev:
    free_netdev(ndev);

GKETH_drv_probe_exit:
    return errorCode;
}

static int __devexit GKETH_drv_remove(struct platform_device *pdev)
{
    struct net_device           *ndev;
    struct GKETH_info           *lp;

    ndev = platform_get_drvdata(pdev);
    if (ndev) {
        lp = (struct GKETH_info *)netdev_priv(ndev);
        unregister_netdev(ndev);
        netif_napi_del(&lp->napi);
        mdiobus_unregister(&lp->new_bus);
        kfree(lp->new_bus.irq);
        platform_set_drvdata(pdev, NULL);
        free_netdev(ndev);
        dev_notice(&pdev->dev, "Removed.\n");
    }

    return 0;
}

#ifdef CONFIG_PM
static int GKETH_drv_suspend(struct platform_device *pdev, pm_message_t state)
{
    int                 errorCode = 0;
    struct net_device           *ndev;
    struct GKETH_info           *lp;
    unsigned long               flags;

    ndev = platform_get_drvdata(pdev);
    if (ndev) {
        if (!netif_running(ndev))
            goto GKETH_drv_suspend_exit;

        lp = (struct GKETH_info *)netdev_priv(ndev);

        napi_disable(&lp->napi);
        netif_device_detach(ndev);
        disable_irq(ndev->irq);

        spin_lock_irqsave(&lp->lock, flags);
        gkhw_disable(lp);
        spin_unlock_irqrestore(&lp->lock, flags);
    }

GKETH_drv_suspend_exit:
    dev_dbg(&pdev->dev, "%s exit with %d @ %d\n",
        __func__, errorCode, state.event);
    return errorCode;
}

static int GKETH_drv_resume(struct platform_device *pdev)
{
    int                 errorCode = 0;
    struct net_device           *ndev;
    struct GKETH_info           *lp;
    unsigned long               flags;

    ndev = platform_get_drvdata(pdev);
    if (ndev) {
        if (!netif_running(ndev))
            goto GKETH_drv_resume_exit;

        lp = (struct GKETH_info *)netdev_priv(ndev);

        spin_lock_irqsave(&lp->lock, flags);
        errorCode = gkhw_enable(lp);
        gkhw_set_link_mode_speed(lp);
        GKETH_rx_rngmng_init(lp);
        GKETH_tx_rngmng_init(lp);
        gkhw_set_desc(lp);
        gkhw_dma_rx_start(lp);
        gkhw_dma_tx_start(lp);
        gkhw_int_enable(lp);
        spin_unlock_irqrestore(&lp->lock, flags);

        if (errorCode) {
            dev_err(&pdev->dev, "gkhw_enable.\n");
        } else {
            enable_irq(ndev->irq);
            netif_device_attach(ndev);
            napi_enable(&lp->napi);
        }
    }

GKETH_drv_resume_exit:
    dev_dbg(&pdev->dev, "%s exit with %d\n", __func__, errorCode);
    return errorCode;
}
#endif

static struct platform_driver GKETH_driver = {
    .probe      = GKETH_drv_probe,
    .remove     = __devexit_p(GKETH_drv_remove),
#ifdef CONFIG_PM
    .suspend        = GKETH_drv_suspend,
    .resume     = GKETH_drv_resume,
#endif
    .driver = {
        .name   = "gk-eth",
        .owner  = THIS_MODULE,
    },
};

static int __init GKETH_init(void)
{
    return platform_driver_register(&GKETH_driver);
}

static void __exit GKETH_exit(void)
{
    platform_driver_unregister(&GKETH_driver);
}

module_init(GKETH_init);
module_exit(GKETH_exit);

MODULE_DESCRIPTION("Goke GK Ethernet Driver");
MODULE_AUTHOR("Goke Microelectronics Inc.");
MODULE_LICENSE("GPL");

