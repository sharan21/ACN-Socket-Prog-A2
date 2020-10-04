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


struct dns_header{

    uint16_t xid;     
    uint16_t flags;    
    uint16_t n_questions;  
    uint16_t n_answers;
    uint16_t n_auth_rec; 
    uint16_t n_add_rec;  

};

struct dns_question{

    char *name;        
    uint16_t q_type;  
    uint16_t q_class; 

}; 


int main(){
    
    struct sockaddr_in server_addr;
    char hostname[1024];

    cin >> hostname;
    
    //Create the client socket, as type SOCK_DGRAM for UDP
    int socketfd = socket (AF_INET, SOCK_DGRAM, 0);

    // Init the server details, htonl() is used for all numeric values
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(0xc0a82334);  //Using an online IP to hex converted, the hex code for the IITH DNS server 192.168.35.52 is 0xc0a82334
    server_addr.sin_port = htons (53); 


    // Init the DNS header
    dns_header header;
    memset (&header, 0, sizeof (dns_header));
    header.xid= htons (0x1234);   
    header.flags = htons (0x0100); 
    header.n_questions = htons(1);

    // Init DNS Question
    dns_question question;
    question.q_type = htons(1);  
    question.q_class = htons(1); 
    char processed[] = "3www8facebook3com0";
    question.name = processed; 

    

    // Create the final packet to send
    size_t packetlen = sizeof (header) + strlen (hostname) + 2 + sizeof (question.q_type) + sizeof (question.q_class);
    uint8_t *packet = (uint8_t *) calloc(packetlen, sizeof (uint8_t));

    memcpy (packet, &header, sizeof (header));
    packet += sizeof (header);
    memcpy (packet, question.name, strlen (hostname) + 2);
    packet += strlen (hostname) + 2;
    memcpy (packet, &question.q_type, sizeof (question.q_type));
    packet += sizeof (question.q_type);
    memcpy (packet, &question.q_class, sizeof (question.q_class));

    
    // Send the packet 
    cout << "Sending the packet to DNS Server" << endl;

    if(sendto (socketfd, packet, packetlen, 0, (struct sockaddr *) &server_addr, (socklen_t) sizeof (server_addr)) < 0) 
        cout << "error while sending" << endl;
    
    
    // Get the response from the DNS server
    socklen_t length = 0;
    uint8_t response[512];
    memset (&response, 0, 512);

    ssize_t bytes = recvfrom (socketfd, response, 512, 0, (struct sockaddr *) &server_addr, &length);

    cout << "No of Bytes received: "<< (int)bytes << endl;

    // After this we need to process the received response to attain the received record and extract the domain name and other information
       
}



