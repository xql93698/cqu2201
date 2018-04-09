/****************************************Copyright (c)****************************************************
**                                蓝精灵6lowpan无线开发平台
**                                  一切为开源应用而作                         
**
**
**--------------File Info---------------------------------------------------------------------------------
** File Name:               radio_para.c
** Last modified Date:      2018-04-07
** Last Version:            v1.0
** Description:             根据节点号初始化物理层参数
** 
**--------------------------------------------------------------------------------------------------------
** Created By:              张校源
** Created date:            2018-04-07
** Version:                 v1.0
** Descriptions:            The original version 初始版本
**
**--------------------------------------------------------------------------------------------------------
** Modified by:             
** Modified date:           
** Version:                 
** Description:             
**
*********************************************************************************************************/
#include "radio_para.h"
#include "globalmacro.h"
#include "runtime/uartstdio.h"
/*********************************************************************************************************
** 是否使能调试功能
*********************************************************************************************************/
#define DEBUG 0
#if DEBUG
#include "runtime/uartstdio.h"
#include <stdio.h>
#define PRINTF(...)   uart_printf(__VA_ARGS__)  //必须使用uart_printf（）函数
#else
#define PRINTF(...)
#endif
/*********************************************************************************************************
  全局变量定义
*********************************************************************************************************/
radio_para radiopara;


//TODO 根据节点号设置不同的功率

    /*
    ** Step 10, 发送功率设置
    **  AT86RF231                      AT86RF233
    **  0x0 --------->3.0db            0x0 --------->4.0db
    **  0x1 --------->2.8db            0x1 --------->3.7db
    **  0x2 --------->2.3db            0x2 --------->3.4db
    **  0x3 --------->1.8db            0x3 --------->3.0db
    **  0x4 --------->1.3db            0x4 --------->2.5db
    **  0x5 --------->0.7db            0x5 --------->2.0db
    **  0x6 --------->0.0db            0x6 --------->1.0db
    **  0x7 --------->-1db             0x7 --------->0.0db
    **  0x8 --------->-2db             0x8 --------->-1db
    **  0x9 --------->-3db             0x9 --------->-2db
    **  0xA --------->-4db             0xA --------->-3db
    **  0xB --------->-5db             0xB --------->-4db
    **  0xC --------->-7db             0xC --------->-6db
    **  0xD --------->-9db             0xD --------->-8db
    **  0xE --------->-12db            0xE --------->-12db
    **  0xF --------->-17db            0xF --------->-17db
    */

/*********************************************************************************************************
** Function name:       radio_para_init
** Descriptions:        物理层参数初始化
** input parameters:    0
** output parameters:   无
** Returned value:      moteid
** Created by:          张校源
** Created Date:        2018-04-07
*********************************************************************************************************/
void radio_para_init(){
    radiopara.shortaddr = get_moteid();
    radiopara.pan_id = get_cluster_name(radiopara.shortaddr);
    radiopara.max_frame_retrise = DEFAULT_MAX_FRAME_RETRIES;
    radiopara.max_csma_retries = 0x03;
    radiopara.tx_power = 0x0B;
    radiopara.channel = DEFAULT_CHANNEL;
#if TDMA_BSM_ENABLE
    radiopara.max_csma_retries = 0x07; //不进行CSMA CA
#else
    radiopara.max_csma_retries = 0x01; //最多一次重试
#endif 
#if ENABLE_PANID_FILTER
    radiopara.panid_filter = 0x0;
#else
    radiopara.panid_filter = 0x1;
#endif
    if(get_moteid() == TIME_SYNCH_NODE )
      radiopara.tx_power = 0x02;
}
/*********************************************************************************************************
** Function name:       report_radio_statistics
** Descriptions:        打印物理层统计信息
** input parameters:    0
** output parameters:   无
** Returned value:      0
** Created by:          张校源
** Created Date:        2018-04-07
*********************************************************************************************************/
void report_radio_statistics(void)
{
  PRINTF("report_radio_statistics: %d %d %d %d %d %d\r\n",
              radiopara.transmit_times,
                  radiopara.badcrc_times,
                    radiopara.transmit_failed_times,
                      radiopara.channel_access_failed_times,
                        radiopara.no_ack_times,
                          radiopara.transmit_success_times);
}

/*********************************************************************************************************
  END FILE 
*********************************************************************************************************/
