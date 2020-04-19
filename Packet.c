//
// Created by my on 09/01/2020.
//


#include "Packet.h"

int isWriteRQT(char *buff, FILE ** outputFile){
    short opcode ;
    /*char *ptr=(char *)malloc(sizeof(buff)+1);
    if(!ptr){
        perror("TTFTP ERROR: Error malloc\n");
        return -1;

    }*/

    memcpy(&opcode, buff, 2);
    //strcpy(ptr, buff);
    //ptr[sizeof(buff)] = '\0';
    //short opcode=*(short *) ptr;
    if (ntohs(opcode)!= WRQ) return 0;
    //ptr += 2;
    char* file_name=strdup(buff+2);
    //printf("file name is %s",file_name);
    //ptr += strlen(file_name)+1;
    char* mode=strdup(buff+2+strlen(file_name)+1);
    printf("IN:WRQ,%s,%s\n",file_name, mode);
    *outputFile = fopen(file_name,"w+");

    if (!*outputFile){
        //
        perror("TTFTP ERROR");
        free(file_name);
        free(mode);
        //free(ptr);
        return -1;

    }
    // printf("free file name");
    free(file_name);
    //printf("free mode:");
    free(mode);
    // printf("returnfrom func");
    // free(ptr);
    return 1;

}


bool isData(char *buff){
    short opcode;
    memcpy(&opcode, buff, 2);
    // printf("data opcode is %hd \n",ntohs(opcode) );
    if (ntohs(opcode) != DATA) return false;
    return true;
}

short identifyBlockNum(char *buff){
    short blockNum;
    memcpy(&blockNum, buff+2, 2);
    return ntohs(blockNum);
}



void prepareBuffAck(struct packet *buff, short ackNum){
    buff->opcode = htons(ACK);
    //printf("%hd buffopcode\n",buff->opcode);
    buff->blockNum = htons(ackNum);
    //printf("%hd blocknum\n",buff->blockNum);
}
