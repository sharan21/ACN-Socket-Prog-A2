#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>

using namespace std;

#define BUFSIZE 1024

char sender_buffer[1024];
char receiver_buffer[1024];

// string sendDataStr, rcvDataStr;
		
void service_socket(int i, int server_sock_fd)
{
	
	if (i == 0){ //current client has pushed data to buffer

		fgets(sender_buffer, BUFSIZE, stdin); //get data
		send(server_sock_fd, sender_buffer, strlen(sender_buffer), 0); // me -> server -> other clients
	}
	else{ //other client -> server -> me
		int message_size = recv(server_sock_fd, receiver_buffer, BUFSIZE, 0); 
		receiver_buffer[message_size] = '\0';

		cout << receiver_buffer << endl;
		cout.flush();
	}
}
		
	
int main()
{
	int server_sock_fd, last_fd, i;
	struct sockaddr_in server_addr;
	fd_set all_fds, current_fds;


	// SOCKET CREATION
	if ((server_sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		cout << "failed to create socket" << endl;
		exit(1);
	}

	// INIT SERVER ADDR
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(4950);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	memset(server_addr.sin_zero, '\0', sizeof server_addr.sin_zero);


	// CONNECT TO SERVER
	if(connect(server_sock_fd, (struct sockaddr *) &server_addr, sizeof(struct sockaddr)) < 0) {
		cout << "failed to connect to server" << endl;
		exit(1);
	}

	// CONFIG THE FD BIT ARRAYS
	FD_ZERO(&all_fds);
	FD_ZERO(&current_fds);

	FD_SET(0, &all_fds); //
	FD_SET(server_sock_fd, &all_fds); //set server fd to 1 

	last_fd = server_sock_fd; //currently only server fd is present
	
	while(true){

		current_fds = all_fds; //update current set of fds, incase an fd has been added/ closed

		if(select(last_fd + 1, &current_fds, NULL, NULL, NULL) < 0){ //once an fd is set, condition will break
			cout << "error occured during select" << endl;
			exit(1);
		}
		// some socket has pushed data

		for(i = 0; i < last_fd + 1; i++){ 
			if(FD_ISSET(i, &current_fds))
				service_socket(i, server_sock_fd);
		}
			
	}

	close(server_sock_fd);
	return 0;
}