/*! \file dmr.c
  \brief DMR Hook functions.
  
  This module hooks some of the DMR packet handler functions,
  in order to extend the functionality of the radio.  Ideally,
  we'd like to use just the hooks, but for the time-being some
  direct patches and callbacks are still necessary.
*/


#include <stdio.h>
#include <string.h>

#include "md380.h"
#include "printf.h"
#include "dmesg.h"
#include "version.h"
#include "tooldfu.h"
#include "config.h"
#include "gfx.h"


void *dmr_call_end_hook(char *pkt){
  /* This hook handles the dmr_contact_check() function, calling
     back to the original function where appropriate.
     
     pkt points to something like this:
     
                    /--dst-\ /--src-\
     08 2a 00 00 00 00 00 63 30 05 54 7c 2c 36
   */
  
  //Destination adr as Big Endian.
  int dst=(pkt[7]|
	   (pkt[6]<<8)|
	   (pkt[5]<<16));
  //Source comes next.
  int src=(pkt[10]|
	   (pkt[9]<<8)|
	   (pkt[8]<<16));
  
  printf("\n");
  printhex((char*)pkt,14);
  printf("\nDMR call from %d to %d ended.\n",
	 src,dst);
  
  //Forward to the original function.
  return dmr_call_end((void*)pkt);
}

void *dmr_call_start_hook(char *pkt){
  /* This hook handles the dmr_contact_check() function, calling
     back to the original function where appropriate.
     
     It is called several times per call, presumably when the
     addresses are resent for late entry.  If you need to trigger
     something to happen just once per call, it's better to put that
     in dmr_call_end_hook().
     
     pkt looks like this:
     
     overhead
     /    /         /--dst-\ /--src-\
     08 1a 00 00 00 00 00 63 30 05 54 73 e3 ae
     10 00 00 00 00 00 00 63 30 05 54 73 2c 36
   */
  
  //Destination adr as Big Endian.
  int dst=(pkt[7]|
	   (pkt[6]<<8)|
	   (pkt[5]<<16));
  
  //All but the top row is overwritten,
  //so any status has to be logged here.
  char buf[15];
  
  //Print the target adress to the screen.
  sprintf(buf,
	  "%d",dst);
  drawascii(buf,
	    160,20);
  
  //Just a dot for logging.
  printf(".");
  
  //Forward to the original function.
  return dmr_call_start(pkt);
}


void *dmr_handle_data_hook(char *pkt, int len){
  /* This hook handles the dmr_contact_check() function, calling
     back to the original function where appropriate.
     
     Packes are up to twelve bytes, but they are always preceeded by
     two bytes of C5000 overhead.
   */
  
  //Turn on the red LED to know that we're here.
  red_led(1);
  
  printf("Data:       ");
  printhex(pkt,len+2);
  printf("\n");
  
  //Forward to the original function.
  return dmr_handle_data(pkt,len);
}


void *dmr_sms_arrive_hook(void *pkt){
  /* This hooks the SMS arrival routine, but as best I can tell,
     dmr_sms_arrive() only handles the header and not the actual
     data payload, which is managed by dmr_handle_data() in each
     fragment chunk.

     *pkt points to a twelve byte header with two bytes of C5000
     overhead.  The body packets will arrive at dmr_handle_data_hook()
     in chunks of up to twelve bytes, varying by data rate.
     
     A full transaction from 3147092 to 99 looks like this:

             header
             |    //flg\ /--dst-\ /--src-\ /flg\ /crc\
SMS header:  08 6a 02 40 00 00 63 30 05 54 88 00 83 0c
       Data: 08 7a 45 00 00 5c 00 03 00 00 40 11 5c a8
       Data: 08 7a 0c 30 05 54 0c 00 00 63 0f a7 0f a7
       Data: 08 72 00 48 d1 dc 00 3e e0 00 92 04 0d 00
       Data: 08 72 0a 00 54 00 68 00 69 00 73 00 20 00
       Data: 08 72 69 00 73 00 20 00 61 00 20 00 74 00
       Data: 08 7a 65 00 73 00 74 00 20 00 66 00 72 00
       Data: 08 7a 6f 00 6d 00 20 00 6b 00 6b 00 34 00
       Data: 08 7a 76 00 63 00 7a 00 21 00 9e 21 5a 5c
   */
  
  //Turn on the red LED to know that we're here.
  red_led(1);
  
  printf("SMS header: ");
  printhex((char*) pkt, 12+2);
  printf("\n");
  
  //Forward to the original function.
  return dmr_sms_arrive(pkt);
}