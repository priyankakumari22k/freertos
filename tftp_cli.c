#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/un.h>
#include<unistd.h>
#include <netinet/in.h>

#define BUFSIZE 1024

char *server_ip;
int server_port;
char transfer_mode[10];
struct sockaddr_in serv_addr;

void send_RRQ(int cli_socket, char *file_name)
{
	int len = 0;
	char buffer[BUFSIZE];
	uint16_t opcode = htons(1);
	uint8_t end_string = 0;
	printf("Enter the required transfer mode\n");
	scanf("%s",transfer_mode);
	//strcpy(transfer_mode,"octet");  //"netascii");
	memcpy(buffer, &opcode, 2);
	len += 2;
	strcpy(buffer + len, file_name);
	len += strlen(file_name);
	memcpy(buffer + len, &end_string, 1);
	len++;
	strcpy(buffer + len, transfer_mode);
        len += strlen(transfer_mode);
	memcpy(buffer + len, &end_string, 1);
	len++;
	int sent_len = sendto(cli_socket,	// client socket
			      buffer,		// transfer buffer
			      len,		// transfer buffer length
			      MSG_CONFIRM,
			      (const struct sockaddr *)&serv_addr,
			      sizeof(serv_addr));
	printf("Read request is sent\n");
}


void send_ACK(int cli_socket, uint16_t block_number)
{
     int len = 0;
     char buffer[BUFSIZE];
     uint16_t opcode = htons(4);
     memcpy(buffer, &opcode, 2);
     len += 2;
     block_number = htons(block_number);
     memcpy(buffer + len, &block_number, 2);
     len += 2;
     int sent_len = sendto(cli_socket, buffer, len, MSG_CONFIRM, (const struct sockaddr *)&serv_addr, sizeof(serv_addr));
     printf("ACK send successfully\n");

}



int main(int argc, char *argv[])
{
	server_ip = argv[1];
	server_port = atoi(argv[2]);

	serv_addr.sin_family = AF_INET;
	inet_pton(AF_INET, server_ip, &serv_addr.sin_addr);
	serv_addr.sin_port = htons(server_port);
	int cli_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if(cli_socket < 0){
		printf("socket not open\n");
	}
	printf("socket file descriptor %d \n ",cli_socket);

	int opcode;
	char source[256]="text.txt";
	send_RRQ(cli_socket, source);

	char buffer[BUFSIZE];
	int addr_len = sizeof(serv_addr);

	// receive response from TFTP Server
	int recv_len=0;
	FILE *dest_file;
	int i = 0;
	 dest_file = fopen("cli_text.txt", "w");
	do
	{
		recv_len = recvfrom(cli_socket, (char *)buffer, BUFSIZE, MSG_WAITALL, (struct sockaddr *)&serv_addr, (socklen_t *) & addr_len);
		memcpy(&opcode, (uint16_t *) & buffer, 2);
		opcode = ntohs(opcode);
		if (opcode == 5)
		{
			char log_message[256];
			// error message opcode found, print a warning error log
			printf("ERROR in transmission\n");
			sprintf(log_message, "Error: %s.", buffer + 2);
			printf("ERROR : %s\n",log_message);
			exit(-1);
		}
		else if(opcode == 3)
		{
			uint16_t block_number;
			memcpy(&block_number, (uint16_t *) & buffer[2], 2);
			block_number = ntohs(block_number);
			printf("RECEIVED block number = %d\n",block_number);
			if (dest_file == NULL)
			{
				printf("File is not opened \n");
			
			}

			for (i = 0; i < recv_len - 4; i++) {
				// skip opcode and block number and write
				fputc(buffer[i + 4], dest_file);
			}
			send_ACK(cli_socket, block_number);
		}
		printf("recv_len value =%d\n",recv_len);

	}while(recv_len == 516);

fclose(dest_file);
printf("File received Successfully\n");
close(cli_socket);
return 0;
}
