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

    int processTime = 0;
    int f_startTime = 0;
    int b_startTime = 0;
    Node *sucByStage = nullptr;
    Node *sucByMachine = nullptr;
    Node *preByStage = nullptr;
    Node *preByMachine = nullptr;

	//The following is_visit is used to find nodes on the critical path.
	bool is_visit = false;

	//Used when finding all mandatory operations.
	int dfn = 0;
	int low = 0;
	int critical_adjacent_count = 0;
	std::vector<Node*> critical_adjacent = std::vector<Node*>(4);

    Node(int jobId, int stageId, int machineId, int processTime)
            : jobId(jobId), stageId(stageId), machineId(machineId), processTime(processTime)
    {}
	Node(const Node& node)
		: jobId(node.jobId), stageId(node.stageId), machineId(node.machineId), processTime(node.processTime),
		f_startTime(node.f_startTime), b_startTime(node.b_startTime)
	{}
};

class Solution
{
	const HFS_Problem& problem;
    std::vector<int> jobSequence;
    std::vector<std::vector<Node *>> f_head_on_machines_in_each_stage;
    std::vector<std::vector<Node *>> b_head_on_machines_in_each_stage;
    std::vector<Node *> f_headJobs;
    std::vector<Node *> b_headJobs;
	std::vector<std::vector<Node *>> operations;

 public:
	enum Solution_Init_Type{RandomInit, Empty};

    explicit Solution(const HFS_Problem &problem, Solution_Init_Type init_type = RandomInit);
	Solution(const HFS_Problem& problem, const std::vector<int>& sequence);
	Solution(const HFS_Problem &problem, const std::string& file_name);
	Solution(const Solution &solution);
	Solution& operator=(const Solution &solution);
	~Solution();

	void initBaseGraph();
	void reset_to_base_graph();

	int decode_forward();
	//The algorithm 1 (DecodeToGraph) in the paper
	int decode_forward_to_graph();
	int refresh_all_graph();
	int refresh_forward_graph();

	void find_all_critical_nodes_forward(std::vector<Node *> &critical_nodes, int span) const;

	//The algorithm 2 (ObtainingMandatoryOperations) in the paper
	void find_mandatory_nodes(std::vector<Node*>& mandatory_nodes, int span) const;

	void delete_node_physical_from_graph(Node* deleted_node);
	void insert_node_at_position(Node* deleted_node, Node* node_before_insert_pos, Node * node_in_pre_stage);

	//The algorithm 3 (MOAIG) in the paper
	int decode_local_search_mandatory_nodes(int original_makeSpan);
	int local_search_mandatory_nodes(const std::vector<Node*>& mandatory_nodes, int original_makeSpan);

	int decode_local_search_critical_nodes(int original_makeSpan);
	int local_search_critical_nodes(const std::vector<Node*>& critical_nodes, int original_makeSpan);

	int decode_local_search_all_nodes_r(int original_makeSpan);
	int local_search_all_nodes_r(int original_makeSpan);

	int decode_local_search_all_nodes_nr(int original_makeSpan);
	int local_search_all_nodes_nr(int original_makeSpan);

	std::pair<int, int> find_best_insert_position(int inserted_job_id);
	std::pair<int, int> find_best_swap(int swapped_index);

	void destruction(std::vector<int>& erased_jobs, int d);

	[[nodiscard]] const std::vector<std::vector<Node *>>& get_operations() const;
    [[nodiscard]] const std::vector<Node *> &get_head_on_machines_in_stage(int stageId) const;
    [[nodiscard]] std::vector<std::vector<Node *>> &get_tail_on_machines();
	void set_job_sequence(const std::vector<int>& job_sequence);
	[[nodiscard]] const std::vector<int>& getJobSequence() const;
	[[nodiscard]] const HFS_Problem& getProblem() const;
	[[nodiscard]] int get_job(int index) const;
	[[nodiscard]] int get_index(int job_id) const;
	void insert_job_at_position(int inserted_job_id, int position);
	int remove_one_job_by_id(int job_id);
	void swap_job_by_index(int index1, int index2);
	void write_to_file(const std::string& file_name) const;
};

inline const std::vector<std::vector<Node *>>& Solution::get_operations() const
{
	return operations;
}

inline const std::vector<Node *>& Solution::get_head_on_machines_in_stage(int stageId) const
{
	return f_head_on_machines_in_each_stage[stageId];
}

inline std::vector<std::vector<Node *>>& Solution::get_tail_on_machines()
{
	return b_head_on_machines_in_each_stage;
}

inline void Solution::set_job_sequence(const std::vector<int>& job_sequence)
{
	jobSequence = job_sequence;
}

inline const std::vector<int>& Solution::getJobSequence() const
{
	return jobSequence;
}

inline const HFS_Problem& Solution::getProblem() const
{
	return problem;
}

inline  int Solution::get_job(int index) const
{
	return jobSequence[index];
}
inline  int Solution::get_index(int job_id) const
{
	return std::find(jobSequence.begin(), jobSequence.end(), job_id) - jobSequence.begin();;
}

inline void Solution::insert_job_at_position(int inserted_job_id, int position)
{
	jobSequence.insert(jobSequence.begin() + position, inserted_job_id);
}

inline int Solution::remove_one_job_by_id(int job_id)
{
	auto it = std::find(jobSequence.begin(), jobSequence.end(), job_id);
	int position = it - jobSequence.begin();
	jobSequence.erase(it);
	return position;
}

inline void Solution::swap_job_by_index(int index1, int index2)
{
	std::swap(jobSequence[index1], jobSequence[index2]);
}
#endif //HFSP_CRITICAL_PATH_SOLUTION_H
