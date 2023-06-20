//
// Created by wangy on 2023/4/24.
//

#ifndef HFSP_CRITICAL_PATH_HFS_PROBLEM_H
#define HFSP_CRITICAL_PATH_HFS_PROBLEM_H

#include <vector>
#include "Job.h"


class HFS_Problem
{
public:
    HFS_Problem(int jobCount, int stageCount);
	explicit HFS_Problem(const std::string& fileName);

    virtual ~HFS_Problem() = default;

    int getNumOfJobs() const;

    int getNumOfStates() const;

	int getNumOfMachinesInStage(int stageId) const;

    const std::vector<Job> &getJobs() const;

private:
    int numOfJobs{};
    int numOfStates{};
    std::vector<int> numOfMachinesInStage{};
    std::vector<Job> jobs{};

    void init(int jobCount, int stageCount);
	void readFile(const std::string& fileName);
};

inline int HFS_Problem::getNumOfJobs() const
{
    return numOfJobs;
}

inline int HFS_Problem::getNumOfStates() const
{
    return numOfStates;
}

/*inline const std::vector<int> &HFS_Problem::getNumOfMachinesInStage() const
{
    return numOfMachinesInStage;
}*/

inline int HFS_Problem::getNumOfMachinesInStage(int stageId) const
{
	return numOfMachinesInStage[stageId];
}

inline const std::vector<Job> &HFS_Problem::getJobs() const
{
    return jobs;
}


#endif //HFSP_CRITICAL_PATH_HFS_PROBLEM_H
