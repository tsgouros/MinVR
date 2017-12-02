# Create network test to run with 'make test' or 'ctest -VV' to see the output

## Run these tests with 'make test' or 'ctest -VV' if you want to see
## the output.  You can also do 'ctest --memcheck' that runs the tests
## with some memory checking enabled.

set (networktests network)
# The numbers here correspond to 'case' statements in the respective
# test program.  See, e.g., datumtest.cpp
set (network_parts 1 2 3)

# For tests where a list of parts has not been defined we add a default of 1:
foreach(networktest ${networktests})
  if(NOT DEFINED "${networktest}_parts")
     set(${networktest}_parts "1")
  endif()

  # link cpp files 
  # set(networktestsrc ${networktestsrc} ${networktest}test.cpp)
  # hard coded for now until I have more tests
  set(networktestsrc ${networktestsrc} networktest.cpp)
endforeach()

# Each of these .cpp files has a function with the same name as the
# file.

create_test_sourcelist(srclist RunNetworkTests.cpp ${networktestsrc})
add_executable(test-network ${srclist})
set_target_properties(test-network PROPERTIES COMPILE_FLAGS -pthread LINK_FLAGS -pthread)
target_link_libraries(test-network MinVR)

# When it's compiled you can run the test-dataindex executable and
# specify a particular test and subtest:
#./test-dataindex queuetest 1
#All that's left is to tell CMake to generate the test cases:

set($ENV{MVRHOME} ${CMAKE_SOURCE_DIR})

foreach(networktest ${networktests})
  foreach(part ${${networktest}_parts})
    add_test(NAME test_${networktest}_${part}
      COMMAND ${CMAKE_BINARY_DIR}/bin/test-network ${networktest}test ${part}
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests-batch/network)
    set_tests_properties(test_${networktest}_${part} PROPERTIES
      FAIL_REGULAR_EXPRESSION "ERROR;FAIL;Test failed")
  endforeach()
endforeach()

add_executable(testnet testnet.cpp)
set_target_properties(testnet PROPERTIES COMPILE_FLAGS -pthread LINK_FLAGS -pthread)
target_link_libraries(testnet MinVR)
