cmake_minimum_required(VERSION 3.23)
project(HFSP_Critical_Path)

set(CMAKE_CXX_STANDARD 14)

add_executable(HFSP_Critical_Path main.cpp HFS_Problem.cpp HFS_Problem.h Job.h Solution.cpp Solution.h Job.cpp PyGantt.cpp PyGantt.h rand.h rand.cpp)

find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
if (Python3_FOUND)
    message("Python include directory: " ${Python3_INCLUDE_DIRS})
    message("Python version is: " ${Python3_VERSION})
    message("Python libs is: " ${Python3_LIBRARIES})
    include_directories(${Python3_INCLUDE_DIRS})
    target_link_libraries(HFSP_Critical_Path ${Python3_LIBRARIES})
endif (Python3_FOUND)

include_directories("D:/coding_net/matplotlib-cpp")
include_directories("D:/Python311/Lib/site-packages/numpy/core/include")