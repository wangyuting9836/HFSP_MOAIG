//
// Created by wangy on 2023/4/24.
//
#include <numeric>
#include <iostream>
#include <stack>
#include <queue>
#include <fstream>
#include <string>
#include <sstream>
#include "Solution.h"
#include "TemporyStorage.h"

Solution::Solution(const HFS_Problem& problem, Solution_Init_Type init_type)
	: problem(problem)
{
	switch (init_type)
	{
	case RandomInit:
		jobSequence.resize(problem.getNumOfJobs());
		std::iota(std::begin(jobSequence), std::end(jobSequence), 0);
		std::shuffle(std::begin(jobSequence), std::end(jobSequence), rand_generator());
		break;
	case Empty:
		break;
	}
	initBaseGraph();
}

Solution::Solution(const HFS_Problem& problem, const std::vector<int>& sequence)
	: problem(problem), jobSequence(sequence)
{
	initBaseGraph();
}

Solution::Solution(const HFS_Problem& problem, const std::string& file_name)
	: problem(problem)
{
	initBaseGraph();

	int num_of_stages = problem.getNumOfStages();
	int num_of_jobs = problem.getNumOfJobs();

	std::ifstream fin(file_name);
	for (int s = 0; s < num_of_stages; ++s)
	{
		int machineCount = problem.getNumOfMachinesInStage(s);
		for (int m = 0; m < machineCount; ++m)
		{
			f_head_on_machines_in_each_stage[s][m]->f_startTime = 0;
			b_head_on_machines_in_each_stage[s][m]->b_startTime = 0;
		}
	}

	int v;
	for (int s = 0; s < num_of_stages; ++s)
	{
		fin >> v;
		int machineCount = problem.getNumOfMachinesInStage(s);
		for (int m = 0; m < machineCount; ++m)
		{
			fin >> v;
			std::string str;
			fin.get();
			std::getline(fin, str);
			std::istringstream ss(str);
			Node* pre_node_at_machine = f_head_on_machines_in_each_stage[s][m];
			while(ss >> v)
			{
				int job_id = v;
				Node* operation = operations[s][job_id];

				pre_node_at_machine->sucByMachine = operation;
				operation->sucByMachine = b_head_on_machines_in_each_stage[s][m];

				b_head_on_machines_in_each_stage[s][m]->preByMachine = operation;
				operation->preByMachine = pre_node_at_machine;

				pre_node_at_machine = operation;
			}

		}
	}
	fin.close();

}

Solution::Solution(const Solution& solution)
	: problem(solution.problem), jobSequence(solution.jobSequence)//, makeSpan(solution.makeSpan)
{
	initBaseGraph();
	int num_of_stages = problem.getNumOfStages();
	int num_of_jobs = problem.getNumOfJobs();
	for (int s = 0; s < num_of_stages; ++s)
	{
		int machineCount = problem.getNumOfMachinesInStage(s);
		for (int m = 0; m < machineCount; ++m)
		{
			f_head_on_machines_in_each_stage[s][m]->f_startTime = solution.f_head_on_machines_in_each_stage[s][m]->f_startTime;
			f_head_on_machines_in_each_stage[s][m]->b_startTime = solution.f_head_on_machines_in_each_stage[s][m]->b_startTime;
			b_head_on_machines_in_each_stage[s][m]->f_startTime = solution.b_head_on_machines_in_each_stage[s][m]->f_startTime;
			b_head_on_machines_in_each_stage[s][m]->b_startTime = solution.b_head_on_machines_in_each_stage[s][m]->b_startTime;
		}
	}

	for (int s = 0; s < num_of_stages; ++s)
	{
		int machineCount = problem.getNumOfMachinesInStage(s);
		for (int m = 0; m < machineCount; ++m)
		{
			Node* f_head_node_at_machine = solution.f_head_on_machines_in_each_stage[s][m];
			Node* node_at_machine = f_head_node_at_machine->sucByMachine;

			Node* pre_node_at_machine = f_head_on_machines_in_each_stage[s][m];
			while (node_at_machine->jobId != -1)
			{
				int job_id = node_at_machine->jobId;
				Node* operation = operations[s][job_id];
				operation->f_startTime = node_at_machine->f_startTime;
				operation->b_startTime = node_at_machine->b_startTime;
				operation->machineId = node_at_machine->machineId;

				pre_node_at_machine->sucByMachine = operation;
				operation->sucByMachine = b_head_on_machines_in_each_stage[s][m];

				b_head_on_machines_in_each_stage[s][m]->preByMachine = operation;
				operation->preByMachine = pre_node_at_machine;

				pre_node_at_machine = operation;
				node_at_machine = node_at_machine->sucByMachine;
			}
		}
	}
}

Solution& Solution::operator=(const Solution& solution)
{
	if (this == &solution)
	{
		return *this;
	}

	jobSequence = solution.jobSequence;

	int num_of_stages = problem.getNumOfStages();
	int num_of_jobs = problem.getNumOfJobs();

	for (int s = 0; s < num_of_stages; ++s)
	{
		int machineCount = problem.getNumOfMachinesInStage(s);
		for (int m = 0; m < machineCount; ++m)
		{
			f_head_on_machines_in_each_stage[s][m]->f_startTime = solution.f_head_on_machines_in_each_stage[s][m]->f_startTime;
			f_head_on_machines_in_each_stage[s][m]->b_startTime = solution.f_head_on_machines_in_each_stage[s][m]->b_startTime;
			b_head_on_machines_in_each_stage[s][m]->f_startTime = solution.b_head_on_machines_in_each_stage[s][m]->f_startTime;
			b_head_on_machines_in_each_stage[s][m]->b_startTime = solution.b_head_on_machines_in_each_stage[s][m]->b_startTime;
		}
	}

	for (int s = 0; s < num_of_stages; ++s)
	{
		int machineCount = problem.getNumOfMachinesInStage(s);
		for (int m = 0; m < machineCount; ++m)
		{
			Node* f_head_node_at_machine = solution.f_head_on_machines_in_each_stage[s][m];
			Node* node_at_machine = f_head_node_at_machine->sucByMachine;
			if (node_at_machine->jobId == -1)
			{
				f_head_on_machines_in_each_stage[s][m]->sucByMachine = b_head_on_machines_in_each_stage[s][m];
				b_head_on_machines_in_each_stage[s][m]->preByMachine = f_head_on_machines_in_each_stage[s][m];
				continue;
			}
			Node* pre_node_at_machine = f_head_on_machines_in_each_stage[s][m];
			while (node_at_machine->jobId != -1)
			{
				int job_id = node_at_machine->jobId;
				Node* operation = operations[s][job_id];
				operation->f_startTime = node_at_machine->f_startTime;
				operation->b_startTime = node_at_machine->b_startTime;
				operation->machineId = node_at_machine->machineId;

				pre_node_at_machine->sucByMachine = operation;
				operation->sucByMachine = b_head_on_machines_in_each_stage[s][m];

				b_head_on_machines_in_each_stage[s][m]->preByMachine = operation;
				operation->preByMachine = pre_node_at_machine;

				pre_node_at_machine = operation;
				node_at_machine = node_at_machine->sucByMachine;
			}
		}
	}

	return *this;
}

Solution::~Solution()
{
	//deleteGraph();

	for (const auto& operations_st_stage : operations)
	{
		for (auto op : operations_st_stage)
		{
			delete op;
		}
	}

	int num_of_stages = problem.getNumOfStages();
	for (int s = 0; s < num_of_stages; ++s)
	{
		int machineCount = problem.getNumOfMachinesInStage(s);
		for (int m = 0; m < machineCount; ++m)
		{
			delete f_head_on_machines_in_each_stage[s][m];
			delete b_head_on_machines_in_each_stage[s][m];
		}
	}

	std::for_each(std::begin(f_headJobs), std::end(f_headJobs), [](const Node* ptr)
	{
		delete ptr;
	});
	std::for_each(std::begin(b_headJobs), std::end(b_headJobs), [](const Node* ptr)
	{
		delete ptr;
	});
}

void Solution::initBaseGraph()
{
	int num_of_jobs = problem.getNumOfJobs();
	int num_of_stages = problem.getNumOfStages();
	const std::vector<Job>& jobs = problem.getJobs();

	operations.resize(num_of_stages);
	for (int s = 0; s < num_of_stages; ++s)
	{
		for (const auto& job : jobs)
		{
			operations[s].emplace_back(new Node(job.getId(), s, -1, job.getProcessTime(s)));
		}
	}

	for (int jobId = 0; jobId < num_of_jobs; ++jobId)
	{
		f_headJobs.emplace_back(new Node(jobId, -1, -1, 0));
		b_headJobs.emplace_back(new Node(jobId, -1, -1, 0));
		f_headJobs[jobId]->sucByStage = b_headJobs[jobId];
		b_headJobs[jobId]->preByStage = f_headJobs[jobId];
	}

	for (int s = 0; s < num_of_stages; ++s)
	{
		for (int jobId = 0; jobId < num_of_jobs; ++jobId)
		{
			operations[s][jobId]->sucByStage = b_headJobs[jobId];
			operations[s][jobId]->preByStage = b_headJobs[jobId]->preByStage;
			b_headJobs[jobId]->preByStage->sucByStage = operations[s][jobId];
			b_headJobs[jobId]->preByStage = operations[s][jobId];
		}
	}

	f_head_on_machines_in_each_stage.resize(num_of_stages);
	b_head_on_machines_in_each_stage.resize(num_of_stages);
	for (int s = 0; s < num_of_stages; ++s)
	{
		int machineCount = problem.getNumOfMachinesInStage(s);
		for (int m = 0; m < machineCount; ++m)
		{
			f_head_on_machines_in_each_stage[s].emplace_back(new Node(-1, s, m, 0));
			b_head_on_machines_in_each_stage[s].emplace_back(new Node(-1, s, m, 0));

			f_head_on_machines_in_each_stage[s][m]->sucByMachine = b_head_on_machines_in_each_stage[s][m];
			b_head_on_machines_in_each_stage[s][m]->preByMachine = f_head_on_machines_in_each_stage[s][m];
		}
	}
}

void Solution::reset_to_base_graph()
{
	int num_of_jobs = problem.getNumOfJobs();
	int num_of_stages = problem.getNumOfStages();

	/*for (int jobId = 0; jobId < num_of_jobs; ++jobId)
	{
		f_headJobs[jobId]->sucByStage = b_headJobs[jobId];
		b_headJobs[jobId]->preByStage = f_headJobs[jobId];
	}*/

	for (int s = 0; s < num_of_stages; ++s)
	{
		int machineCount = problem.getNumOfMachinesInStage(s);
		for (int m = 0; m < machineCount; ++m)
		{
			f_head_on_machines_in_each_stage[s][m]->sucByMachine = b_head_on_machines_in_each_stage[s][m];
			b_head_on_machines_in_each_stage[s][m]->preByMachine = f_head_on_machines_in_each_stage[s][m];
		}
	}
}

int Solution::decode_forward()
{
	int num_of_stages = problem.getNumOfStages();

	TS::zero_storage();

	//first stage
	for (auto job_id : jobSequence)
	{
		int mt = min_element(TS::m_idle_time[0].begin(), TS::m_idle_time[0].end()) - TS::m_idle_time[0].begin();
		TS::c_time[job_id][0] = TS::m_idle_time[0][mt] + operations[0][job_id]->processTime;
		TS::m_idle_time[0][mt] = TS::c_time[job_id][0];
	}

	TS::sequence_of_other_stage = jobSequence;
	//other stage. FIFO and FAM
	for (int s = 1; s < num_of_stages; ++s)
	{
		std::sort(TS::sequence_of_other_stage.begin(), TS::sequence_of_other_stage.end(), [&](int job1, int job2)
		{
			return TS::c_time[job1][s - 1] < TS::c_time[job2][s - 1];
		});
		for (int first_come_job_id : TS::sequence_of_other_stage)
		{
			int mt = min_element(TS::m_idle_time[s].begin(), TS::m_idle_time[s].end()) - TS::m_idle_time[s].begin();
			TS::c_time[first_come_job_id][s] =
				std::max(TS::m_idle_time[s][mt], TS::c_time[first_come_job_id][s - 1])
				+ operations[s][first_come_job_id]->processTime;
			TS::m_idle_time[s][mt] = TS::c_time[first_come_job_id][s];
		}
	}
	return *std::max_element(TS::m_idle_time[num_of_stages - 1].begin(), TS::m_idle_time[num_of_stages - 1].end());
}

int Solution::decode_forward_to_graph()
{
	reset_to_base_graph();
	int num_of_stages = problem.getNumOfStages();

	auto assign_job_fun = [&](Node* cur_operation_node, int job_id, int stage_id, int machine_id)
	{
		cur_operation_node->machineId = machine_id;

		cur_operation_node->preByMachine = b_head_on_machines_in_each_stage[stage_id][machine_id]->preByMachine;
		cur_operation_node->sucByMachine = b_head_on_machines_in_each_stage[stage_id][machine_id];

		b_head_on_machines_in_each_stage[stage_id][machine_id]->preByMachine->sucByMachine = cur_operation_node;
		b_head_on_machines_in_each_stage[stage_id][machine_id]->preByMachine = cur_operation_node;

		b_head_on_machines_in_each_stage[stage_id][machine_id]->f_startTime =
			cur_operation_node->f_startTime + cur_operation_node->processTime;
	};

	TS::zero_storage();
	//first stage
	for (auto job_id : jobSequence)
	{
		Node* cur_operation_node = operations[0][job_id];

		int mt = min_element(TS::m_idle_time[0].begin(), TS::m_idle_time[0].end()) - TS::m_idle_time[0].begin();
		cur_operation_node->f_startTime = TS::m_idle_time[0][mt];
		TS::c_time[job_id][0] = cur_operation_node->f_startTime + cur_operation_node->processTime;
		TS::m_idle_time[0][mt] = TS::c_time[job_id][0];

		assign_job_fun(cur_operation_node, job_id, 0, mt);
	}

	TS::sequence_of_other_stage = jobSequence;
	//other stage. FIFO and FAM
	for (int s = 1; s < num_of_stages; ++s)
	{
		std::sort(TS::sequence_of_other_stage.begin(), TS::sequence_of_other_stage.end(), [&](int job1, int job2)
		{
			return TS::c_time[job1][s - 1] < TS::c_time[job2][s - 1];
		});
		for (int first_come_job_id : TS::sequence_of_other_stage)
		{
			Node* cur_operation_node = operations[s][first_come_job_id];

			int mt = min_element(TS::m_idle_time[s].begin(), TS::m_idle_time[s].end()) - TS::m_idle_time[s].begin();
			cur_operation_node->f_startTime = std::max(TS::m_idle_time[s][mt], TS::c_time[first_come_job_id][s - 1]);

			TS::c_time[first_come_job_id][s] = cur_operation_node->f_startTime + cur_operation_node->processTime;
			TS::m_idle_time[s][mt] = TS::c_time[first_come_job_id][s];

			assign_job_fun(cur_operation_node, first_come_job_id, s, mt);
		}
	}

	//计算逆向时间
	for (auto sit = std::rbegin(b_head_on_machines_in_each_stage); sit != std::rend(b_head_on_machines_in_each_stage); ++sit)
	{
		const std::vector<Node*>& tailOnMachines = *sit;
		for (auto mit = std::rbegin(tailOnMachines); mit != std::rend(tailOnMachines); ++mit)
		{
			Node* node = (*mit)->preByMachine;
			while (node->jobId != -1)
			{
				node->b_startTime = std::max(node->sucByMachine->b_startTime + node->sucByMachine->processTime,
					node->sucByStage->b_startTime + node->sucByStage->processTime);
				node = node->preByMachine;
			}
			node->b_startTime = node->sucByMachine->b_startTime + node->sucByMachine->processTime;
		}
	}

	return *std::max_element(TS::m_idle_time[num_of_stages - 1].begin(), TS::m_idle_time[num_of_stages - 1].end());
}

int Solution::refresh_all_graph()
{
	//Recalculate forward complete time
	for (const auto& headOnMachines : f_head_on_machines_in_each_stage)
	{
		for (auto headOnMachine : headOnMachines)
		{
			Node* node = headOnMachine->sucByMachine;
			while (node->jobId != -1)
			{
				node->f_startTime = std::max(node->preByMachine->f_startTime + node->preByMachine->processTime,
					node->preByStage->f_startTime + node->preByStage->processTime);

				//node->is_visit = false;
				//node->dfn = 0;
				//node->low = 0;
				//node->critical_adjacent_count = 0;

				node = node->sucByMachine;
			}
			node->f_startTime = node->preByMachine->f_startTime + node->preByMachine->processTime;
		}
	}

	//Recalculate backward complete time
	for (auto sit = std::rbegin(b_head_on_machines_in_each_stage); sit != std::rend(b_head_on_machines_in_each_stage); ++sit)
	{
		const std::vector<Node*>& tailOnMachines = *sit;
		for (auto mit = std::rbegin(tailOnMachines); mit != std::rend(tailOnMachines); ++mit)
		{
			Node* node = (*mit)->preByMachine;
			while (node->jobId != -1)
			{
				node->b_startTime = std::max(node->sucByMachine->b_startTime + node->sucByMachine->processTime,
					node->sucByStage->b_startTime + node->sucByStage->processTime);
				node = node->preByMachine;
			}
			node->b_startTime = node->sucByMachine->b_startTime + node->sucByMachine->processTime;
		}
	}

	int lastStageId = problem.getNumOfStages() - 1;
	int span = (*std::max_element(std::begin(b_head_on_machines_in_each_stage[lastStageId]),
		std::end(b_head_on_machines_in_each_stage[lastStageId]), [](const Node* n1, const Node* n2)
		{
			return n1->f_startTime < n2->f_startTime;
		}))->f_startTime;
	return span;
}

int Solution::refresh_forward_graph()
{
	//Recalculate forward complete time
	for (const auto& headOnMachines : f_head_on_machines_in_each_stage)
	{
		for (auto headOnMachine : headOnMachines)
		{
			Node* node = headOnMachine->sucByMachine;
			while (node->jobId != -1)
			{
				node->f_startTime = std::max(node->preByMachine->f_startTime + node->preByMachine->processTime,
					node->preByStage->f_startTime + node->preByStage->processTime);
				node = node->sucByMachine;
			}
			node->f_startTime = node->preByMachine->f_startTime + node->preByMachine->processTime;
		}
	}

	int lastStageId = problem.getNumOfStages() - 1;
	int span = (*std::max_element(std::begin(b_head_on_machines_in_each_stage[lastStageId]),
		std::end(b_head_on_machines_in_each_stage[lastStageId]), [](const Node* n1, const Node* n2)
		{
			return n1->f_startTime < n2->f_startTime;
		}))->f_startTime;
	return span;
}

void Solution::find_mandatory_nodes(std::vector<Node*>& mandatory_nodes, int span) const
{
	int num_of_stages = problem.getNumOfStages();

	std::vector<Node*> critical_nodes;
	std::queue<Node*> nodesQueue;

	Node critical_graph_start_node(INT32_MIN, INT32_MIN, INT32_MIN, INT32_MIN);
	Node critical_graph_end_node(INT32_MAX, INT32_MAX, INT32_MAX, INT32_MAX);

	for (Node* b_head : b_head_on_machines_in_each_stage[num_of_stages - 1])
	{
		Node* node = b_head->preByMachine;
		if (node->jobId != -1 && node->f_startTime + node->processTime == span)
		{
			critical_graph_start_node.critical_adjacent[critical_graph_start_node.critical_adjacent_count] = node;
			++critical_graph_start_node.critical_adjacent_count;
			node->critical_adjacent[node->critical_adjacent_count] = &critical_graph_start_node;
			++node->critical_adjacent_count;
			node->is_visit = true;
			critical_nodes.emplace_back(node);
			nodesQueue.push(node);
		}
	}

	while (!nodesQueue.empty())
	{
		Node* node = nodesQueue.front();
		nodesQueue.pop();
		Node* pre_node_on_machine = node->preByMachine;
		if (pre_node_on_machine->jobId != -1
			&& pre_node_on_machine->f_startTime + pre_node_on_machine->processTime == node->f_startTime)
		{
			node->critical_adjacent[node->critical_adjacent_count] = pre_node_on_machine;
			++node->critical_adjacent_count;
			pre_node_on_machine->critical_adjacent[pre_node_on_machine->critical_adjacent_count] = node;
			++pre_node_on_machine->critical_adjacent_count;
			if (!pre_node_on_machine->is_visit)
			{
				pre_node_on_machine->is_visit = true;
				critical_nodes.emplace_back(pre_node_on_machine);
				nodesQueue.push(pre_node_on_machine);
			}
		}

		Node* pre_node_at_stage = node->preByStage;
		if (pre_node_at_stage->stageId != -1
			&& pre_node_at_stage->f_startTime + pre_node_at_stage->processTime == node->f_startTime)
		{
			node->critical_adjacent[node->critical_adjacent_count] = pre_node_at_stage;
			++node->critical_adjacent_count;
			pre_node_at_stage->critical_adjacent[pre_node_at_stage->critical_adjacent_count] = node;
			++pre_node_at_stage->critical_adjacent_count;
			if (!pre_node_at_stage->is_visit)
			{
				pre_node_at_stage->is_visit = true;
				critical_nodes.emplace_back(pre_node_at_stage);
				nodesQueue.push(pre_node_at_stage);
			}
		}
	}

	for (Node* f_head : f_head_on_machines_in_each_stage[0])
	{
		Node* node = f_head->sucByMachine;
		if (node->jobId != -1 && node->b_startTime + node->processTime == span)
		{
			critical_graph_end_node.critical_adjacent[critical_graph_end_node.critical_adjacent_count] = node;
			++critical_graph_end_node.critical_adjacent_count;
			node->critical_adjacent[node->critical_adjacent_count] = &critical_graph_end_node;
			++node->critical_adjacent_count;
		}
	}

	std::stack<std::pair<Node*, Node*>> nodeStack;
	nodeStack.emplace(&critical_graph_start_node, nullptr);
	int timeStamp = 1;

	while (!nodeStack.empty())
	{
		std::pair<Node*, Node*>& cur_pair = nodeStack.top();
		Node* curNode = cur_pair.first;
		Node* parentNode = cur_pair.second;
		if (curNode->dfn == 0)
		{
			curNode->dfn = curNode->low = timeStamp;
			++timeStamp;
		}

		bool flag = false;

		while (true)
		{
			if (curNode->critical_adjacent_count == 0)
			{
				break;
			}
			Node* childNode = curNode->critical_adjacent[curNode->critical_adjacent_count - 1];;
			--curNode->critical_adjacent_count;
			if (childNode == parentNode)
			{
				continue;
			}
			if (childNode->dfn == 0)
			{
				nodeStack.emplace(childNode, curNode);
				flag = true;
				break;
			}
			else
			{
				curNode->low = std::min(curNode->low, childNode->dfn);
			}
		}

		if (!flag)
		{
			std::pair<Node*, Node*> tp = nodeStack.top();
			nodeStack.pop();
			Node* node = std::get<0>(tp);
			Node* pNode = std::get<1>(tp);
			if (!nodeStack.empty())
			{
				if (pNode->dfn <= node->low && pNode != &critical_graph_start_node)
				{
					mandatory_nodes.emplace_back(pNode);
				}
				pNode->low = std::min(node->low, pNode->low);
			}
		}
	}

	for (auto node_ptr : critical_nodes)
	{
		node_ptr->is_visit = false;
		node_ptr->dfn = 0;
		node_ptr->low = 0;
		node_ptr->critical_adjacent_count = 0;
	}
}

void Solution::find_all_critical_nodes_forward(std::vector<Node*>& critical_nodes, int span) const
{
	std::queue<Node*> nodesQueue;

	for (Node* f_head : f_head_on_machines_in_each_stage[0])
	{
		Node* node = f_head->sucByMachine;
		if (node->jobId != -1 && node->b_startTime + node->processTime == span)
		{
			node->is_visit = true;
			critical_nodes.emplace_back(node);
			nodesQueue.push(node);
		}
	}

	while (!nodesQueue.empty())
	{
		Node* node = nodesQueue.front();
		nodesQueue.pop();
		Node* suc_node_on_machine = node->sucByMachine;
		if (suc_node_on_machine->jobId != -1 && !suc_node_on_machine->is_visit
			&& suc_node_on_machine->b_startTime + suc_node_on_machine->processTime == node->b_startTime)
		{

			suc_node_on_machine->is_visit = true;
			critical_nodes.emplace_back(suc_node_on_machine);
			nodesQueue.push(suc_node_on_machine);
		}

		Node* suc_node_at_stage = node->sucByStage;
		if (suc_node_at_stage->stageId != -1 && !suc_node_at_stage->is_visit
			&& suc_node_at_stage->b_startTime + suc_node_at_stage->processTime == node->b_startTime)
		{
			suc_node_at_stage->is_visit = true;
			critical_nodes.emplace_back(suc_node_at_stage);
			nodesQueue.push(suc_node_at_stage);
		}
	}

	for (auto node_ptr : critical_nodes)
	{
		node_ptr->is_visit = false;
	}
}

void Solution::delete_node_physical_from_graph(Node* deleted_node)
{
	//Disconnection on the same machine
	deleted_node->preByMachine->sucByMachine = deleted_node->sucByMachine;
	deleted_node->sucByMachine->preByMachine = deleted_node->preByMachine;

	//Disconnection on front and back stages
	deleted_node->preByStage->sucByStage = deleted_node->sucByStage;
	deleted_node->sucByStage->preByStage = deleted_node->preByStage;
}

void Solution::insert_node_at_position(Node* deleted_node, Node* node_before_insert_pos, Node* node_in_pre_stage)
{
	node_before_insert_pos->sucByMachine->preByMachine = deleted_node;
	deleted_node->sucByMachine = node_before_insert_pos->sucByMachine;

	node_before_insert_pos->sucByMachine = deleted_node;
	deleted_node->preByMachine = node_before_insert_pos;

	node_in_pre_stage->sucByStage->preByStage = deleted_node;
	deleted_node->sucByStage = node_in_pre_stage->sucByStage;

	node_in_pre_stage->sucByStage = deleted_node;
	deleted_node->preByStage = node_in_pre_stage;

	deleted_node->machineId = node_before_insert_pos->machineId;
}

//The algorithm MOAIG in the paper
int Solution::decode_local_search_mandatory_nodes(int original_makeSpan)
{
	std::vector<Node*> mandatory_nodes;

	while (true)
	{
		mandatory_nodes.clear();
		find_mandatory_nodes(mandatory_nodes, original_makeSpan);
		int new_span = local_search_mandatory_nodes(mandatory_nodes, original_makeSpan);
		if (new_span < original_makeSpan)
		{
			original_makeSpan = new_span;
		}
		else
		{
			break;
		}
	}
	return original_makeSpan;
}

int Solution::local_search_mandatory_nodes(const std::vector<Node*>& mandatory_nodes, int original_makeSpan)
{
	int stage_count = problem.getNumOfStages();
	for (auto deleteNode : mandatory_nodes)
	{
		Node* originalPreNodeOnMachine = deleteNode->preByMachine;

		int current_stage_id = deleteNode->stageId;
		delete_node_physical_from_graph(deleteNode);

		int f_start_time_of_pre_stage =
			deleteNode->preByStage->f_startTime + deleteNode->preByStage->processTime;

		int b_start_time_of_suc_stage =
			deleteNode->sucByStage->b_startTime + deleteNode->sucByStage->processTime;

		const auto& heads = f_head_on_machines_in_each_stage[current_stage_id];
		const auto& tails = b_head_on_machines_in_each_stage[current_stage_id];
		int machineCount = problem.getNumOfMachinesInStage(current_stage_id);
		int bestPathLengthThroughDelNode = original_makeSpan;
		Node* nodeBeforeBestInsertPos;
		for (int mid = 0; mid < machineCount; ++mid)
		{
			if (mid == deleteNode->machineId)//Try the original processing machine
			{
				int a_point_offset = INT16_MIN;
				Node* nodeBeforeAPoint = heads[mid];
				int f_start_time_of_node_before_A = INT32_MIN;
				if (current_stage_id == 0)
				{
					if (deleteNode->preByMachine == heads[mid])
					{
						a_point_offset = 0;
					}
				}
				else
				{
					a_point_offset = 0;
					nodeBeforeAPoint = deleteNode->preByMachine;
					if (nodeBeforeAPoint->f_startTime + nodeBeforeAPoint->processTime <= f_start_time_of_pre_stage)
					{
						//Find position A to the right from the deleted position
						f_start_time_of_node_before_A = nodeBeforeAPoint->f_startTime;
						Node* c_node = nodeBeforeAPoint->sucByMachine;
						int f_star_time_c_node;
						while (c_node->jobId != -1)
						{
							f_star_time_c_node = std::max(
								f_start_time_of_node_before_A + nodeBeforeAPoint->processTime,
								c_node->preByStage->f_startTime + c_node->preByStage->processTime);
							if (c_node->f_startTime + c_node->processTime > f_start_time_of_pre_stage)
							{
								break;
							}
							nodeBeforeAPoint = c_node;
							f_start_time_of_node_before_A = f_star_time_c_node;
							c_node = nodeBeforeAPoint->sucByMachine;
							++a_point_offset;
						}
					}
					else
					{
						//Find position A to the left from the deleted position
						--a_point_offset;
						nodeBeforeAPoint = nodeBeforeAPoint->preByMachine;
						while (nodeBeforeAPoint->jobId != -1)
						{
							if (nodeBeforeAPoint->f_startTime + nodeBeforeAPoint->processTime
								<= f_start_time_of_pre_stage)
							{
								break;
							}
							nodeBeforeAPoint = nodeBeforeAPoint->preByMachine;
							--a_point_offset;
						}
					}
				}

				int b_point_offset = INT16_MIN;
				Node* nodeAfterBPoint = tails[mid];
				int b_start_time_of_node_after_B = INT32_MIN;
				if (current_stage_id == stage_count - 1)
				{
					if (deleteNode->sucByMachine == tails[mid])
					{
						b_point_offset = 0;
					}
				}
				else
				{
					b_point_offset = 0;
					nodeAfterBPoint = deleteNode->sucByMachine;
					//b_start_time_of_node_after_B = INT32_MIN;
					if (nodeAfterBPoint->b_startTime + nodeAfterBPoint->processTime
						<= b_start_time_of_suc_stage)
					{
						//Find position B to the left from the deleted position
						b_start_time_of_node_after_B = nodeAfterBPoint->b_startTime;
						Node* c_node = nodeAfterBPoint->preByMachine;
						int b_star_time_c_node;
						while (c_node->jobId != -1)
						{
							b_star_time_c_node = std::max(
								b_start_time_of_node_after_B + nodeAfterBPoint->processTime,
								c_node->sucByStage->b_startTime + c_node->sucByStage->processTime);
							if (c_node->b_startTime + c_node->processTime > b_start_time_of_suc_stage)
							{
								break;
							}
							nodeAfterBPoint = c_node;
							b_start_time_of_node_after_B = b_star_time_c_node;
							c_node = nodeAfterBPoint->preByMachine;
							++b_point_offset;
						}
					}
					else
					{
						//Find position B to the right from the deleted position
						--b_point_offset;
						nodeAfterBPoint = nodeAfterBPoint->sucByMachine;
						while (nodeAfterBPoint->jobId != -1)
						{
							if (nodeAfterBPoint->b_startTime + nodeAfterBPoint->processTime
								<= b_start_time_of_suc_stage)
							{
								break;
							}
							nodeAfterBPoint = nodeAfterBPoint->sucByMachine;
							--b_point_offset;
						}
					}
				}

				if (a_point_offset >= 0 && b_point_offset >= 0)
				{
					//B is less than or equal to A. Delete positions in B and A.
					continue;
				}
				else if (a_point_offset < 0 && b_point_offset < 0)
				{
					//A is greater than B. Position A is before the deletion position and position B is after the deletion position.
					int longestValue;
					Node* nodeBeforeInsertPos;
					Node* nodeAfterInsertPos;

					nodeBeforeInsertPos = deleteNode->sucByMachine;
					nodeAfterInsertPos = nodeBeforeInsertPos->sucByMachine;
					int f_start_time = nodeBeforeInsertPos->preByMachine->f_startTime;
					while (nodeBeforeInsertPos != nodeAfterBPoint)
					{
						f_start_time =
							std::max(f_start_time
									 + nodeBeforeInsertPos->preByMachine->processTime,
								nodeBeforeInsertPos->preByStage->f_startTime
								+ nodeBeforeInsertPos->preByStage->processTime);
						longestValue =
							std::max(f_start_time_of_pre_stage,
								f_start_time + nodeBeforeInsertPos->processTime)
							+ deleteNode->processTime +
							std::max(b_start_time_of_suc_stage,
								nodeAfterInsertPos->b_startTime + nodeAfterInsertPos->processTime);
						if (longestValue < bestPathLengthThroughDelNode)
						{
							bestPathLengthThroughDelNode = longestValue;
							nodeBeforeBestInsertPos = nodeBeforeInsertPos;
						}
						nodeBeforeInsertPos = nodeAfterInsertPos;
						nodeAfterInsertPos = nodeBeforeInsertPos->sucByMachine;
					}

					nodeAfterInsertPos = deleteNode->preByMachine;
					nodeBeforeInsertPos = nodeAfterInsertPos->preByMachine;
					int b_start_time = nodeAfterInsertPos->sucByMachine->b_startTime;
					while (nodeAfterInsertPos != nodeBeforeAPoint)
					{
						b_start_time =
							std::max(b_start_time
									 + nodeAfterInsertPos->sucByMachine->processTime,
								nodeAfterInsertPos->sucByStage->b_startTime
								+ nodeAfterInsertPos->sucByStage->processTime);

						longestValue =
							std::max(f_start_time_of_pre_stage,
								nodeBeforeInsertPos->f_startTime + nodeBeforeInsertPos->processTime)
							+ deleteNode->processTime +
							std::max(b_start_time_of_suc_stage,
								b_start_time + nodeAfterInsertPos->processTime);
						if (longestValue < bestPathLengthThroughDelNode)
						{
							bestPathLengthThroughDelNode = longestValue;
							nodeBeforeBestInsertPos = nodeBeforeInsertPos;
						}
						nodeAfterInsertPos = nodeBeforeInsertPos;
						nodeBeforeInsertPos = nodeAfterInsertPos->preByMachine;
					}
				}
				else if (a_point_offset < 0) //a_point_offset < 0 && b_point_offset >= 0
				{
					//Position A before the deletion position and position B before the deletion position (point B may be the deletion position).
					if (-a_point_offset > b_point_offset)
					{
						//A is greater than B.
						int longestValue;
						Node* nodeAfterInsertPos = nodeAfterBPoint;
						Node* nodeBeforeInsertPos = nodeAfterInsertPos->preByMachine;
						int b_start_time = b_start_time_of_node_after_B;
						while (true)
						{
							longestValue =
								std::max(f_start_time_of_pre_stage,
									nodeBeforeInsertPos->f_startTime + nodeBeforeInsertPos->processTime)
								+ deleteNode->processTime +
								std::max(b_start_time_of_suc_stage,
									b_start_time + nodeAfterInsertPos->processTime);
							if (longestValue < bestPathLengthThroughDelNode)
							{
								bestPathLengthThroughDelNode = longestValue;
								nodeBeforeBestInsertPos = nodeBeforeInsertPos;
							}
							if (nodeBeforeInsertPos == nodeBeforeAPoint)
							{
								break;
							}
							nodeAfterInsertPos = nodeBeforeInsertPos;
							nodeBeforeInsertPos = nodeAfterInsertPos->preByMachine;
							b_start_time =
								std::max(b_start_time
										 + nodeAfterInsertPos->sucByMachine->processTime,
									nodeAfterInsertPos->sucByStage->b_startTime
									+ nodeAfterInsertPos->sucByStage->processTime);
						}
					}
					else
					{
						//B is less than or equal to A.
						int longestValue = f_start_time_of_pre_stage + b_start_time_of_suc_stage
										   + deleteNode->processTime;
						if (longestValue < bestPathLengthThroughDelNode)
						{
							bestPathLengthThroughDelNode = longestValue;
							nodeBeforeBestInsertPos = nodeBeforeAPoint;
						}
					}
				}
				else //a_point_offset >= 0 && b_point_offset < 0
				{
					//Position B is after the deletion position and position A is after the deletion position (point B may be the deletion position).
					if (-b_point_offset > a_point_offset)
					{
						//A is greater than B.
						int longestValue;
						Node* nodeBeforeInsertPos = nodeBeforeAPoint;
						Node* nodeAfterInsertPos = nodeBeforeInsertPos->sucByMachine;
						int f_start_time = f_start_time_of_node_before_A;
						while (true)
						{
							longestValue =
								std::max(f_start_time_of_pre_stage,
									f_start_time + nodeBeforeInsertPos->processTime)
								+ deleteNode->processTime +
								std::max(b_start_time_of_suc_stage,
									nodeAfterInsertPos->b_startTime + nodeAfterInsertPos->processTime);
							if (longestValue < bestPathLengthThroughDelNode)
							{
								bestPathLengthThroughDelNode = longestValue;
								nodeBeforeBestInsertPos = nodeBeforeInsertPos;
							}
							if (nodeAfterInsertPos == nodeAfterBPoint)
							{
								break;
							}
							nodeBeforeInsertPos = nodeAfterInsertPos;
							nodeAfterInsertPos = nodeBeforeInsertPos->sucByMachine;
							f_start_time =
								std::max(f_start_time
										 + nodeBeforeInsertPos->preByMachine->processTime,
									nodeBeforeInsertPos->preByStage->f_startTime
									+ nodeBeforeInsertPos->preByStage->processTime);
						}
					}
					else
					{
						//B is less than or equal to A.
						int longestValue = f_start_time_of_pre_stage + b_start_time_of_suc_stage
										   + deleteNode->processTime;
						if (longestValue < bestPathLengthThroughDelNode)
						{
							bestPathLengthThroughDelNode = longestValue;
							nodeBeforeBestInsertPos = nodeAfterBPoint->preByMachine;
						}
					}
				}
			}
			else
			{
				//Try another machine.
				int f_complete_time_machine = tails[mid]->f_startTime;
				int b_complete_time_machine = heads[mid]->b_startTime;

				if (f_complete_time_machine - f_start_time_of_pre_stage
					>= b_complete_time_machine - b_start_time_of_suc_stage)
				{
					//Find position A and position B from left to right
					bool is_find_A_pos = false;
					int longestValue;
					Node* nodeBeforeInsertPos = heads[mid];
					Node* nodeAfterInsertPos = nodeBeforeInsertPos->sucByMachine;
					while (nodeBeforeInsertPos != tails[mid])
					{
						if (!is_find_A_pos &&
							nodeAfterInsertPos->f_startTime + nodeAfterInsertPos->processTime
							> f_start_time_of_pre_stage)
						{
							//Position A was found.
							is_find_A_pos = true;
						}

						if (nodeAfterInsertPos->b_startTime + nodeAfterInsertPos->processTime
							<= b_start_time_of_suc_stage)
						{
							//Position B was found.
							longestValue =
								std::max(f_start_time_of_pre_stage,
									nodeBeforeInsertPos->f_startTime + nodeBeforeInsertPos->processTime)
								+ deleteNode->processTime
								+ b_start_time_of_suc_stage;
							if (longestValue < bestPathLengthThroughDelNode)
							{
								bestPathLengthThroughDelNode = longestValue;
								nodeBeforeBestInsertPos = nodeBeforeInsertPos;
							}
							break;
						}

						if (is_find_A_pos)
						{
							longestValue =
								std::max(f_start_time_of_pre_stage,
									nodeBeforeInsertPos->f_startTime + nodeBeforeInsertPos->processTime)
								+ deleteNode->processTime +
								std::max(b_start_time_of_suc_stage,
									nodeAfterInsertPos->b_startTime + nodeAfterInsertPos->processTime);
							if (longestValue < bestPathLengthThroughDelNode)
							{
								bestPathLengthThroughDelNode = longestValue;
								nodeBeforeBestInsertPos = nodeBeforeInsertPos;
							}
						}
						nodeBeforeInsertPos = nodeAfterInsertPos;
						nodeAfterInsertPos = nodeBeforeInsertPos->sucByMachine;
					}
				}
				else
				{
					//Find position A and position B from right to left
					bool is_find_B_pos = false;
					int longestValue;
					Node* nodeAfterInsertPos = tails[mid];
					Node* nodeBeforeInsertPos = nodeAfterInsertPos->preByMachine;
					while (nodeAfterInsertPos != heads[mid])
					{
						if (!is_find_B_pos &&
							nodeBeforeInsertPos->b_startTime + nodeBeforeInsertPos->processTime
							> b_start_time_of_suc_stage)
						{
							//Position B was found.
							is_find_B_pos = true;
						}

						if (nodeBeforeInsertPos->f_startTime + nodeBeforeInsertPos->processTime
							<= f_start_time_of_pre_stage)
						{
							//Position A was found.
							longestValue =
								f_start_time_of_pre_stage
								+ deleteNode->processTime
								+ std::max(b_start_time_of_suc_stage,
									nodeAfterInsertPos->b_startTime + nodeAfterInsertPos->processTime);

							if (longestValue < bestPathLengthThroughDelNode)
							{
								bestPathLengthThroughDelNode = longestValue;
								nodeBeforeBestInsertPos = nodeBeforeInsertPos;
							}
							break;
						}

						if (is_find_B_pos)
						{
							longestValue =
								std::max(f_start_time_of_pre_stage,
									nodeBeforeInsertPos->f_startTime + nodeBeforeInsertPos->processTime)
								+ deleteNode->processTime +
								std::max(b_start_time_of_suc_stage,
									nodeAfterInsertPos->b_startTime + nodeAfterInsertPos->processTime);
							if (longestValue < bestPathLengthThroughDelNode)
							{
								bestPathLengthThroughDelNode = longestValue;
								nodeBeforeBestInsertPos = nodeBeforeInsertPos;
							}
						}
						nodeAfterInsertPos = nodeBeforeInsertPos;
						nodeBeforeInsertPos = nodeAfterInsertPos->preByMachine;
					}
				}
			}
		}
		if (bestPathLengthThroughDelNode < original_makeSpan)
		{

			insert_node_at_position(deleteNode, nodeBeforeBestInsertPos, deleteNode->preByStage);
			int new_span = refresh_all_graph();
			return new_span;
		}
		else
		{
			insert_node_at_position(deleteNode, originalPreNodeOnMachine, deleteNode->preByStage);
		}
	}
	return original_makeSpan;
}

int Solution::decode_local_search_critical_nodes(int original_makeSpan)
{
	std::vector<Node*> critical_nodes;

	while (true)
	{
		critical_nodes.clear();
		find_all_critical_nodes_forward(critical_nodes, original_makeSpan);

		int new_span = local_search_critical_nodes(critical_nodes, original_makeSpan);
		if (new_span < original_makeSpan)
		{
			original_makeSpan = new_span;
		}
		else
		{
			break;
		}
	}
	return original_makeSpan;
}

int Solution::local_search_critical_nodes(const std::vector<Node*>& critical_nodes, int original_makeSpan)
{
	for (auto deleteCriticalNode : critical_nodes)
	{
		Node* originalPreNodeOnMachine = deleteCriticalNode->preByMachine;
		Node* preNodeAtStage = deleteCriticalNode->preByStage;
		Node* sucNodeAtStage = deleteCriticalNode->sucByStage;

		int stageId = deleteCriticalNode->stageId;
		delete_node_physical_from_graph(deleteCriticalNode);
		int makeSpanAfterDeleteNode = refresh_all_graph();

		const auto& heads = f_head_on_machines_in_each_stage[stageId];
		const auto& tails = b_head_on_machines_in_each_stage[stageId];
		int machineCount = problem.getNumOfMachinesInStage(stageId);
		int bestSpan = original_makeSpan;
		Node* nodeBeforeBestInsertPos;
		for (int mid = 0; mid < machineCount; ++mid)
		{
			Node* nodeBeforeInsertPos = heads[mid];
			Node* nodeAfterInsertPos = nodeBeforeInsertPos->sucByMachine;

			while (nodeBeforeInsertPos != tails[mid])
			{
				int longestValue =
					std::max(preNodeAtStage->f_startTime + preNodeAtStage->processTime,
						nodeBeforeInsertPos->f_startTime + nodeBeforeInsertPos->processTime)
					+ deleteCriticalNode->processTime +
					std::max(sucNodeAtStage->b_startTime + sucNodeAtStage->processTime,
						nodeAfterInsertPos->b_startTime + nodeAfterInsertPos->processTime);
				int makeSpanAfterInsert = std::max(longestValue, makeSpanAfterDeleteNode);
				if (makeSpanAfterInsert < bestSpan)
				{
					bestSpan = makeSpanAfterInsert;
					nodeBeforeBestInsertPos = nodeBeforeInsertPos;
				}
				nodeBeforeInsertPos = nodeAfterInsertPos;
				nodeAfterInsertPos = nodeBeforeInsertPos->sucByMachine;
			}
		}
		if (bestSpan < original_makeSpan)
		{
			insert_node_at_position(deleteCriticalNode, nodeBeforeBestInsertPos, preNodeAtStage);
			//solution.setMakeSpan(bestSpan);
			int new_span = refresh_all_graph();
			//assert(a == bestSpan);
			return new_span;
		}
		else
		{
			insert_node_at_position(deleteCriticalNode, originalPreNodeOnMachine, preNodeAtStage);
		}
	}
	return original_makeSpan;
}

int Solution::decode_local_search_all_nodes_r(int original_makeSpan)
{
	std::vector<Node*> critical_nodes;

	while (true)
	{
		int new_span = local_search_all_nodes_r(original_makeSpan);
		if (new_span < original_makeSpan)
		{
			original_makeSpan = new_span;
		}
		else
		{
			break;
		}
	}
	return original_makeSpan;
}

int Solution::local_search_all_nodes_r(int original_makeSpan)
{
	int num_of_jobs = problem.getNumOfJobs();
	int num_of_stages = problem.getNumOfStages();

	for (int s = 0; s < num_of_stages; ++s)
	{
		for (int jobId = 0; jobId < num_of_jobs; ++jobId)
		{
			Node* deleteNode = operations[s][jobId];
			Node* originalPreNodeOnMachine = deleteNode->preByMachine;
			Node* preNodeAtStage = deleteNode->preByStage;
			Node* sucNodeAtStage = deleteNode->sucByStage;

			int stageId = deleteNode->stageId;
			delete_node_physical_from_graph(deleteNode);
			int makeSpanAfterDeleteNode = refresh_all_graph();

			const auto& heads = f_head_on_machines_in_each_stage[stageId];
			const auto& tails = b_head_on_machines_in_each_stage[stageId];
			int machineCount = problem.getNumOfMachinesInStage(stageId);
			int bestSpan = original_makeSpan;
			Node* nodeBeforeBestInsertPos;
			for (int mid = 0; mid < machineCount; ++mid)
			{
				Node* nodeBeforeInsertPos = heads[mid];
				Node* nodeAfterInsertPos = nodeBeforeInsertPos->sucByMachine;

				while (nodeBeforeInsertPos != tails[mid])
				{
					int longestValue =
						std::max(preNodeAtStage->f_startTime + preNodeAtStage->processTime,
							nodeBeforeInsertPos->f_startTime + nodeBeforeInsertPos->processTime)
						+ deleteNode->processTime +
						std::max(sucNodeAtStage->b_startTime + sucNodeAtStage->processTime,
							nodeAfterInsertPos->b_startTime + nodeAfterInsertPos->processTime);
					int makeSpanAfterInsert = std::max(longestValue, makeSpanAfterDeleteNode);
					if (makeSpanAfterInsert < bestSpan)
					{
						bestSpan = makeSpanAfterInsert;
						nodeBeforeBestInsertPos = nodeBeforeInsertPos;
					}
					nodeBeforeInsertPos = nodeAfterInsertPos;
					nodeAfterInsertPos = nodeBeforeInsertPos->sucByMachine;
				}
			}
			if (bestSpan < original_makeSpan)
			{
				insert_node_at_position(deleteNode, nodeBeforeBestInsertPos, preNodeAtStage);
				//solution.setMakeSpan(bestSpan);
				int new_span = refresh_all_graph();
				//assert(a == bestSpan);
				return new_span;
			}
			else
			{
				insert_node_at_position(deleteNode, originalPreNodeOnMachine, preNodeAtStage);
			}
		}
	}

	return original_makeSpan;
}

int Solution::decode_local_search_all_nodes_nr(int original_makeSpan)
{
	std::vector<Node*> critical_nodes;

	while (true)
	{
		int new_span = local_search_all_nodes_nr(original_makeSpan);
		if (new_span < original_makeSpan)
		{
			original_makeSpan = new_span;
		}
		else
		{
			break;
		}
	}
	return original_makeSpan;
}

int Solution::local_search_all_nodes_nr(int original_makeSpan)
{
	int num_of_jobs = problem.getNumOfJobs();
	int num_of_stages = problem.getNumOfStages();

	for (int s = 0; s < num_of_stages; ++s)
	{
		for (int jobId = 0; jobId < num_of_jobs; ++jobId)
		{
			Node* deleteNode = operations[s][jobId];
			Node* originalPreNodeOnMachine = deleteNode->preByMachine;
			Node* preNodeAtStage = deleteNode->preByStage;

			int stageId = deleteNode->stageId;
			delete_node_physical_from_graph(deleteNode);
			//int makeSpanAfterDeleteNode = refresh_all_graph();

			const auto& heads = f_head_on_machines_in_each_stage[stageId];
			const auto& tails = b_head_on_machines_in_each_stage[stageId];
			int machineCount = problem.getNumOfMachinesInStage(stageId);
			int bestSpan = original_makeSpan;
			Node* nodeBeforeBestInsertPos;
			for (int mid = 0; mid < machineCount; ++mid)
			{
				Node* nodeBeforeInsertPos = heads[mid];
				Node* nodeAfterInsertPos = nodeBeforeInsertPos->sucByMachine;

				while (nodeBeforeInsertPos != tails[mid])
				{
					insert_node_at_position(deleteNode, nodeBeforeInsertPos, preNodeAtStage);
					int makeSpanAfterInsert = refresh_forward_graph();
					if (makeSpanAfterInsert < bestSpan)
					{
						bestSpan = makeSpanAfterInsert;
						nodeBeforeBestInsertPos = nodeBeforeInsertPos;
					}
					delete_node_physical_from_graph(deleteNode);
					nodeBeforeInsertPos = nodeAfterInsertPos;
					nodeAfterInsertPos = nodeBeforeInsertPos->sucByMachine;
				}
			}
			if (bestSpan < original_makeSpan)
			{
				insert_node_at_position(deleteNode, nodeBeforeBestInsertPos, preNodeAtStage);
				//solution.setMakeSpan(bestSpan);
				//int new_span = refresh_all_graph();
				//assert(a == bestSpan);
				return bestSpan;
			}
			else
			{
				insert_node_at_position(deleteNode, originalPreNodeOnMachine, preNodeAtStage);
			}
		}
	}
	return original_makeSpan;
}

std::pair<int, int> Solution::find_best_insert_position(int inserted_job_id)
{
	int min_span = INT_MAX;
	int best_pos;
	for (int pos = 0; pos <= jobSequence.size(); ++pos)
	{
		jobSequence.insert(jobSequence.begin() + pos, inserted_job_id);
		int span = decode_forward();
		if (span < min_span)
		{
			min_span = span;
			best_pos = pos;
		}
		jobSequence.erase(jobSequence.begin() + pos);
	}
	//jobSequence.insert(jobSequence.begin() + best_pos, inserted_job_id);
	return { min_span, best_pos };
}

std::pair<int, int> Solution::find_best_swap(int swapped_index)
{
	int min_span = INT_MAX;
	int best_index;
	for (int index = 0; index < jobSequence.size(); ++index)
	{
		if (index == swapped_index)
		{
			continue;
		}
		std::swap(jobSequence[index], jobSequence[swapped_index]);
		int span = decode_forward();
		if (span < min_span)
		{
			min_span = span;
			best_index = index;
		}
		std::swap(jobSequence[index], jobSequence[swapped_index]);
	}
	return { min_span, best_index };
}

void Solution::destruction(std::vector<int>& erased_jobs, int d)
{
	int k = 0;
	while (k < d)
	{
		int pos = wyt_rand(jobSequence.size());
		erased_jobs.emplace_back(jobSequence[pos]);
		jobSequence.erase(jobSequence.begin() + pos);
		++k;
	}
}

void Solution::write_to_file(const std::string& file_name) const
{
	std::ofstream o_file;
	o_file.open(file_name);
	int stageId = 0;
	for (const auto& headOnMachines : f_head_on_machines_in_each_stage)
	{
		o_file << stageId << std::endl;
		int machineId = 0;
		for (const auto& head : headOnMachines)
		{
			o_file << machineId << std::endl;
			Node* node = head->sucByMachine;
			while (node->jobId != -1)
			{
				o_file << node->jobId << "\t";
				node = node->sucByMachine;
			}
			++machineId;
			o_file << std::endl;
		}
		++stageId;
	}
	o_file.close();

}







