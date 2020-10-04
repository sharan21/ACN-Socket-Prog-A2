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
    uint16_t dnstype;  
    uint16_t dnsclass; 

}; 

typedef struct{

    uint16_t compression;
    uint16_t type;
    uint16_t class_type;
    uint32_t ttl;
    uint16_t length;
    struct in_addr addr;

} __attribute__((packed)) dns_record_a_t;


int main(){
    
    struct sockaddr_in server_addr;
    
    char hostname[] = "www.facebook.com";
    char* newhostname;
    
    //Create the client socket, as type SOCK_DGRAM for UDP
    int socketfd = socket (AF_INET, SOCK_DGRAM, 0);
    
    server_addr.sin_family = AF_INET;
    
    //Using an online IP to hex converted, the hex code for the IITH DNS server 192.168.35.52 is 0xc0a82334
    server_addr.sin_addr.s_addr = htonl(0xc0a82334); // all numberic values are sent using htonl()
    server_addr.sin_port = htons (53); 


    // Init the DNS header
    dns_header header;
    memset (&header, 0, sizeof (dns_header));
    header.xid= htons (0x1234);   
    header.flags = htons (0x0100); 
    header.n_questions = htons(1);

    // Init DNS Question
    dns_question question;
    question.dnstype = htons(1);  
    question.dnsclass = htons(1); 

    //DNS Name Preprocessing
    question.name = (char *)calloc(strlen(hostname) + 2, sizeof (char));   
    memcpy (question.name + 1, &hostname, strlen(hostname));


    // Create the final packet to send
    size_t packetlen = sizeof (header) + strlen (newhostname) + 2 + sizeof (question.dnstype) + sizeof (question.dnsclass);
    uint8_t *packet = (uint8_t *) calloc(packetlen, sizeof (uint8_t));
    uint8_t *p = (uint8_t *)packet;

    memcpy (p, &header, sizeof (header));
    p += sizeof (header);
    memcpy (p, question.name, strlen (newhostname) + 2);
    p += strlen (newhostname) + 2;
    memcpy (p, &question.dnstype, sizeof (question.dnstype));
    p += sizeof (question.dnstype);
    memcpy (p, &question.dnsclass, sizeof (question.dnsclass));

    
    // Send the packet 
    cout << "Sending the packet to DNS Server" << endl;

    if(sendto (socketfd, packet, packetlen, 0, (struct sockaddr *) &server_addr, (socklen_t) sizeof (server_addr)) < 0) 
        cout << "error while sending" << endl;
    
    
    // Get the response from the DNS server
    socklen_t length = 0;
    uint8_t response[512];
    memset (&response, 0, 512);

    ssize_t bytes = recvfrom (socketfd, response, 512, 0, (struct sockaddr *) &server_addr, &length);

    // After this we need to process the received response to attain the domain name

        
}



