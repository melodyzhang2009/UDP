/*
 ============================================================================
 Name        : client.c
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

struct sockaddr * sock_addr;
socklen_t addr_size;
int clientSocket;
/*Receive message from server*/
unsigned char recvbuffer[1024];

void printMessage(unsigned char* buffer, int size) {
	for (int i = 0; i < size; i++) {
		if (i > 0)
			printf(":");
		printf("%02X", *(buffer + i));
	}
	printf("\n\n");
}

void messageToBuffer(unsigned char* buffer, struct cellPacket packet) {
	memcpy(buffer, start_of_packet_id, 2);
	memcpy(buffer + 2, &packet.client_id, 1);
	memcpy(buffer + 3, access_permit, 2);
	memcpy(buffer + 5, &packet.segment_no, 1);
	memcpy(buffer + 6, &packet.length, 1);
	memcpy(buffer + 7, &packet.technology, 1);
	memcpy(buffer + 8, &packet.subscriber_no, 4);
	memcpy(buffer + 12, end_of_packet_id, 2);
}

void sendAndRecv(unsigned char* buffer, int size) {
	int recvsize = -1;
	int retryCount = 0;
	memset(recvbuffer, 0, 1024);
	while (recvsize == -1 && retryCount < 3) {
		/*Send message to server*/
		sendto(clientSocket, buffer, size, 0, sock_addr, addr_size);

		/*show send message */
		printf("Send packet to server: \n");
		printMessage(buffer, size);

		recvsize = recvfrom(clientSocket, recvbuffer, 1024, 0, NULL, NULL);
		retryCount++;
	}

	if (recvsize == -1) {
		printf("Server does not respond  \n\n");
	} else {
			printf("Received ack from server: \n");
		}
		printMessage(recvbuffer, recvsize);

	}

int main() {
	struct sockaddr_in serverAddr;
	/*Create UDP socket*/
	clientSocket = socket(PF_INET, SOCK_DGRAM, 0);
	struct timeval tv;
	tv.tv_sec = 3;      // set timeout 3 sec
	setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	/*Configure settings in address struct*/
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(7891);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
	/*Initialize size variable to be used later on*/
	addr_size = sizeof serverAddr;
	sock_addr = (struct sockaddr *) &serverAddr;


	/* packet 1 * for access permit ack */
	const struct cellPacket packet1 = { .client_id = 1, .segment_no = 1,
			.length = 1, .technology = 4, .subscriber_no = 4085546805U };
	int size1 = 14; // packet1's buffersize
	unsigned char buffer1[size1];
	messageToBuffer(buffer1, packet1);
	sendAndRecv(buffer1, size1);

	/* packet 2 * for not paid ack  */
	struct cellPacket packet2 =  { .client_id = 2, .segment_no = 1,
			.length = 1, .technology = 3, .subscriber_no = 4086668821U };
	int size2 = 14; //packet2's buffersize
	unsigned char buffer2[size2];
	messageToBuffer(buffer2, packet2);
	sendAndRecv(buffer2, size2);

	/* packet 3 * for not exist rej */
	struct cellPacket packet3 =  { .client_id = 3, .segment_no = 1,
			.length = 1, .technology = 2, .subscriber_no = 1508787552U };
	int size3 = 14; //packet3's buffersize
	unsigned char buffer3[size3];
	messageToBuffer(buffer3, packet3);
	sendAndRecv(buffer3, size3);

	return 0;
}
