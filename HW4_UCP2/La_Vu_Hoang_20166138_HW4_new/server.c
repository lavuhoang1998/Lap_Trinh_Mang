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

char *seperate(char* buff)
{
	// return NULL if input string is empty
	if (strlen(buff) == 0)
		return NULL;
	
	char numbers[BUFF_SIZE], letters[BUFF_SIZE], *result; 
	int i, count_number = 0, count_letter = 0;
	memset(numbers, '\0', BUFF_SIZE);
	memset(letters, '\0', BUFF_SIZE);
	result = malloc(BUFF_SIZE+1);

	// seperate numbers and letters
	for (i = 0; buff[i] != '\0'; i++)
	{
		if (isdigit(buff[i]))
			numbers[count_number++] = buff[i];
		else if (isalpha(buff[i]))
			letters[count_letter++] = buff[i];
		else
			return NULL;
	}
	sprintf(result, "%s\n%s", numbers, letters);
	return result;
}

int main(int argc, char *argv[])
{
	int sockfd, rcvBytes, sendBytes;
	socklen_t sin_size;
	char buff[BUFF_SIZE];
	char recvBuff[BUFF_SIZE];
	char done[BUFF_SIZE];

	struct sockaddr_in servaddr;
	struct sockaddr_in cliaddr1, cliaddr2;
	
	short server_PORT;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <UDP SERVER PORT>\n", argv[0]);
		exit(1);
	}
	server_PORT = atoi(argv[1]);

//Step 1: Construct socket
	if((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0){
		perror("Error: ");
		return 0;
	}

//Step 2: Bind address to socket
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(server_PORT);

	if(bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
		perror("Error: ");
		return 0;
	}
	printf("Server started!\n");

	sin_size = sizeof(cliaddr1);
	recvfrom(sockfd, recvBuff, BUFF_SIZE, 0,(struct sockaddr *) &cliaddr1, &sin_size); //connect with client1
	strcpy(buff,"user1");
	sendto(sockfd, buff, strlen(buff), 0, (struct sockaddr *) &cliaddr1, sizeof(cliaddr1));

	sin_size = sizeof(cliaddr2);
	recvfrom(sockfd, recvBuff, BUFF_SIZE, 0,(struct sockaddr *) &cliaddr2, &sin_size); //connect with client2
	strcpy(buff,"user2");
	sendto(sockfd, buff, strlen(buff), 0, (struct sockaddr *) &cliaddr2, sizeof(cliaddr2));


	
//Step 3: Communicate with client
	while(1)
	{
		memset(buff,'\0', sizeof(buff));
		memset(done,'\0', sizeof(done));
		memset(recvBuff, '\0', sizeof(recvBuff));
		sin_size = sizeof(cliaddr1);
		rcvBytes = recvfrom(sockfd, recvBuff, BUFF_SIZE, 0,(struct sockaddr *) &cliaddr1, &sin_size); //receive message from client
		if(rcvBytes < 0){
			perror("Error: ");
			return 0;
		}
		else{
			// handle string from client
			recvBuff[rcvBytes] = '\0';
			printf("%s\n", recvBuff);
			char *reply = seperate(recvBuff);

			// if string contain symbol return Error
			if (reply == NULL)
				reply = "Error";

			// return result to client
			sendBytes = sendto(sockfd, reply, strlen(reply), 0, (struct sockaddr*)&cliaddr2, sin_size);
			if (sendBytes < 0)
				perror("\nError: ");
		}
	
		if(strcmp(recvBuff,"exit") == 0) exit(1);
	}
	close(sockfd);
	return 0;
}
