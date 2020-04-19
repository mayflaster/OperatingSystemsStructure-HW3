//
// Created by my on 09/01/2020.
//

#ifndef TFTP_PACKET_H
#define TFTP_PACKET_H
#include <stdio.h>
#include <stdint-gcc.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>

#define MAXTRANSMISION 516
#define SECTOWAIT 3

typedef enum opcodeTypes{WRQ=2,DATA=3,ACK=4 } opcode;

struct packet{
    short opcode;// to understand the type of the packet data/ack
    short blockNum;
} __attribute__((packed));


/************************************************
 Name:isWriteRQT
 This function checks if WRQ packed accepted successfully
 If its illigal, prints error and return 0 or negative number
 Otherwise, openes new file and return 1
 *************************************************/

int isWriteRQT(char *buff, FILE **outputFile);
/************************************************
 Name:isData
this function checks if we got block with data opcode
 *************************************************/
bool isData(char *buff);//currentBlock to know if it is valid data block
/************************************************
 Name:identifyBlockNum
this function checks what is the block number of the accepted packet and returns it
 *************************************************/
short identifyBlockNum(char *buff);
/************************************************
 Name:prepareBuffAck
this function prepare the packet before ack sending to the client
 the packet includes the opcode of ack and current ack number
 *************************************************/
void prepareBuffAck(struct packet *buff, short ackNum);


#endif //TFTP_PACKET_H
