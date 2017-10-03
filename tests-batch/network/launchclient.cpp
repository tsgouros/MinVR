#include "net/VRNetClient.h"

// Program to launch one VRNetClient that connects to server
// Created client executes syncSwapBuffersAcrossAllNodes
// Program intended to be executed by forked child process in testclient.cpp
int main(int argc, char* argv[]) {
	srand(time(NULL));
	long r = random();
	MinVR::VRNetClient client = MinVR::VRNetClient("localhost", "3490");
	if (r % 2 == 0) {
		sleep(5);
	}
	std::cout << "lc: SYNC SWAP BUFFERS REQUEST" << std::endl;
	client.syncSwapBuffersAcrossAllNodes();
	exit(0);
}
