#include <net/VRNetServer.h>

using namespace std;

namespace MinVR {

#define PORT "3490"  // the port users will be connecting to
#define BACKLOG 100	 // how many pending connections queue will hold

#ifndef WIN32
void sigchld_handler(int s) {
	while (waitpid(-1, NULL, WNOHANG) > 0);
}
#endif

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }  
  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// global variables necessary for thread control
client_control_t *VRNetServer::ctr = (client_control_t*) malloc(sizeof(client_control_t));
int VRNetServer::numClients = 0; 

VRNetServer::VRNetServer(const std::string &listenPort, int numExpectedClients) { 

  // init global variables
  numClients = numExpectedClients; 
  ctr->respond = PTHREAD_COND_INITIALIZER; 
  ctr->mutex = PTHREAD_MUTEX_INITIALIZER;
  FD_ZERO(&clientFDs);
  maxFD = 0;

#ifdef WIN32  // Winsock implementation

  WSADATA wsaData;
  // listen on sock_fd, new connection on new_fd
  SOCKET sockfd = INVALID_SOCKET;
  SOCKET new_fd = INVALID_SOCKET;
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_storage their_addr; // connector's address information
  socklen_t sin_size;
  const char yes = 1;
  char s[INET6_ADDRSTRLEN];
  int rv;
  
  rv = WSAStartup(MAKEWORD(2,2), &wsaData);
  if (rv != 0) {
    cerr << "WSAStartup failed with error: " << rv << endl;
    exit(1);
  }

  ZeroMemory(&hints, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = AI_PASSIVE; // use my IP

  if ((rv = getaddrinfo(NULL, listenPort.c_str(), &hints, &servinfo)) != 0) {
    cerr << "getaddrinfo() failed with error: " << rv << endl;
    WSACleanup();
    exit(1);
  }

  // loop through all the results and bind to the first we can
  for (p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == INVALID_SOCKET) {
      cerr << "socket() failed with error: " << WSAGetLastError() << endl;
      continue;
    }
    
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == SOCKET_ERROR) {
      cerr << "setsockopt() failed with error: " << WSAGetLastError() << endl;
      closesocket(sockfd);
      WSACleanup();
      exit(1);
    }

    if (bind(sockfd, p->ai_addr, (int)p->ai_addrlen) == SOCKET_ERROR) {
      closesocket(sockfd);
      sockfd = INVALID_SOCKET;
      cerr << "bind() failed with error: " << WSAGetLastError() << endl;
      continue;
    }
    
    break;
  }

  if (p == NULL) {
    cerr << "server: failed to bind" << endl;
    exit(2);
  }

  freeaddrinfo(servinfo); // all done with this structure

  if (listen(sockfd, BACKLOG) == SOCKET_ERROR) {
    cerr << "listen failed with errror: " << WSAGetLastError() << endl;
    closesocket(sockfd);
    WSACleanup();
    exit(1);
  }

  // Should we do the "reap all dead processes" as in the linux implementation below?

  printf("server: waiting for connections...\n");

  int numConnected = 0;
  while (numConnected < numExpectedClients) {
    sin_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if (new_fd == INVALID_SOCKET) {
      cerr << "server: got invalid socket while accepting connection" << endl;
      continue;
    }
    
    // Disable Nagle's algorithm on the client's socket
    char value = 1;
    setsockopt(new_fd, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value));

    numConnected++;
    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);

    FD_SET(new_fd,&clientFDs); // add fd to client fd list
    if (new_fd > maxFD) {
      maxFD = new_fd; 
    }
    _clientSocketFDs.push_back(new_fd);
  
  }


#else  // BSD sockets implementation

  numClients = numExpectedClients;
  int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_storage their_addr; // connector's address information
  socklen_t sin_size;
  struct sigaction sa;
  int yes=1;
  char s[INET6_ADDRSTRLEN];
  int rv;
  
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE; // use my IP
  
  if ((rv = getaddrinfo(NULL, listenPort.c_str(), &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    exit(1);
  }
  
  // loop through all the results and bind to the first we can
  for (p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("server: socket");
      continue;
    }
    
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
      perror("setsockopt");
      exit(1);
    }
    
    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("server: bind");
      continue;
    }
    
    break;
  }
  
  if (p == NULL)  {
    fprintf(stderr, "server: failed to bind\n");
    exit(2);
  }
  
  freeaddrinfo(servinfo); // all done with this structure
  
  if (listen(sockfd, BACKLOG) == -1) {
    perror("listen");
    exit(1);
  }
  
  sa.sa_handler = sigchld_handler; // reap all dead processes
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  printf("server: waiting for connections...\n");

  int numConnected = 0;
  while (numConnected < numExpectedClients) {
    sin_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if (new_fd == -1) {
      perror("accept");
      continue;
    }
    
    // Disable Nagle's algorithm on the client's socket
    char value = 1;
    setsockopt(new_fd, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value));

    numConnected++;
    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
    
    FD_SET(new_fd,&clientFDs); // add fd to client fd list

    if (new_fd > maxFD) {
      maxFD = new_fd; 
    }
    _clientSocketFDs.push_back(new_fd);
  }

#endif
}

VRNetServer::~VRNetServer() {
  for (std::vector<SOCKET>::iterator i=_clientSocketFDs.begin(); i < _clientSocketFDs.end(); i++) {
	#ifdef WIN32
      closesocket(*i);
    #else
      close(*i);
    #endif
  }

#ifdef WIN32
  WSACleanup();
#endif
}

// client handle event data interfacing (thread)
void *VRNetServer::handleClientED(void *args) {

  // parse input arguments 
  client_service_t *resInfo = (client_service_t *) args;

  // wait for event data from the clients socketFD
  VRDataQueue::serialData ed = waitForAndReceiveEventData(resInfo->client);

  // after we have the data we need to update the client control
  pthread_mutex_lock(&ctr->mutex);

  // add the received data to the client control queue
  ctr->dq.addSerializedQueue(ed);
  ctr->ready++; 
  
  // check to see if this was the last client
  if (ctr->ready == numClients) {
      printf("broadcast\n");
      pthread_cond_broadcast(&ctr->respond);

      // we can assume that we have all the data at this point so we will serialize it 
      ctr->sq = ctr->dq.serialize();
  }

  // if this is not the last client wait in conditional loop 
  while (ctr->ready != numClients) {
      printf("Client %d waiting...\n",resInfo->client);
      pthread_cond_wait(&ctr->respond, &ctr->mutex);
    }
  
  // return control of client ctr struct
  pthread_mutex_unlock(&ctr->mutex);
  
  // broadcast the serialized data to this client 
  sendEventData(resInfo->client,ctr->sq);
  
  // done with this thread :) 
  pthread_exit(NULL);
}

// client handle swap buffer interfacing (thread)
void* VRNetServer::handleClientSB(void* args) {

    // parse input arguments
    client_service_t* resInfo = (client_service_t*)args;

    // wait for event data from the clients socketFD
    waitForAndReceiveSwapBuffersRequest(resInfo->client);

    // after we have the data we need to update the client control
    pthread_mutex_lock(&ctr->mutex);

    // add the received data to the client control queue
    ctr->ready++;

    // check to see if this was the last client
    if (ctr->ready == numClients) {
        printf("broadcast\n");
        pthread_cond_broadcast(&ctr->respond); 
    }

    // if this is not the last client wait in conditional loop
    while (ctr->ready != numClients) {
        printf("Client %d waiting...\n", resInfo->client);
        pthread_cond_wait(&ctr->respond, &ctr->mutex);
    }

    // return control of client ctr struct
    pthread_mutex_unlock(&ctr->mutex);

    // broadcast the serialized data to this client
    sendSwapBuffersNow(resInfo->client);

    free(resInfo); 

    // done with this thread :)
    pthread_exit(NULL);
  }
  
// This is the threaded function that would handle event data synchronization
// it starts a thread for each client and synchronizes them to signal at the same time
void VRNetServer::JarodFunction(msgType mt){

    // initialize counter variables
    int cHandleCount = 0; // counter for select loop
    ctr->ready = 0;  // counter for thread sync

    // array to store client thread ids
    pthread_t *cHandleThreads = (pthread_t *) malloc(sizeof(pthread_t) * numClients);

    // make a copy of the client FD set to filter as we loop through select. 
    // we need to filter FD's out of the set once they have been flagged because 
    // we cannot gaurantee that we will read the socket BEFORE the next loop tick
    // and we don't want select to return twice for the same event. 
    fd_set *serviceSet = (fd_set *) malloc(sizeof(fd_set)); // this we will modify each loop 
    fd_set *runningSet = (fd_set *) malloc(sizeof(fd_set)); // this we will reset before each call 

    // store all of our client file descriptors 
    memcpy(serviceSet,&clientFDs,sizeof(fd_set));

    // look for possible reads until we have read all clients
    while (cHandleCount != numClients) {

        // store remaining client file descriptors into the running set (reset running set) 
        memcpy(runningSet, serviceSet, sizeof(fd_set));

        // select potential reads
        int result = select(maxFD + 1, runningSet, NULL, NULL, NULL);

        // see which sockets were marked by the select() call
        for (std::vector<SOCKET>::iterator i = _clientSocketFDs.begin();
             i < _clientSocketFDs.end(); i++) {

            // if this fd is ready to read create a thread to handle it 
            if (FD_ISSET(*i, serviceSet)) {

                client_service_t* arg = (client_service_t*)malloc(sizeof(client_service_t));
                arg->client = *i;
                
                // generate handle thread
                if (mt == EVENTDATA) {
                    pthread_create(&cHandleThreads[cHandleCount], 0, (void* (*)(void*))VRNetServer::handleClientED, (void*)arg);
                } else if (mt == SWAPBUFFER) {
                    pthread_create(&cHandleThreads[cHandleCount], 0, (void* (*)(void*))VRNetServer::handleClientSB, (void*)arg);
                }

                // remove this fd from the set of fds that needs to be serviced
                FD_CLR(*i, serviceSet);

                // incremented the number of threads we've serviced 
                cHandleCount++;
            }
      }
    }

    // need to join up with all the threads before we can move on 
    for (int i = 0; i<numClients;i++){
      int join_status = pthread_join(cHandleThreads[i],NULL); 

      // did the join work? 
      if (join_status) { 
        fprintf(stderr,"Failed to join thread %d\n",cHandleThreads[i]); 
      }
    }
    
    // ok we are good to clean up 
    free(cHandleThreads); 
    free(serviceSet); 
    free(runningSet); 
}

// Wait for and receive an eventData message from every client, add
// them together and send them out again.
VRDataQueue::serialData VRNetServer::syncEventDataAcrossAllNodes(VRDataQueue::serialData eventData) {

    VRDataQueue dataQueue = VRDataQueue(eventData);
    ctr->dq = VRDataQueue(eventData);

    //this function will delegate threads to deal with all the clients at once 
    // JarodFunction(EVENTDATA);
    // return ctr->sq; 

    // TODO: rather than a for loop, could use a select() system call
    // here (I think) to figure out whic{h socket is ready for a read in
    // the situation where one client is ready but other(s) are not
    for (std::vector<SOCKET>::iterator itr=_clientSocketFDs.begin();
         itr < _clientSocketFDs.end(); itr++) {
      VRDataQueue::serialData ed = waitForAndReceiveEventData(*itr);

      dataQueue.addSerializedQueue(ed);
    }

    VRDataQueue::serialData dq = dataQueue.serialize();
    // 2. send new combined inputEvents array out to all clients
    for (std::vector<SOCKET>::iterator itr = _clientSocketFDs.begin();
         itr < _clientSocketFDs.end(); itr++) {
        sendEventData(*itr, dq);
    }

    return dq;
}

// This variant is not used by the server, but is part of the net
// interface definition for convenience.  Empty definition here to
// satisfy the compiler.
VRDataQueue::serialData
VRNetServer::syncEventDataAcrossAllNodes() {

  VRDataQueue::serialData out = "";
  return out;
}

void VRNetServer::syncSwapBuffersAcrossAllNodes() {
  // 1. wait for, receive, and parse a swap_buffers_request message
  // from every client
  
  // JarodFunction(SWAPBUFFER); 
  // return; 

  // TODO: rather than a for loop could use a select() system call here (I think) to figure out which socket is ready for a read in the situation where 1 is ready but other(s) are not
  for (std::vector<SOCKET>::iterator itr=_clientSocketFDs.begin();
       itr < _clientSocketFDs.end(); itr++) {
    waitForAndReceiveSwapBuffersRequest(*itr);
  }
  
  // 2. send a swap_buffers_now message to every client
  for (std::vector<SOCKET>::iterator itr=_clientSocketFDs.begin();
       itr < _clientSocketFDs.end(); itr++) {
    sendSwapBuffersNow(*itr);
  }
}

} // end namespace MinVR  
