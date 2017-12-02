#ifndef VRNETCLIENT_H
#define VRNETCLIENT_H

#include "VRNetInterface.h"

#ifndef WIN32
  #include <netinet/tcp.h>
  #include <arpa/inet.h>
#endif

#define FORCE_CONNECTION 5

namespace MinVR {

class VRNetClient : public VRNetInterface {
 public:

  VRNetClient(const std::string &serverIP, const std::string &serverPort);
  ~VRNetClient();

  VRDataQueue::serialData
    syncEventDataAcrossAllNodes(VRDataQueue::serialData eventData);

  void syncSwapBuffersAcrossAllNodes();

  CLIENT_STATUS status; 

 private:

  SOCKET _socketFD;

};


} // end namespace MinVR  


#endif
