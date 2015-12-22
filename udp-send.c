/*
       Paul Mohr
       UDP
       Get machine name
       Get machine LAN Ethernet port
       Send to a remote address from the command line or input
       ping back and forth n times to verify that things happen somewhat in order?
       so COMMAND where[192.168.1.2 i.e.] and count
       Merge with windows version
*/
#define  LINUX                // WIN for Winsock and BSD for BSD sockets

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
#define  IP_ADDR     "192.168.1.12" // IP address of destination
#define  IP_ADDR2    "192.168.1.2" // IP address of source

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

//===== Main program ==========================================================
//FIXME ARGV[] ARGC
int main(){
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
  if (client_s < 0)  {    printf("*** ERROR - socket() failed \n");    exit(-1);  }
  server_addr.sin_family = AF_INET;                 // Address family to use
  server_addr.sin_port = htons(PORT_NUM);           // Port number to use
  server_addr.sin_addr.s_addr = inet_addr(IP_ADDR2); // IP address to use
  if (bind(client_s, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
          perror("bind failed");          exit(1); }
  sprintf(out_buf, "Test message from %s %d to %s %d or THIS %s?"
		  ,IP_ADDR2,PORT_NUM,IP_ADDR,PORT_NUM,lookip());
  server_addr.sin_addr.s_addr = inet_addr(IP_ADDR); // TO address
  retcode = sendto(client_s, out_buf, (strlen(out_buf) + 1), 0,
    (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (retcode < 0)  { printf("*** ERROR - sendto() failed \n"); exit(-1); }
  server_addr.sin_addr.s_addr = inet_addr(IP_ADDR); // IP address to use
  addr_len = sizeof(server_addr);
  retcode = recvfrom(client_s, in_buf, sizeof(in_buf), 0,
    (struct sockaddr *)&server_addr, &addr_len);
  if (retcode < 0)  { printf("*** ERROR - recvfrom() failed \n");    exit(-1);  }
  printf("Received from server: %s \n", in_buf);
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
