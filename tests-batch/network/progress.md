#MinVR Networking Progress Log
## cshum (chris.c.shum@gmail.com)

## `sendall` creating compiler errors 4/8/2017

Renamed `sendall` to `sendall2`, updated all calls to `sendall2` --> results in the same error as seen before

```
In file included from /Users/cshum/Documents/Brown/Senior/YURT/MinVR/src/main/VRMain.cpp:21:
/Users/cshum/Documents/Brown/Senior/YURT/MinVR/src/net/VRNetClient.h:26:12: error:
      too many arguments to function call, expected 2, have 3; did you mean
      'VRNetInterface::sendall2'?
    return sendall2(_socketFD, buf, len);
           ^~~~~~~~
           VRNetInterface::sendall2
/Users/cshum/Documents/Brown/Senior/YURT/MinVR/src/net/VRNetInterface.h:51:13: note:
      'VRNetInterface::sendall2' declared here
        static int sendall2(SOCKET socketID, const unsigned char *buf, i...
                   ^
1 error generated.
make[2]: *** [src/CMakeFiles/MinVR.dir/main/VRMain.cpp.o] Error 1
make[1]: *** [src/CMakeFiles/MinVR.dir/all] Error 2
make: *** [all] Error 2
```

Renamed `sendall` to `sendall2`, called `sendall` to see if there is a stray sendall method we're not aware of --> doesn't seem like it

```
In file included from /Users/cshum/Documents/Brown/Senior/YURT/MinVR/src/main/VRMain.cpp:21:
/Users/cshum/Documents/Brown/Senior/YURT/MinVR/src/net/VRNetClient.h:26:12: error: use of undeclared identifier 'sendall'; did you mean 'VRNetInterface::sendall2'?
    return sendall(_socketFD, buf, len);
           ^~~~~~~
           VRNetInterface::sendall2
/Users/cshum/Documents/Brown/Senior/YURT/MinVR/src/net/VRNetInterface.h:51:13: note: 'VRNetInterface::sendall2' declared here
        static int sendall2(SOCKET socketID, const unsigned char *buf, int len); //TODO
                   ^
1 error generated.
make[2]: *** [src/CMakeFiles/MinVR.dir/main/VRMain.cpp.o] Error 1
make[1]: *** [src/CMakeFiles/MinVR.dir/all] Error 2
make: *** [all] Error 2
```

## `sendall` and `receiveall` unexpected outputs 4/8/2017
```cpp
unsigned char buf[1];
std::cout << buf << std::endl;
```
prints
```
<host> e.g. localhost or <port> e.g. 3490
```
Interestingly, problem occurs regardless of what the char array is called. Maybe initializing an array conveniently accesses some memory that contains the host/port. (?)
Could be pointing to first argument of the previous function call.

Problem solved by initializing buffer to a null state:
```
unsigned char *buf = new unsigned char[1];
```

## creating test setup for server and client instantiation 4/15/2017
The goal is to create a test setup where it is easy to perform one-to-many (server-to-client) networking tests. For simpler tests that only require clients to send off one signal, one-to-many is easily doable with a for loop to create as many clients as needed and send signals. This task is harder to achieve for tests that require clients to send and wait - a for loop structure is not usable because we need clients to be up and polling instead of using a for loop to iterate through them. One solution is to have each client program spawn one client, and use a bash script to launch multiple clients at once.

Update (5/1/2017): Using `fork()` and `wait()` system calls, it is possible to launch multiple clients, send event data and wait for server response.
