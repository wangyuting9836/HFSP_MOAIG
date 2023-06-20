//
// Created by wangy on 2023/4/24.
//

#ifndef HFSP_CRITICAL_PATH_SOLUTION_H
#define HFSP_CRITICAL_PATH_SOLUTION_H

#include <vector>
#include <unordered_map>
#include "HFS_Problem.h"

class Node
{
public:
    int jobId = -1;
    int stageId = -1;
    int machineId = -1;
	int machineIndex = -1; //机器上的顺序，第几个加工
    int processTime = 0;
    int startTime = 0;
    int tailTime = 0;
    Node *sucByStage = nullptr;
    Node *sucByMachine = nullptr;
    Node *preByStage = nullptr;
    Node *preByMachine = nullptr;

    Node(int jobId, int stageId, int machineId, int processTime)
            : jobId(jobId), stageId(stageId), machineId(machineId), processTime(processTime)
    {}
};

class Solution
{
	const HFS_Problem& problem;
    std::vector<int> jobSequence;
	std::vector<std::vector<int>> jobCountOnMachinesInEachStage;
    std::vector<std::vector<Node *>> headOnMachinesInEachStage;
    std::vector<std::vector<Node *>> tailOnMachinesInEachStage;
    std::vector<Node *> headJobs;
    std::vector<Node *> tailJobs;
    int makeSpan{};

public:
    explicit Solution(const HFS_Problem &problem);
	Solution(const HFS_Problem& problem, const std::vector<int>& sequence);
	Solution(const HFS_Problem &problem, const std::string& fileName);
    ~Solution();

	void reCalculateMakeSpan();

	static void refreshStartTimeFromNode(Node *beginNode);

	static void refreshTailTimeFromNode(Node *beginNode);

	static void refreshStatTailTimeForNode(Node* node);

    void decode();

    void findOneCriticalPathMachineFirst(std::vector<Node *> &criticalPath) const;

    void findOneCriticalPathJobFirst(std::vector<Node*>& criticalPath) const;

    void findAllCriticalNodes(std::vector<Node *> &criticaNodes) const;

	void findCutNodes(std::vector<Node*>& cutNodes) const;

    int deleteNodeWithRefresh(Node *node);

	void deleteNodeWithoutRefresh(Node* node);

	void moveNode(Node *deletedNode, Node *afterNode);

    void print() const;

	int getJobCount(int stageId, int machineId) const
	{
		return jobCountOnMachinesInEachStage[stageId][machineId];
	}

    const std::vector<Node *> &getHeadOnMachinesInStage(int stageId) const
    {
        return headOnMachinesInEachStage[stageId];
    }

    const std::vector<Node *> &getTailOnMachinesInStage(int stageId) const
    {
        return tailOnMachinesInEachStage[stageId];
    }

    int getMakeSpan() const
    {
        return makeSpan;
    }

	void setMakeSpan(int span)
	{
		makeSpan = span;
	}

	const std::vector<int>& getJobSequence() const
	{
		return jobSequence;
	}

	const HFS_Problem& getProblem() const
	{
		return problem;
	}
};

#endif //HFSP_CRITICAL_PATH_SOLUTION_H
