//
// Created by wangy on 2023/4/24.
//

#include <fstream>
#include "Job.h"

Job::Job(int id, int operationCount)
{
    this->id = id;
    processTime.resize(operationCount);
    for (auto &v: processTime)
    {
        v = wyt_rand(3, 10);
    }
}

Job::Job(int id, int operationCount, std::ifstream& fin)
{
	this->id = id;
	processTime.resize(operationCount);
	for (auto &v: processTime)
	{
		fin >> v;
	}
}
