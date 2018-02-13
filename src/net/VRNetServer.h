#ifndef VRNETSERVER_H
#define VRNETSERVER_H

#include "VRNetInterface.h"

#ifndef WIN32
  #include <netinet/tcp.h>
  #include <netdb.h>
  #include <arpa/inet.h>
  #include <sys/wait.h>
  #include <signal.h>
#endif

#include <math/VRMath.h>
#include <config/VRDataIndex.h>
#include <pthread.h> // pthreads
#include <sys/select.h> // select()

namespace MinVR {

  // struct to contain variables to sychronize client response threads 
  typedef struct client_control {
    pthread_cond_t respond; 
    pthread_mutex_t mutex; 
    int ready; 
    VRDataQueue dq; 
    VRDataQueue::serialData sq; 
  } client_control_t; 

  // struct to pass as argument to client response threads
  typedef struct client_service { 
    SOCKET client;
  } client_service_t; 

  enum msgType {
    EVENTDATA, 
    SWAPBUFFER
  };

class VRNetServer : public VRNetInterface {
 public:

  VRNetServer(const std::string &listenPort, int numExpectedClients);
  ~VRNetServer();

  VRDataQueue::serialData
    syncEventDataAcrossAllNodes(VRDataQueue::serialData eventData);
  VRDataQueue::serialData syncEventDataAcrossAllNodes();

  void syncSwapBuffersAcrossAllNodes();

  // initialize client control static variable
  static client_control_t* ctr;
  static int numClients;

  private:
  
  static void *handleClientED(void *);
  static void* handleClientSB(void *);
  void JarodFunction(msgType);
  // variable to store client file descriptors 
  std::vector<SOCKET> _clientSocketFDs;

  // fd set of all the client file descriptors 
  fd_set clientFDs; 

  // variables to control select loop 
  int maxFD;


};

}

#endif
