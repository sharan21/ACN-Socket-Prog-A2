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

void service_socket(int client_fd, fd_set *tot_fds_used, int server_soc_fd, int last_fd)
{
	int rec_buffer_size, j;
	
	if ((rec_buffer_size = recv(client_fd, receiver_buffer, 1024, 0)) == 0){
		close(client_fd);
		FD_CLR(client_fd, tot_fds_used);
	}
	else{ 
		for(j = 0; j <= last_fd; j++){
			if (FD_ISSET(j, tot_fds_used) && j != server_soc_fd && j != client_fd) // socket has data, that isnt server socket
				send(j, receiver_buffer, rec_buffer_size, 0); 
				
		}		
	}	
}
		
void accept_client_connection(fd_set *tot_fds_used, int *last_fd, int server_soc_fd, struct sockaddr_in client_addr)
{
	socklen_t client_addr_len;
	int client_fd;
	
	client_addr_len = sizeof(struct sockaddr_in);

	if((client_fd = accept(server_soc_fd, (struct sockaddr *) &client_addr, &client_addr_len)) < 0){
		cout << "Failed to accept client" << endl;
		exit(1);
	}
	else{
		FD_SET(client_fd, tot_fds_used);
		
		if(client_fd > *last_fd)
			*last_fd = client_fd;

		cout << "connected to new client!" << endl;
	}
}
	

int main()
{
	int flag = 1, server_soc_fd = 0, last_fd, i;
	fd_set tot_fds_used, read_fds;
	
	struct sockaddr_in my_addr, client_addr;
	
	FD_ZERO(&tot_fds_used);
	FD_ZERO(&read_fds);
	

	if((server_soc_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		cout << "Failed to init socket" << endl;
		exit(1);
	}
		
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(4950);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	memset(my_addr.sin_zero, '\0', sizeof my_addr.sin_zero);
		
	if(setsockopt(server_soc_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)) < 0){
		cout << " Failed to set socket options" << endl;
		exit(1);
	}
		
	if(bind(server_soc_fd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) < 0){
		cout << "Failed to bind" << endl;
		exit(1);
	}

	if(listen(server_soc_fd, 10) < 0){
		cout << "Failed to start listening" << endl;
		exit(1);
	}

	cout << "Started listening on port 4950...";
	cout.flush();

	FD_SET(server_soc_fd, &tot_fds_used);
	
	last_fd = server_soc_fd;

	while(true){
		read_fds = tot_fds_used; //update

		if(select(last_fd+1, &read_fds, NULL, NULL, NULL) < 0){ //
			cout << "error occured during select()" << endl;
			exit(1);
		}
		
		for (i = 0; i < last_fd+1 ; i++){
			if (FD_ISSET(i, &read_fds)){ //one of the sockets needs to be serviced
				if (i == server_soc_fd) // the server sent data, it needs to accept a new client
					accept_client_connection(&tot_fds_used, &last_fd, server_soc_fd, client_addr);
				else
					service_socket(i, &tot_fds_used, server_soc_fd, last_fd); // a client has sent data into its socket
			}
		}
	}
	return 0;
}