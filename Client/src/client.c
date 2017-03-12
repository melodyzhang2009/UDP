/*
 ============================================================================
 Name        : client.c
 Author      : Melody
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
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

void messageToBuffer(unsigned char* buffer, struct dataPacket packet) {
	memcpy(buffer, start_of_packet_id, 2);
	memcpy(buffer + 2, &packet.client_id, 1);
	memcpy(buffer + 3, data, 2);
	memcpy(buffer + 5, &packet.segment_no, 1);
	memcpy(buffer + 6, &packet.length, 1);
	memcpy(buffer + 7, packet.payload, packet.length);
	memcpy(buffer + 7 + packet.length, end_of_packet_id, 2);
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
		if (recvsize == 8) {
			printf("Received ack from server: \n");
		} else if (recvsize == 10) {
			printf("Received reject from server: \n");
		}
		printMessage(recvbuffer, recvsize);

	}
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

	/* packet 1 */
	const char msg1[] = "I love Sisi";

	const struct dataPacket packet1 = { .client_id = 1, .segment_no = 1,
			.payload = (unsigned char*) msg1, .length = sizeof(msg1) };
	int size1 = 9 + packet1.length; // packet1's buffersize
	unsigned char buffer1[size1];
	messageToBuffer(buffer1, packet1);
	sendAndRecv(buffer1, size1);

	/* packet 2 * for test error 1 */
	const char msg2[] = "I love Melody";
	struct dataPacket packet2 =  { .client_id = 1, .segment_no = 3,
			.payload = (unsigned char*) msg2, .length = sizeof(msg2) };
	int size2 = 9 + packet2.length; //packet2's buffersize
	unsigned char buffer2[size2];
	messageToBuffer(buffer2, packet2);
	sendAndRecv(buffer2, size2);

	/* packet 3 * for test error 2 */
	struct dataPacket packet3 = packet1;
	packet3.segment_no = 3;
	int size3 = 9 + packet3.length; //packet3's buffersize
	unsigned char buffer3[size3];
	messageToBuffer(buffer3, packet3);
	*(buffer3 + 6) = 8; // length changed to 8;
	sendAndRecv(buffer3, size3);

	/* packet 4 * for test error 3 */
	struct dataPacket packet4 = packet1;
	packet4.segment_no = 4;
	int size4 = 9 + packet4.length;
	unsigned char buffer4[size4];
	messageToBuffer(buffer4, packet4);
	sendAndRecv(buffer4, size4-2);

	/* packet 5 * for test error 4 */
	struct dataPacket packet5 = packet1;
	packet5.segment_no = 4;
	int size5 = 9 + packet5.length;
	unsigned char buffer5[size5];
	messageToBuffer(buffer5, packet5);
	sendAndRecv(buffer5, size5);
	return 0;
}
