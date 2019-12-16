/*
* @Author: nam
* @Date:   2018-09-30 17:16:36
* @Last Modified by:   nam
* @Last Modified time: 2018-10-11 20:51:56
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define MAX 1024

char *seperate(char* buff)
{
	// return NULL if input string is empty
	if (strlen(buff) == 0)
		return NULL;
	
	char numbers[MAX], letters[MAX], *result; 
	int i, i_number = 0, i_letter = 0;
	memset(numbers, '\0', MAX);
	memset(letters, '\0', MAX);
	result = malloc(MAX+1);

	// seperate numbers and letters
	for (i = 0; buff[i] != '\0'; i++)
	{
		if (isdigit(buff[i]))
			numbers[i_number++] = buff[i];
		else if (isalpha(buff[i]))
			letters[i_letter++] = buff[i];
		else
			return NULL;
	}
	sprintf(result, "%s\n%s", numbers, letters);
	return result;
}

int main(int argc, char const *argv[])
{
	int server_sock;
	socklen_t sin_size;
	char buff[MAX+1];
	int bytes_sent, bytes_received;
	struct sockaddr_in server;
	struct sockaddr_in client;

	// valid number of argument
	if (argc != 2)
	{
		printf("Usage: ./server PortNumber\n\n");
		return 0;
	}

	// construct UDP socket
	if ((server_sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		perror("\nError: ");
		exit(0);
	}

	// bind address to socket
	server.sin_family = AF_INET;
	server.sin_port = htons(atoi(argv[1]));
	server.sin_addr.s_addr = INADDR_ANY;
	memset(server.sin_zero, '\0', sizeof server.sin_zero);

	if (bind(server_sock, (struct sockaddr*)&server, sizeof(struct sockaddr)) == -1)
	{
		perror("\nError: ");
		exit(0);
	}

	// communicate with client
	while(1)
	{
		// get string from client
		sin_size = sizeof(struct sockaddr_in);
		bytes_received = recvfrom(server_sock, buff, MAX-1, 0, (struct sockaddr*)&client, &sin_size);

		if (bytes_received < 0)
			perror("\nError: ");
		else
		{
			// handle string from client
			buff[bytes_received] = '\0';

			char *reply = seperate(buff);

			// if string contain symbol return Error
			if (reply == NULL)
				reply = "Error";

			// return result to client
			bytes_sent = sendto(server_sock, reply, strlen(reply), 0, (struct sockaddr*)&client, sin_size);
			if (bytes_sent < 0)
				perror("\nError: ");
		}
	}

	close(server_sock);
	return 0;
}
