cmake_minimum_required(VERSION 2.8) 
project(testCMakeApp)

include(PICMake)

pi_add_target(${PROJECT_NAME} BIN src REQUIRED Qt4 OpenCV)

set(CMAKE_AUTOMOC ON)

find_package(Qt4 REQUIRED) # find Qt4
find_package(OpenCV REQUIRED) # find OpenCV

include_directories(${QT_INCLUDES} ${OPENCV_INCLUDE_DIRS})
add_executable(${PROJECT_NAME} src/main.cpp src/another.cpp)
target_link_libraries(${PROJECT_NAME} Qt4::QtGui Qt4::QtXml ${OpenCV_LIBS})# + testCMakeApp
# -- main.cpp
# -- CMakeLists.txt

