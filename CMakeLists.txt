cmake_minimum_required(VERSION 3.23)
project(HFSP_Critical_Path)

set(CMAKE_CXX_STANDARD 17)

add_executable(HFSP_Critical_Path main.cpp HFS_Problem.cpp HFS_Problem.h Job.h Solution.cpp Solution.h Job.cpp rand.h rand.cpp
        utils.cpp
        utils.h
        IGWS.cpp
        IGWS.h
        PaperAlgorithm.cpp
        PaperAlgorithm.h
        MOAIG.cpp
        MOAIG.h
        CSA.cpp
        CSA.h
        TCSNSA.cpp
        TCSNSA.h
        MetaheuristicAlgorithms.cpp
        MetaheuristicAlgorithms.h
        TemporyStorage.cpp
        TemporyStorage.h)
