#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BACKLOG 5
#define MAX 1024

// account node
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
	
	if((f = fopen(filename, "r")) == NULL)
	{
		printf("Cannot open file!\n");
		exit(0);
	}

	// load accounts to linked list
	while(fscanf(f, "%s %s %d\n", username, password, &status) != EOF)
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
	while(current != NULL)
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
	char filename[] = "account.txt";

	// valid number of argument
	if (argc != 2)
	{
		printf("Usage: ./server PortNumber\n\n");
		return 0;
	}

	int listen_sock, conn_sock;
	char username[MAX], password[MAX], *reply;
	int bytes_sent, bytes_received;
	struct sockaddr_in server;
	struct sockaddr_in client;
	socklen_t sin_size;
	node_t *found;
	int pid;

	// load file txt to linked list
	node_t *account_list = load_data(filename);

	// Construct a TCP socket to listen connection request
	if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){
		perror("\nError: ");
		return 0;
	}
	
	// Bind address to socket
	memset(&server, '\0', sizeof server);
	server.sin_family = AF_INET;         
	server.sin_port = htons(atoi(argv[1]));
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(listen_sock, (struct sockaddr*)&server, sizeof(server))==-1){
		perror("\nError: ");
		return 0;
	}     
	
	// Listen request from client
	if (listen (listen_sock, BACKLOG) == -1){
		perror("\nError: ");
		return 0;
	}
	
	puts("Server up and running...\n");

	// Communicate with client
	while(1) 
	{
		// accept request
		sin_size = sizeof(struct sockaddr_in);
		if ((conn_sock = accept(listen_sock,( struct sockaddr *)&client, &sin_size)) == -1) 
			perror("\nError: ");
  
		printf("\nYou got a connection from %s\n", inet_ntoa(client.sin_addr) ); /* prints client's IP */

		// start conversation on other fork
		pid = fork();
		if (pid < 0){
			perror("Error");
			return 1;
		}

		// on child process
		if (pid == 0) {
			// receive username
			if (0 >= (bytes_received = recv(conn_sock, username, MAX-1, 0))) {
				printf("\nConnection closed\n");
				break;
			}
			username[bytes_received] = '\0';

			// check username existence
			if ((found = find_node(account_list, username))) {
				if (found->status == 1)
					reply = "1"; // username found
				else
					reply = "2"; // username found but has been locked
			}
			else
				reply = "0"; // username not found

			// echo to client
			if (0 >= (bytes_sent = send(conn_sock, reply, strlen(reply), 0))) {
				printf("\nConnection closed\n");
				break;
			}

			int count = 0; // count password repeatation

			while (1) {
				// receive password
				memset(password, '\0', MAX);
				if (0 >= (bytes_received = recv(conn_sock, password, MAX-1, 0))) {
					printf("\nConnection closed\n");
					break;
				}
				password[bytes_received] = '\0';

				// validate password
				if (0 == strcmp(found->password, password))
					reply = "1"; // pass valid, reply 1
				else {
					count++;
					if (count == 3) {
						reply = "2"; // wrong pass 3 times, reply 2
						found->status = 0; // then lock account
					}
					else
						reply = "0"; // wrong pass < 3 times, reply 0
				}

				// echo to client
				if (0 >= (bytes_sent = send(conn_sock, reply, strlen(reply), 0))) {
					printf("\nConnection closed\n");
					break;
				}
			}
			// save linked list state
			save_list(account_list, filename);
			close(conn_sock);
		} else { 
			// on parent process
			close(conn_sock);
		}
	}
	close(listen_sock);
	return 0;
}
