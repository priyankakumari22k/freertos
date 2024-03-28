#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/un.h>
#include<unistd.h>
#include<string.h>
#include <netinet/in.h>
#include<pthread.h>

#define BUFSIZE 2024
#define MAX 512
#define TIMEOUT_SECONDS 10
struct data
{
   char *mode_t;
   char *filename_t;
   struct sockaddr cli_addr;
};

void text_mode_transfer(FILE * src_file, int data_sock,struct sockaddr cli_addr)
{
	char buffer[BUFSIZE];
	uint16_t opcode;
	int recv_len;
	size_t cli_size = sizeof(cli_addr);
	char c;
	int i = 4;
	uint16_t block_counter = 1;
	do {
		c = fgetc(src_file);
		if (c != EOF)
		{
			buffer[i] = c;
			i++;
		}

		if(i == MAX+4 || c == EOF)
		{
			printf("i = %d\n",i);
			opcode = htons(3);
			uint16_t block = htons(block_counter);
			memcpy(buffer, &opcode, 2);
			memcpy(buffer + 2, &block, 2);
			printf("sending packet = %d \n",block_counter);
			recv_len = sendto(data_sock, buffer, i, MSG_CONFIRM, (const struct sockaddr *) &cli_addr, sizeof(cli_addr));
			printf("buffer length = %d\n",recv_len);
			memset(buffer, 0, BUFSIZE);

			// wait for ACK response from the client
			
			fd_set readfds;
  			struct timeval timeout;

    			FD_ZERO(&readfds);
    			FD_SET(data_sock, &readfds);

   	 		timeout.tv_sec = TIMEOUT_SECONDS;
    			timeout.tv_usec = 0;
			printf("Waiting for ACK\n");
		        int ready = select(data_sock+1, &readfds, NULL, NULL, &timeout);	
			if (ready == -1) {
        			perror("Error in select");
        			//close(data_sock);
       				 exit(-1);
		        } else if (ready == 0) {
       				 printf("Timeout: No ACK received within %d seconds.\n",10);
    			} else {
				recv_len = recvfrom(data_sock, (char *)buffer, BUFSIZE, MSG_WAITALL, (struct sockaddr *)&cli_addr, (socklen_t *) & cli_size);
			       }
			// retrieve block number from transfer buffer
			memcpy(&block, (uint16_t *) & buffer[2], 2);

			block = ntohs(block);
		 	printf("RECEIVED block number = %d\n",block);
			if (block == block_counter)
			{
				i=4;
				block_counter++;
			}
			else
			{
				i=4;
				fseek(src_file, -512, SEEK_CUR);
			}
	    	}
	}while (c != EOF);

	printf("FULL FILE TRANSFFERED\n");
	fclose(src_file);
	close(data_sock);
}



void binary_mode_transfer(FILE * src_file, int data_sock,struct sockaddr cli_addr)
{
        char buffer[BUFSIZE];
        uint16_t opcode;
        int recv_len;
        size_t cli_size = sizeof(cli_addr);
        char c;
        int i = 4;
        uint16_t block_counter = 1;
        do {
                c = fgetc(src_file);
                if (c != EOF)
                {
                        buffer[i] = c;
                        i++;
                }

                if(i == MAX+4 || c == EOF)
                {
                        printf("i = %d\n",i);
                        opcode = htons(3);
                        uint16_t block = htons(block_counter);
                        memcpy(buffer, &opcode, 2);
                        memcpy(buffer + 2, &block, 2);
                        printf("sending packet = %d \n",block_counter);
                        recv_len = sendto(data_sock, buffer, i, MSG_CONFIRM, (const struct sockaddr *) &cli_addr, sizeof(cli_addr));
                        printf("buffer length = %d\n",recv_len);
                        memset(buffer, 0, BUFSIZE);

                        // wait for ACK response from the client
                        printf("Waiting for ACK\n");
                        recv_len = recvfrom(data_sock, (char *)buffer, BUFSIZE, MSG_WAITALL, (struct sockaddr *)&cli_addr, (socklen_t *) & cli_size);

                        // retrieve block number from transfer buffer
                        memcpy(&block, (uint16_t *) & buffer[2], 2);

                        block = ntohs(block);
                        printf("RECEIVED block number = %d\n",block);
                        if (block == block_counter)
                        {
                                i=4;
                                block_counter++;
                        }
                        else
                        {
                                i=4;
                                fseek(src_file, -512, SEEK_CUR);
                        }
                }
        }while (c != EOF);

        printf("FULL FILE TRANSFFERED\n");
        fclose(src_file);
        close(data_sock);
}



void * Handle_transfer(struct data *info) 
{ 
    sleep(1); 
    char *mode;
    char *file_name;
    mode = info->mode_t;
    file_name = info->filename_t;
    struct sockaddr cli_addr = info->cli_addr;
    printf("Printing from Thread Handler\n");
    printf("mode =%s\n",mode);
    printf("file_name =%s\n",file_name);
    printf("cli_addr =%d\n",cli_addr);

    int data_sock = socket(AF_INET, SOCK_DGRAM, 0);
    FILE *src_file;

    if(strncmp(mode, "netascii", 8) == 0)
    {
	    src_file = fopen(file_name, "r");
	    if (src_file == NULL)
		{
			printf("ERROR:  file not found or opened\n");
			return;
		}
		else
		{
			text_mode_transfer(src_file, data_sock, cli_addr);
		}
    
    }
    else if(strncmp(mode, "octet", 5) == 0)
    {
	    printf("Transfer in binary mode\n");
	    src_file = fopen(file_name, "rb");
            if (src_file == NULL)
                {
                        printf("ERROR:  file not found or opened\n");
                        return;
                }
	    else
                {
                        binary_mode_transfer(src_file, data_sock, cli_addr);
                } 
    }

    return NULL; 
}


void invalid_opcode(int sockfd,struct sockaddr cli_addr)
{
	uint8_t end_string = 0;
	char buffer[BUFSIZE];
	char error_message[512];
	uint16_t opcode = htons(5);
	uint16_t error_code = htons(4);

	memcpy(buffer, &opcode, 2);
	memcpy(buffer + 2, &error_code, 2);
	strcpy(error_message, "Illegal TFTP operation");
	strcpy(buffer + 2, error_message);
	memcpy(buffer + strlen(error_message) + 2, &end_string, 1);

	int sent_len = sendto(sockfd,
			      buffer,
			      strlen(error_message) + 4,
			      MSG_CONFIRM,
			      (const struct sockaddr *)&cli_addr,
			      sizeof(cli_addr));

	printf("Error message correctly sent\n");
	exit(-1);
}


int main(int argc, char *argv[])
{
	int port = atoi(argv[1]);
	int sockfd;
	char log_message[1024]={'\0'};

	struct sockaddr cli_addr;
        size_t cli_size = sizeof(cli_addr);
        int recv_len;
        char buffer[BUFSIZE];
        char file_name[512];
        char mode[10];
        uint16_t opcode;
	
	struct sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        serv_addr.sin_port = htons(port);

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	int bound = bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	
	char ip[17];
	inet_ntop(serv_addr.sin_family, (void *)&serv_addr.sin_addr, ip, sizeof(ip));
	printf("server IP address %s\n",ip);	

	while(1)
	{
//		printf("\nHiiiiiii\n");
		int len=0;
		recv_len = recvfrom(sockfd, (char *)buffer, BUFSIZE, MSG_WAITALL, &cli_addr, (socklen_t *) &cli_size);
//		printf("BELOW RECEIVE\n");
		memcpy(&opcode, (uint16_t *) & buffer, 2);
		opcode = ntohs(opcode);
		strcpy(file_name, buffer + 2);
		strcpy(mode, buffer + 3 + strlen(file_name));
		sprintf(log_message,"Received opcode: %d, file name: %s and mode: %s.",opcode, file_name, mode);	
		printf("%s\n",log_message);


		struct data arg;
		arg.mode_t = mode;
		arg.filename_t = file_name;
		arg.cli_addr = cli_addr;

		if(opcode==1)
		{
			pthread_t thread_id;
			pthread_create(&thread_id, NULL, Handle_transfer,&arg);
//			printf("above thread join\n");
			pthread_join(thread_id, NULL);
			exit(0);	
		}
		else 
		{
		  invalid_opcode(sockfd,cli_addr);
		}

	}

		if(sockfd>0)
                {
   	             close(sockfd);
        	     sockfd=-1;
                }
return 0;
}
