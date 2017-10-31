#include <stdio.h>
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

//dev dependencies
#include <errno.h>
#include <string>
#include <cstdlib>

#include "net/VRNetClient.h"
#include "net/VRNetServer.h"
#include "net/VRNetInterface.h"

extern "C" {
    #include <pthread.h>
}

#define PORT "3069"
#define NUMCLIENTS 2

void *ls(void *blank){
   
        //launch server on port 
        MinVR::VRNetServer server = MinVR::VRNetServer(PORT,NUMCLIENTS);
        printf("Server filled connections: Status OK\n"); 
        //MinVR::VRDataQueue::serialData eventData = server.syncEventDataAcrossAllNodes("a");
        //printf("how about here"); 
    
        pthread_exit(NULL);
    
}

void *lc(void *blank){
    srand(time(NULL));
	long r = random();
    MinVR::VRNetClient client = MinVR::VRNetClient("localhost", PORT);
    
    printf("CLIENT GOT : %d\n",client.status); 
	if (r % 2 == 0) {
		sleep(5);
    }
    
    sleep(2);
	//std::cout << "lc: SYNC SWAP BUFFERS REQUEST" << std::endl;
	//client.syncSwapBuffersAcrossAllNodes();
	return (void *) client.status;
}


//this function tests the server and the client in tandem 
int main(int argc,char* argv[]){
    pthread_t stID, cid1, cid2; 
    pthread_t cids[NUMCLIENTS];
    pthread_once_t once_control = PTHREAD_ONCE_INIT;

    const std::string defaultPort = "3049"; 
    int defaultExpectedClients = 2; 
    int blank = 1;

    int st_status = pthread_create(&stID,NULL,ls,(void *) blank);
     
    //put a small break before starting the clients for clarity
    sleep(2); 

    int ct_status; //check the client threads

    for (int i = 1; i <= NUMCLIENTS; i++){
        printf("client thread %d started\n",i);
        ct_status = pthread_create(&cids[i - 1],NULL,lc,(void *) blank); 
        
        if(ct_status){
            printf("Failed to create client thread %d\n",i);
        }

    }
       
    //MinVR::VRNetServer server = MinVR::VRNetServer(PORT,NUMCLIENTS);

   

    void *rs;
    pthread_join(cids[0],&rs);
    printf("Main Thread continuing\n");

    pthread_exit(NULL); 

    printf("did we exit?"); 

    return 12; 
}

int try_with_fork(void){
    return 1;
}
