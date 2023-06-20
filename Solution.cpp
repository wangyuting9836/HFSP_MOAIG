//
// Created by wangy on 2023/4/24.
//
#include <numeric>
#include <iostream>
#include <stack>
#include <queue>
#include <fstream>
#include "Solution.h"

Solution::Solution(const HFS_Problem& problem)
		:problem(problem)
{
	jobSequence.resize(problem.getNumOfJobs());
	std::iota(std::begin(jobSequence), std::end(jobSequence), 0);
	std::shuffle(std::begin(jobSequence), std::end(jobSequence), rand_generator());
}

Solution::Solution(const HFS_Problem& problem, const std::vector<int>& sequence)
		:problem(problem), jobSequence(sequence)
{

}

Solution::Solution(const HFS_Problem& problem, const std::string& fileName)
		:problem(problem)
{
	jobSequence.resize(problem.getNumOfJobs());
	std::ifstream fin(fileName);
	for (auto& job : jobSequence)
	{
		fin >> job;
	}
	fin.close();
}


void Solution::decode()
{
	int jobCount = problem.getNumOfJobs();
	int stageCount = problem.getNumOfStates();
	const std::vector<Job>& jobs = problem.getJobs();

	for (int jobId = 0; jobId < jobCount; ++jobId)
	{
		headJobs.emplace_back(new Node(jobId, -1, -1, 0));
		tailJobs.emplace_back(new Node(jobId, -1, -1, 0));
		headJobs[jobId]->sucByStage = tailJobs[jobId];
		tailJobs[jobId]->preByStage = headJobs[jobId];
	}

	jobCountOnMachinesInEachStage.resize(stageCount);
	headOnMachinesInEachStage.resize(stageCount);
	tailOnMachinesInEachStage.resize(stageCount);
	for (int s = 0; s < stageCount; ++s)
	{
		int machineCount = problem.getNumOfMachinesInStage(s);
		jobCountOnMachinesInEachStage[s].resize(machineCount, 0);
		for (int m = 0; m < machineCount; ++m)
		{
			headOnMachinesInEachStage[s].emplace_back(new Node(-1, s, m, 0));
			tailOnMachinesInEachStage[s].emplace_back(new Node(-1, s, m, 0));

			headOnMachinesInEachStage[s][m]->sucByMachine = tailOnMachinesInEachStage[s][m];
			tailOnMachinesInEachStage[s][m]->preByMachine = headOnMachinesInEachStage[s][m];

		}
	}

	std::vector<Node*> preOperationsByStage = headJobs;
	std::vector<Node*> preOperationsByMachine;

	makeSpan = INT32_MIN;
	auto assignJobOnStatge = [&](int jobId, int statgeId)
	{
		Node* preOperByStage = preOperationsByStage[jobId];
		Node* preOperByMachine = *std::min_element(std::begin(preOperationsByMachine),
				std::end(preOperationsByMachine),
				[](const auto& l, const auto& r)
				{
					return l->startTime + l->processTime < r->startTime + r->processTime;
				});
		int processTime = jobs[jobId].getProcessTime(statgeId);
		int machineId = preOperByMachine->machineId;

		Node* curJobNode = new Node(jobId, statgeId, machineId, processTime);

		curJobNode->preByMachine = preOperByMachine;
		curJobNode->sucByMachine = tailOnMachinesInEachStage[statgeId][machineId];
		preOperByMachine->sucByMachine = curJobNode;
		tailOnMachinesInEachStage[statgeId][machineId]->preByMachine = curJobNode;

		curJobNode->preByStage = preOperByStage;
		curJobNode->sucByStage = tailJobs[jobId];
		preOperByStage->sucByStage = curJobNode;
		tailJobs[jobId]->preByStage = curJobNode;

		curJobNode->startTime = std::max(preOperByMachine->startTime + preOperByMachine->processTime,
				preOperByStage->startTime + preOperByStage->processTime);


		tailOnMachinesInEachStage[statgeId][machineId]->startTime = curJobNode->startTime + processTime;

		curJobNode->machineIndex = jobCountOnMachinesInEachStage[statgeId][machineId];
		++jobCountOnMachinesInEachStage[statgeId][machineId];
		tailOnMachinesInEachStage[statgeId][machineId]->machineIndex = jobCountOnMachinesInEachStage[statgeId][machineId];

		makeSpan = std::max(makeSpan, tailOnMachinesInEachStage[statgeId][machineId]->startTime);
		preOperationsByMachine[machineId] = curJobNode;
		preOperationsByStage[jobId] = curJobNode;
	};

	//第一阶段，按照jobSequence安排工件
	preOperationsByMachine = headOnMachinesInEachStage[0];
	for (auto j : jobSequence)
	{
		assignJobOnStatge(jobs[j].getId(), 0);
	}

	//其他阶段，按照first-come-first-served安排工件
	for (int s = 1; s < stageCount; ++s)
	{
		preOperationsByMachine = headOnMachinesInEachStage[s];
		std::vector<Node*> firstComeFirstServed = headOnMachinesInEachStage[s - 1];
		for (auto& node : firstComeFirstServed)
		{
			node = node->sucByMachine;
		}
		while (true)
		{
			Node* firstComeJob = *std::min_element(std::begin(firstComeFirstServed), end(firstComeFirstServed),
					[](const auto& l, const auto& r)
					{
						int ctime1 = l->jobId == -1 ? INT32_MAX : l->startTime +
								l->processTime;
						int ctime2 = r->jobId == -1 ? INT32_MAX : r->startTime +
								r->processTime;
						return ctime1 < ctime2;
					});
			if (firstComeJob->jobId == -1)
			{
				break;
			}
			assignJobOnStatge(firstComeJob->jobId, s);
			firstComeFirstServed[firstComeJob->machineId] = firstComeJob->sucByMachine;
		}
	}

	//计算逆向时间
	for (auto sit = std::rbegin(tailOnMachinesInEachStage); sit != std::rend(tailOnMachinesInEachStage); ++sit)
	{
		const std::vector<Node*>& tailOnMachines = *sit;
		for (auto mit = std::rbegin(tailOnMachines); mit != std::rend(tailOnMachines); ++mit)
		{
			Node* node = (*mit)->preByMachine;
			while (node->jobId != -1)
			{
				node->tailTime = std::max(node->sucByMachine->tailTime + node->sucByMachine->processTime,
						node->sucByStage->tailTime + node->sucByStage->processTime);
				node = node->preByMachine;
			}
			node->tailTime = node->sucByMachine->tailTime + node->sucByMachine->processTime;
		}
	}
}


void Solution::reCalculateMakeSpan()
{
	int lastStageId = problem.getNumOfStates() - 1;
	makeSpan = (*std::max_element(std::begin(tailOnMachinesInEachStage[lastStageId]),
			std::end(tailOnMachinesInEachStage[lastStageId]), [](const Node* l, const Node* r)
			{
				return l->startTime < r->startTime;
			}))->startTime;
}

void Solution::print() const
{
	int stageId = 0;
	for (const auto& headOnMachines : headOnMachinesInEachStage)
	{
		std::cout << stageId << std::endl;
		int machineId = 0;
		for (const auto& head : headOnMachines)
		{
			std::cout << "\t" << machineId << ":\t";
			Node* node = head->sucByMachine;
			while (node->jobId != -1)
			{
				std::cout << "(" << node->jobId;
				std::cout << "," << node->startTime;
				std::cout << "," << node->processTime << ")";
				node = node->sucByMachine;
			}
			++machineId;
			std::cout << std::endl;
		}
		++stageId;
	}

	int jobId = 0;
	for (const auto& head : headJobs)
	{
		std::cout << jobId;
		std::cout << "\t";
		Node* node = head->sucByStage;
		while (node->stageId != -1)
		{
			std::cout << "(" << node->stageId;
			std::cout << "," << node->machineId;
			std::cout << "," << node->startTime;
			std::cout << "," << node->processTime << ")";
			node = node->sucByStage;
		}
		std::cout << std::endl;
		++jobId;
	}
}

void Solution::findOneCriticalPathMachineFirst(std::vector<Node*>& criticalPath) const
{
	criticalPath.clear();
	int stageCount = problem.getNumOfStates();
	Node* critical_node = *std::max_element(std::begin(tailOnMachinesInEachStage[stageCount - 1]),
			std::end(tailOnMachinesInEachStage[stageCount - 1]),
			[](const Node* l, const Node* r)
			{
				return l->startTime < r->startTime;
			});
	critical_node = critical_node->preByMachine;
	while (critical_node->jobId != -1)
	{
		criticalPath.emplace_back(critical_node);
		if (critical_node->preByMachine->startTime + critical_node->preByMachine->processTime ==
				critical_node->startTime)
		{
			critical_node = critical_node->preByMachine;
		}
		else
		{
			critical_node = critical_node->preByStage;
		}
	}
}

void Solution::findOneCriticalPathJobFirst(std::vector<Node*>& criticalPath) const
{
    criticalPath.clear();
    int stageCount = problem.getNumOfStates();
    Node* critical_node = *std::max_element(std::begin(tailOnMachinesInEachStage[stageCount - 1]),
                                            std::end(tailOnMachinesInEachStage[stageCount - 1]),
                                            [](const Node* l, const Node* r)
                                            {
                                                return l->startTime < r->startTime;
                                            });
    critical_node = critical_node->preByMachine;
    while (critical_node->jobId != -1)
    {
        criticalPath.emplace_back(critical_node);
        if (critical_node->preByStage->stageId != -1 && critical_node->preByStage->startTime + critical_node->preByStage->processTime ==
            critical_node->startTime)
        {
            critical_node = critical_node->preByStage;
        }
        else
        {
            critical_node = critical_node->preByMachine;
        }
    }
}

void Solution::findCutNodes(std::vector<Node*>& cutNodes) const
{
	int stageCount = problem.getNumOfStates();

	Node start(INT32_MIN, INT32_MIN, INT32_MIN, INT32_MIN);
	Node end(INT32_MAX, INT32_MAX, INT32_MAX, INT32_MAX);

	std::unordered_map<Node*, std::list<Node*>> childNodes;

	std::stack<std::tuple<Node*, Node*, int>> nodeStack;
	nodeStack.emplace(&start, nullptr, 0);
	std::unordered_map<Node*, int> dfn, low;
	int timeStamp = 1;

	while (!nodeStack.empty())
	{
		std::tuple<Node*, Node*, int>& curTuple = nodeStack.top();
		Node* curNode = std::get<0>(curTuple);
		Node* parentNode = std::get<1>(curTuple);
		if(dfn[curNode] == 0)
		{
			dfn[curNode] = low[curNode] = timeStamp;
			++timeStamp;
			if (curNode == &start)
			{
				for (Node* node : tailOnMachinesInEachStage[stageCount - 1])
				{
					if (node->startTime == makeSpan)
					{
						childNodes[curNode].emplace_back(node->preByMachine);
					}
				}
			}
			else if (curNode == &end)
			{
				for (Node* node : headOnMachinesInEachStage[0])
				{
					if (node->tailTime == makeSpan)
					{
						childNodes[curNode].emplace_back(node->sucByMachine);
					}
				}
			}
			else
			{
				if (curNode->preByMachine->jobId != -1)
				{
					if (curNode->preByMachine->startTime + curNode->preByMachine->processTime ==
							curNode->startTime)
					{
						childNodes[curNode].emplace_back(curNode->preByMachine);
					};
				}
				if (curNode->preByStage->stageId != -1)
				{
					if (curNode->preByStage->startTime + curNode->preByStage->processTime ==
							curNode->startTime)
					{
						childNodes[curNode].emplace_back(curNode->preByStage);
					}
				}
				if (curNode->sucByMachine->jobId != -1)
				{
					if (curNode->sucByMachine->tailTime + curNode->sucByMachine->processTime ==
							curNode->tailTime)
					{
						childNodes[curNode].emplace_back(curNode->sucByMachine);
					};
				}
				if (curNode->sucByStage->stageId != -1)
				{
					if (curNode->sucByStage->tailTime + curNode->sucByStage->processTime ==
							curNode->tailTime)
					{
						childNodes[curNode].emplace_back(curNode->sucByStage);
					}
				}
				if (curNode->preByMachine->stageId == 0 && curNode->preByMachine->jobId == -1)
				{
					childNodes[curNode].emplace_back(&end);
				}
				if (curNode->sucByMachine->stageId == stageCount - 1 && curNode->sucByMachine->jobId == -1)
				{
					childNodes[curNode].emplace_back(&start);
				}
			}
		}

		bool flag = false;

		auto it = std::begin(childNodes[curNode]);
		while(it != std::end(childNodes[curNode]))
		{
			Node* childNode = *it;
			childNodes[curNode].erase(it++);
			if (childNode == parentNode)
			{
				continue;
			}
			if (dfn[childNode] == 0)
			{
				++std::get<2>(curTuple);
				nodeStack.emplace(childNode, curNode, 0);
				flag = true;
				break;
			}
			else
			{
				low[curNode] = std::min(low[curNode], dfn[childNode]);
			}
		}

		if (!flag)
		{
			std::tuple<Node*, Node*, int> tp = nodeStack.top();
			nodeStack.pop();
			Node* node = std::get<0>(tp);
			Node* pNode = std::get<1>(tp);
			if(nodeStack.empty())
			{
				if(std::get<2>(tp) > 1)
				{
					cutNodes.emplace_back(node);
				}
			}
			else
			{
				if (dfn[pNode] <= low[node] && pNode != &start)
				{
					cutNodes.emplace_back(pNode);
				}
				low[pNode] = std::min(low[std::get<0>(tp)], low[pNode]);
			}
		}
	}
}

void Solution::findAllCriticalNodes(std::vector<Node*>& criticalNodes) const
{
	std::queue<Node*> nodesQueue;

	int stageCount = problem.getNumOfStates();

	for (Node* node : tailOnMachinesInEachStage[stageCount - 1])
	{
		if (node->startTime == makeSpan)
		{
			nodesQueue.push(node);
		}
	}

	while (!nodesQueue.empty())
	{
		Node* node = nodesQueue.front();
		nodesQueue.pop();

		if (node->preByMachine->jobId != -1)
		{
			if (node->preByMachine->startTime + node->preByMachine->processTime ==
					node->startTime)
			{
				criticalNodes.emplace_back(node->preByMachine);
				nodesQueue.push(node->preByMachine);
			};
		}
		if (node->preByStage != nullptr && node->preByStage->stageId != -1)
		{
			if (node->preByStage->startTime + node->preByStage->processTime ==
					node->startTime)
			{
				criticalNodes.emplace_back(node->preByStage);
				nodesQueue.push(node->preByStage);
			}
		}
	}
}

//广度优先
void Solution::refreshStartTimeFromNode(Node* beginNode)
{
	int maxStartTime = INT32_MIN;
	std::queue<Node*> updateNodes;
	updateNodes.push(beginNode);
	while (!updateNodes.empty())
	{
		Node* node = updateNodes.front();
		updateNodes.pop();
		if (node->jobId != -1 && node->stageId != -1)
		{
			node->startTime = std::max(node->preByMachine->startTime + node->preByMachine->processTime,
					node->preByStage->startTime + node->preByStage->processTime);

		}
		else if (node->jobId == -1)
		{
			node->startTime = node->preByMachine->startTime + node->preByMachine->processTime;
		}

		if (node->sucByMachine != nullptr)
		{
			updateNodes.push(node->sucByMachine);
		}

		if (node->sucByStage != nullptr)
		{
			updateNodes.push(node->sucByStage);
		}
	}
}

//广度优先
void Solution::refreshTailTimeFromNode(Node* beginNode)
{
	std::queue<Node*> updateNodes;
	updateNodes.push(beginNode);
	while (!updateNodes.empty())
	{
		Node* node = updateNodes.front();
		updateNodes.pop();
		if (node->jobId != -1 && node->stageId != -1)
		{
			node->tailTime = std::max(node->sucByMachine->tailTime + node->sucByMachine->processTime,
					node->sucByStage->tailTime + node->sucByStage->processTime);

		}
		else if (node->jobId == -1)
		{
			node->tailTime = node->sucByMachine->tailTime + node->sucByMachine->processTime;
		}

		if (node->preByMachine != nullptr)
		{
			updateNodes.push(node->preByMachine);
		}

		if (node->preByStage != nullptr)
		{
			updateNodes.push(node->preByStage);
		}
	}
}

void Solution::refreshStatTailTimeForNode(Node* node)
{
	node->startTime = std::max(node->preByMachine->startTime + node->preByMachine->processTime,
			node->preByStage->startTime + node->preByStage->processTime);
	node->tailTime = std::max(node->sucByMachine->tailTime + node->sucByMachine->processTime,
			node->sucByStage->tailTime + node->sucByStage->processTime);
}

int Solution::deleteNodeWithRefresh(Node* node)
{
	//同一机器断开
	node->preByMachine->sucByMachine = node->sucByMachine;
	node->sucByMachine->preByMachine = node->preByMachine;

	//机器相关
	refreshStartTimeFromNode(node->sucByMachine);
	refreshTailTimeFromNode(node->preByMachine);

	//工件相关
	int oldProcessTime = node->processTime;
	node->processTime = 0;

	node->startTime = node->preByStage->startTime + node->preByStage->processTime;
	refreshStartTimeFromNode(node->sucByStage);
	node->tailTime = node->sucByStage->tailTime + node->sucByStage->processTime;
	refreshTailTimeFromNode(node->preByStage);

	//恢复删除节点的处理时间
	node->processTime = oldProcessTime;

	--jobCountOnMachinesInEachStage[node->stageId][node->machineId];
	//计算移除后makeSpan，关键路径的第一个和最后一个工序移除不会变好，只需要遍历最后一个阶段。
	int lastStageId = problem.getNumOfStates() - 1;
	return (*std::max_element(std::begin(tailOnMachinesInEachStage[lastStageId]),
			std::end(tailOnMachinesInEachStage[lastStageId]), [](const Node* l, const Node* r)
			{
				return l->startTime < r->startTime;
			}))->startTime;
}

void Solution::deleteNodeWithoutRefresh(Node* node)
{
	//同一机器断开
	node->preByMachine->sucByMachine = node->sucByMachine;
	node->sucByMachine->preByMachine = node->preByMachine;

	node->startTime = node->preByStage->startTime + node->preByStage->processTime;
	node->tailTime = node->sucByStage->tailTime + node->sucByStage->processTime;

	--jobCountOnMachinesInEachStage[node->stageId][node->machineId];
}

void Solution::moveNode(Node* deletedNode, Node* afterNode)
{
	deletedNode->sucByMachine = afterNode->sucByMachine;
	deletedNode->preByMachine = afterNode;
	deletedNode->sucByMachine->preByMachine = deletedNode;
	afterNode->sucByMachine = deletedNode;

	deletedNode->machineId = afterNode->machineId;

	++jobCountOnMachinesInEachStage[afterNode->stageId][afterNode->machineId];
}

Solution::~Solution()
{
	for (const auto& headOnMachines : headOnMachinesInEachStage)
	{
		for (const auto& head : headOnMachines)
		{
			Node* preNode = head;
			Node* node = head->sucByMachine;
			delete head;
			while (node != nullptr)
			{
				preNode = node;
				node = node->sucByMachine;
				delete preNode;
			}
		}
	}

	std::for_each(std::begin(headJobs), std::end(headJobs), [](const Node* ptr)
	{
		delete ptr;
	});
	std::for_each(std::begin(tailJobs), std::end(tailJobs), [](const Node* ptr)
	{
		delete ptr;
	});
}







