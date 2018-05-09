/****************************************Copyright (c)****************************************************
**                                
**
**
**--------------File Info---------------------------------------------------------------------------------
** File Name:           P2P.c
** Last modified Date:  2018-04-11
** Last Version:        v1.0
** Description:         
**
**--------------------------------------------------------------------------------------------------------
** Created By:          Xaiohan Y
** Created date:        2012-06-21
** Version:             v1.0
** Descriptions:        The original version 
**
**--------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Version:
** Description:
**
*********************************************************************************************************/
#include "includes.h"
#include "P2P.h"

/*********************************************************************************************************
**  Define global variaty
*********************************************************************************************************/
#define DEBUG 1
#if DEBUG
#include "runtime/uartstdio.h"
#define PRINTF(...)   uart_printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif


    yxh_frame802154_t frame_to_send;
    yxh_frame802154_t rec_frame;
    
    uint8_t payload_to_send[2]={1,2};
    uint8_t frame_sequence = 0;
    
    //uint8_t synch_node_flag  = 0x01;

  time_para synch={
    0,
    0,
    0,
    0,
  timeoffset_calc,
  get_synch_time
  };
/*********************************************************************************************************
** Function name:       get_synch_time
** Descriptions:        获取同步时间
** input parameters:    0
** output parameters:   无
** Returned value:      0
** Created by:          张校源
** Created Date:        2018-04-07
*********************************************************************************************************/

rtimer_clock_t get_synch_time(time_para *timepara)
{
  return rtimer_arch_now()-timepara->time_offset;
} 

/*********************************************************************************************************
** Function name:       get_synch_time
** Descriptions:        计算时间偏置
** input parameters:    0
** output parameters:   无
** Returned value:      0
** Created by:          张校源
** Created Date:        2018-04-07
*********************************************************************************************************/


void timeoffset_calc(time_para *timepara,uint32_t time)
{
  timepara->time_offset = RTIMER_NOW() -time;
  //对周期取与
  timepara->time_offset_period_align=timepara->time_offset%PEROID_LENGTH;
  //若果为负数
  if(timepara->time_offset_period_align<0)
      timepara->time_offset_period_align+=PEROID_LENGTH;
} 




/*********************************************************************************************************
** Function name:       addr_len
** Descriptions:        determine the length of address fields
** input parameters:    uint8_t mode
** output parameters:   static uint8_t 
** Returned value:      0
** Created by:          Xiaohan Y
** Created Date:        2018-04-11
*********************************************************************************************************/
CC_INLINE static uint8_t                                  //??
addr_len(uint8_t mode)
{
  switch(mode) {
  case FRAME802154_SHORTADDRMODE:  /* 16-bit address */
    return 2;
  case FRAME802154_LONGADDRMODE:   /* 64-bit address */
    return 8;
  default:
    return 0;
  }
}

/*********************************************************************************************************
** Function name:       field_len
** Descriptions:        determine the length of variable fields  
** input parameters:    frame802154_t *p, field_length_t *flen
** output parameters:   none
** Returned value:      0
** Created by:          Xiaohan Y
** Created Date:        2018-04-11
*********************************************************************************************************/
static void
field_len(yxh_frame802154_t *p, field_length_t *flen)
{
  /* init flen to zeros */
  memset(flen, 0, sizeof(field_length_t));
  
  /* Determine lengths of each field based on fcf and other args */
  if(p->fcf.dest_addr_mode & 3) {
    flen->dest_pid_len = 2;
  }
  if(p->fcf.src_addr_mode & 3) {
    flen->src_pid_len = 2;
  }
  
  /* Set PAN ID compression bit if src pan id matches dest pan id. */
  if(p->fcf.dest_addr_mode & 3 && p->fcf.src_addr_mode & 3 &&
     p->src_pid == p->dest_pid) {
       p->fcf.panid_compression = 1;
       
       /* compressed header, only do dest pid */
       flen->src_pid_len = 0;
     } else {
       p->fcf.panid_compression = 0;
     }
  
  /* determine address lengths */
  flen->dest_addr_len = addr_len(p->fcf.dest_addr_mode & 3);
  flen->src_addr_len = addr_len(p->fcf.src_addr_mode & 3);
}


/*********************************************************************************************************
** Function name:       yxh_frame802154_create
** Descriptions:        Creates a frame for transmission over the air
** input parameters:    frame802154_t *p, uint8_t *buf
** output parameters:   pose: the total length (Byte) of the frame except payload CRC
** Returned value:      0
** Created by:          Xiaohan Y
** Created Date:        2018-04-11
*********************************************************************************************************/

/*----------------------------------------------------------------------------*/
/**
 *   \brief Creates a frame for transmission over the air.  This function is
 *   meant to be called by a higher level function, that interfaces to a MAC.
 *
 *   \param p Pointer to frame802154_t struct, which specifies the
 *   frame to send.
 *
 *   \param buf Pointer to the buffer to use for the frame.
 *
 *   \return The length of the frame header
*/
int
yxh_frame802154_create(yxh_frame802154_t *p, uint8_t *buf)       
{
  int c;
  field_length_t flen;
  uint8_t pos;

  field_len(p, &flen);
  
  /* OK, now we have field lengths.  Time to actually construct */
  /* the outgoing frame, and store it in buf */
  buf[0] = (p->fcf.frame_type & 7) |
    ((p->fcf.security_enabled & 1) << 3) |
    ((p->fcf.frame_pending & 1) << 4) |
    ((p->fcf.ack_required & 1) << 5) |
    ((p->fcf.panid_compression & 1) << 6);
  buf[1] = ((p->fcf.dest_addr_mode & 3) << 2) |
    ((p->fcf.frame_version & 3) << 4) |
    ((p->fcf.src_addr_mode & 3) << 6);

  /* sequence number */
  buf[2] = p->seq;
  pos = 3;                               //Use pose because after the 3rd Bytes, the length of the address field is variable.

  /* Destination PAN ID */
  if(flen.dest_pid_len == 2) {
    buf[pos++] = p->dest_pid & 0xff;
    buf[pos++] = (p->dest_pid >> 8) & 0xff;
  }

  /* Destination address */
  for(c = 0;c<flen.dest_addr_len;c++) {
    buf[pos++] = p->dest_addr[c];
  }

  /* Source PAN ID */
  if(flen.src_pid_len == 2) {
    buf[pos++] = p->src_pid & 0xff;
    buf[pos++] = (p->src_pid >> 8) & 0xff;
  }

  /* Source address */
  for(c = 0;c<flen.src_addr_len;c++) {
    buf[pos++] = p->src_addr[c];
  }
  
  buf[pos++] = p->frame_seq;
  
    buf[pos++] = p->time_stamp & 0xff;
    buf[pos++] = (p->time_stamp >> 8) & 0xff;
    buf[pos++] = (p->time_stamp >> 16) & 0xff;
    buf[pos++] = (p->time_stamp >> 24) & 0xff;
 
  buf[pos++] = p->send_type;
  
  for(c = 0;c<p->payload_len;c++) {
    buf[pos++] = p->payload[c];
  }
  return (int)pos;
}


/*********************************************************************************************************
** Function name:       frame_para_init
** Descriptions:        Init the parameters of the frame
** input parameters:    none
** output parameters:   none
** Returned value:      0
** Created by:          Xiaohan Y
** Created Date:        2018-04-11
*********************************************************************************************************/
void frame_para_init(yxh_frame802154_t *p,uint8_t frame_type)
{
    time_para *timepara = & synch; 
    //init the fcf
    p->fcf.frame_type=0x01;
    p->fcf.security_enabled=0x00;
    p->fcf.frame_pending=0x00;
    p->fcf.ack_required=0x01;
    //p2p_frame.panid_compression=;                  //this is to be determined in field_flen()
    p->fcf.dest_addr_mode=FRAME802154_SHORTADDRMODE;                      //short address
    p->fcf.frame_version=0x01;
    p->fcf.src_addr_mode=FRAME802154_SHORTADDRMODE;                        //short address
    //init the seq
    p->seq=0x00;
    //init the PAN id

    p->src_pid=get_cluster_name(get_moteid());
    //init the Adress

    p->src_addr[0]=get_moteid();
    p->src_addr[1]=0x00;
    
    p->time_stamp = timepara->get_synch_time(timepara);
    
    p->payload=payload_to_send;
    p->payload_len=sizeof(payload_to_send);
  //  if(p2p_frame.dest_addr[0] == DEST_ADDR){}
     if(frame_type ==FRAME_TYPE_P2P )
     {
       p->dest_pid=get_cluster_name(get_moteid()+1);
       p->dest_addr[0]=get_moteid()+1;  
       p->dest_addr[1]= 0x00;
     } else if(frame_type ==FRAME_TYPE_TIME_SYNCH ) 
      {
       p->dest_pid = 0xFFFF;
       p->dest_addr[0] = 0xFF;
       p->dest_addr[1] = 0xFF;
      }
    p->frame_seq = frame_sequence++;
    p->time_stamp = get_synch_time(timepara);
    p->send_type = frame_type;
    
}

/*********************************************************************************************************
** Function name:       p2p_frame_send
** Descriptions:        send the frame
** input parameters:    null
** output parameters:   none
** Returned value:      0
** Created by:          Xiaohan Y
** Created Date:        2018-04-11
*********************************************************************************************************/
void yxh_frame_send(void *ptr)
{
    static struct ctimer ct;

    uint8_t frame_buf[100];
   //initialize the p2p frame parameters
    //frame_para_init();
    //yxh_frame802154_t p2p_frame=p2p_frame;
    //Initialize the p2p frame, put the structure value into a buffer
      int sendlength = yxh_frame802154_create(&frame_to_send, frame_buf);
      //Send p2p frame
     int state = NETSTACK_RADIO.send(frame_buf, sendlength); 

     //print send frame and result(state)
     PRINTF("The send frame is:");
     for(uint8_t a=0;a<sendlength;a++)
     {
        PRINTF("%x ",frame_buf[a]);
     }
     PRINTF("\r\n");   
     
     if(frame_to_send.send_type == FRAME_TYPE_TIME_SYNCH)
       return;

     PRINTF("The send state is %d\r\n",state);                  //   state = enSendState = RADIO_TX_OK=0; 
     if(state != 0)
     {
     ctimer_set(&ct,CLOCK_SECOND/10,yxh_frame_send,NULL);
     }
}

/*********************************************************************************************************
** Function name:       yxh_frame_parse
** Descriptions:        Parses an input frame
** input parameters:    uint8_t *data, int len, yxh_frame802154_t *pf
** output parameters:   
** Returned value:      0
** Created by:          Xiaohan Y
** Created Date:        2018-04-18
*********************************************************************************************************/

/*----------------------------------------------------------------------------*/
/**
 *   \brief Parses an input frame.  Scans the input frame to find each
 *   section, and stores the information of each section in a
 *   yxh_frame_t structure.
 *
 *   \param data The input data from the radio chip.
 *   \param len The size of the input data
 *   \param pf The yxh_frame_t struct to store the parsed frame information.
 */

void yxh_frame802154_parse(void)
{
 
  PRINTF("@");
  
  static struct ctimer ct1;
  //static struct ctimer ct2;
  
  time_para * timesynch = &synch;
  uint8_t *p; 
  yxh_frame802154_fcf_t fcf;
  int c;
  
  uint8_t *data = packetbuf_dataptr();
  int len = (int)packetbuf_datalen();
  yxh_frame802154_t *pf = &rec_frame;
   
  if(len <= 3) {
    return ;
  }
 
  
  p = data;

  /* decode the FCF */
  fcf.frame_type = p[0] & 7;
  fcf.security_enabled = (p[0] >> 3) & 1;
  fcf.frame_pending = (p[0] >> 4) & 1;
  fcf.ack_required = (p[0] >> 5) & 1;
  fcf.panid_compression = (p[0] >> 6) & 1;

  fcf.dest_addr_mode = (p[1] >> 2) & 3;
  fcf.frame_version = (p[1] >> 4) & 3;
  fcf.src_addr_mode = (p[1] >> 6) & 3;

  /* copy fcf and seqNum */
  memcpy(&pf->fcf, &fcf, sizeof(frame802154_fcf_t));
  pf->seq = p[2];
  p += 3;                             /* Skip first three bytes */

  /* Destination address, if any */
  if(fcf.dest_addr_mode) {
    /* Destination PAN */
    pf->dest_pid = p[0] + (p[1] << 8);
    p += 2;
    
    /* Destination address */
/*     l = addr_len(fcf.dest_addr_mode); */
/*     for(c = 0; c < l; c++) { */
/*       pf->dest_addr.u8[c] = p[l - c - 1]; */
/*     } */
/*     p += l; */
    if(fcf.dest_addr_mode == FRAME802154_SHORTADDRMODE) {
      linkaddr_copy((linkaddr_t *)&(pf->dest_addr), &linkaddr_null);
      pf->dest_addr[0] = p[0];
      pf->dest_addr[1] = p[1];
      p += 2;
    } else if(fcf.dest_addr_mode == FRAME802154_LONGADDRMODE) {
      for(c = 0; c < 8; c++) {
        pf->dest_addr[c] = p[c];
      }
      p += 8;
    }
  } else {
    linkaddr_copy((linkaddr_t *)&(pf->dest_addr), &linkaddr_null);
    pf->dest_pid = 0;
  }
  
  /*
  if(fcf.frame_type == 0x2 && len == 3)
  {
    PRINTF("ACK  received!\r\n");
    return;
  }
  */
  
  /* Source address, if any */
  if(fcf.src_addr_mode) {
    /* Source PAN */
    if(!fcf.panid_compression) {
      pf->src_pid = p[0] + (p[1] << 8);
      p += 2;
    } else {
      pf->src_pid = pf->dest_pid;
    }

    /* Source address */
/*     l = addr_len(fcf.src_addr_mode); */
/*     for(c = 0; c < l; c++) { */
/*       pf->src_addr.u8[c] = p[l - c - 1]; */
/*     } */
/*     p += l; */
    if(fcf.src_addr_mode == FRAME802154_SHORTADDRMODE) {
      linkaddr_copy((linkaddr_t *)&(pf->src_addr), &linkaddr_null);
      pf->src_addr[0] = p[0];
      pf->src_addr[1] = p[1];
      p += 2;
    } else if(fcf.src_addr_mode == FRAME802154_LONGADDRMODE) {
      for(c = 0; c < 8; c++) {
        pf->src_addr[c] = p[c];
      }
      p += 8;
    }
  } else {
    linkaddr_copy((linkaddr_t *)&(pf->src_addr), &linkaddr_null);
    pf->src_pid = 0;
  }

    
    pf->frame_seq = p[0];
    p++;
    
    pf->time_stamp = p[3] | p[2] | p[1] | p[0];
    p += 4;
    
    pf->send_type = p[0];
    
  /* header length */
  c = p - data;          
  /* payload length */
  pf->payload_len = (len - c);
  /* payload */
  pf->payload = p;
      

  
  if(get_moteid() != TIME_SYNCH_NODE )
  {
    //非同步节点接收到包后根据不同帧类型（send_frame），做不同处理
    switch(pf->send_type){
      
    case FRAME_TYPE_TIME_SYNCH:
      //只同步一次
      if(timesynch->IsSyched)
      {
        break;
      }
      timesynch->IsSyched = true;
      
      //打印接收到的帧
      uart_printf("The received frame is: ");
      for(uint8_t a=0;a<len;a++)
      {
        PRINTF("%x ",data[a]);
      }
      PRINTF("\r\n");      
      
      PRINTF("Time synchronized!\r\n");
      //计算时间偏置
      timesynch->timeoffset(timesynch,timesynch->time_stamp);
      PRINTF("time-offset is %d the time is  %d \r\n",synch.time_offset,timesynch->get_synch_time(timesynch));
      //源节点进行时间同步后，经过5秒的时间开始发P2P帧(undone)
      if(get_moteid() == SRC_ADDR)
      {
        yxh_frame802154_t p2p_frame;
        frame_para_init(&p2p_frame,FRAME_TYPE_P2P);
        frame_to_send = p2p_frame;
        ctimer_set(&ct1,5*CLOCK_SECOND,yxh_frame_send, NULL); 
        //ctimer_set(&ct2,5*CLOCK_SECOND,yxh_frame_send, ptr); 
      }
      return;
    
    
  case FRAME_TYPE_P2P:
    
    //打印接收到的帧
    uart_printf("The received frame is: ");
    for(uint8_t a=0;a<len;a++)
    {
      PRINTF("%x ",data[a]);
    }
    PRINTF("\r\n");   
    
    //目的节点收到帧后，做响应，不转发
    if(get_moteid() == (uint16_t)DEST_ADDR)
    {
      PRINTF("p2pFrame receiceved by destination node!\r\n");
    }else{
      //中继节点收到帧后通过设定ctimer进行转发
      PRINTF("p2pFrame receiceved!\r\n");
      
      yxh_frame802154_t p2p_frame;
      frame_para_init(&p2p_frame,FRAME_TYPE_P2P);
      frame_to_send = p2p_frame;
      ctimer_set(&ct1,CLOCK_SECOND,yxh_frame_send, NULL); 
    }   
      }  //switch end  
    
    }else{
      //同步节点只发送同步帧，接收到包后什么都不做
      return;  
    }


  }
  

/*********************************************************************************************************
** FFunction name:       time_synch_gps
** Descriptions:        进行GPS时间同步信号模拟
** input parameters:    同步帧发送次数
** output parameters:   无
** Returned value:      0
** Created by:          袁小涵
** Created Date:        2018-05-07
*********************************************************************************************************/




void time_synch_gps(void *ptr)
{
    static struct ctimer ct1;

    
    uint8_t time_to_synch;
    if(ptr!=NULL)
    {
      PRINTF("time_synch_gps %d\r\n",(*(uint8_t *)ptr));
      time_to_synch=(*(uint8_t *)ptr)--;
      if(!time_to_synch)
          return;
    }

    //NETSTACK_RADIO.set_value(SET_CHANNEL,17);
    yxh_frame802154_t synch_frame;
    frame_para_init(&synch_frame,FRAME_TYPE_TIME_SYNCH);
    frame_to_send = synch_frame;
    yxh_frame_send(NULL);
    ctimer_set(&ct1,CLOCK_SECOND,time_synch_gps, ptr); 

}







/*********************************************************************************************************
  END FILE
*********************************************************************************************************/