#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX 1024

int main(int argc, char const *argv[])
{
	// valid number of argument
	if (argc != 3)
	{
		printf("Usage: ./client IPAddress PortNumber\n\n");
		return 0;
	}

	int client_sock;
	char username[MAX], password[MAX], buff[MAX];
	struct sockaddr_in server_addr;
	int bytes_sent, bytes_received;

	// Construct socket
	client_sock = socket(AF_INET, SOCK_STREAM, 0);

	// Specify server address
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);

	// Request to connect server
	if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0)
	{
		printf("\nError!Can not connect to sever! Client exit imediately! ");
		return 0;
	}

	// Communicate with server

	// get username
	puts("Please enter username and password");
	printf("Username: ");
	fgets(username, MAX, stdin);
	username[strcspn(username, "\n")] = '\0';

	// send username to server
	if (0 >= (bytes_sent = send(client_sock, username, strlen(username), 0)))
	{
		printf("\nConnection closed!\n");
		return 0;
	}

	// receive server reply
	if (0 >= (bytes_received = recv(client_sock, buff, MAX - 1, 0)))
	{
		printf("\nError!Cannot receive data from sever!\n");
		return 0;
	}

	// exit if user not found on server
	buff[bytes_received] = '\0';
	if (0 == strcmp(buff, "0"))
	{
		puts("Username not found, now exitting...\n");
		return 0;
	}
	else if (0 == strcmp(buff, "2"))
	{
		puts("Your account has been locked, now exitting...\n");
		return 0;
	}

	while (1)
	{
		// get password
		printf("Password: ");
		fgets(password, MAX, stdin);
		password[strcspn(password, "\n")] = '\0';

		// send password to server
		if (0 >= (bytes_sent = send(client_sock, password, strlen(password), 0)))
		{
			printf("\nConnection closed!\n");
			return 0;
		}

		// receive server reply
		memset(buff, '\0', MAX);
		if (0 >= (bytes_received = recv(client_sock, buff, MAX - 1, 0)))
		{
			printf("\nError!Cannot receive data from sever!\n");
			return 0;
		}
		buff[bytes_received] = '\0';

		// analyze server reply
		if (0 == strcmp(buff, "0"))
		{ // if pass is wrong < 3 times
			puts("\nPassword is wrong, please input again\n");
			continue;
		}
		else if (0 == strcmp(buff, "2"))
		{ // if pass is wrong 3 times
			puts("\nPassword is wrong 3rd time, your account has been locked\n");
			return 0;
		}
		else if (0 == strcmp(buff, "1"))
		{ // if pass is right
			puts("\nLogged in, press ENTER to log out and exit\n");
			getchar();
			return 0;
		}
		else
		{
			// debug section
			puts(buff);
			puts("Server error\n");
			return 1;
		}
	}

	// Close socket
	close(client_sock);
	return 0;
}