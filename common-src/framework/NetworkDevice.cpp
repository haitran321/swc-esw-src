/**
* $Id: NetworkDevice.cpp 6568 2011-02-25 23:44:07Z ste38548 $
*/
#include "NetworkDevice.h"
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <netinet/ip.h>
#include <sys/types.h>
#include <net/ethernet.h>

NetworkDevice::NetworkDevice(NetworkSide side,
                             const string &host,
                             unsigned short port) :

_side(side)
{
   // Save network address information.
   
   if (side == NetworkServer)
   {
      _local.hostName = host;
      _local.port = port;
      _remote.hostName = "";
      _remote.port = 0;
   }
   else
   {
      _local.hostName = "";
      _local.port = 0;
      _remote.hostName = host;
      _remote.port = port;
   }
}

NetworkDevice::NetworkDevice() :
_side(NetworkClient)
{
   _local.hostName = "";
   _local.port = 0;
   _remote.hostName = "";
   _remote.port = 0;
}

int NetworkDevice::getSockaddr(
   const string &host, 
   unsigned short port,
   sockaddr_in &sockaddr
)
{
   struct hostent *h = NULL;

   // Initialize the server's Internet address structure.

   memset(&sockaddr, 0, sizeof(sockaddr));
   sockaddr.sin_family = AF_INET;

   // Attempt To get address of host

   if ((h = gethostbyname(host.c_str())) == NULL)
   {
      return(ERROR);
   }

   // Obtain inet addr information.

   sockaddr.sin_addr.s_addr = 
      (reinterpret_cast<in_addr *>(h->h_addr))->s_addr;

   // Swap port number, if necessary.
    
   sockaddr.sin_port = htons(port);
   return(OK);
}

int NetworkDevice::getSocketOption(
   int option,
   int &optValue
)
{
   unsigned int optLength = sizeof(int);
   return(getsockopt(_fd, SOL_SOCKET, option, 
                     reinterpret_cast<char *>(&optValue), &optLength));
}

int NetworkDevice::setSocketOption(
   int option,         
   int optValue        
)
{
   return(setsockopt(_fd, SOL_SOCKET, option, 
                     reinterpret_cast<char *>(&optValue),
                     sizeof(optValue)) == 0);
}

int NetworkDevice::setRemoteSockaddr(
   const string &host, 
   unsigned short port
)
{
   _remote.hostName = host;
   _remote.port = port;
   return(getSockaddr(_remote.hostName, _remote.port, _remote.sockaddr));
}

string NetworkDevice::getInetAddrString(const string &host)
{
   string inetAddr = "";
   struct hostent *hostData = gethostbyname(host.c_str());
   if (hostData != NULL)
   {
      struct in_addr *inetAddress = (struct in_addr *)hostData->h_addr_list[0];
      inetAddr = inet_ntoa(*inetAddress);
   }
   return(inetAddr);
}

int NetworkDevice::resolveArp(const string &host)
{
   // resolve the destination's MAC address
   return(OK);
}

int NetworkDevice::makePermanentArpEntry(const string &host, int timeoutSecs)
{
   return(OK);
}


void NetworkDevice::outputSocketInfo()
{

    printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n"
           "Socket Info:\n"
           "    _fd = 0x%08X (%d)\n"
           "    _remote.port = %d\n"
           //"    _remote.sockaddr.sin_len = %d\n" //u_char
           //"    _remote.sockaddr.sin_family = %d\n" //u_char
           "    _remote.sockaddr.sin_port = %d\n" //u_short
           "    _remote.sockaddr.sin_addr = 0x%08x (%d.%d.%d.%d)\n" //u_int32_t
           "    _remote.sockaddr.sin_zero[0] = %d\n" //char
           "    _remote.sockaddr.sin_zero[1] = %d\n" //char
           "    _remote.sockaddr.sin_zero[2] = %d\n" //char
           "    _remote.sockaddr.sin_zero[3] = %d\n" //char
           "    _remote.sockaddr.sin_zero[4] = %d\n" //char
           "    _remote.sockaddr.sin_zero[5] = %d\n" //char
           "    _remote.sockaddr.sin_zero[6] = %d\n" //char
           "    _remote.sockaddr.sin_zero[7] = %d\n" //char
           "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n",
           _fd, _fd, 
           _remote.port, 
           _remote.sockaddr.sin_port, 
           _remote.sockaddr.sin_addr.s_addr, 
           (_remote.sockaddr.sin_addr.s_addr & 0xFF000000) >> 24, 
           (_remote.sockaddr.sin_addr.s_addr & 0x00FF0000) >> 16, 
           (_remote.sockaddr.sin_addr.s_addr & 0x0000FF00) >> 8, 
           _remote.sockaddr.sin_addr.s_addr & 0x000000FF, 
           _remote.sockaddr.sin_zero[0], 
           _remote.sockaddr.sin_zero[1], 
           _remote.sockaddr.sin_zero[2], 
           _remote.sockaddr.sin_zero[3], 
           _remote.sockaddr.sin_zero[4], 
           _remote.sockaddr.sin_zero[5], 
           _remote.sockaddr.sin_zero[6], 
           _remote.sockaddr.sin_zero[7]);
           
}
