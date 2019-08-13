#include <stdio.h>
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#define MAX 80 
#define PORT 4600 

void process_packet(int sockfd, char *buff, ssize_t len);

int comm(int sockfd) {
    char buff[MAX]; 
    ssize_t amt_read = 0;
    // infinite loop for chat 
    for (;;) { 
        bzero(buff, MAX); 
  
        // read the message from client and copy it in buffer 
        amt_read = read(sockfd, buff, sizeof(buff)); 
        // print buffer which contains the client contents 

        process_packet(sockfd, buff, amt_read);

        bzero(buff, MAX); 
        //n = 0; 
        // copy server message in the buffer 
  
        // and send that buffer to client 
        //write(sockfd, buff, sizeof(buff)); 
  
        // if msg contains "Exit" then server exit and chat ended. 
        //if (strncmp("exit", buff, 4) == 0) { 
            //printf("Server Exit...\n"); 
            //break; 
        //} 
    } 
    return 0;
}

int main(int argc, char *argv[]) {
    int sockfd, connfd;
    unsigned int len; 
    struct sockaddr_in servaddr, cli; 
  
    // socket create and verification 
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    } else {
        printf("Socket successfully created..\n"); 
    }

    bzero(&servaddr, sizeof(servaddr)); 
  
    // assign IP, PORT 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(PORT); 
  
    // Binding newly created socket to given IP and verification 
    if ((bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) { 
        printf("socket bind failed...\n"); 
        exit(0); 
    } else {
        printf("Socket successfully binded..\n"); 
    }
  
    // Now server is ready to listen and verification 
    if ((listen(sockfd, 5)) != 0) { 
        printf("Listen failed...\n"); 
        exit(0); 
    } else {
        printf("Server listening..\n"); 
    }

    len = sizeof(cli); 
  
    // Accept the data packet from client and verification 
    connfd = accept(sockfd, (struct sockaddr*)&cli, &len); 
    if (connfd < 0) { 
        printf("server acccept failed...\n"); 
        exit(0); 
    } else {
        printf("server acccept the client...\n"); 
    }
  
    // Function for chatting between client and server 
    comm(connfd); 
  
    // After chatting close the socket 
    close(sockfd); 
    return 0;
}

/*
0x14, 0x0, 0x44, 0x53, 0x49, 0x54, 0xa, 0x0, 
    0x0, 0x0, 0x35, 0x37, 0x30, 0x38, 0x38, 0x30, 
    0x31, 0x36, 0x34, 0x0, };
in text:
    , , D, S, I, T, 
    , , 
    , , 5, 7, 0, 8, 8, 0, 
    1, 6, 4, , "
    */

char DSIT_HEADER[] = {0x14, 0x0, 0x44, 0x53, 0x49, 0x54, 0xa, 0x0};
char DSIT_RESPONSE[] = {0x37, 0x80, 0x4, 0x64, 0x73, 0x49, 0x4e, 0x1,
    0xfe, 0x4, 0x1, 0x1, 0x1, 0x1, 0xa, 0xfe,
    0xa, 0x35, 0x37, 0x30, 0x38, 0x38, 0x30, 0x31,
    0x36, 0x34, 0x0, 0x1, 0xfe, 0x0, 0xa, 0xfe,
    0x8, 0x35, 0x37, 0x30, 0x38, 0x38, 0x30, 0x31,
    0x36, 0x34, 0xfc, 0x0, 0x1, 0xf9, 0x0, 0x1,
    0xf9, 0x0, 0x1, 0xfd, 0x0, 0x1, 0xfe};

void print_packet(const char *buff, const int len) {
    int i;
    printf("From client: \n {");
    for (i = 0; i < len; i++) {
        if ((i % 16 ) == 0) {
            printf("\n");
        }
        printf("0x%x, ", ((unsigned short)buff[i]) & 0x00ff);
    }
    printf("};\n");
    printf("in text:");
    for (i = 0; i < len; i++) {
        if ((i % 16 ) == 0) {
            printf("\n");
        }
        if (buff[i] == '\r') {
            printf("'\r',");
        }else if (buff[i] == '\n') {
            printf("'\n',");
        } else {
            printf("%c, ", buff[i]);
        }
    }
    printf("\"\n");
}

static char overflow_buff[1<<16];
static ssize_t overflow_buff_pos = 0;
void process_packet(int sockfd, char *buff, ssize_t len) {
    int i, header_offset;
    int msg_len = 0;
    char msg_type[5];
    ssize_t pos;

    msg_type[4] = '\0';
    if (len < 7) {
        fprintf(stderr, "len is too small!  (len = %ld\n", len);
    }


    // DSIT is a special case.
    if (!strncmp(buff, DSIT_HEADER, len)) {
        printf("DSIT Packet Detected:\n");
        print_packet(buff, len);
        printf("Token = '%s'\n", buff+10);

        printf("Sending:'");
        for (i = 0; i < sizeof(DSIT_RESPONSE); i++) {
            if ((i % 8 ) == 0) {
                printf("\n");
            }
            printf("%c, ", DSIT_RESPONSE[i]);
        }
        write(sockfd, DSIT_RESPONSE, sizeof(DSIT_RESPONSE)); 
        return;
    }

    // If we are continuing a previous packed, then construction the new one.
    if (overflow_buff_pos > 0) {
        memcpy(overflow_buff + overflow_buff_pos, buff, len);

        len += overflow_buff_pos;
        //printf("overflow, here is the new constructed packet:\n");
        //print_packet(overflow_buff, len);

        overflow_buff_pos = 0;
        buff = overflow_buff;
    }

    print_packet(buff, len);

    pos = 0;
    while (pos < len) {
        msg_len = (unsigned char)buff[pos];
        if (!buff[pos+1]) {
            header_offset = 2;
        } else {
            header_offset = 3;
        }
        msg_type[0] = buff[pos+header_offset+0];
        msg_type[1] = buff[pos+header_offset+1];
        msg_type[2] = buff[pos+header_offset+2];
        msg_type[3] = buff[pos+header_offset+3];
        if (msg_len < 6) {break;}

        // DSLG has a variable length since it is logging.
        if (!strncmp(msg_type, "DSLG", 4)) {
            msg_len = 11;
            pos += 11;
            while (pos < len && buff[pos]) {
                pos ++;
                msg_len ++;
            }
            printf("msg_type = '%s', msg_len = %d, pos = %ld\n", msg_type, msg_len, pos);
            if (pos < len) {
                printf("Log Message: %s\n", buff + pos - msg_len + 11);
                pos++; // Need to increment past the NULL "\0".
            }
        } else {
            printf("msg_type = '%s', msg_len = %d, pos = %ld\n", msg_type, msg_len, pos);
            pos += msg_len;
        }

    }

    printf("pos = %ld, len = %ld\n", pos, len);
    if (pos >= len) {
        pos -= msg_len;
        memmove(overflow_buff, buff + pos, len - pos);
        overflow_buff_pos = len - pos;
    }
}
