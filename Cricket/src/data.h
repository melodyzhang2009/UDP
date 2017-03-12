/*
 ============================================================================
 Name        : Data.c
 Author      : Melody
 Version     :
 Copyright   : Your copyright notice
 ============================================================================
 */

static const unsigned char start_of_packet_id[] = {0XFF, 0XFF};
static const unsigned char end_of_packet_id[] = {0XFF,0XFF};
static const unsigned char data[] = {0XFF,0XF1};
static const unsigned char ack[] = {0XFF,0XF2};
static const unsigned char reject[] = {0XFF,0XF3};
static const unsigned char reject_out_of_sequence[] = {0XFF,0XF4};
static const unsigned char reject_length_mismatch[] = {0XFF,0XF5};
static const unsigned char reject_end_of_packet_missing[] = {0XFF,0XF6};
static const unsigned char reject_duplicate_packet[] = {0XFF,0XF7};

static const unsigned char access_permit[] = {0XFF, 0XF8};
static const unsigned char not_paid[] = {0XFF,0XF9};
static const unsigned char not_exist[] = {0XFF,0XFA};


struct cellPacket {
	unsigned char client_id;
	unsigned char segment_no;
	unsigned char length;
	unsigned char technology;
	unsigned long subscriber_no;
};

