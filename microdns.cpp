
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

// We use a special SOCKET type for easier Windows porting
#define SOCKET int


const char *help =
    "microdns: This is a tiny DNS server that does only one thing: It \n"
    "always sends a given IPv4 IP to any and all queries sent to the server. \n"
    "The IP to send the user is given in the first argument; the second optional \n"
    "argument is the IP of this tiny DNS server.  If the second argument is not \n"
    "given, microdns binds to \"0.0.0.0\": All the IP addresses the server has. \n\n"
    "For example, to have micrdns always give the IP address 10.1.2.3 on the \n"
    "IP 127.0.0.1: \n"
    "microdns 10.1.2.3 127.0.0.1\n";

const char *usage = "Usage: microdns {ip to give out} [{ip of microdns server}]";

/* This is the header placed before the 4-byte IP; we change the last four
 * bytes to set the IP we give out in replies */
#define P_LEN 16
char packet[P_LEN] = { 0xc0,0x0c,0x00,0x01,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x04,0x7f,0x7f,0x7f,0x7f };

// Function Prototypes
void set_ip(char*);
SOCKET get_port(uint32_t ip, struct sockaddr_in *dns_udp);

int main(int argc, char **argv) 
{
    // check agrs
    if(argc==2 && strcmp(argv[1],"-h")==0) 
    {
        printf("%s\n", help);
        exit(1);
    }
    if(argc < 2 || argc > 3) 
    {
        printf("%s\n", usage);
        exit(1);
    }

    // Set the IP we give everyone
    set_ip(argv[1]);

    // Set the IP we bind to (default is "0", which means "all IPs)
    uint32_t ip = 0; // 0.0.0.0; default bind IP
    if(argc == 3)
    {
        ip = inet_addr(argv[2]);
        if(ip == INADDR_NONE)
        {
            printf("Problem with bind IP %s\n", argv[2]);
            exit(-1);
        }
    }
    
    struct sockaddr_in dns_udp;
    SOCKET sock = get_port(ip,&dns_udp);

    char buffer[512];
    socklen_t buffer_size = sizeof(buffer);

    // Now that we know the IP and are on port 53, process incoming DNS requests
    for(;;) 
    {
        // Get data from UDP port 53
        int recv_len = recvfrom(sock, buffer, 255, 0, (struct sockaddr *)&dns_udp, &buffer_size);

        // Roy Arends check: We only answer questions
        if(recv_len < 3 || (buffer[2] & 0x80) != 0x00) continue;

        // Prepare the reply
        if(recv_len > 12) 
        {
            /* Make this an answer */
            buffer[2] |= 0x80;
            if(buffer[11] == 0)
            {
                // EDNS not supported
                // We add an additional answer
                buffer[7]++;
            }
            else
            {
                buffer[3] &= 0xf0;
                buffer[3] |= 4; // NOTIMPL
            }
        }

        if(buffer[11] == 0)
        {
            // Again, EDNS not supported
            for(int a=0; a<P_LEN; a++)
            {
                buffer[recv_len + a] = packet[a];
            }
        }

        // Send the reply
        sendto(sock, buffer, recv_len + P_LEN, 0, (struct sockaddr *)&dns_udp, sizeof(struct sockaddr));
    }

    return 0;
}

// Set the IP we give everyone
void set_ip(char *str_ip)
{
    uint32_t ip;
    ip = inet_addr(str_ip);
    ip = ntohl(ip);
    packet[12] = (ip & 0xff000000) >> 24;
    packet[13] = (ip & 0x00ff0000) >> 16;
    packet[14] = (ip & 0x0000ff00) >>  8;
    packet[15] = (ip & 0x000000ff);
}

// Get port: Get a port locally and return the socket the port is on
SOCKET get_port(uint32_t ip, struct sockaddr_in *dns_udp) 
{
    /* Bind to port 53 */
    SOCKET sock = socket(AF_INET,SOCK_DGRAM,0);
    if(sock == -1)
    {
        perror("socket error");
        exit(-1);
    }

    memset(dns_udp,0,sizeof(struct sockaddr_in));
    dns_udp->sin_family = AF_INET;
    dns_udp->sin_port = htons(53);
    dns_udp->sin_addr.s_addr = ip;

    if(bind(sock, (struct sockaddr *)dns_udp, sizeof(struct sockaddr_in)) == -1)
    {
        perror("bind error");
        exit(-1);
    }

    /* Linux kernel bug */
    /* fcntl(sock, F_SETFL, O_NONBLOCK); */

    return sock;
}

