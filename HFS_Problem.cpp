//
// Created by wangy on 2023/4/24.
//

#include <fstream>
#include <iostream>
#include "HFS_Problem.h"
#include "rand.h"
#include "Solution.h"


HFS_Problem::HFS_Problem(int jobCount, int stageCount) : numOfJobs(jobCount), numOfStages(stageCount)
{
    init(jobCount, stageCount);
}

HFS_Problem::HFS_Problem(const std::string& fileName)
{
	readFromFile(fileName);
}

void HFS_Problem::init(int jobCount, int stageCount)
{
    numOfMachinesInStage.resize(numOfStages);
    for(auto& v : numOfMachinesInStage)
    {
       	v = wyt_rand(2, 2);
    }
    for(int i = 0; i < jobCount; ++i)
    {
        jobs.emplace_back(i, numOfStages);
    }

	for(const auto& job : jobs)
	{
		total_process_time += job.get_total_process_time();
	}
}


void HFS_Problem::readFromFile(const std::string& fileName)
{
	std::ifstream fin(fileName);
	if (!fin)
	{
		std::cout << "File open error: " + fileName << std::endl;
		return;
	}
	fin >> numOfJobs;
	fin >> numOfStages;
	for(int s = 0; s < numOfStages; ++s)
	{
		int mCount;
		fin >> mCount;
		numOfMachinesInStage.emplace_back(mCount);
	}

	for(int j = 0; j < numOfJobs; ++j)
	{
		jobs.emplace_back(j, numOfStages, false);
	}

	int pt;
	for(int s = 0; s < numOfStages; ++s)
	{
		for(int j = 0; j < numOfJobs; ++j)
		{
			fin >> pt;
			jobs[j].setProcessTime(s, pt);
		}
	}

	fin.close();

	for(const auto& job : jobs)
	{
		total_process_time += job.get_total_process_time();
	}
}

void HFS_Problem::writeToFile(const std::string& fileName) const
{
	std::ofstream pFOut(fileName);
	pFOut << numOfJobs << "\t" << numOfStages << std::endl;
	for (int s = 0; s < numOfStages; ++s)
	{
		pFOut << numOfMachinesInStage[s] << "\t";
	}
	pFOut << std::endl;

	for (int s = 0; s < numOfStages; ++s)
	{
		for (const auto& job : jobs)
		{
			pFOut << job.getProcessTime(s) << "\t";
		}
		pFOut << std::endl;
	}
	pFOut.close();
}

void HFS_Problem::writeToGurobiFile(const std::string& fileName) const
{
	/*F = fields.IntegerField('Jobs')
	G = fields.IntegerField('Stages')
	J_g = fields.ListField('Number of Machines in each Stage')
	process_time = fields.MatrixField('Processing times of jobs')*/

	std::ofstream pFOut(fileName);
	pFOut << "Jobs:" << numOfJobs << std::endl;
	pFOut << "Stages:" << numOfStages << std::endl;

	pFOut << "Number of Machines at each Stage:" << std::endl;
	for (int s = 0; s < numOfStages; ++s)
	{
		pFOut << numOfMachinesInStage[s] << "\t";
	}
	pFOut << std::endl;

	pFOut << "Processing times of jobs:" << std::endl;
	for (int s = 0; s < numOfStages; ++s)
	{
		for (const auto& job : jobs)
		{
			pFOut << job.getProcessTime(s) << "\t";
		}
		pFOut << std::endl;
	}
	pFOut.close();
}





