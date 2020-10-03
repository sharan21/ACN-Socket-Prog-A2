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
		
void send_recv(int i, int sockfd)
{
	
	int nbyte_recvd;
	
	if (i == 0){

		fgets(sender_buffer, BUFSIZE, stdin);

		if (strcmp(sender_buffer, "bye") == 0) {
			exit(0);
		}
		else
			send(sockfd, sender_buffer, strlen(sender_buffer), 0);
	}
	else{
		int message_size = recv(sockfd, receiver_buffer, BUFSIZE, 0);
		receiver_buffer[message_size] = '\0';
		cout << receiver_buffer << endl;
		cout.flush();
	}
}
		

	
int main()
{
	int sockfd, fdmax, i;
	struct sockaddr_in server_addr;
	fd_set master;
	fd_set read_fds;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		cout << "failed to create socket" << endl;
		exit(1);
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(4950);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	memset(server_addr.sin_zero, '\0', sizeof server_addr.sin_zero);
	
	if(connect(sockfd, (struct sockaddr *) &server_addr, sizeof(struct sockaddr)) < 0) {
		cout << "failed to connect to server" << endl;
		exit(1);
	}

	FD_ZERO(&master);
	FD_ZERO(&read_fds);
	FD_SET(0, &master);
	FD_SET(sockfd, &master);
	fdmax = sockfd;
	
	while(true){

		read_fds = master;

		if(select(fdmax+1, &read_fds, NULL, NULL, NULL) < 0){
			cout << "error occured during select" << endl;
			exit(1);
		}
		
		for(i=0; i <= fdmax; i++ )
			if(FD_ISSET(i, &read_fds))
				send_recv(i, sockfd);
	}

	close(sockfd);

	return 0;
}