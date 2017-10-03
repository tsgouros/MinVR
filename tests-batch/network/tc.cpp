#include "net/VRNetClient.h"
#include "config/VRDataIndex.h"
#include "config/VRDataQueue.h"
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>


int main(int argc, char* argv[]) {
  int defaultchoice = 2; // program by default instantiates 2 clients
  int choice = defaultchoice;

  if (argc > 1) {
    if(sscanf(argv[1], "%d", &choice) != 1) {
      printf("Couldn't parse that input as a number\n");
      return -1;
    }
  }

  std::vector<pid_t> pids(choice);

  srand(time(NULL));

  // forks n processes; each process connects to server and sends a
  // syncEventDataAcrossAllNodes
  for (int i = 0; i < choice; i++) {
    pids[i] = fork();
    long r = random();

    if (pids[i] < 0) {
      printf("fork() failed\n");
    } else if (pids[i] == 0) {
      //printf("Child process %d (parent %d) syncSwapBuffersAcrossAllNodes_test\n", getpid(), getppid());
    	// MinVR::VRNetClient client = MinVR::VRNetClient("localhost", "3490");
      // if (r % 2 == 0) {
      //   sleep(5);
      // }
       std::cout << "tc: SYNC SWAP BUFFERS REQUEST" << std::endl;
    	// client.syncSwapBuffersAcrossAllNodes();
      // exit(0);
      int ret = execl("bin/launchclient", "bin/launchclient", (char*)NULL);
      if (ret < 0) {
        std::cerr << "execl failed: " << errno << '\n';
      } else std::cout << "ret=" << ret << std::endl;
    }
  }

  // waits for n child processes to finish running
  for (int i = 0; i < choice; ++i) {
    int status;
    while (-1 == waitpid(pids[i], &status, 0));
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        std::cout << "WIFSIGNALED=" << WIFSIGNALED(status) << '\n';
        std::cout << "WIFSTOPPED=" << WIFSTOPPED(status) << '\n';
        std::cout << "WTERMSIG=" << WTERMSIG(status) << '\n';
        std::cout << "WSTOPSIG=" << WSTOPSIG(status) << '\n';
        cerr << "Process " << i << " (pid " << pids[i] << ") failed" << endl;
        exit(1);
    }
  }
  std::cout << "CLIENT SUCCESS" << std::endl;
}
