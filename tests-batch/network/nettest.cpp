#include <stdio.h>
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "net/VRNetClient.h"
#include "net/VRNetServer.h"
#include "config/VRDataIndex.h"
#include "config/VRDataQueue.h"

//dev dependencies
#include <errno.h>
#include <string>
#include <pthread.h>

#include <cstdlib>

// If a 'make test' shows a test failing, try 'ctest -VV' to get more
// detail.  This will also print out whatever goes to std::cout.

int testConnection();

// You can make this long to get better timing data.
#define LOOP for (int loopctr = 0; loopctr < 10; loopctr++)
#define NUMCLIENTS 10
#define PORT "3069"

int nettest(int argc, char* argv[]) {
  
  int defaultchoice = 1;
  
  int choice = defaultchoice;

  if (argc > 1) {
    if(sscanf(argv[1], "%d", &choice) != 1) {
      printf("Couldn't parse that input as a number\n");
      return -1;
    }
  }

  int output;

  // When you add a case here, run it by adding a number to the
  // sequence in this directory's CMakeLists.txt file.
  switch(choice) {
  case 1:
    output = testConnection();
    break;
    
    // Add case statements to handle values of 2-->10
  default:
    std::cout << "Test #" << choice << " does not exist!\n";
    output = -1;
  }
  
  return output;
}

void *launchServer(void *blank){
  
       //launch server on port 
       MinVR::VRNetServer server = MinVR::VRNetServer(PORT,NUMCLIENTS);
       printf("%d",server); 

       //MinVR::VRDataQueue::serialData eventData = server.syncEventDataAcrossAllNodes("a");
   
       pthread_exit(NULL);
   
}

void *launchClients(void *blank){
  srand(time(NULL));
  long r = random();
  MinVR::VRNetClient client = MinVR::VRNetClient("localhost", PORT);
  if (r % 2 == 0) {
      sleep(5);
  }
 
 sleep(2);
 //std::cout << "lc: SYNC SWAP BUFFERS REQUEST" << std::endl;
 //client.syncSwapBuffersAcrossAllNodes();
exit(0);
}


int testConnection(){ 
  pthread_t stID, cid1, cid2; 
  pthread_t cids[NUMCLIENTS];
  pthread_once_t once_control = PTHREAD_ONCE_INIT;
  void * blank;

  //int st_status = pthread_once(&once_control,ls);
  int st_status = pthread_create(&stID,NULL,&launchServer,NULL);

  if (st_status != 0){
      printf("gloop\n"); 
  } else {
      printf("Server thread started\n"); 
  }
  
  //put a small break before starting the clients for clarity
  //sleep(2); 

  int ct_status; //check the client threads

  for (int i = 1; i <= NUMCLIENTS; i++){
    printf("client thread %d started\n",i);
      ct_status = pthread_create(&cids[i - 1],NULL,&launchClients,NULL); 
      printf("client thread %d status: %d\n",i,ct_status); 

  }
  
  //What is hanging???
  //MinVR::VRNetServer server = MinVR::VRNetServer(PORT,NUMCLIENTS);
  //int ret = execl("bin/testserver","bin/testserver",(char *) NULL); 

  printf("Main Thread continuing\n"); 

  pthread_exit(NULL); 

  //I guess if it doesn't hang we can consider it a pass for now?
  //try to push errors through

  return 0; 
}


