#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

int main() {
	// Disable output buffering
	setbuf(stdout, NULL);

	// You can use print statements as follows for debugging, they'll be visible when running tests.
	printf("Logs from your program will appear here!\n");

	// Uncomment this block to pass the first stage
	//
	 int server_fd, client_addr_len;
	 struct sockaddr_in client_addr;
	
	 server_fd = socket(AF_INET, SOCK_STREAM, 0);
	 if (server_fd == -1) {
	 	printf("Socket creation failed: %s...\n", strerror(errno));
	 	return 1;
	 }
	
	 // Since the tester restarts your program quite often, setting REUSE_PORT
	 // ensures that we don't run into 'Address already in use' errors
	 int reuse = 1;
	 if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) {
	 	printf("SO_REUSEPORT failed: %s \n", strerror(errno));
	 	return 1;
	 }
	
	 struct sockaddr_in serv_addr = { .sin_family = AF_INET ,
	 								 .sin_port = htons(4221),
	 								 .sin_addr = { htonl(INADDR_ANY) },
	 								};
	
	 if (bind(server_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0) {
	 	printf("Bind failed: %s \n", strerror(errno));
	 	return 1;
	 }
	
	 int connection_backlog = 5;
	 if (listen(server_fd, connection_backlog) != 0) {
	 	printf("Listen failed: %s \n", strerror(errno));
	 	return 1;
	 }
	
	 printf("Waiting for a client to connect...\n");
	 client_addr_len = sizeof(client_addr);
	
	int client_fd = 0;
	client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len);
	if (client_fd <= 0)
	{
		printf("Accept failed: %s \n", strerror(errno));
		return 1;
	}
	 printf("Client connected\n");

	int buffer_size = 2048;
	char recv_buffer[buffer_size];
	int recved_size = recv(client_fd, recv_buffer, buffer_size, 0);
	if (recved_size == -1)
	{
		printf("Read error: %s \n", strerror(errno));
		return -1;
	}
	printf("received data=\n=====\n%s\n=====\n", recv_buffer);
	char* token = strtok(recv_buffer, " ");
	token = strtok(NULL, " ");
	if (token != NULL)
	{
		printf("token=%s\n", token);
		if (strcmp(token, "/") == 0)
		{
			char reponse[] = "HTTP/1.1 200 OK\r\n\r\n";
			if (send(client_fd, reponse, strlen(reponse), 0) == -1)
			{
				printf("Send reponse failed: %s \n", strerror(errno));
			}
		}
		else if (strncmp(token, "/echo", 5) == 0)
		{
			char* echo = token + 1;
			echo = strchr(echo, '/');
			if (echo == NULL)
				echo = "";
			else
				echo++;
			int echo_len = strlen(echo);
			char *reponse = (char*)malloc(128 + echo_len);
			sprintf(reponse, 
			"HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %d\r\n\r\n%s",
			echo_len, echo);
			if (send(client_fd, reponse, strlen(reponse), 0) == -1)
			{
				printf("Send reponse failed: %s \n", strerror(errno));
			}
			free(reponse);
		}
		else
		{
			char reponse[] = "HTTP/1.1 404 Not Found\r\n\r\n";
			if (send(client_fd, reponse, strlen(reponse), 0) == -1)
			{
				printf("Send reponse failed: %s \n", strerror(errno));
			}
		}
		
	}
	
	 close(server_fd);

	return 0;
}
