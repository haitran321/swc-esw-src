/**
* $Id: NetworkDevice.h 6568 2011-02-25 23:44:07Z ste38548 $
*
* Class NetworkDevice extends class Device and provides access 
* to network devices via the standard sockets interface. This is 
* the base class for UDP and TCP network devices providing 
* functionality common to both 
*
*/
#ifndef NetworkDevice_H
#define NetworkDevice_H

#include<netinet/in.h>
#include <string>
#include "Device.h"

using namespace std;

/** Network side */

typedef enum
{
   NetworkClient,
   NetworkServer
} NetworkSide;

/** Protocol type */

typedef enum
{
   TCP,
   UDP
} ProtocolType;

class NetworkDevice : public Device
{
public:

   /**
   * Constructor
   *
   * @param side Side of the network connection
   * @param host Host name
   * @param port Port number
   */
   NetworkDevice(NetworkSide side,
                 const string &host,
                 unsigned short port);

   /**
   * Constructor
   *
   */
   NetworkDevice();

   /**
   * Method getSockaddr builds sockaddr information based on input 
   * host and port. 
   *
   * @param host Host name
   * @param port Port number
   * @param sockaddr Sockaddr information
   * @return Status of operation//   int count = timeoutSecs;
//   TimeValue time(1, TimeValue::Seconds);
//   int waitTicks = 0;
//   waitTicks = time.convertTo<int>(TimeValue::ClockTicks);sockaddr_in
//   while (count != 0 && resolveArp(host) == ERROR)
//   {
//      taskDelay(waitTicks);
//      count--;
//   }
//   if (count == 0)
//   {
//      return(ERROR);
//   }
   */
   static int getSockaddr(const string &host, unsigned short port,
                             sockaddr_in &sockaddr);

   /**
   * Method setRemoteSockaddr sets remote sockaddr based on input 
   * host and port. 
   *  
   * @param host Host name
   * @param port Port number
   * @return Status of operation
   */
   int setRemoteSockaddr(const string &host,
                                           unsigned short port);

   /**
   * Method getSocketOption obtains socket option data. 
   *
   * @param option Option code
   * @param optValue Option value
   * @return Status of operation
   */
   int getSocketOption(int option, int &optValue);

   /**
   * Method setSocketOption sets socket option data.
   *
   * @param option Option code
   * @param optValue Option value
   * @return Status of operation
   */
   int setSocketOption(int option, int optValue);
   
   /**
   * Method getInetAddrString returns the dot notation internet 
   * address for the input host name. 
   *
   * @param hostName Host name
   * @return Dot notation internet address, empty string if not 
   *         found.
   */
   static string getInetAddrString(const string &host);

   /**
    * Returns the local host of this network device 
    * 
    * @return Local host of this network device
    */
   inline const string& getLocalHost();
   
   /**
    * Returns the local port
    * 
    * @return Local port
    */
   inline unsigned short getLocalPort();

   /**
    * Returns the remote host of this network device 
    * 
    * @return Remote host of this network device
    */
   inline const string& getRemoteHost();

   /**
    * Returns the remote port
    * 
    * @return Remote port
    */
   inline unsigned short getRemotePort();

   /**
   * Method resolveArp attempts to resolve the ARP entry for the 
   * remote host name. 
   *
   * @param host Destination host name to resolve ARP entry for
   * @return Status of operation 
   */
   static int resolveArp(const string &host);

   /**
   * Method makePermanentArpEntry attempts to install a permanent 
   * ARP entry for a remote host name until timeoutSecs is reached.
   *
   * @param host Destination host name to resolve ARP entry
   * @param timeoutSecs Number of seconds afterwhich to give up the ARP 
   *                    resolution attemps. Defaults to 40 minutes due to 40
   *                    minuted startup time alloted to RIMS per the RIMS spec.
   * @return OK if successful, ERROR otherwise.
   */
   static int makePermanentArpEntry(const string &host,
                                       int timeoutSecs = 2400);
   
   void outputSocketInfo();
   
protected:

   /** Host definition */

   typedef struct
   {
      string hostName;
      unsigned short port;
      sockaddr_in sockaddr;
   } NetworkAddressInformation;

   /** Local network information */

   NetworkAddressInformation _local;

   /** Remote network information */

   NetworkAddressInformation _remote;

   /** Network connection side */

   NetworkSide _side;

};

inline const string &NetworkDevice::getLocalHost()
{
   return(_local.hostName);
}

inline unsigned short NetworkDevice::getLocalPort()
{
   return(_local.port);
}

inline const string &NetworkDevice::getRemoteHost()
{
   return(_remote.hostName);
}

inline unsigned short NetworkDevice::getRemotePort()
{
   return(_remote.port);
}
#endif
