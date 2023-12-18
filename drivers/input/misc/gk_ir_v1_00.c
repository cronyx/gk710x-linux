/*
 * drivers/input/misc/gk_ir_v1_00.c
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
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/input.h>

#include <asm/irq.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/mach-types.h>

#include <mach/hardware.h>
#include <mach/ir.h>
#include <mach/io.h>
#include <mach/irqs.h>
#include <mach/rct.h>

#include <plat/input.h>
#if defined(CONFIG_SYSTEM_USE_NETLINK_MSG_IR) || defined(CONFIG_SYSTEM_USE_NETLINK_MSG_433_KEY) || defined(CONFIG_SYSTEM_USE_NETLINK_MSG_433_GATE)
#include <plat/gk_net_link.h>
#endif


#define BOARD_FPGA  1
#ifdef BOARD_FPGA
#define IR_INPUT_CLK                24000000    // for 27MHZ Crystal
#else
#define IR_INPUT_CLK                24000000    // for 24MHZ Crystal
#endif

#define IR_FREQ                     38          // for NEC unit:MHz

#define IRR_MAX_DATA        0x0100
static u16      IrBuffer[IRR_MAX_DATA]  = {0};
static u16      IrWritePointer          = 0;
static u16      IrDataNumber            = 0;        //number of the received data.
static u16      IrSeqNumber             = 0;            //number of the sequence command
static u16      IrReadPointer           = 0;
static u8       IrBufferOver            = 0;
static u8       IrBufferOverApi         = 0;
static u8       IRDataAvailable         = 0;

#if defined(CONFIG_INPUT_GOKE_IR)
static uint32_t lastKey                 = 0;
static uint32_t lastDeviceId            = 0;
static uint32_t lastRepeatFlag          = 0;
#endif

/* ========================================================================= */
#define MAX_IR_BUFFER               (512)
#define HW_FIFO_BUFFER              (48)
#define MSC_GKIRKEYINFO         0x5
#define GK_IR_KEYPRESS_TIMEOUT  300

static struct gk_key_table keymap[] =
{
    {GKINPUT_IR_KEY,    {.ir_key    = {KEY_POWER,    3,    0x0b}}},    //POWER
    {GKINPUT_IR_KEY,    {.ir_key    = {KEY_0,        3,    0x0c}}},    //KEY 0
    {GKINPUT_IR_KEY,    {.ir_key    = {KEY_1,        3,    0x1b}}},    //KEY 1
    {GKINPUT_IR_KEY,    {.ir_key    = {KEY_2,        3,    0x0f}}},    //KEY 2
    {GKINPUT_IR_KEY,    {.ir_key    = {KEY_3,        3,    0x03}}},    //KEY 3
    {GKINPUT_IR_KEY,    {.ir_key    = {KEY_4,        3,    0x19}}},    //KEY 4
    {GKINPUT_IR_KEY,    {.ir_key    = {KEY_5,        3,    0x11}}},    //KEY 5
    {GKINPUT_IR_KEY,    {.ir_key    = {KEY_6,        3,    0x01}}},    //KEY 6
    {GKINPUT_IR_KEY,    {.ir_key    = {KEY_7,        3,    0x09}}},    //KEY 7
    {GKINPUT_IR_KEY,    {.ir_key    = {KEY_8,        3,    0x1d}}},    //KEY 8
    {GKINPUT_IR_KEY,    {.ir_key    = {KEY_9,        3,    0x0d}}},    //KEY 9

    {GKINPUT_END},
};

/* ========================================================================= */
struct gk_ir_frame_info
{
    u32     frame_head_size;
    u32     frame_data_size;
    u32     frame_end_size;
    u32     frame_repeat_head_size;
};

struct gk_ir_info
{
    struct input_dev        *pinput_dev;
    unsigned char __iomem   *regbase;

    u32                     id;

    unsigned int            irq;

    int (*ir_parse)(struct gk_ir_info *pirinfo, u32 *uid);
    int                ir_pread;
    int                ir_pwrite;
    u16                tick_buf[MAX_IR_BUFFER];

    struct gk_ir_frame_info     frame_info;
    u32                 frame_data_to_received;

    u32                 last_ir_uid;
    u32                 last_ir_flag;
    spinlock_t          keylock;

    struct gk_key_table     *pkeymap;
    struct gk_ir_controller *pcontroller_info;
};

typedef enum
{
    GD_IR_RECEIVER = 1,   //!< Receiver function.
    GD_IR_TRANSMITTER,    //!< Transmitter function.
} GD_IR_TYPE_E;

/* ========================================================================= */
static void gk_ir_disable(struct gk_ir_info *pirinfo)
{
    disable_irq(pirinfo->irq);
}

static void gk_ir_enable(struct gk_ir_info *pirinfo)
{
    u32 edges;

    pirinfo->frame_data_to_received = pirinfo->frame_info.frame_head_size
                                      + pirinfo->frame_info.frame_data_size;

    BUG_ON(pirinfo->frame_data_to_received > MAX_IR_BUFFER);

    if (pirinfo->frame_data_to_received > HW_FIFO_BUFFER)
    {
        edges = HW_FIFO_BUFFER;
        pirinfo->frame_data_to_received -= HW_FIFO_BUFFER;
    }
    else
    {
        edges = pirinfo->frame_data_to_received;
        pirinfo->frame_data_to_received = 0;
    }

    enable_irq(pirinfo->irq);
}

#if defined(CONFIG_INPUT_GOKE_IR)
static int gk_ir_get_tick_size(struct gk_ir_info *pirinfo)
{
    int                size = 0;

    if (pirinfo->ir_pread > pirinfo->ir_pwrite)
        size = MAX_IR_BUFFER - pirinfo->ir_pread + pirinfo->ir_pwrite;
    else
        size = pirinfo->ir_pwrite - pirinfo->ir_pread;

    return size;
}

void gk_ir_inc_read_ptr(struct gk_ir_info *pirinfo)
{
    BUG_ON(pirinfo->ir_pread == pirinfo->ir_pwrite);

    pirinfo->ir_pread++;
    if (pirinfo->ir_pread >= MAX_IR_BUFFER)
        pirinfo->ir_pread = 0;
}

void gk_ir_move_read_ptr(struct gk_ir_info *pirinfo, int offset)
{
    for (; offset > 0; offset--)
    {
        gk_ir_inc_read_ptr(pirinfo);
    }
}

static u16 gk_ir_read_data(    struct gk_ir_info *pirinfo, int pointer)
{
    BUG_ON(pointer < 0);
    BUG_ON(pointer >= MAX_IR_BUFFER);
    BUG_ON(pointer == pirinfo->ir_pwrite);

    return pirinfo->tick_buf[pointer];
}

static inline void gk_ir_write_data(struct gk_ir_info *pirinfo, u16 val)
{
    BUG_ON(pirinfo->ir_pwrite < 0);
    BUG_ON(pirinfo->ir_pwrite >= MAX_IR_BUFFER);

    pirinfo->tick_buf[pirinfo->ir_pwrite] = val;

    pirinfo->ir_pwrite++;

    if (pirinfo->ir_pwrite >= MAX_IR_BUFFER)
        pirinfo->ir_pwrite = 0;
}

#if 0
static int gk_input_report_ir(struct gk_ir_info *pirinfo, u32 uid)
{

    if (!pirinfo->pkeymap)
        return -1;

    if ((pirinfo->last_ir_uid == uid) && (pirinfo->last_ir_flag))
    {
        pirinfo->last_ir_flag--;
        return 0;
    }
    pirinfo->last_ir_uid = uid;

    //input_report_key(pirinfo->pinput_dev, pirinfo->pkeymap[i].ir_key.key_code, 1);
    //input_report_key(pirinfo->pinput_dev, pirinfo->pkeymap[i].ir_key.key_code, 0);
    //printk( "IR_KEY [%d]\n", pirinfo->pkeymap[i].ir_key.key_code);

    return 0;
}
#endif


#include "gk_ir_nec.c"
void gk_ir_set_protocol(struct gk_ir_info *pirinfo,
                            enum gk_ir_protocol protocol_id)
{
    memset(pirinfo->tick_buf, 0x0, sizeof(pirinfo->tick_buf));
    pirinfo->ir_pread  = 0;
    pirinfo->ir_pwrite = 0;

    switch (protocol_id)
    {
    case GK_IR_PROTOCOL_NEC:
    default:
        printk("Protocol NEC[%d]\n", protocol_id);
        pirinfo->ir_parse = gk_ir_nec_parse;
        gk_ir_get_nec_info(&pirinfo->frame_info);
        break;
    }
}
#endif

static void gk_ir_init(struct gk_ir_info *pirinfo)
{
    u16 prescaler;
    int i;

    prescaler = (u16)((IR_INPUT_CLK/(IR_FREQ * 1000)) - 1);

    gk_ir_writel( REG_PMU_IRQ_EN, 0x0);
    gk_ir_writel( REG_PMU_IRQ_CLR_RTC, IRQ_EN_RTC);
    gk_ir_writel( REG_PMU_IRQ_CLR_IRR, IRQ_EN_IRR);
    gk_ir_writel( REG_PMU_IRQ_CLR_FPC, IRQ_EN_FPC);
    gk_ir_writel( REG_PMU_IRQ_CLR_GPIO, IRQ_EN_GPIO);
    gk_ir_writel( REG_PMU_IRQ_CLR_CEC, IRQ_EN_CEC);
    gk_ir_writel( REG_PMU_IRQ_CLR_ADC, IRQ_EN_ADC);
    gk_ir_writel( REG_PMU_IRQ_CLR_IRT, IRQ_EN_IRT);

#if 0// for pmu gpio test
    // ALL_EDGE
    gk_ir_setbitsl(PMU_GPIO_AFSEL, 1 << PMU_GPIO_2);        // PIN_SW_USED
    gk_ir_setbitsl(PMU_GPIO_DIR, 1 << PMU_GPIO_2);          // INPUT
    gk_ir_clrbitsl(PMU_GPIO_IS, 1 << PMU_GPIO_2);           // EDGE_TRIGGER
    gk_ir_setbitsl(PMU_GPIO_IBE, 1 << PMU_GPIO_2);          // ALL_EDGE
    gk_ir_setbitsl(PMU_GPIO_IE, 1 << PMU_GPIO_2);           // Enable

    // ALL_EDGE
    gk_ir_setbitsl(PMU_GPIO_AFSEL, 1 << PMU_GPIO_3);        // PIN_SW_USED
    gk_ir_setbitsl(PMU_GPIO_DIR, 1 << PMU_GPIO_3);          // INPUT
    gk_ir_clrbitsl(PMU_GPIO_IS, 1 << PMU_GPIO_3);           // EDGE_TRIGGER
    gk_ir_setbitsl(PMU_GPIO_IBE, 1 << PMU_GPIO_3);          // ALL_EDGE
    gk_ir_setbitsl(PMU_GPIO_IE, 1 << PMU_GPIO_3);           // Enable
    //
    gk_ir_writel( REG_PMU_IRQ_EN, IRQ_EN_IRT | IRQ_EN_RTC | IRQ_EN_IRR | IRQ_EN_GPIO);
#else
    gk_ir_writel( REG_PMU_IRQ_EN, IRQ_EN_IRT | IRQ_EN_RTC | IRQ_EN_IRR);
#endif

    gk_ir_disable(pirinfo);

    gk_ir_writel( REG_IRR_PROG, 0x08);
    gk_ir_writel( REG_IRR_PRESCALER_H, (u8)(((prescaler) >> 8) & 0xff));
    gk_ir_writel( REG_IRR_PRESCALER_L, (u8)(prescaler & 0xff));

#if defined(CONFIG_INPUT_GOKE_433)
    gk_ir_writel( REG_IRR_PROG, 0x07);  // �������½�����Ч����������жϣ����ڴ���433 wifi
#elif defined(CONFIG_INPUT_GOKE_IR)
    gk_ir_writel( REG_IRR_PROG, 0x02);  // ��������Ч�����ڴ������
#endif
    gk_ir_writel( REG_IRR_IRQMASK_H, 0xFF);
    gk_ir_writel( REG_IRR_IRQMASK_L, 0xFF);

    IrDataNumber    = 0;
    IrSeqNumber     = 0;
    IrReadPointer   = 0;
    IrWritePointer  = 0;
    IrBufferOver    = 0;
    IRDataAvailable = 0;
    memset(&IrBuffer, 0, sizeof(IrBuffer));

    if (pirinfo->pcontroller_info->protocol >= GK_IR_PROTOCOL_END)
        pirinfo->pcontroller_info->protocol = GK_IR_PROTOCOL_NEC;
#if defined(CONFIG_INPUT_GOKE_IR)
    gk_ir_set_protocol(pirinfo, pirinfo->pcontroller_info->protocol);
#endif

    gk_ir_enable(pirinfo);

    for(i=0; i<sizeof(keymap)/sizeof(struct gk_key_table); i++)
    {
        set_bit(EV_KEY, pirinfo->pinput_dev->evbit);
        set_bit(pirinfo->pkeymap[i].ir_key.raw_id,
                pirinfo->pinput_dev->keybit);
    }
}

typedef union   /* IRR_IRR_Read */
{
    u32 all;
    struct
    {
        u32 irr_pulse27                 : 6;
        u32 edge                        : 1;
        u32 valid                       : 1;
        u32                             : 24;
    } bitc;
} GH_IRR_IRR_READ_S;

static u16 ir_count2time(u32 count)
{
    return (u16)((10000 * count) /(IR_FREQ * 10));
}


#if defined(CONFIG_INPUT_GOKE_IR)
/*!
*******************************************************************************
**
** \brief  Checks the status of an instance.
**
**         This function returns the status the of the receiver or transmitter.
**         If receiving is complete, it returns the number of received data.
**         If transmitting is complete, it returns the number of transmitted
**         data.
**
** \param  handle  Handle of receiver or transmitter to check.
** \param  pNum    Number of time data stored in the IR buffer.
**
** \return Possible return codes:
**         - #GD_OK
**         - #GD_ERR_IR_NO_SEQUENCE          Receiving is not completed yet.
**         - #GD_ERR_OUT_OF_MEMORY           Receiver buffer overflow
**         - #GD_ERR_IR_TRANSMIT_INPROCESS   Transmitting is not yet complete.
**         - #GD_ERR_INVALID_HANDLE
**
******************************************************************************/

int GD_IR_Status(GD_IR_TYPE_E handle, u16* pNum)
{
    u32 ReturnValue;

    if(handle == GD_IR_RECEIVER )
    {
        ReturnValue = -1;
        if (IrDataNumber)
            ReturnValue = 0;
        if (IrBufferOverApi)
        {
            IrBufferOverApi = 0;
            ReturnValue = -1;
        }
        *pNum = IrSeqNumber;
        return ReturnValue;
    }
    else
        return -1;
}


/*!
*******************************************************************************
**
** \brief  Reads data from the IR receice buffer.
**
** \param  handle  Handle of IR receiver.
** \param  pData   Pointer to data array.
** \param  num     Number of data to be read.
**
** \return Possible return codes:
**         - #GD_OK
**         - #GD_ERR_INVALID_HANDLE
**
******************************************************************************/
int GD_IR_Receive(GD_IR_TYPE_E handle, u16 *pData, u16 num)
{
    u16 TimeData;
    u16 ix;

    if (handle != GD_IR_RECEIVER) return -1;

    for (ix=0; ix<num; ix++)
    {
        TimeData = IrBuffer[IrReadPointer++];
        *pData++ = TimeData;
        IrDataNumber--;
        if (IrReadPointer >= IRR_MAX_DATA)
            IrReadPointer = 0;
    }

    return 0;
}


/*!
*******************************************************************************

This function decodes the Infrared data of NEC mode.

\note    The device ID is a 16-bit custom code (Custom Code, Custom Code?.
         Where the upper 8 bits refer to the Custom Code and the lower 8 bits
         refer to the Custom Code'.

\param   handle      Handle of the receiver device in the IR driver.
\param   pKeyValue   Pointer to IR key value.
\param   pRepeatFlag Pointer to repeat flag of remote controller.
\param   pDeviceID   Pointer to device ID of the remote controller.

\return  Return values from GD_IR_Status, most important are:
         \c GD_OK
         \c GD_ERR_IR_NO_SEQUENCE = no signal received
         own return values:
         \c GM_ERR_IR_INVAL_SEQ = no valid sequence detected

******************************************************************************/
int GM_IRR_NEC_Mode(u16 *pDeviceID, u8 *pKeyValue, u8 *pRepeatFlag)
{
    u16     Data = 0,nData = 0,TmpData = 0;
    u8      CtlData=0;
    u8      NCtlData=0;
    u8      IRData=0;
    u8      NIRData=0;
    u16     ix, iy=0;
    u8      RepeatFlg=0 ;
    u8      IR_State=0;  /* must init this value as 0 !*/
    int     err;
    static u8 firstRepeat = 1;

    /* preset return values to 0 */
    *pKeyValue       = 0;
    *pRepeatFlag     = 0;
    *pDeviceID       = 0;

    err = GD_IR_Status(GD_IR_RECEIVER, &nData);
    if(err)
        return(err);
    else
    {
        for (ix=0; ix<=nData; ix++)
        {
            GD_IR_Receive(GD_IR_RECEIVER,&Data,1);
            //if ( Data & GD_IR_EDGE_FLAG)
            {
                TmpData = Data;
                if ( IR_State == 0 )
                {
                    if ( ( TmpData < 5500 ) && ( TmpData > 4500 ) )
                    {
                        RepeatFlg=0;
                        IR_State= 1;
                        CtlData=0;
                        NCtlData=0;
                        IRData=0;
                        NIRData=0;
                        iy = 0;
                    }
                    else if ( ( TmpData < 3000 ) && ( TmpData > 2500 ) )
                    {
                        RepeatFlg=1;
                        IR_State= 20;
                        CtlData=0;
                        NCtlData=0;
                        IRData=0;
                        NIRData=0;
                        iy = 0;
                    }
                    else
                    {
                        IR_State = 20;
                    }
                }
                else if ( IR_State == 1 )
                {
                    if ( ( TmpData < 1200 ) && ( TmpData > 850 ) )
                    {
                        CtlData = CtlData;
                        iy ++;
                        if ( iy == 8 )
                        {
                            IR_State = 2;
                            iy = 0;
                        }
                    }
                    else if ( ( TmpData < 2300 ) && ( TmpData > 2000 ) )
                    {
                        CtlData  = CtlData + (1<<iy);
                        iy ++;
                        if ( iy == 8 )
                        {
                            IR_State = 2;
                            iy = 0;
                        }
                    }
                    else
                    {
                        IR_State = 20;
                    }
                }
                else if ( IR_State == 2 )
                {
                    if ( ( TmpData < 1200 ) && ( TmpData > 850 ) )
                    {
                        NCtlData  = NCtlData ;
                        iy ++;
                        if ( iy == 8 )
                        {
                            IR_State = 3;
                            iy = 0;
                        }
                    }
                    else if ( ( TmpData < 2300 ) && ( TmpData > 2000 ) )
                    {
                        NCtlData  = NCtlData + (1<<iy) ;
                        iy ++;
                        if ( iy == 8 )
                        {
                            IR_State = 3;
                            iy = 0;
                        }
                    }
                    else
                    {
                        IR_State = 20;
                    }
                }
                else if ( IR_State == 3 )
                {
                    if ( ( TmpData < 1200 ) && ( TmpData > 850 ) )
                    {
                        IRData  = IRData ;
                        iy ++;
                        if ( iy == 8 )
                        {
                            IR_State = 4;
                            iy = 0;
                        }
                    }
                    else if ( ( TmpData < 2300 ) && ( TmpData > 2000 ) )
                    {
                        IRData  = IRData + (1<<iy) ;
                        iy ++;
                        if ( iy == 8 )
                        {
                            IR_State = 4;
                            iy = 0;
                        }
                    }
                    else
                    {
                        IR_State = 20;
                    }
                }
                else if ( IR_State == 4 )
                {
                    if ( ( TmpData < 1200 ) && ( TmpData > 850 ) )
                    {
                        NIRData  = NIRData ;
                        iy ++;
                        if ( iy == 8 )
                        {
                            IR_State = 5;
                            iy = 0;
                        }
                    }
                    else if ( ( TmpData < 2300 ) && ( TmpData > 2000 ) )
                    {
                        NIRData  = NIRData + (1<<iy) ;
                        iy ++;
                        if ( iy == 8 )
                        {
                            /* validate data */
                            if(NIRData == (u8)(~(IRData)))
                            {
                                IR_State = 5;
                                iy = 0;
                            }
                            else
                            {
                                IR_State = 20;
                            }
                        }
                    }
                    else
                    {
                        IR_State = 20;
                    }
                }
            } /* if ( Data & GD_IR_EDGE_FLAG) */
        } /* for (ix=0;ix<=nData;ix++) */

        *pKeyValue       = IRData;
        *pDeviceID       = (CtlData << 8) + NCtlData;

        if(*pDeviceID == 0x449b) //fujitsu ir
        {
#if defined(CONFIG_SYSTEM_USE_NETLINK_MSG_IR)
            GK_NET_LINK_MSG_S msg;
            msg.module = GK_NL_IR;
            msg.len = GK_NL_IR_MSG_LEN;
            msg.data.gk_nl_ir_data.key = IRData;
            gk_nl_send_message(&msg);
#endif
            printk("key = 0x%02x --- deviceid = 0x%x\n", IRData, *pDeviceID);
        }

        if(*pKeyValue == 0 && *pDeviceID == 0)
        {
            *pRepeatFlag = 1;
        }
        else
        {
            *pRepeatFlag = 0;
        }

#if 1
        if(RepeatFlg == 1)
        {
            /* we ignore the first key repeat message since it comes too soon */
            if(firstRepeat == 1)
                firstRepeat = 0;
            else
            {
                *pRepeatFlag     = RepeatFlg;
                return(0);
            }
        }

        if(IR_State == 5)
        {
            firstRepeat = 1;
            return(0);
        }
        else /* no valid char received */
            return(-1);
#endif
    } /* else */

    return 0;
}

#define MIN_HI_TIME 0x10
#define LNG_HI_TIME 0x24
#define MAX_HI_TIME 0x38
#define MIN_LO_TIME 0x08
#define LNG_LO_TIME 0x1E
#define MAX_LO_TIME 0x30

#define MAX_IR_DATA 80
#define GD_IR_VAL_BIT_MASK           0x00FF



/*---------------------------------------------------------------------------*/
/* local data                                                                */
/*---------------------------------------------------------------------------*/
static u16 IRC_ActIrData[MAX_IR_DATA];

/*!
*******************************************************************************

Function decodes the Infrared data to RC-5 data.

\param   pRc5Data   Pointer RC-5 data.
\param   nData      Number of data.
\param   pIrData    Pointer to IR data.

\return  Returns 0 in case of success or error otherwise.

******************************************************************************/
static int DecodeRC5(u16 *pRc5Data, int nData, u16 *pIrData)
{
    int ExpectPosEdge;
    int IrTimeVal;
    u32 RC5_Dataword;
    int OneBitsSeq, BitCount;

    RC5_Dataword  = 0;
    ExpectPosEdge = 1; // NOTE: Initial condition, 'expect positive edge'
    OneBitsSeq    = 1; // NOTE: Initial condition, 'receiving '1'-bits'
    BitCount      = 0;

//    GM_PrStr("\nRc5Decode["); GM_PrHex(nData,2); GM_PrStr("]\n");
    while (nData--)
    {
        IrTimeVal  = *pRc5Data++;
        IrTimeVal &= GD_IR_VAL_BIT_MASK;                   // only Timing-Info
//        GM_PrStr(" "); GM_PrHex(IrTimeVal,2);
        if (ExpectPosEdge)
        {
            // if expecting a 'PositiveSignalEdge'
            ExpectPosEdge = 0;
            if ((IrTimeVal<MIN_HI_TIME)||(IrTimeVal>MAX_HI_TIME))
                return 1; // if 'illegal' timing values
            if (OneBitsSeq)
            {
                // if 'receiveing '1'-bit sequence
                RC5_Dataword = RC5_Dataword << 1; // shift up previous data bits
                RC5_Dataword |= 0x0001; // add '1'-bit
                BitCount++;
            }
            if (IrTimeVal>=LNG_HI_TIME)
                OneBitsSeq = 0; // if transition into 'receiving '0'-bits sequence
        }
        else
        {
            // else expecting a 'NegativeSignalEdge'
            ExpectPosEdge = 1;
            if (!nData)
            {
                // if expecting final 'stop' time-duration
                if (IrTimeVal<MAX_LO_TIME)
                {
//                    GM_PrStr("<-NotEnd");
                    return 2; // if 'illegal' timing values
                }
                IrTimeVal = MAX_LO_TIME-1; // convert into 'valid' timing value
            }
            if ((IrTimeVal<MIN_LO_TIME)||(IrTimeVal>MAX_LO_TIME))
                return 3;   // if 'illegal' timing values
            if (!OneBitsSeq)
            {
                // if 'receiveing '0'-bit sequence
                // shift up previous data bits, insert '0'-bit
                RC5_Dataword = RC5_Dataword << 1;
                BitCount++;
            }
            if (IrTimeVal>=LNG_LO_TIME)
                OneBitsSeq = 1; // if transition into 'receiving '1'-bits sequence
        }
    }
//    GM_PrStr("\nResultBits:"); GM_PrHex(BitCount,2);
    if (BitCount!=14) return 4;
    *pIrData = RC5_Dataword;
    return 0;
}


/*!
*******************************************************************************

This function decodes the Infrared data of RC-5 mode.

\note    The device ID is a 16-bit custom code (Custom Code, Custom Code).
         Where the upper 8 bits refer to the Custom Code and the lower 8 bits
         refer to the Custom Code'.

\param   handle      Handle of the receiver device in the IR driver.
\param   pKeyValue   Pointer to IR key value.
\param   pRepeatFlag Pointer to repeat flag of remote controller.
\param   pDeviceID   Pointer to device ID of the remote controller.

\return  Return values from GD_IR_Status, most important are:
         \c GD_OK
         \c GD_ERR_IR_NO_SEQUENCE = no signal received
         own return values:
         \c GM_ERR_IR_INVAL_SEQ = no valid sequence detected

******************************************************************************/
int GM_IRR_RC5_Mode(u16 *pDeviceID, u8 *pKeyValue, u8 *pRepeatFlag)
{
    static u8 LastTogInfo=0;
    u16 nNewIrcTimingData;
    u16 DecodedData;
    int err, ix, Count, KVal, KRep, KDev;

    //-- preset return values --
    KVal = 0;
    KRep = 0;
    KDev = 0;

    err = GD_IR_Status(GD_IR_RECEIVER, &nNewIrcTimingData);
    if(err)
    {
        return(err);
    }

    nNewIrcTimingData  += 1;    // plus 'Stop' Timing value
    Count = nNewIrcTimingData;
    do
    {
        // transfer IR-Timing data (to local RxBuffer)
        ix = Count; // (multiple chuncs 'invalid data' if necessary)
        if (ix>MAX_IR_DATA)
            ix = MAX_IR_DATA;
        Count -= ix;
        GD_IR_Receive(GD_IR_RECEIVER, IRC_ActIrData, ix); // 'empty' IR-Data
    } while (Count);
    err = -1;  // def. return code

    if (nNewIrcTimingData<MAX_IR_DATA)
    {
        // if not to much data
//        GM_PrStr("\nTry to decode RC5:");
        ix = DecodeRC5(IRC_ActIrData, nNewIrcTimingData, &DecodedData);
        if (!ix)
        {
//            GM_PrStr("->"); GM_PrHex(DecodedData,4);
            KVal  = DecodedData & 0x3F;
            if (!(DecodedData&0x1000)) KVal |= 0x40;
            KDev = (DecodedData>>6)  & 0x1F;
            ix   = (DecodedData>>11) & 1;  // get Toggle Info
            if (ix==(int)LastTogInfo) KRep = 1; // if not 'Toggling' => Set Repeat-Flag
            LastTogInfo = ix;   // save for next time
            err = 0;
        }
    }
//    else GM_PrStr("\n{CanNotBeRc5Data}");
    *pKeyValue       = KVal ;
    *pRepeatFlag     = KRep;
    *pDeviceID       = KDev;
    return err;
}


/* ---------------------------------------------------------------------- */
static void gk_ir_event_Notifier(struct gk_ir_info    *pirinfo)
{
    uint32_t deviceId       = 0;
    uint32_t scancode       = 0;
    uint32_t repeatFlag     = 0;
    uint32_t keyInfo        = 0;
    unsigned long flags;


    spin_lock_irqsave(&pirinfo->keylock, flags);
    /* Get IR key */
    if(GM_IRR_NEC_Mode((uint16_t*)&deviceId, (uint8_t*)&scancode, (uint8_t*)&repeatFlag) == 0)
    {

        if(!deviceId)
        {
            deviceId = lastDeviceId;
        }
        /*report if deviceId or keytype changed*/
        if(deviceId != lastDeviceId || repeatFlag != lastRepeatFlag)
        {
            lastDeviceId = deviceId;
            lastRepeatFlag = repeatFlag;

            keyInfo = (lastDeviceId<<16) | lastRepeatFlag;
            //printk("keyInfo: %x\n", keyInfo);

            //input_event(pirinfo->pinput_dev, EV_MSC, MSC_GKIRKEYINFO, keyInfo);
            //input_sync(pirinfo->pinput_dev);
        }

        //currentKeydownJiffies = jiffies;

        if(repeatFlag == 0)
        {
            lastKey = scancode;
        }
        /* key repeat message must be in a certain time interval */
        //else if (currentKeydownJiffies - lastKeyJiffies > msecs_to_jiffies(GK_IR_KEYPRESS_TIMEOUT)) {
        //    lastKeyJiffies = currentKeydownJiffies;
        //    printk("No sequence is received\n");
        //    goto out;
        //}
        else
        {
            scancode = lastKey;
        }
        //lastKeyJiffies = currentKeydownJiffies;

        //printk("report key: 0x%02x\n", scancode);
        /*key event is triggered by key pressed*/
        input_report_key(pirinfo->pinput_dev, scancode, 1);
        input_report_key(pirinfo->pinput_dev, scancode, 0);
        input_sync(pirinfo->pinput_dev);
#if 0
        /*can't get status of key released, just to released key without report */
        if(!!test_bit(scancode, pirinfo->pinput_dev->key) == 1)
        {
            clear_bit(scancode, pirinfo->pinput_dev->key);
        }
#endif
    }
    else
    {
        //printk("GM_IRR_NEC_Mode: error\n");
    }

    spin_unlock_irqrestore(&pirinfo->keylock, flags);
    return;
}
#endif

static irqreturn_t gk_ir_irq(int irq, void *devid)
{
    struct gk_ir_info    *pirinfo;
    int i;
#if defined(CONFIG_INPUT_GOKE_433)
    static int start = 0;
    static int short_start = 0;
    static int counting = 0;
    u16 ts = 0;
#endif
    u32 irdata = 0,irtdata = 0,irq_flag = 0;
    u16 tmp_value = 0;
    GH_IRR_IRR_READ_S   m_irr_read;

    pirinfo = (struct gk_ir_info *)devid;

    BUG_ON(pirinfo->ir_pread < 0);
    BUG_ON(pirinfo->ir_pread >= MAX_IR_BUFFER);

    irq_flag = gk_ir_readl(REG_PMU_IRQ_STATUS);
    //printk("irq status = 0x%x \n", irq_flag);

    if(irq_flag & IRQ_EN_IRR) // irr irq
    {
        irdata  = gk_ir_readl(REG_IRR_IRR_READ);
        irtdata = gk_ir_readl(REG_IRR_IRT_READ);

        m_irr_read.all = irdata;
        if (m_irr_read.bitc.valid == 1)
        {
            tmp_value = ((irtdata & 0x00000003) | ((irdata << 2) & 0x000000fc));

#if defined(CONFIG_INPUT_GOKE_433)  // 433 wifi����
            // �ж�̫�죬���ж��д�ӡ�ᶪ�ж�
            // printk("ir value1 = %4d  -- %4d    edge = %d\n", tmp_value, ir_count2time(tmp_value), m_irr_read.bitc.edge);
            if(tmp_value >= 0xfC )  // 16ms����ʼ���������ε����
            {
                // ������ο�ʼ��¼
                if(start==0)
                {
                    start = 1;
                    counting = 0;
                }
                else
                {
                    if(short_start)
                        start = 0;
                    if(counting > 45)
                    {
                        printk("counting = %d \n", counting);

                        if(counting==50)
                        {
                            int j=0;
                            //for(i=0;i<counting;i++)
                            {
                                //printk("%d\n", IrBuffer[i]);  // ���е���������
                            }
                            if((IrBuffer[2] + IrBuffer[3]) < 1200
                                && (IrBuffer[4] + IrBuffer[5]) < 1200)
                            {
                                // �Ŵ�����
                                /* start
                                *      ---+                                +----
                                *         |                                |
                                *         +--------------------------------+
                                *                     2500us
                                */
                                /* bit 0 [T=200us]
                                *         +----------------+    +---
                                *         |                |    |
                                *      ---+                +----+
                                *              +4T          -T
                                */

                                /* bit 1 [T=200us]
                                *         +----+                +---
                                *         |    |                |
                                *      ---+    +----------------+
                                *           +T         -4T
                                */
#if defined(CONFIG_SYSTEM_USE_NETLINK_MSG_433_GATE)
                                GK_NET_LINK_MSG_S msg;
                                msg.module = GK_NL_IR_433_GATE;
                                for(i=2;i<counting;i+=2)
                                {
                                    //printk("%d", IrBuffer[i]<400?1:0);  // ����ֵ�ж�һ��bitλ���ʼ��������������ʼͷ������
                                                                        // Լ���ߵ�ƽ�̵�Ϊ1������Ϊ0���̵�ƽ��200us���ң�����ƽ
                                                                        // ��800us����
                                    i+=2;
                                    i+=2;
                                    msg.data.gk_nl_433_data.data[j++] = IrBuffer[i]<400?1:0;
                                }
                                msg.len = j;
                                gk_nl_send_message(&msg);
#endif
                            }
                            else if((IrBuffer[2] + IrBuffer[3]) > 1600
                                && (IrBuffer[4] + IrBuffer[5]) > 1600)
                            {
                                // Կ������
                                /* start
                                *      ---+                                +----
                                *         |                                |
                                *         +--------------------------------+
                                *                     3200us
                                */
                                /* bit 0 [T=400us]
                                *         +----------------+    +---
                                *         |                |    |
                                *      ---+                +----+
                                *              +4T           -T
                                */
                                /* bit 1 [T=400us]
                                *         +----+                +---
                                *         |    |                |
                                *      ---+    +----------------+
                                *           +T        -4T
                                */
#if defined(CONFIG_SYSTEM_USE_NETLINK_MSG_433_KEY)
                                GK_NET_LINK_MSG_S msg;
                                msg.module = GK_NL_IR_433_KEY;
                                for(i=2;i<counting;i+=2)
                                {
                                    //printk("%d", IrBuffer[i]<900?1:0);  // ����ֵ�ж�һ��bitλ���ʼ��������������ʼͷ������
                                                                        // Լ���ߵ�ƽ�̵�Ϊ1������Ϊ0���̵�ƽ��500us���ң�����ƽ
                                                                        // ��1500us����
                                    i+=2;
                                    msg.data.gk_nl_433_data.data[j++] = IrBuffer[i]<900?1:0;
                                }
                                msg.len = j;
                                gk_nl_send_message(&msg);
#endif
                            }
                            //printk("\n");
                        }
#if 0
                        else
                        {
                            for(i=0;i<counting;i++)
                            {
                                printk("%d\n", IrBuffer[i]);  // ���е���������
                            }
                        }
#endif
                    }
                    //else printk("skip %d\n", counting);
                }

                short_start = 0;

            }
            else
            {
                counting++;
                short_start = 1;

                ts = ir_count2time(tmp_value);  //ͳ�������ʱ��(us)������ʱ�䣬���������Ҳ��

                IrBuffer[counting] = ts;
                if(counting >= IRR_MAX_DATA) counting = 0;
            }
#elif defined(CONFIG_INPUT_GOKE_IR)//�����Ǻ���ң�����İ�������
            if (tmp_value < 0xfC)
            {
                gk_ir_writel( REG_IRR_PROG, 0x03);
                tmp_value = ir_count2time(tmp_value);

                //printk("ir value = 0x%x\n", tmp_value);

                IrBuffer[IrWritePointer++] = tmp_value;
                IrDataNumber++;
                IrSeqNumber++;
                IRDataAvailable = 1;
            }
            else if (tmp_value >= 0xfC)
            {
                gk_ir_writel( REG_IRR_PROG,0x02);
                //printk("ir value = 0x%x\n", tmp_value);
                gk_ir_event_Notifier(pirinfo);
                IrDataNumber    = 0;
                IrSeqNumber     = 0;
                IrReadPointer   = 0;
                IrWritePointer  = 0;
                memset(&IrBuffer, 0, sizeof(IrBuffer));
            }
#endif
            if (IrDataNumber>=IRR_MAX_DATA)
            {
                IrBufferOverApi = 1;
            }
            if (IrWritePointer >= IRR_MAX_DATA)
            {
                IrWritePointer = 0;
            }
        }
        gk_ir_writel( REG_PMU_IRQ_CLR_IRR, IRQ_EN_IRR); // clear irr irq

#if defined(CONFIG_INPUT_GOKE_IR)
        if(lastKey == 0x0b)
        {
            printk("Ready to power down!\n");
            gk_power_off();
        }
#endif
    }
    if(irq_flag & IRQ_EN_IRT) // irt irq
    {
        gk_ir_writel( REG_PMU_SYS_REG_CFG1, 1);
        gk_ir_writel( REG_PMU_IRQ_CLR_IRT, IRQ_EN_IRT); // clear irt irq
    }

    if(irq_flag & IRQ_EN_RTC) // rtc irq
    {
        gk_ir_writel( REG_PMU_IRQ_CLR_RTC, IRQ_EN_RTC);        // clear rtc irq
    }
    if(irq_flag & IRQ_EN_FPC) // fpc irq
    {
        gk_ir_writel( REG_PMU_IRQ_CLR_FPC, IRQ_EN_FPC);        // clear fpc irq
    }
    if(irq_flag & IRQ_EN_GPIO) // gpio irq
    {
        u32 regval = 0;
        regval = gk_ir_readl(PMU_GPIO_RIS);
#if defined(CONFIG_SYSTEM_USE_NETLINK_MSG_PMU_GPIO)
        u32 mask = 1;
        for (i = 0; i < 8; i++)
        {
            if(regval&mask)
            {
                GK_NET_LINK_MSG_S msg;
                msg.module = GK_NL_PMU_GPIO;
                msg.data.gk_nl_gpio_data.id = i;
                msg.data.gk_nl_gpio_data.val = (gk_ir_readl(MCU_GPIO_BASE + 0x03f0) & mask)?1:0;
                //printk("irq pmu gpio%d = %d\n", msg.data.gk_nl_gpio_data.id,
                //    msg.data.gk_nl_gpio_data.val);
                msg.len = GK_NL_GPIO_MSG_LEN;
                gk_nl_send_message(&msg);
            }
            mask <<= 1;
        }
#endif
        gk_ir_writel( PMU_GPIO_IC, regval);
        gk_ir_writel( REG_PMU_IRQ_CLR_GPIO, IRQ_EN_GPIO);        // clear gpio irq
    }
    if(irq_flag & IRQ_EN_CEC) // cec irq
    {
        gk_ir_writel( REG_PMU_IRQ_CLR_CEC, IRQ_EN_CEC);        // clear cec irq
    }
    if(irq_flag & IRQ_EN_ADC) // adc irq
    {
        gk_ir_writel( REG_PMU_IRQ_CLR_ADC, IRQ_EN_ADC);        // clear adc irq
    }
    return IRQ_HANDLED;
}

static int __devinit gk_ir_probe(struct platform_device *pdev)
{
    int                    retval;

    struct gk_ir_info        *pirinfo;

    //struct proc_dir_entry        *input_file;
    struct input_dev            *pinput_dev;

    pinput_dev = input_allocate_device();
    if (!pinput_dev)
    {
        dev_err(&pdev->dev, "input_allocate_device fail!\n");
        retval = -ENOMEM;
        goto ir_errorCode_na;
    }
    pinput_dev->name = "GKInput";
    pinput_dev->phys = "gk/input0";
    pinput_dev->id.bustype = BUS_HOST;

    retval = input_register_device(pinput_dev);
    if (retval)
    {
        dev_err(&pdev->dev, "Register input_dev failed!\n");
        goto gk_input_probe_free_input_dev;
    }
    //pbinfo->pdev = pdev;
    //pbinfo->pinput_dev = pinput_dev;

    pirinfo = kzalloc(sizeof(struct gk_ir_info), GFP_KERNEL);
    if (!pirinfo)
    {
        dev_err(&pdev->dev, "Failed to allocate pirinfo!\n");
        retval = -ENOMEM;
        goto gk_input_probe_unregister_input_dev;
    }

    pirinfo->pcontroller_info = (struct gk_ir_controller *)pdev->dev.platform_data;
    if ((pirinfo->pcontroller_info == NULL) )
    {
        dev_err(&pdev->dev, "Platform data is NULL!\n");
        retval = -ENXIO;
        goto ir_errorCode_pinfo;
    }

    pirinfo->regbase = (unsigned char __iomem *)MCU_IRR_BASE;
    pirinfo->id = pdev->id;
    pirinfo->irq = IR_IRQ;

    pirinfo->last_ir_uid = 0;
    pirinfo->last_ir_flag = 0;
    pirinfo->pinput_dev = pinput_dev;
    //pirinfo->pinput_dev = pboard_info->pinput_dev;
    pirinfo->pkeymap = keymap;

    platform_set_drvdata(pdev, pirinfo);

    gk_ir_init(pirinfo);

    retval = request_irq(pirinfo->irq, gk_ir_irq, IRQF_TRIGGER_LOW,
                            dev_name(&pdev->dev), pirinfo);
    if (retval)
    {
        dev_err(&pdev->dev, "Request IRQ failed!\n");
        goto ir_errorCode_free_platform;
    }
    printk("ir request irq: %d \n", pirinfo->irq);

    printk("IR Host Controller probed!\n");

    goto ir_errorCode_na;

ir_errorCode_free_platform:
    platform_set_drvdata(pdev, NULL);

ir_errorCode_pinfo:
    kfree(pirinfo);

gk_input_probe_unregister_input_dev:
    input_unregister_device(pinput_dev);

    return retval;

gk_input_probe_free_input_dev:
    input_free_device(pinput_dev);
    //pbinfo->pdev = NULL;
    //pbinfo->pinput_dev = NULL;

ir_errorCode_na:
    return retval;
}

static int __devexit gk_ir_remove(struct platform_device *pdev)
{
    struct gk_ir_info    *pirinfo;
    int                retval = 0;

    pirinfo = platform_get_drvdata(pdev);

    if (pirinfo)
    {
        free_irq(pirinfo->irq, pirinfo);
        input_unregister_device(pirinfo->pinput_dev);
        platform_set_drvdata(pdev, NULL);
        kfree(pirinfo);
    }

    dev_notice(&pdev->dev,
               "Remove GK IR Controller.\n");

    return retval;
}

static struct platform_driver gk_ir_driver =
{
    .probe      = gk_ir_probe,
    .remove     = __devexit_p(gk_ir_remove),
    .driver     =
    {
        .name   = "ir",
        .owner  = THIS_MODULE,
    },
};

static int __init gk_input_init(void)
{
    int retval = 0;

    retval = platform_driver_register(&gk_ir_driver);
    if (retval)
        printk(KERN_ERR "Register gk_input_driver failed %d!\n",
            retval);

    return retval;
}

static void __exit gk_input_exit(void)
{
    platform_driver_unregister(&gk_ir_driver);
}

module_init(gk_input_init);
module_exit(gk_input_exit);

MODULE_DESCRIPTION("GOKE Input Driver");
MODULE_AUTHOR("Goke Microelectronics Inc.");
MODULE_LICENSE("GPL");

