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
int parseMessage(unsigned char* recvbuf, int size, struct dataPacket *parsed) {
	// parse end_of_packet_id
	if (*(short *)(recvbuf+size-2) != *(short *)end_of_packet_id) {
		return -1;
	}

	// check start_of_packet_id
	if (*(short *)recvbuf != *(short *)start_of_packet_id) {
		return -1;
	}
	recvbuf +=2;

	// parse client id
	parsed->client_id = *(recvbuf);
	recvbuf +=1;

	// check type is data
	if (*(short *)recvbuf != *(short *)data) {
		return -1;
	}
	recvbuf += 2;

	// parse segment no.
	parsed->segment_no = *(recvbuf);
	recvbuf += 1;

	// parse length
	parsed->length = *(recvbuf);
	recvbuf += 1;

	// check length
	if (parsed->length + 9 != size) {
		return -1;
	}

	// parse payload
	parsed->payload = recvbuf;
	recvbuf += parsed->length;

	// parse end_of_packet_id
	if (*(short *)recvbuf != *(short *)end_of_packet_id) {
			return -1;
	}
	return 0;
}

int main(){
  printf("server is running ...");
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

  while(1) {
	  packetCount++;

    /* Try to receive any incoming UDP datagram. Address and port of
      requesting client will be stored on serverStorage variable */
	unsigned char recvbuffer[1024];
	memset(recvbuffer, 0, 1024);
    int recvsize = recvfrom(udpSocket,recvbuffer,1024,0,(struct sockaddr *)&serverStorage, &addr_size);

    struct dataPacket parsed;
    int result = parseMessage(recvbuffer, recvsize, &parsed);
    if (result == 0 && parsed.segment_no == packetCount) {
    	// send ack

    	/* ACK message to Buffer */
    	int acksize = 8;
        unsigned char ackbuffer[acksize];
        memcpy(ackbuffer, start_of_packet_id, 2);
        *(ackbuffer+2) = parsed.client_id;
        memcpy(ackbuffer+3, ack, 2);
    	*(ackbuffer+5) = parsed.segment_no;
    	memcpy(ackbuffer+6, end_of_packet_id, 2);

    	sendto(udpSocket,ackbuffer,8,0,(struct sockaddr *)&serverStorage,addr_size);
    } else {
    	// send reject

    	// REJ message to Buffer
    	int rejsize = 10;
    	unsigned char rejbuffer[rejsize];
    	memcpy(rejbuffer, start_of_packet_id, 2);
        *(rejbuffer+2) = parsed.client_id;
    	memcpy(rejbuffer+3, reject, 2);
    	if (*(short *)(recvbuffer + recvsize - 2) != *(short *)end_of_packet_id) {
    		memcpy(rejbuffer+5, reject_end_of_packet_missing, 2);
    	} else if (parsed.length != recvsize - 9) {
    		memcpy(rejbuffer+5, reject_length_mismatch, 2);
    	} else if (parsed.segment_no > packetCount) {
    		memcpy(rejbuffer+5, reject_out_of_sequence, 2);
    	} else if (parsed.segment_no == packetCount -1) {
    		memcpy(rejbuffer+5, reject_duplicate_packet, 2);
    		packetCount--;
    	}
    	*(rejbuffer+7) = parsed.segment_no;
    	memcpy(rejbuffer+8, end_of_packet_id, 2);

    	sendto(udpSocket,rejbuffer,10,0,(struct sockaddr *)&serverStorage,addr_size);
    }

  }

  return 0;
}
