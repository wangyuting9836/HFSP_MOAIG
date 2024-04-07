//
// Created by wangy on 2023/4/24.
//

#include <fstream>
#include "Job.h"

Job::Job(int id, int operationCount, bool is_init_process_time)
{
    this->id = id;
    processTime.resize(operationCount, 0);
	if(is_init_process_time)
	{
		for (auto& v : processTime)
		{
			v = wyt_rand(3, 7);
			//v = wyt_rand(1, 1);
			total_process_time += v;
		}
	}
}

Job::Job(int id, int operationCount, std::ifstream& fin)
{
	this->id = id;
	processTime.resize(operationCount);
	for (auto &v: processTime)
	{
		fin >> v;
		total_process_time += v;
	}
}

