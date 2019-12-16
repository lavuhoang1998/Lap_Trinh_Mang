#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define BUFF_SIZE 255
int main(int argc, char *argv[])
{
	int sockfd, rcvBytes, sendBytes;
	socklen_t sin_size;
	char buff[BUFF_SIZE], user[20];
	struct sockaddr_in servaddr;

	char *server_IP;
	short server_PORT;

	if (argc != 3) {
		fprintf(stderr, "Usage: %s <ServerIP> [<EchoPort>]\n", argv[0]);
		exit(1);
	}
	server_IP = argv[1];
	server_PORT = atoi(argv[2]);

//Step 1: Construct socket
	if((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
	{
		perror("Error: ");
		return 0;
	}
//Step 2: Define the address of the server
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(server_IP);
	servaddr.sin_port = htons(server_PORT);

	sin_size = sizeof(servaddr);
	strcpy(buff, "Hello");
	sendto(sockfd, buff, strlen(buff), 0,(struct sockaddr *) &servaddr, sin_size); //send message to server
	recvfrom(sockfd, user, BUFF_SIZE, 0,(struct sockaddr *) &servaddr, &sin_size);
//Step 3: Communicate with server

	user[strlen(user) -1] = '\0';
	printf("%s\n", user);
	if(strcmp(user,"user1") == 0){

		printf("Enter to exit\n");
		do {
			printf("Send to server: ");
			fgets(buff, BUFF_SIZE, stdin);
			if(buff[0] == '\n'){
				strcpy(buff,"exit");
				sendto(sockfd, buff, strlen(buff), 0,(struct sockaddr *) &servaddr, sizeof(servaddr));
				exit(1);
			}
			buff[strlen(buff) - 1] = '\0';
		//scanf("%s", buff);
		
			sin_size = sizeof(servaddr);
			sendBytes = sendto(sockfd, buff, strlen(buff), 0,(struct sockaddr *) &servaddr, sin_size); //send message to server
			if(sendBytes < 0){
				perror("Error ");
				return 0;
			}
		} while(1);
	}

	if(strcmp(user,"user2") == 0){
		for(;;){
			rcvBytes = recvfrom(sockfd, buff, BUFF_SIZE, 0,(struct sockaddr *) &servaddr, &sin_size); //receive message from server
			if(rcvBytes < 0){
				perror("Error 2");
				return 0;
			}
			buff[rcvBytes] = '\0';
			if(strcmp(buff,"exit") == 0) exit(1);

			printf("%s\n", buff);
		}
	}
	
	close(sockfd);
	return 0;
}