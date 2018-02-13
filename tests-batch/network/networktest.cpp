#include <stdio.h>
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "net/VRNetClient.h"
#include "net/VRNetServer.h"
#include "config/VRDataIndex.h"
#include "config/VRDataQueue.h"

#include <errno.h>
#include <string>
#include <cstdlib>

extern "C" {
    #include <pthread.h>
}

// If a 'make test' shows a test failing, try 'ctest -VV' to get more
// detail.  This will also print out whatever goes to std::cout.

int testConnection(); //test that clients and servers can connect
int testEventData(); //test that we can sync and swap event data
int testSwapBuffer(); //test that we can sync and send a swap buffer request 

// You can make this long to get better timing data.
#define LOOP for (int loopctr = 0; loopctr < 10; loopctr++)
#define NUMCLIENTS 5
#define PORT "3490"
#define TEST_DATA "a"

/*
**********************************************************************
* A LOT OF REPEATED CODE AND NEED TO CHECK FOR FAILURES BETTER *******      
* ALSO ADD MORE MODULAR TESTS FOR RECIEVEALL AND SENDALL *************
* GENERALLY CLEAN ****************************************************
**********************************************************************
*/

//global variable that each thread will update with the status of its task
int tasks[NUMCLIENTS];
pthread_mutex_t task_mutex = PTHREAD_MUTEX_INITIALIZER; 

int networktest(int argc, char* argv[]) {
  
  fprintf(stderr,"Hello World\n"); 
  // if running a specific test and no test is specified 
  int defaultchoice = 1;
  int choice = defaultchoice;

  // if there is a command line argument passed use this as the test number
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

  case 2:
    output = testEventData(); 
    break;

  case 3: 
    output = testSwapBuffer();
    break; 
    
  default:
    std::cout << "Test #" << choice << " does not exist!\n";
    output = -1;
  }
  
  return output;
}

void *testServer(void *blank){
  //launch server on port, if it gets all connections it will exit
  MinVR::VRNetServer server = MinVR::VRNetServer(PORT,NUMCLIENTS);
  pthread_exit((void *) 0);
}

// pass in the index of the task array this thread will update
void *testClient(void *ti){
  srand(time(NULL));
  long r = random();
  
  // cast to integer big enough to hold pointer
  intptr_t task_index = (intptr_t) ti;

  MinVR::VRNetClient client = MinVR::VRNetClient("127.0.0.1", PORT);

  pthread_mutex_lock(&task_mutex);
  if (client.status == 0){
    tasks[task_index] = 0;
  } else {
    tasks[task_index] = 1;
  }
  pthread_mutex_unlock(&task_mutex);

  sleep(5);
  
  pthread_exit((void *) 0); 
}


int testConnection(){ 
  pthread_t serverTID, clientTID[NUMCLIENTS];
  pthread_attr_t ct_attr, st_attr; 

  pthread_attr_init(&st_attr); 
  pthread_attr_setdetachstate(&st_attr, PTHREAD_CREATE_JOINABLE);
  pthread_attr_setschedpolicy(&st_attr, SCHED_FIFO); 

  int st_status = pthread_create(&serverTID,&st_attr,&testServer,NULL);

  pthread_attr_destroy(&st_attr); 

  if (st_status != 0){
      printf("Server thread creation failure"); 
      return 1;
  } 

  //avoid thread clobbering
  sleep(2); 

  int ct_status; //check the client threads

  pthread_attr_init(&ct_attr);
  pthread_attr_setdetachstate(&ct_attr, PTHREAD_CREATE_JOINABLE);
  pthread_attr_setschedpolicy(&ct_attr, SCHED_FIFO);

  for (int index = 0; index < NUMCLIENTS; index++){

      ct_status = pthread_create(&clientTID[index],&ct_attr,&testClient,(void *) index); 

      if (ct_status != 0) {
        printf("client thread %d creation failure",index);
        return 1; 
      }

  }

  pthread_attr_destroy(&ct_attr); 

  // wait for child threads then cut 
  pthread_exit(NULL); 

  fprintf(stderr,"NOOOO\n"); 
  int return_val = 0; 

  for (int i = 0; i < NUMCLIENTS; i++){
    if(tasks[i]){
      return_val = -1;
    }
  }

  return return_val; 
}



void *testServer_ed(void *blank)
{
  //launch server on port, if it gets all connections it will exit
  MinVR::VRNetServer server = MinVR::VRNetServer(PORT, NUMCLIENTS);
  //wait for event data sync requests from clients
  MinVR::VRDataQueue::serialData eventData = server.syncEventDataAcrossAllNodes(TEST_DATA);
  pthread_exit((void *)0);
}

// pass in the index of the task array this thread will update
void *testClient_ed(void *ti) {
  srand(time(NULL));
  long r = random();

  // cast to integer big enough to hold pointer
  intptr_t task_index = (intptr_t)ti;

  MinVR::VRNetClient client = MinVR::VRNetClient("127.0.0.1", PORT);

  MinVR::VRDataQueue::serialData eventData = client.syncEventDataAcrossAllNodes(TEST_DATA);

  pthread_mutex_lock(&task_mutex);
  if (client.status == 0) {
    tasks[task_index] = 0;
  } else {
      tasks[task_index] = 1;
    }
  pthread_mutex_unlock(&task_mutex);

    sleep(5); 

    pthread_exit((void *)0);
}

int testEventData() {
  pthread_t stID, cids[NUMCLIENTS];
  pthread_attr_t ct_attr, st_attr;

  pthread_attr_init(&st_attr);
  pthread_attr_setdetachstate(&st_attr, PTHREAD_CREATE_JOINABLE);
  pthread_attr_setschedpolicy(&st_attr, SCHED_FIFO);

  int st_status = pthread_create(&stID, &st_attr, &testServer_ed, NULL);

  pthread_attr_destroy(&st_attr);

  if (st_status != 0)
  {
    printf("Server thread creation failure");
    return 1;
  }

  printf("made server thread\n");

  //avoid thread clobbering
  sleep(2); 

  int ct_status; //check the client threads

  pthread_attr_init(&ct_attr);
  pthread_attr_setdetachstate(&ct_attr, PTHREAD_CREATE_JOINABLE);
  pthread_attr_setschedpolicy(&ct_attr, SCHED_FIFO);

  for (int index = 0; index < NUMCLIENTS; index++)
  {

    ct_status = pthread_create(&cids[index], &ct_attr, &testClient_ed, (void *)index);

    if (ct_status != 0)
    {
      printf("client thread %d creation failure", index);
      return 1;
    }
  }

  pthread_attr_destroy(&ct_attr);

  // wait for child threads then cut
  pthread_exit(NULL);

  int return_val = 0;

  for (int i = 0; i < NUMCLIENTS; i++)
  {
    if (tasks[i] == 0)
    {
      return_val = 1;
    }
  }

  return return_val;
}

void *testServer_sb(void *blank)
{
  //launch server on port, if it gets all connections it will exit
  MinVR::VRNetServer server = MinVR::VRNetServer(PORT, NUMCLIENTS);

  //wait for event data sync requests from clients
  server.syncSwapBuffersAcrossAllNodes();

  pthread_exit((void *)0);
}

// pass in the index of the task array this thread will update
void *testClient_sb(void *ti)
{
  srand(time(NULL));
  long r = random();

  // cast to integer big enough to hold pointer
  intptr_t task_index = (intptr_t)ti;

  MinVR::VRNetClient client = MinVR::VRNetClient("127.0.0.1", PORT);

  client.syncSwapBuffersAcrossAllNodes();

  //if it doesn't hang and the client is ok I guess it passes..
  pthread_mutex_lock(&task_mutex);
  if (client.status == 0) {
    tasks[task_index] = 0;
  } else {
    tasks[task_index] = 1;
  }
  pthread_mutex_unlock(&task_mutex);

  sleep(5);

  pthread_exit((void *)0);
}

int testSwapBuffer(){
  pthread_t stID, cids[NUMCLIENTS];
  pthread_attr_t ct_attr, st_attr;

  pthread_attr_init(&st_attr);
  pthread_attr_setdetachstate(&st_attr, PTHREAD_CREATE_JOINABLE);
  pthread_attr_setschedpolicy(&st_attr, SCHED_FIFO);

  int st_status = pthread_create(&stID, &st_attr, &testServer_sb, NULL);

  pthread_attr_destroy(&st_attr);

  if (st_status != 0)
  {
    printf("Server thread creation failure");
    return 1;
  }

  //avoid thread clobbering
  sleep(2); 

  int ct_status; //check the client threads

  pthread_attr_init(&ct_attr);
  pthread_attr_setdetachstate(&ct_attr, PTHREAD_CREATE_JOINABLE);
  pthread_attr_setschedpolicy(&ct_attr, SCHED_FIFO);

  for (int index = 0; index < NUMCLIENTS; index++)
  {

    ct_status = pthread_create(&cids[index], &ct_attr, &testClient_sb, (void *)index);

    if (ct_status != 0)
    {
      printf("client thread %d creation failure", index);
      return 1;
    }
  }

  pthread_attr_destroy(&ct_attr);

  // wait for child threads then cut
  pthread_exit(NULL);

  int return_val = 0;

  for (int i = 0; i < NUMCLIENTS; i++) {
    if (tasks[i] == 0) {
      return_val = 1;
    }
  }

  return return_val;
}
