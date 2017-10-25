#include <stdio.h>
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

//dev dependencies
#include <errno.h>
#include <string>
#include <pthread.h>

#include "launch.h" // access network setup commands


#include <cstdlib>


// int launchClients(int numClients) {
//     return 1; 
// }

#define PORT "3069"
#define NUMCLIENTS 10

void *ls(void *blank){
   
        //launch server on port 
        MinVR::VRNetServer server = MinVR::VRNetServer(PORT,NUMCLIENTS);
        printf("Server filled connections: Status OK\n"); 
        MinVR::VRDataQueue::serialData eventData = server.syncEventDataAcrossAllNodes("a");
        //printf("how about here"); 
    
        pthread_exit(NULL);
    
}

void *lc(void *blank){
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


//this function tests the server and the client in tandem 
int main(int argc,char* argv[]){
    pthread_t stID, cid1, cid2; 
    pthread_t cids[NUMCLIENTS];
    pthread_once_t once_control = PTHREAD_ONCE_INIT;

    const std::string defaultPort = "3049"; 
    int defaultExpectedClients = 2; 
    int blank = 1;

    //int st_status = pthread_once(&once_control,ls);
    int st_status = pthread_create(&stID,NULL,ls,(void *) blank);

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
        ct_status = pthread_create(&cids[i - 1],NULL,lc,(void *) blank); 
        printf("client thread %d status: %d\n",i,ct_status); 

    }
       
    //MinVR::VRNetServer server = MinVR::VRNetServer(PORT,NUMCLIENTS);
    //int ret = execl("bin/testserver","bin/testserver",(char *) NULL); 

    printf("Main Thread continuing\n"); 

    pthread_exit(NULL); 

    return 12; 
}
