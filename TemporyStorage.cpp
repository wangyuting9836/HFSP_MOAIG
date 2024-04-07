//
// Created by wangy on 2024/2/6.
//

#include "TemporyStorage.h"

std::vector<std::vector<int>> TemporyStorage::c_time;
std::vector<std::vector<int>> TemporyStorage::m_idle_time;
std::vector<int> TemporyStorage::sequence_of_other_stage;
std::vector<int> TemporyStorage::U;
std::vector<int> TemporyStorage::CF;
std::vector<int> TemporyStorage::RCL;

void TemporyStorage::init_storage(const HFS_Problem& problem)
{
	int num_of_jobs = problem.getNumOfJobs();
	int num_of_stages = problem.getNumOfStages();
	//各工序的正向完工时间
	c_time = std::vector<std::vector<int>>(num_of_jobs, std::vector<int>(num_of_stages));
	//每个阶段每个机器的可用时间
	m_idle_time = std::vector<std::vector<int>>(num_of_stages);
	for (int s = 0; s < problem.getNumOfStages(); s++)
	{
		m_idle_time[s].resize(problem.getNumOfMachinesInStage(s));
	}

	U = std::vector<int>(num_of_jobs);
	CF = std::vector<int>(num_of_jobs);
	RCL = std::vector<int>(num_of_jobs);
}

void TemporyStorage::zero_storage()
{
	for (auto& vec : c_time)
	{
		std::fill(vec.begin(), vec.end(), 0);
	}
	for (auto& vec : m_idle_time)
	{
		std::fill(vec.begin(), vec.end(), 0);
	}
}