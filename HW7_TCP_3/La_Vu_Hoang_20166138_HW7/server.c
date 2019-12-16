#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

#define PORT 5500  /* Port that will be opened */
#define BACKLOG 20 /* Number of allowed connections */
#define MAX 1024

typedef struct node
{
	char username[MAX];
	char password[MAX];
	int status;
	struct node *next;
} node_t;

// load data from text file to linked list
node_t *load_data(char *filename)
{
	int status, count = 0; // number of accounts
	FILE *f;
	char username[MAX], password[MAX];
	node_t *head, *current;
	head = current = NULL;

	// open file data
	printf("Loading data from file...\n");

	if ((f = fopen(filename, "r")) == NULL)
	{
		printf("Cannot open file!\n");
		exit(0);
	}

	// load accounts to linked list
	while (fscanf(f, "%s %s %d\n", username, password, &status) != EOF)
	{
		// create new node
		node_t *node = malloc(sizeof(node_t));
		strcpy(node->username, username);
		strcpy(node->password, password);
		node->status = status;

		// add node to list
		if (head == NULL)
			current = head = node;
		else
			current = current->next = node;
		count++;
	}

	fclose(f);
	printf("LOADED SUCCESSFULY %d ACCOUNT(S)\n", count);
	return head;
}

// find a node exist in linked list given username
node_t *find_node(node_t *head, char *username)
{
	node_t *current = head;
	while (current != NULL)
	{
		if (0 == strcmp(current->username, username))
			return current;
		current = current->next;
	}
	return NULL;
}

// save list to text file
void save_list(node_t *head, char *filename)
{
	FILE *f;
	f = fopen(filename, "w");
	node_t *current;
	for (current = head; current; current = current->next)
		fprintf(f, "%s %s %d\n", current->username, current->password, current->status);
	fclose(f);
}

int main(int argc, char const *argv[])
{
	// valid number of argument
	if (argc != 2)
	{
		printf("Usage: ./server PortNumber\n\n");
		return 0;
	}

	char filename[] = "account.txt";
	int i, maxi, maxfd, listenfd, connfd, sockfd;
	int nready, client[FD_SETSIZE];
	ssize_t ret;
	fd_set readfds, allset;
	socklen_t clilen;
	struct sockaddr_in cliaddr, servaddr;

	char username[MAX], password[MAX], *reply;
	node_t *found;
	int bytes_sent;

	// load file txt to linked list
	node_t *account_list = load_data(filename);

	//Step 1: Construct a TCP socket to listen connection request
	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{ /* calls socket() */
		perror("\nError: ");
		return 0;
	}

	//Step 2: Bind address to socket
	memset(&servaddr, '\0', sizeof servaddr);
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(argv[1]));
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
	{
		perror("\nError: ");
		return 0;
	}

	//Step 3: Listen request from client
	if (listen(listenfd, BACKLOG) == -1)
	{ /* calls listen() */
		perror("\nError: ");
		return 0;
	}

	maxfd = listenfd; /* initialize */
	maxi = -1;		  /* index into client[] array */
	for (i = 0; i < FD_SETSIZE; i++)
		client[i] = -1; /* -1 indicates available entry */
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);

	puts("Server up and running...\n");

	//Step 4: Communicate with clients
	while (1)
	{
		readfds = allset; /* structure assignment */
		nready = select(maxfd + 1, &readfds, NULL, NULL, NULL);
		if (nready < 0)
		{
			perror("\nError: ");
			return 0;
		}

		if (FD_ISSET(listenfd, &readfds))
		{ /* new client connection */
			clilen = sizeof(cliaddr);
			if ((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen)) < 0)
				perror("\nError: ");
			else
			{
				printf("You got a connection from %s\n", inet_ntoa(cliaddr.sin_addr)); /* prints client's IP */
				for (i = 0; i < FD_SETSIZE; i++)
					if (client[i] < 0)
					{
						client[i] = connfd; /* save descriptor */
						break;
					}
				if (i == FD_SETSIZE)
				{
					printf("\nToo many clients");
					close(connfd);
				}

				FD_SET(connfd, &allset); /* add new descriptor to set */
				if (connfd > maxfd)
					maxfd = connfd; /* for select */
				if (i > maxi)
					maxi = i; /* max index in client[] array */

				if (--nready <= 0)
					continue; /* no more readable descriptors */
			}
		}

		for (i = 0; i <= maxi; i++)
		{ /* check all clients for data */
			if ((sockfd = client[i]) < 0)
				continue;
			if (FD_ISSET(sockfd, &readfds))
			{
				// receive username
				if (0 >= (ret = recv(sockfd, username, MAX - 1, 0)))
				{
					printf("\nConnection closed\n");
					break;
				}
				username[ret] = '\0';

				// check username existence
				if ((found = find_node(account_list, username)))
				{
					if (found->status == 1)
						reply = "1"; // username found
					else
						reply = "2"; // username found but has been locked
				}
				else
					reply = "0"; // username not found

				// echo to client
				if (0 >= (bytes_sent = send(sockfd, reply, strlen(reply), 0)))
				{
					printf("\nConnection closed\n");
					break;
				}

				int count = 0; // count password repeatation

				while (1)
				{
					// receive password
					memset(password, '\0', MAX);
					if (0 >= (ret = recv(sockfd, password, MAX - 1, 0)))
					{
						printf("\nConnection closed\n");
						break;
					}
					password[ret] = '\0';

					// validate password
					if (0 == strcmp(found->password, password))
						reply = "1"; // pass valid, reply 1
					else
					{
						count++;
						if (count == 3)
						{
							reply = "2";	   // wrong pass 3 times, reply 2
							found->status = 0; // then lock account
						}
						else
							reply = "0"; // wrong pass < 3 times, reply 0
					}

					// echo to client
					if (0 >= (bytes_sent = send(sockfd, reply, strlen(reply), 0)))
					{
						printf("\nConnection closed\n");
						break;
					}
				}
				// save linked list state
				save_list(account_list, filename);

				FD_CLR(sockfd, &allset);
				close(sockfd);
				client[i] = -1;

				if (--nready <= 0)
					break; /* no more readable descriptors */
			}
		}
	}

	return 0;
}