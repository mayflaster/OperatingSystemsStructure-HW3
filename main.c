
#include "Packet.h"

void error(char *msg) {

    perror(msg);
    exit(1);
}
int main(int argc, char **argv) {

    //const int WAIT_FOR_PACKET_TIMEOUT = 3;
    const int NUMBER_OF_FAILURES = 7;
    struct sockaddr_in serverAddr ;//initiate new struct
    int sockfd = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if (sockfd < 0)
        error("TTFTP ERROR");
// Set fields:
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    unsigned short portno= atoi(argv[1]);
    serverAddr.sin_port = htons(portno);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (argc < 2) {
        error("TTFTP ERROR");
    }

    if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) { // connect socket to server
        close(sockfd);
        error("TTFTP ERROR");
    }
    //initiate new struct
    socklen_t clientAddLen;
    struct timeval time_val_st; //for checking time_out error
    time_val_st.tv_sec = SECTOWAIT;
    time_val_st.tv_usec =0;
    // int ack=0; //current block number
    int select_;//select function return value
    int receiveLen;//len of the info that the client sent to the server
    int fd;
    int timeoutExpiredCount = 0;
    bool fatalErr=0;
    struct packet ackPacket = {0,0};
    bool notData= false;

    while(true) {
        // printf("here1");
        int lastWriteSize=0;
        timeoutExpiredCount = 0;
        fatalErr = false;
        short ack=0;
        struct sockaddr_in ClientAddr ={0} ;
        clientAddLen = sizeof(ClientAddr);
        notData = false;
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        // printf("here2");

        char buf[MAXTRANSMISION] = {0};
        receiveLen = recvfrom(sockfd, buf, MAXTRANSMISION, 0, (struct sockaddr *) &ClientAddr, &clientAddLen);
        if (receiveLen < 0) {
            close(sockfd);
            error("TTFTP ERROR");
        }

        time_val_st.tv_sec = SECTOWAIT;
        time_val_st.tv_usec =0;
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        /* select_= select(sockfd+1,&read_fds, NULL,NULL,  &time_val_st);
         if(select_ < 0 ) {
             close(sockfd);
             fclose(file);
             error("TTFTP ERROR: Select failed/n");

         }*/
        FILE * file=NULL;
        fd = isWriteRQT(buf,&file);
        if(fd<0){
            close(sockfd);
            printf("FLOWERROR: illegal request\n");
            return 0;
        }
        if(fd == 0){
            printf("FLOWERROR: illegal request\n");
            continue;
        }
        prepareBuffAck(&ackPacket, ack);
        if(sendto(sockfd,(void *)(&ackPacket), sizeof(ackPacket), 0 ,(struct sockaddr *) &ClientAddr, clientAddLen)<0){
            close(sockfd);
            fclose(file);
            error("TTFTP ERROR");
        }
        printf("OUT:ACK,%d\n",ackPacket.blockNum);
        do {
            do {


                do {

                    // TODO: Wait WAIT_FOR_PACKET_TIMEOUT to see if something appears
                    // for us at the socket (we are waiting for DATA)
                    time_val_st.tv_sec = SECTOWAIT;
                    time_val_st.tv_usec =0;
                    FD_ZERO(&read_fds);
                    FD_SET(sockfd, &read_fds);

                    select_= select(sockfd+1,&read_fds, NULL,NULL,  &time_val_st);

                    if(select_ < 0 ) {
                        close(sockfd);
                        fclose(file);
                        error("TTFTP ERROR");

                    }
                    if (select_>0) {//  if there was something at the socket and we are here not because of a timeout
                        notData=false;
                        receiveLen = recvfrom(sockfd, buf, MAXTRANSMISION, 0, (struct sockaddr *) &ClientAddr, &clientAddLen);

                        //  if(receiveLen<MAXTRANSMISION){
                        //           buf[receiveLen]='\0';
                        // }
                        if (receiveLen < 0) {
                            close(sockfd);
                            error("TTFTP ERROR");
                        }
                    }
                    if (select_==0) {// TODO: Time out expired while waiting for data to appear at the socket
                        printf("FLOWERROR: Time Expired\n");
                        prepareBuffAck(&ackPacket, ack);
                        if(sendto(sockfd, (void *)(&ackPacket), sizeof(ackPacket) ,0 ,(struct sockaddr *) &ClientAddr, clientAddLen)<0){
                            close(sockfd);
                            fclose(file);
                            error("TTFTP ERROR");
                        }
                        printf("OUT:ACK %d\n", ack);
                        notData = true;
                        timeoutExpiredCount++;
                    }

                    if (timeoutExpiredCount >= NUMBER_OF_FAILURES) {
                        //close(sockfd);
                        //fclose(file);
                        printf("FLOWERROR: More than allowed number of failures\n");
                        fatalErr = true;
                        notData = false;
                    }

                } while (notData);// TODO: Continue while some socket was ready
                // but recvfrom somehow failed to read the data
                if(!fatalErr){
                    if (!isData(buf)) // TODO: We got something else but DATA
                    {
                        printf("FLOWERROR: Illegal opcode packet\n");
                        fatalErr = true;
                        break;
                        // FATAL ERROR BAIL OUT
                    }
                    if (identifyBlockNum(buf) != ack+1)// TODO: The incoming block number is not what we have
                        // expected, i.e. this is a DATA pkt but the block number
                        // in DATA was wrong (not last ACK’s block number + 1)
                    {
                        printf("FLOWERROR: Wrong block number\n");
                        fatalErr = true;
                        break;
                        // FATAL ERROR BAIL OUT
                    }
                }
            } while (false);
            if (!fatalErr) {
                timeoutExpiredCount =0;
                printf("IN:DATA,%d, %d\n",ack+1, receiveLen );
                ack++;
                prepareBuffAck(&ackPacket, ack);
                if (sendto(sockfd, (void *)(&ackPacket), sizeof(ackPacket) , 0,
                           (struct sockaddr *) &ClientAddr, clientAddLen) < 0) {
                    close(sockfd);
                    fclose(file);
                    error("TTFTP ERROR");
                }
                printf("OUT:ACK %d\n", ack);
                lastWriteSize = fwrite(buf+4, sizeof(char), receiveLen-4,file); // write next bulk of data
                if(lastWriteSize!=receiveLen-4){
                    close(sockfd);
                    fclose(file);
                    error("TTFTP ERROR");
                }
                printf("WRITING:%d\n", lastWriteSize);

                //char buf[MAXTRANSMISION] = {0};

                if (lastWriteSize < MAXTRANSMISION-4){
                    //close(sockfd);
                    fclose(file);
                    printf("RECVOK\n");
                }
// TODO: send ACK packet to the client
            }
            else {
                // #TODO: close the output file
                printf("RECVFAIL\n");
                fclose(file);
                //close(sockfd);
                break;
            }
        } while (lastWriteSize == MAXTRANSMISION-4); // Have blocks left to be read from client (not end of transmission)*/
    }

}