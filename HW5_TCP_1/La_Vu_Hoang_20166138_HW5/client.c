#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFF_SIZE 1024

int main (int argc, char const *argv[])
{
	// valid number of argument
	if (argc != 3)
	{
		printf("Usage: ./client IPAddress PortNumber\n\n");
		return 0;
	}

	int client_sockfd;
	int recvBytes, sendBytes;

	struct sockaddr_in serv_addr;
	
	char buff[BUFF_SIZE];

	// Construct socket
	client_sockfd = socket(AF_INET,SOCK_STREAM,0);
	
	// Specify server address
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[2]));
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	
	// Request to connect server
	if(connect(client_sockfd, (struct sockaddr*)&serv_addr, sizeof(struct sockaddr)) < 0){
		printf("\nError!Can not connect to sever! Client exit imediately! ");
		return 0;
	}
		
	// Communicate with server			
	while(1){

		//send message
		printf("\nInsert string to send (ENTER to quit): ");
		fgets(buff, BUFF_SIZE, stdin);

		// exit if user only input ENTER
		if (buff[0] == '\n') break;

		// replace \n with \0
		buff[strcspn(buff, "\n")] = '\0';

		sendBytes = send(client_sockfd, buff, strlen(buff), 0);
		if(sendBytes <= 0){
			printf("\nConnection closed!\n");
			break;
		}
		
		// receive echo reply
		recvBytes = recv(client_sockfd, buff, BUFF_SIZE-1, 0);
		if(recvBytes <= 0){
			printf("\nError!Cannot receive data from sever!\n");
			break;
		}
		
		// print reply
		buff[recvBytes] = '\0';
		puts("Reply from server:");
		puts(buff);
	}
	
	// Close socket
	close(client_sockfd);
	return 0;
}
