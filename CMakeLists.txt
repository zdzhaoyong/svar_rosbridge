cmake_minimum_required(VERSION 2.8)
project(svar_rosbridge)

include(PICMake)
set(CMAKE_CXX_STANDARD 11)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()


pi_add_target(svar_rosbridge SHARED src REQUIRED System GSLAM DEPENDENCY -lboost_system -lcrypto)

pi_report_target()

# Now do make install
pi_install(TARGETS svar_rosbridge)
