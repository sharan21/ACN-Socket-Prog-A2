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

void service_socket(int client_fd, fd_set *all_fds, int server_soc_fd, int last_fd) 
{
	int rec_buffer_size, j;
	
	if ((rec_buffer_size = recv(client_fd, receiver_buffer, 1024, 0)) == 0){ //client wants to close connection
		close(client_fd);
		FD_CLR(client_fd, all_fds); //set 0 to bit corresponding to this clients fd
	}
	else{ // a client has pushed a message to server, broadcast to all other active sockets

		for(j = 0; j <= last_fd; j++){ 
			if (FD_ISSET(j, all_fds) && j != server_soc_fd && j != client_fd) // check if active client fd
				send(j, receiver_buffer, rec_buffer_size, 0); //send message to client with active fd
				
		}		
	}	
}
		
void accept_client_connection(fd_set *all_fds, int *last_fd, int server_soc_fd, struct sockaddr_in client_addr)
{
	socklen_t client_addr_len;
	int client_fd;
	
	client_addr_len = sizeof(struct sockaddr_in);

	if((client_fd = accept(server_soc_fd, (struct sockaddr *) &client_addr, &client_addr_len)) < 0){
		cout << "Failed to accept client" << endl;
		exit(1);
	}
	else{
		FD_SET(client_fd, all_fds); //set the client fd to 1
		
		if(client_fd > *last_fd) // update the last fd/ no of active dfs
			*last_fd = client_fd;

		cout << "connected to new client!" << endl;
	}
}
	

int main()
{
	int flag = 1, server_soc_fd = 0, last_fd, i; // last fd stores the 
	fd_set all_fds, current_fds; 
	
	struct sockaddr_in server_addr, client_addr;
	
	FD_ZERO(&all_fds); //a bit array of all the socket fds
	FD_ZERO(&current_fds); //a bit array of the servicable fds
	

	if((server_soc_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		cout << "Failed to init socket" << endl;
		exit(1);
	}
		
	// INIT SERVER ADDR
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(4950);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	memset(server_addr.sin_zero, '\0', sizeof server_addr.sin_zero);


	//INIT SOCKET	
	if(setsockopt(server_soc_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)) < 0){
		cout << " Failed to set socket options" << endl;
		exit(1);
	}

	//BIND SOCKET	
	if(bind(server_soc_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0){
		cout << "Failed to bind" << endl;
		exit(1);
	}

	//START LISTENING
	if(listen(server_soc_fd, 10) < 0){
		cout << "Failed to start listening" << endl;
		exit(1);
	}

	cout << "Started listening on port 4950...";
	cout.flush(); //empty stdout buffer

	FD_SET(server_soc_fd, &all_fds); //set server fd to 1 since server is now active
	
	last_fd = server_soc_fd;  //currently only server fd is present, so last_fd = 1

	while(true){

		current_fds = all_fds; //update current set of fds, incase an fd has been added/ closed

		if(select(last_fd + 1, &current_fds, NULL, NULL, NULL) < 0){ //once an fd is set, condition will break
			cout << "error occured during select()" << endl;
			exit(1);
		}
		
		for (i = 0; i < last_fd + 1 ; i++){ //iterate through list of fds
			if (FD_ISSET(i, &current_fds)){ //check which socket sent data
				if (i == server_soc_fd) // server accepting new client
					accept_client_connection(&all_fds, &last_fd, server_soc_fd, client_addr);
				else
					service_socket(i, &all_fds, server_soc_fd, last_fd); // a client has sent data to server
			}
		}
	}
	return 0;
}