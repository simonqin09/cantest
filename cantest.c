/*
 * cantest.c
 *
 *  Created on: 2017-5-16
 *      Author: simon
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>


int main(int argc, char *argv[])
{
	int s, nbytes;
	char *array[2] = {"-r", "-s"};
	struct sockaddr_can addr;
	struct ifreq ifr;
	struct can_frame frame;
	struct can_filter rfilter[1];


	/* handle (optional) flags first */
	if(argc != 3) {
		fprintf(stderr, "Usage:  %s <-r> <can interface name> for receiving\nor <-s> <can interface name> for sending\n", argv[0]);
		exit(1);
	}
    /* create socket */
	if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0)
	{
	    perror("Create socket failed");
	    exit(-1);
	}

	/* set up can interface */
	strcpy(ifr.ifr_name, argv[2]);
	printf("can port is %s\n",ifr.ifr_name);
	/* assign can device */
	ioctl(s, SIOCGIFINDEX, &ifr);
    addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;
    /* bind can device */
	if(bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		perror("Bind can device failed\n");
		close(s);
		exit(-2);
	}

	/* configure receiving */
	if(!strcmp(argv[1],array[0]))
	{
	    /* set filter for only receiving packet with can id 0x1F */
	    rfilter[0].can_id = 0x1F;
	    rfilter[0].can_mask = CAN_SFF_MASK;
	    if(setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter)) < 0)
	    {
	    	perror("set receiving filter error\n");
	    	close(s);
	    	exit(-3);
	    }
	    /* keep reading */
	    while(1){
	        nbytes = read(s, &frame, sizeof(frame));
	        if(nbytes > 0)
	        {
	        	printf("%s ID=%#x data length=%d\n", ifr.ifr_name, frame.can_id, frame.can_dlc);
	        	for (int i=0; i < frame.can_dlc; i++)
	        		printf("%#x ", frame.data[i]);
	        	printf("\n");
	        }
	    }
	}
	/* configure sending */
	else if(!strcmp(argv[1],array[1]))
	{
        /* configure can_id and can data length */
		frame.can_id = 0x1F;
        frame.can_dlc = 8;
        printf("%s ID=%#x data length=%d\n", ifr.ifr_name, frame.can_id, frame.can_dlc);
        /* prepare data for sending: 0x11,0x22...0x88 */
        for (int i=0; i<8; i++)
        {
        	frame.data[i] = ((i+1)<<4) | (i+1);
        	printf("%#x ", frame.data[i]);
        }
        printf("Sent out\n");
        /* Sending data */
        if(write(s, &frame, sizeof(frame)) < 0)
		{
            perror("Send failed");
            close(s);
            exit(-4);
		}
    }
	/* wrong parameter input situation */
	else
	{
	    printf("wrong parameter input\n");
	}

	close(s);
    return 0;
}
