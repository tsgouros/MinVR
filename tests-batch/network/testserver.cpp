#include <net/VRNetServer.h>

// NOTE program to test VRNetServer receiveall
// Intended to be used in tandem with testclient sendall test program
/*
int main() {
	MinVR::VRNetServer server = MinVR::VRNetServer("3490", 1);
	unsigned char *buf = new unsigned char[1];
	int status = server.receiveall(buf, 1);
	if (status == -1) {
		std::cerr << "Test error: receiveall failed." << std::endl;
		exit(1);
	}
	std::cout << "OK: " << buf << std::endl;
}
*/

// NOTE program to test waitForAndReceiveSwapBuffersRequestAcrossAllNodes
// Intended to be used in tandem with testserver sendSwapBuffersRequest program
/*
int main() {
	MinVR::VRNetServer server = MinVR::VRNetServer("3490", 3);
	server.waitForAndReceiveSwapBuffersRequestAcrossAllNodes();
	std::cout << "OK" << std::endl;
}
*/

// NOTE program to test waitForAndReceiveSwapBuffersRequestAcrossAllNodes
// Accepts two args, selects how many clients to connect to, runs it, returns the result.
/*
int main(int argc, char* argv[]) {
  int defaultchoice = 1;
  int choice = defaultchoice;

  if (argc > 1) {
    if(sscanf(argv[1], "%d", &choice) != 1) {
      printf("Couldn't parse that input as a number\n");
      return -1;
    }
  }

	MinVR::VRNetServer server = MinVR::VRNetServer("3490", choice);
	server.waitForAndReceiveSwapBuffersRequestAcrossAllNodes();
	std::cout << "OK" << std::endl;
}
*/

// NOTE program to test syncEventDataAcrossAllNodes with mulitple expected clients
/*
int main(int argc, char* argv[]) {
  int defaultchoice = 2; //program by default expects 2 clients to connect
  int choice = defaultchoice; //choice defines how many clients server should expect to connect

  // server accepts one argument
  if (argc > 1) {
    if(sscanf(argv[1], "%d", &choice) != 1) {
      printf("Couldn't parse that input as a number\n");
      return -1;
    }
  }

	MinVR::VRNetServer server = MinVR::VRNetServer("3490", choice);
	MinVR::VRDataQueue::serialData eventData = server.syncEventDataAcrossAllNodes("a");
	std::cout << "server sent: " << eventData << std::endl;
}
*/

// NOTE program to test syncSwapBuffersAcrossAllNodes_test with mulitple expected clients
int main(int argc, char* argv[]) {
  int defaultchoice = 2; //program by default expects 2 clients to connect
  int choice = defaultchoice; //choice defines how many clients server should expect to connect

  // server accepts one argument
  if (argc > 1) {
    if(sscanf(argv[1], "%d", &choice) != 1) {
      printf("Couldn't parse that input as a number\n");
      return -1;
    }
  }

	MinVR::VRNetServer server = MinVR::VRNetServer("3490", choice);
	server.syncSwapBuffersAcrossAllNodes_test();
  std::cout << "done" << '\n';
}
