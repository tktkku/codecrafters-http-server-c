#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

const char REQUEST_200[] = "HTTP/1.1 200 OK\r\n\r\n";
const char REQUEST_404[] = "HTTP/1.1 404 Not Found\r\n\r\n";

const char MSG_SEND_FAILED[] = "Failed to send: %s\n";
typedef enum 
{
	GET,
	POST
} request_type;

typedef struct 
{
	struct
	{
		request_type type;
		char *path;
		char *version;
	} request;
	struct 
	{
		char* url;
		char* port;
	} host;
	struct 
	{
		char* agent;
	} user_agent;
	int content_len;
	char* body;
} request_header;

int main(int argc, char* argv[]) {
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
	while (1)
	{
		int client_fd = 0;
		client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len);
		if (client_fd <= 0)
		{
			printf("Accept failed: %s \n", strerror(errno));
			return 1;
		}
		printf("Client connected\n");
		int pid = fork();
		if (pid == 0)
		{
			int buffer_size = 2048;
			char recv_buffer[buffer_size];
			int recved_size = recv(client_fd, recv_buffer, buffer_size, 0);
			if (recved_size == -1)
			{
				printf("Read error: %s \n", strerror(errno));
				return -1;
			}
			printf("received data=\n=====\n%s\n=====\n", recv_buffer);
			char* token = strtok(recv_buffer, "\n");
			// parse header
			request_header header;
			while (token != NULL)
			{
				printf("token=%s\n", token);
				if (strncmp(token, "GET", 3) == 0)
				{
					printf("parse GET start\n");
					int t_len = strlen(token);
					char* t_copy = (char*)malloc(t_len + 1);
					strcpy(t_copy, token);

					header.request.type = GET;
					
					char* p1 = strchr(t_copy, ' ');
					char* p2 = strchr(p1 + 1, ' ');
					char* p3 = strchr(p2 + 1, '\r');

					header.request.path = (char*)malloc(p2 - p1);
					memset(header.request.path, 0, p2 - p1);
					strncpy(header.request.path, p1 + 1, p2 - p1 - 1);

					header.request.version = (char*)malloc(p3 - p2);
					memset(header.request.version, 0, p3 - p2);
					strncpy(header.request.version, p2 + 1, p3 - p2 - 1);

					free(t_copy);
					printf("parse GET end\n");
				}
				else if (strncmp(token, "POST", 4) == 0)
				{
					printf("parse POST start\n");
					int t_len = strlen(token);
					char* t_copy = (char*)malloc(t_len + 1);
					strcpy(t_copy, token);

					header.request.type = POST;
					
					char* p1 = strchr(t_copy, ' ');
					char* p2 = strchr(p1 + 1, ' ');
					char* p3 = strchr(p2 + 1, '\r');
					
					header.request.path = (char*)malloc(p2 - p1);
					memset(header.request.path, 0, p2 - p1);
					strncpy(header.request.path, p1 + 1, p2 - p1 - 1);

					header.request.version = (char*)malloc(p3 - p2);
					memset(header.request.version, 0, p3 - p2);
					strncpy(header.request.version, p2 + 1, p3 - p2 - 1);

					free(t_copy);
					printf("parse POST end\n");
				}
				else if (strncmp(token, "Host:", 5) == 0)
				{
					printf("parse Host start\n");
					int t_len = strlen(token);
					char* t_copy = (char*)malloc(t_len + 1);
					strcpy(t_copy, token);

					char* p1 = strchr(t_copy, ' ');
					char* p2 = strchr(p1 + 1, ':');
					char* p3 = strchr(p2 + 1, '\r');

					header.host.url = (char*)malloc(p2 - p1);
					memset(header.host.url, 0, p2 - p1);
					strncpy(header.host.url, p1 + 1, p2 - p1 - 1);

					header.host.port = (char*)malloc(p3 - p2);
					memset(header.host.port, 0, p3 - p2);
					strncpy(header.host.port, p2 + 1, p3 - p2 - 1);

					free(t_copy);
					printf("parse Host end\n");
				}
				else if (strncmp(token, "User-Agent:", 11) == 0)
				{
					printf("parse User-Agent start\n");
					int t_len = strlen(token);
					char* t_copy = (char*)malloc(t_len + 1);
					strcpy(t_copy, token);

					char* p1 = strchr(t_copy, ' ');
					char* p2 = strchr(p1 + 1, '\r');

					header.user_agent.agent = (char*)malloc(p2 - p1);
					memset(header.user_agent.agent, 0, p2 - p1);
					strncpy(header.user_agent.agent, p1 + 1, p2 - p1 - 1);
					
					free(t_copy);
					printf("parse User-Agent end\n");
				}
				else if (strncmp(token, "Content-Length:", 15) == 0)
				{
					printf("parse Content-Length start\n");
					int t_len = strlen(token);
					char* t_copy = (char*)malloc(t_len + 1);
					strcpy(t_copy, token);

					char* p1 = strchr(t_copy, ' ');
					char* p2 = strchr(p1 + 1, '\r');
					char *tmp = (char*)malloc(p2 - p1);
					memset(tmp, 0, p2 - p1);
					strncpy(tmp, p1 + 1, p2 - p1 - 1);
					header.content_len = atoi(tmp);
					free(tmp);
					printf("parse Content-Length end\n");
				}
				else if (strcmp(token, "\r") == 0)
				{
					printf("parse body start\n");
					token = strtok(NULL, "\n");
					if (token != NULL && header.content_len > 0 && strlen(token) > 0)
					{
						header.body = (char*)malloc(header.content_len);
						memcpy(header.body, token, header.content_len);
					}
					printf("parse body end\n");
				}
				token = strtok(NULL, "\n");
			}
			printf("parse finished\n");
			if (header.request.type == GET)
			{
				if (strcmp(header.request.path, "/") == 0)
				{
					if (send(client_fd, REQUEST_200, strlen(REQUEST_200), 0) == -1)
					{
						printf(MSG_SEND_FAILED, strerror(errno));
					}
				}
				else if (strncmp(header.request.path, "/echo", 5) == 0)
				{
					char* echo = header.request.path + 1;
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
						printf(MSG_SEND_FAILED, strerror(errno));
					}
					free(reponse);
				}
				else if (strcmp(header.request.path, "/user-agent") == 0)
				{
					int agent_len = strlen(header.user_agent.agent);
					char* reponse = (char*)malloc(128 + agent_len);
					sprintf(reponse, 
					"HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %d\r\n\r\n%s",
					agent_len, header.user_agent.agent);
					if (send(client_fd, reponse, strlen(reponse), 0) == -1)
					{
						printf(MSG_SEND_FAILED, strerror(errno));
					}
					free(reponse);
				}
				else if (
					strncmp(header.request.path, "/files", 5) == 0 
					&&
					argc == 3 && strncmp(argv[1], "--directory", 11) == 0 && strlen(argv[2]) >= 1
					)
				{
					char* root_path = argv[2];
					printf("File root path=%s\n", root_path);
					char* file = header.request.path + 1;
					file = strchr(file, '/');
					printf("file=%s\n", file);
					if (file == NULL || strcmp(file, "/") == 0)
					{
						printf("Empty file name\n");
						if (send(client_fd, REQUEST_404, strlen(REQUEST_404), 0) == -1)
						{
							printf(MSG_SEND_FAILED, strerror(errno));
						}
					}
					else
					{
						char *file_path = NULL;
						if (root_path != NULL)
						{
							file_path = (char*)malloc(strlen(root_path) + strlen(file) + 1);
							sprintf(file_path, "%s%s", root_path, file);
						}

						FILE *f = fopen(file_path, "rb");
						if (f != NULL)
						{
							fseek(f, 0L, SEEK_END);
							int file_size = ftell(f);
							fseek(f, 0L, SEEK_SET);
							unsigned char *file_buffer = (unsigned char*)malloc(file_size);
							fread(file_buffer, file_size, 1, f); 

							int off = 0;
							char *reponse = (char*)malloc(128 + file_size);
							sprintf(reponse, 
							"HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\nContent-Length: %d\r\n\r\n",
							file_size);
							off = strlen(reponse);
							memcpy(reponse + off, file_buffer, file_size);
							if (send(client_fd, reponse, strlen(reponse), 0) == -1)
							{
								printf(MSG_SEND_FAILED, strerror(errno));
							}
							free(reponse);
							free(file_buffer);
							fclose(f);
						}
						else
						{
							printf("Open file:%s error %s\n", file_path, strerror(errno));
							if (send(client_fd, REQUEST_404, strlen(REQUEST_404), 0) == -1)
							{
								printf(MSG_SEND_FAILED, strerror(errno));
							}
						}
						if (file_path)
						{
							free(file_path);
						}
					}
				}
				else
				{
					printf("could not find path: %s \n", header.request.path);
					printf("client fd = %d\n", client_fd);
					if (send(client_fd, REQUEST_404, strlen(REQUEST_404), 0) == -1)
					{
						printf(MSG_SEND_FAILED, strerror(errno));
					}
				}
			}
			else if (header.request.type == POST)
			{
				if (
					strncmp(header.request.path, "/files", 5) == 0 
					&&
					argc == 3 && strncmp(argv[1], "--directory", 11) == 0 && strlen(argv[2]) >= 1
					)
				{
					char* root_path = argv[2];
					printf("File root path=%s\n", root_path);
					char* file = header.request.path + 1;
					file = strchr(file, '/');
					printf("file=%s\n", file);
					if (file == NULL || strcmp(file, "/") == 0)
					{
						printf("Empty file name\n");
						if (send(client_fd, REQUEST_404, strlen(REQUEST_404), 0) == -1)
						{
							printf(MSG_SEND_FAILED, strerror(errno));
						}
					}
					else
					{
						char *file_path = NULL;
						if (root_path != NULL)
						{
							file_path = (char*)malloc(strlen(root_path) + strlen(file) + 1);
							sprintf(file_path, "%s%s", root_path, file);
						}

						FILE *f = fopen(file_path, "wb+");
						if (f != NULL)
						{
							printf("Writing file start\n");
							fwrite(header.body, header.content_len, 1, f);

							char *reponse = "HTTP/1.1 201 Created\r\n\r\n";
							if (send(client_fd, reponse, strlen(reponse), 0) == -1)
							{
								printf(MSG_SEND_FAILED, strerror(errno));
							}
							
							printf("Writing file end\n");
							fclose(f);
						}
						else
						{
							printf("Open file:%s error %s\n", file_path, strerror(errno));
							if (send(client_fd, REQUEST_404, strlen(REQUEST_404), 0) == -1)
							{
								printf(MSG_SEND_FAILED, strerror(errno));
							}
						}
						if (file_path)
						{
							free(file_path);
						}
					}
				}
				else
				{
					printf("could not find path: %s \n", header.request.path);
					printf("client fd = %d\n", client_fd);
					if (send(client_fd, REQUEST_404, strlen(REQUEST_404), 0) == -1)
					{
						printf(MSG_SEND_FAILED, strerror(errno));
					}
				}
			}
			printf("Free buffer\n");
			if (header.request.path)
			{
				free(header.request.path);
			}
			if (header.request.version)
			{
				free(header.request.version);
			}
			if (header.host.url)
			{
				free(header.host.url);
			}
			if (header.host.port)
			{
				free(header.host.port);
			}
			if (header.user_agent.agent)
			{
				free(header.user_agent.agent);
			}
			if (header.body)
			{
				free(header.body);
			}
			
			printf("Close client %d\n", client_fd);

			shutdown(client_fd, SHUT_RDWR);
			close(client_fd);
			exit(0);
		}		
		else if (pid == -1)
		{
			printf("fork() error: %s\n", strerror(errno));
		}
	}
	close(server_fd);
	return 0;
}
