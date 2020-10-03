#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <iostream>

using namespace std;
	
#define PORT 4950


char receiver_buffer[1024], buf[1024];

		
void service_socket(int client_fd, fd_set *master, int sockfd, int fdmax)
{
	int rec_buffer_size, j;
	
	if ((rec_buffer_size = recv(client_fd, receiver_buffer, 1024, 0)) == 0){
		close(client_fd);
		FD_CLR(client_fd, master);
	}
	else{ 
		for(j = 0; j <= fdmax; j++){
			if (FD_ISSET(j, master) && j != sockfd && j != client_fd) // socket has data, that isnt server socket
				send(j, receiver_buffer, rec_buffer_size, 0); 
				
		}		
	}	
}
		
void accept_client_connection(fd_set *master, int *fdmax, int sockfd, struct sockaddr_in client_addr)
{
	socklen_t client_addr_len;
	int client_fd;
	
	client_addr_len = sizeof(struct sockaddr_in);

	if((client_fd = accept(sockfd, (struct sockaddr *) &client_addr, &client_addr_len)) < 0){
		cout << "Failed to accept client" << endl;
		exit(1);
	}
	else{
		FD_SET(client_fd, master);
		
		if(client_fd > *fdmax)
			*fdmax = client_fd;

		cout << "connected to new client!" << endl;
	}
}
	

int main()
{
	int flag = 1, sockfd = 0, fdmax, i;
	fd_set master, read_fds;
	
	struct sockaddr_in my_addr, client_addr;
	
	FD_ZERO(&master);
	FD_ZERO(&read_fds);
	

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		cout << "Failed to init socket" << endl;
		exit(1);
	}
		
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(4950);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	memset(my_addr.sin_zero, '\0', sizeof my_addr.sin_zero);
		
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)) < 0){
		cout << " Failed to set socket options" << endl;
		exit(1);
	}
		
	if(bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) < 0){
		cout << "Failed to bind" << endl;
		exit(1);
	}

	if(listen(sockfd, 10) < 0){
		cout << "Failed to start listening" << endl;
		exit(1);
	}

	cout << "Started listening on port 4950...";
	cout.flush();

	FD_SET(sockfd, &master);
	
	fdmax = sockfd;

	while(true){
		read_fds = master;

		if(select(fdmax+1, &read_fds, NULL, NULL, NULL) < 0){
			cout << "error occured during select()" << endl;
			exit(1);
		}
		
		for (i = 0; i <= fdmax; i++){
			if (FD_ISSET(i, &read_fds)){
				if (i == sockfd)
					accept_client_connection(&master, &fdmax, sockfd, client_addr);
				else
					service_socket(i, &master, sockfd, fdmax);
			}
		}
	}
	return 0;
}