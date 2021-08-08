cmake_minimum_required(VERSION 2.8)
project(svar_rosbridge)

include(cmake/PICMake.cmake)

set(CMAKE_CXX_STANDARD 11)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

INCLUDE_DIRECTORIES(src)

pi_add_target(svar_rosbridge SHARED src REQUIRED System DEPENDENCY -lboost_system -lcrypto)
pi_add_target(simple_pubsub BIN examples/simple_pubsub.cpp REQUIRED System)
pi_add_target(vis_image BIN examples/vis_image.cpp REQUIRED System OpenCV)
pi_add_target(call_service BIN examples/call_service.cpp REQUIRED System)


pi_report_target()

# Now do make install
pi_install(TARGETS svar_rosbridge)
