//
// Created by wangy on 2023/4/24.
//

#include <fstream>
#include <iostream>
#include "HFS_Problem.h"
#include "rand.h"
#include "Solution.h"


HFS_Problem::HFS_Problem(int jobCount, int stageCount) : numOfJobs(jobCount), numOfStates(stageCount)
{
    init(jobCount, stageCount);
}

HFS_Problem::HFS_Problem(const std::string& fileName)
{
	readFile(fileName);
}

void HFS_Problem::init(int jobCount, int stageCount)
{
    numOfMachinesInStage.resize(numOfStates);
    for(auto& v : numOfMachinesInStage)
    {
        v = wyt_rand(2, 3);
    }
    for(int i = 0; i < jobCount; ++i)
    {
        jobs.emplace_back(i, numOfStates);
    }
}


void HFS_Problem::readFile(const std::string& fileName)
{
	std::ifstream fin(fileName);
	if (!fin)
	{
		std::cout << "File open error: " + fileName << std::endl;
		return;
	}
	fin >> numOfStates;
	fin >> numOfJobs;
	for(int s = 0; s < numOfStates; ++s)
	{
		int mCount;
		fin >> mCount;
		numOfMachinesInStage.emplace_back(mCount);
	}

	for(int j = 0; j < numOfJobs; ++j)
	{
		jobs.emplace_back(j, numOfStates, fin);
	}
	fin.close();
}





