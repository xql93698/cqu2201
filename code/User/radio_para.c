/****************************************Copyright (c)****************************************************
**                                ������6lowpan���߿���ƽ̨
**                                  һ��Ϊ��ԴӦ�ö���                         
**
**
**--------------File Info---------------------------------------------------------------------------------
** File Name:               radio_para.c
** Last modified Date:      2018-04-07
** Last Version:            v1.0
** Description:             ���ݽڵ�ų�ʼ�����������
** 
**--------------------------------------------------------------------------------------------------------
** Created By:              ��УԴ
** Created date:            2018-04-07
** Version:                 v1.0
** Descriptions:            The original version ��ʼ�汾
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
  ȫ�ֱ�������
*********************************************************************************************************/
radio_para radiopara;


//TODO ���ݽڵ�����ò�ͬ�Ĺ���

    /*
    ** Step 10, ���͹�������
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
** Descriptions:        �����������ʼ��
** input parameters:    0
** output parameters:   ��
** Returned value:      moteid
** Created by:          ��УԴ
** Created Date:        2018-04-07
*********************************************************************************************************/
void radio_para_init(){
    radiopara.shortaddr = get_moteid();
    radiopara.pan_id = get_cluster_name(radiopara.shortaddr);
    radiopara.max_frame_retrise = DEFAULT_MAX_FRAME_RETRIES;
    radiopara.max_csma_retries = 0x03;
    radiopara.tx_power = 0x0D;
    radiopara.channel = DEFAULT_CHANNEL;
#if ENABLE_PANID_FILTER
    radiopara.panid_filter = 0;
#else
    radiopara.panid_filter = 1;
#endif
    if(get_moteid() == TIME_SYNCH_NODE )
      radiopara.tx_power = 0x02;
}
/*********************************************************************************************************
** Function name:       report_radio_statistics
** Descriptions:        ��ӡ������ͳ����Ϣ
** input parameters:    0
** output parameters:   ��
** Returned value:      0
** Created by:          ��УԴ
** Created Date:        2018-04-07
*********************************************************************************************************/
void report_radio_statistics(void)
{
  uart_printf("transmit_times %d badcrc_times %d  transmit_failed_times %d channel_access_failed_times %d no_ack_times %d ,transmit_success_times %d\r\n",
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