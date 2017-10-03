# Create a library from the source files in this directory.
add_executable (testclient testclient.cpp)
add_executable (tc tc.cpp)
add_executable (testserver testserver.cpp)
add_executable (launchclient launchclient.cpp)

target_link_libraries (testclient MinVR)
target_link_libraries (tc MinVR)
target_link_libraries (testserver MinVR)
target_link_libraries (launchclient MinVR)
