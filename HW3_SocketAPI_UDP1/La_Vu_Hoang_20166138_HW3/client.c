#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#define BUFF_SIZE 1024

int main (int argc, char const *argv[])
{
	int client_sock;
	char buff[BUFF_SIZE];
	struct sockaddr_in server_addr;
	int bytes_sent,bytes_received;
	socklen_t sin_size;
	
	// valid number of argument
	if (argc != 3)
	{
		printf("Usage: ./client IPAddress PortNumber\n\n");
		return 0;
	}

	// Construct a UDP socket
	if ((client_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0 )
	{
		perror("\nError: ");
		exit(0);
	}

	// Define the address of the server
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	
	while (1)
	{
		// Communicate with server
		printf("\nInsert string to send (ENTER to quit): ");
		fgets(buff, BUFF_SIZE, stdin);

		// exit if user only input ENTER
		if (buff[0] == '\n') break;

		// replace \n with \0
		buff[strcspn(buff, "\n")] = '\0';

		// send to server
		sin_size = sizeof(struct sockaddr);
		bytes_sent = sendto(client_sock, buff, strlen(buff), 0, (struct sockaddr *) &server_addr, sin_size);
		if(bytes_sent < 0)
		{
			perror("Error: ");
			close(client_sock);
			return 0;
		}

		// send reply
		bytes_received = recvfrom(client_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *) &server_addr, &sin_size);
		if(bytes_received < 0)
		{
			perror("Error: ");
			close(client_sock);
			return 0;
		}
		
		// print out reply
		buff[bytes_received] = '\0';
		printf("Reply from server:\n");
		puts(buff);
	}

	close(client_sock);
	return 0;
}
