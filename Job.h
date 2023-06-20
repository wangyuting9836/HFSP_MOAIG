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
public:
    Job(int id, int operationCount);
	Job(int id, int operationCount, std::ifstream& fin);

    int getId() const;

    int getProcessTime(int operation) const;

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


#endif //HFSP_CRITICAL_PATH_JOB_H
