/*
       Paul Mohr
       UDP
       Get machine name
       Get machine LAN Ethernet port
       Send to a remote address from the command line or input
       ping back and forth n times to verify that things happen somewhat in order?
       so COMMAND where[192.168.1.2 i.e.] and count
       Merge with windows version
       Need a way to define WIN or LINUX at compile
*/

#define VERSION_MAJOR 0
#define VERSION_MINOR 2
//#define  WIN               // WIN for Winsock and BSD for BSD sockets

//Fix that nasty CYGWIN stuff
#ifdef WIN
#define __USE_W32_SOCKETS
#endif
//----- Include files ---------------------------------------------------------
#include <stdio.h>          // Needed for printf()
#include <string.h>         // Needed for memcpy() and strcpy()
#include <stdlib.h>         // Needed for exit()

#ifdef LINUX
  #include <netdb.h>
  #include <sys/socket.h>
  #include "ifaddrs.h"
  #include "netinet/in.h"
  #include "arpa/inet.h"
#endif
#ifdef WIN
  #include <winsock2.h>
  #include<Ws2tcpip.h>
  #include <windows.h>
  #pragma comment(lib,"ws2_32.lib") //Winsock Library
#endif
#ifdef BSD
  #include <sys/types.h>    // Needed for sockets stuff
  #include <netinet/in.h>   // Needed for sockets stuff
  #include <sys/socket.h>   // Needed for sockets stuff
  #include <arpa/inet.h>    // Needed for sockets stuff
  #include <fcntl.h>        // Needed for sockets stuff
  #include <netdb.h>        // Needed for sockets stuff
#endif

//----- Defines ---------------------------------------------------------------
#define  PORT_NUM          49152 // was 1050 Port number used
char sourceIP[]="192.168.1.12";
char destIP[]="192.168.1.2";

char addrp[INET6_ADDRSTRLEN];

#ifdef LINUX
char* lookip(){
    struct ifaddrs *iflist, *iface;
    char *fault="There is no IP as we got a fault";
    const void *addr;
    if (getifaddrs(&iflist) < 0) {        perror("getifaddrs");        return fault;    }
    for (iface = iflist; iface; iface = iface->ifa_next) {
        int af = iface->ifa_addr->sa_family;
        switch (af) {
            case AF_INET:
                addr = &((struct sockaddr_in *)iface->ifa_addr)->sin_addr;
                break;
            default:
                addr = NULL;
                break;
        }
        if (addr) {
            if (inet_ntop(af, addr, addrp, sizeof addrp) == NULL) {perror("inet_ntop");continue;}
            if ( strstr(addrp,"192") == addrp){
            	freeifaddrs(iflist);return addrp;
            }
            if ( strstr(iface->ifa_name,"eth") == iface->ifa_name){
            	freeifaddrs(iflist);return addrp;
            }
        }
    }
    freeifaddrs(iflist);
    return addrp;
}
#endif
#ifdef WIN
char lana[256];
char fail[]="Failing";
char *lookip(){
	char hostname[1024];
	hostname[1023] = '\0';
	gethostname(hostname, 1023);
    WSADATA wsaData;
    int iResult;
    INT iRetval;
    DWORD dwRetval;
    int i = 1;
    struct addrinfo *result = NULL;
    struct addrinfo *ptr = NULL;
    struct addrinfo hints;
    struct sockaddr_in  *sockaddr_ipv4;
    LPSOCKADDR sockaddr_ip;
    char ipstringbuffer[46];
    DWORD ipbufferlength = 46;
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) { printf("WSAStartup failed: %d\n", iResult);return fail;}
    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    dwRetval = getaddrinfo(hostname, NULL, &hints, &result);
    if ( dwRetval != 0 ) {
        printf("getaddrinfo failed with error: %d\n", dwRetval);
        WSACleanup();
        return fail;
    }
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {
        switch (ptr->ai_family) {
            case AF_UNSPEC:
                break;
            case AF_INET:
                sockaddr_ipv4 = (struct sockaddr_in *) ptr->ai_addr;
                sprintf(lana,"%s",inet_ntoa(sockaddr_ipv4->sin_addr) );
                break;
            case AF_INET6:
                break;
            case AF_NETBIOS:
                break;
            default:
                break;
        }
    }
    freeaddrinfo(result);
    WSACleanup();
    return lana;
}

#endif

//===== Main program ==========================================================
int main(int argc, char **argv){
	if (argc != 2){
		printf("Run this with a destination IP like 192.168.1.2\n");return 1;
	}
printf("Version %d.%d UDP send and receive \n",VERSION_MAJOR,VERSION_MINOR);
#ifdef WIN
  WORD wVersionRequested = MAKEWORD(1,1);       // Stuff for WSA functions
  WSADATA wsaData;                              // Stuff for WSA functions
#endif
  int                  client_s;        // Client socket descriptor
  struct sockaddr_in   server_addr;     // Server Internet address
  int                  addr_len;        // Internet address length
  char                 out_buf[4096];   // Output buffer for data
  char                 in_buf[4096];    // Input buffer for data
  int                  retcode;         // Return code
#ifdef WIN
  WSAStartup(wVersionRequested, &wsaData); //INIT winsock
#endif
  client_s = socket(AF_INET, SOCK_DGRAM, 0);
  if (client_s < 0)  {    printf("Step 1 -- socket create failed \n");    exit(-1);  }
  server_addr.sin_family = AF_INET;                 // Address family to use
  server_addr.sin_port = htons(PORT_NUM);           // Port number to use
//This is an issue FIXME
  server_addr.sin_addr.s_addr = inet_addr(lookip()); // IP address to use
  if (bind(client_s, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
          printf("Step 2 - bind failed\n");          exit(1); }
  sprintf(out_buf, "Test message from %s %d to %s %d detected IP %s"
		  ,lookip(),PORT_NUM,argv[1],PORT_NUM,lookip());
  server_addr.sin_addr.s_addr = inet_addr(argv[1]); // TO address
  retcode = sendto(client_s, out_buf, (strlen(out_buf) + 1), 0,
    (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (retcode < 0)  { printf("*** ERROR - sendto() failed \n"); exit(-1); }
  server_addr.sin_addr.s_addr = inet_addr(lookip()); // IP address to use
  addr_len = sizeof(server_addr);
  retcode = recvfrom(client_s, in_buf, sizeof(in_buf), 0,
    (struct sockaddr *)&server_addr, &addr_len);
  if (retcode < 0)  { printf("*** ERROR - recvfrom() failed \n");    exit(-1);  }
  printf("Received: %s \n", in_buf);
#ifdef WIN
  retcode = closesocket(client_s);
  if (retcode < 0)  {    printf("*** ERROR - closesocket() failed \n");    exit(-1);  }
#endif
#ifdef BSD
  retcode = close(client_s);
  if (retcode < 0)  {    printf("*** ERROR - close() failed \n");    exit(-1);  }
#endif
#ifdef WIN
  WSACleanup(); //clean winsock
#endif
  return(0);
}
