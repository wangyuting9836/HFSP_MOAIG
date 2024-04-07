//
// Created by wangy on 2023/4/24.
//

#ifndef HFSP_CRITICAL_PATH_JOB_H
#define HFSP_CRITICAL_PATH_JOB_H

#include <vector>
#include "rand.h"

class Job
{
    int id = -1;
    std::vector<int> processTime{};
	int total_process_time = 0;
public:
    Job(int id, int operationCount, bool is_init_process_time = true);
	Job(int id, int operationCount, std::ifstream& fin);

    int getId() const;

    int getProcessTime(int operation) const;
    void setProcessTime(int operation, int process_time);
	int get_total_process_time() const;
    virtual ~Job() = default;
};

inline int Job::getId() const
{
    return id;
}

inline int Job::getProcessTime(int operation) const
{
    return processTime[operation];
}

inline void Job::setProcessTime(int operation, int process_time)
{
	total_process_time -= processTime[operation];
	processTime[operation] = process_time;
	total_process_time += processTime[operation];
}

inline int Job::get_total_process_time() const
{
	return total_process_time;
}


#endif //HFSP_CRITICAL_PATH_JOB_H
