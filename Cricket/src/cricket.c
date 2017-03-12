/*
 ============================================================================
 Name        : server.c
 Author      : Melody
 Version     :
 Copyright   : Your copyright notice
 ============================================================================
 */

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "data.h"



/* return errorcode if wrong*/
void parseMessage(unsigned char* recvbuf, int size, struct cellPacket *parsed) {

	//start_of_parket_id
	recvbuf +=2;

	// parse client id
	parsed->client_id = *(recvbuf);
	recvbuf +=1;

	// check access_permit
	recvbuf += 2;

	// parse segment no.
	parsed->segment_no = *(recvbuf);
	recvbuf += 1;

	// parse length
	parsed->length = *(recvbuf);
	recvbuf += 1;

	// parse technology
	parsed->technology =*(recvbuf);
	recvbuf += 1;

	// parse subscriber_no
	parsed->subscriber_no =*(unsigned long*)(recvbuf);
}

int main(){
  printf("server is running ...\n");
  fflush(stdout);

  struct sockaddr_in serverAddr;
  struct sockaddr_storage serverStorage;
  socklen_t addr_size;

  /*Create UDP socket*/
  int udpSocket = socket(AF_INET, SOCK_DGRAM, 0);


  /*Configure settings in address struct*/
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(7891);
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

  /*Bind socket with address struct*/
  bind(udpSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));


  /*Initialize size variable to be used later on*/
  addr_size = sizeof serverStorage;

  int packetCount = 0;
  fflush(stdout);
  while(1) {
	  packetCount++;

    /* Try to receive any incoming UDP datagram. Address and port of
      requesting client will be stored on serverStorage variable */
	unsigned char recvbuffer[1024];
	memset(recvbuffer, 0, 1024);
    int recvsize = recvfrom(udpSocket,recvbuffer,1024,0,(struct sockaddr *)&serverStorage, &addr_size);


    struct cellPacket parsed;
    parseMessage(recvbuffer, recvsize, &parsed);

    unsigned long cellno1 = 4085546805U;
    unsigned long cellno2 = 4086808821U;
    unsigned long cellno3 = 4086668821U;
    unsigned char tech2 = 2;
    unsigned char tech3 = 3;
    unsigned char tech4 = 4;


    if (((int)(parsed.subscriber_no - cellno1) == 0 && parsed.technology == tech4) || ((int)(parsed.subscriber_no - cellno2) == 0 && parsed.technology == tech2)){
    	/* send access permit to Buffer */
    	int acksize = 14;
    	unsigned char ackbuffer[acksize];
    	memcpy(ackbuffer, start_of_packet_id, 2);
    	*(ackbuffer+2) = parsed.client_id;
    	memcpy(ackbuffer+3, access_permit, 2);
    	*(ackbuffer+5) = parsed.length;
    	*(ackbuffer+6) = parsed.segment_no;
    	*(ackbuffer+7) = parsed.technology;
    	memcpy(ackbuffer+8, &parsed.subscriber_no, 4);
    	memcpy(ackbuffer+12, end_of_packet_id, 2);

    	sendto(udpSocket,ackbuffer,14,0,(struct sockaddr *)&serverStorage,addr_size);


    } else if ((int)(parsed.subscriber_no - cellno3) == 0  && parsed.technology == tech3) {


    	/* send not paid message to Buffer */
    	int acksize = 14;
        unsigned char ackbuffer[acksize];
        memcpy(ackbuffer, start_of_packet_id, 2);
        *(ackbuffer+2) = parsed.client_id;
        memcpy(ackbuffer+3, not_paid, 2);
        *(ackbuffer+5) = parsed.length;
    	*(ackbuffer+6) = parsed.segment_no;
    	*(ackbuffer+7) = parsed.technology;
    	memcpy(ackbuffer+8, &parsed.subscriber_no, 4);
    	memcpy(ackbuffer+12, end_of_packet_id, 2);

    	sendto(udpSocket,ackbuffer,14,0,(struct sockaddr *)&serverStorage,addr_size);
    } else {

    	// send not exist reject

    	// REJ message to Buffer
    	int rejsize = 14;
        unsigned char ackbuffer[rejsize];
    	memcpy(ackbuffer, start_of_packet_id, 2);
    	*(ackbuffer+2) = parsed.client_id;
    	memcpy(ackbuffer+3, not_exist, 2);
    	*(ackbuffer+5) = parsed.length;
    	*(ackbuffer+6) = parsed.segment_no;
    	*(ackbuffer+7) = parsed.technology;
    	memcpy(ackbuffer+8, &parsed.subscriber_no, 4);
    	memcpy(ackbuffer+12, end_of_packet_id, 2);

    	sendto(udpSocket,ackbuffer,14,0,(struct sockaddr *)&serverStorage,addr_size);
    }

  }

  return 0;
}
