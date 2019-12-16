
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#define BUFF_SIZE 255

char num[BUFF_SIZE];
char charr[BUFF_SIZE];
int error = 0;
void processRecvBuff(char *receive){	//process message from client with ASCII
	int count_Num = 0, count_Char = 0;
	memset(num,'\0',sizeof(num));
	memset(charr,'\0', sizeof(charr));
	for (int i = 0; i < strlen(receive); i++)
	{
		if(receive[i] > 47 && receive[i] < 58){
			num[count_Num] = receive[i];
			count_Num ++;
		}
		else 
			if((receive[i] > 64 && receive[i] < 91) || (receive[i] > 96 && receive[i] < 123)){
				charr[count_Char] = receive[i];
				count_Char ++;
			}
			else {
				error = 1;
				break;
			}
	}
}

int main(int argc, char *argv[])
{
	int sockfd, rcvSize;
	unsigned int len;
	char buff[BUFF_SIZE];
	char recvBuff[BUFF_SIZE];
	char done[BUFF_SIZE];
	struct sockaddr_in servaddr, cliaddr1, cliaddr2;
	short serv_PORT;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <UDP SERVER PORT>\n", argv[0]);
		exit(1);
	}
	serv_PORT = atoi(argv[1]);
//Step 1: Construct socket
	if((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0){
		perror("Error: ");
		return 0;
	}
//Step 2: Bind address to socket
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(serv_PORT);

	if(bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
		perror("Error: ");
		return 0;
	}
	printf("Server started!\n");

	len = sizeof(cliaddr1);
	recvfrom(sockfd, recvBuff, BUFF_SIZE, 0,(struct sockaddr *) &cliaddr1, &len); //connect with client1
	strcpy(buff,"user1");
	sendto(sockfd, buff, strlen(buff), 0, (struct sockaddr *) &cliaddr1, sizeof(cliaddr1));

	len = sizeof(cliaddr2);
	recvfrom(sockfd, recvBuff, BUFF_SIZE, 0,(struct sockaddr *) &cliaddr2, &len); //connect with client2
	strcpy(buff,"user2");
	sendto(sockfd, buff, strlen(buff), 0, (struct sockaddr *) &cliaddr2, sizeof(cliaddr2));


	
//Step 3: Communicate with client
	for ( ; ; )
	{
		memset(buff,'\0', sizeof(buff));
		memset(done,'\0', sizeof(done));
		memset(recvBuff, '\0', sizeof(recvBuff));
		len = sizeof(cliaddr1);
		rcvSize = recvfrom(sockfd, recvBuff, BUFF_SIZE, 0,(struct sockaddr *) &cliaddr1, &len); //receive message from client
		if(rcvSize < 0){
			perror("Error: ");
			return 0;
		}
		recvBuff[rcvSize] = '\0';

		printf("Receive from client: %s\n", recvBuff);

		processRecvBuff(recvBuff);
		
		if(error == 1){
			strcpy(buff,"error");
			sendto(sockfd, buff, strlen(buff), 0,(struct sockaddr *) &cliaddr2, sizeof(cliaddr2)); //error message
			error = 0;
		}
		else{
			if(num[0] != '\0')
				sendto(sockfd, num, rcvSize, 0,(struct sockaddr *) &cliaddr2, sizeof(cliaddr2));
			if(charr[0] != '\0')
				sendto(sockfd, charr, rcvSize, 0,(struct sockaddr *) &cliaddr2, sizeof(cliaddr2));
		}

		if(strcmp(recvBuff,"exit") == 0) exit(1);
	}
	return 0;
}
