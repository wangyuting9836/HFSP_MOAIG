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
	void writeToFile(const std::string& fileName) const;
	void writeToGurobiFile(const std::string& fileName) const;
    virtual ~HFS_Problem() = default;

    [[nodiscard]] int getNumOfJobs() const;

    [[nodiscard]] int getNumOfStages() const;

	[[nodiscard]] int getNumOfMachinesInStage(int stageId) const;

    [[nodiscard]] const std::vector<Job> &getJobs() const;
	[[nodiscard]] int get_total_process_time() const;

private:
    int numOfJobs{};
    int numOfStages{};
    std::vector<int> numOfMachinesInStage{};
    std::vector<Job> jobs{};
	int total_process_time = 0;
    void init(int jobCount, int stageCount);
	void readFromFile(const std::string& fileName);

};

inline int HFS_Problem::getNumOfJobs() const
{
    return numOfJobs;
}

inline int HFS_Problem::getNumOfStages() const
{
    return numOfStages;
}

inline int HFS_Problem::get_total_process_time() const
{
	return total_process_time;
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
