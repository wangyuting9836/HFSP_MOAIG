//
// Created by wangy on 2024/1/31.
//
/*
?ztop, H., Fatih Tasgetiren, M., Eliiyi, D. T., & Pan, Q.-K. (2019). Metaheuristic algorithms for the hybrid flowshop scheduling problem. Computers & Operations Research, 111, 177¨C196. https://doi.org/10.1016/j.cor.2019.06.009
*/

#ifndef _METAHEURISTICALGORITHMS_H_
#define _METAHEURISTICALGORITHMS_H_

#include <vector>
#include "HFS_Problem.h"
class SolutionMA
{
	const HFS_Problem& problem;
	int make_span;
	std::vector<int> job_sequence;

 public:
	explicit SolutionMA(const HFS_Problem& problem);
	SolutionMA(const SolutionMA& solution) = default;
	SolutionMA& operator=(const SolutionMA& solution);

	[[nodiscard]] int evaluate();
	std::pair<int, int> find_best_position(int inserted_job_id);
	std::pair<int, int> find_best_position_block(const std::vector<int>& erased_jobs);
	std::pair<int, int> find_best_swap(int swapped_index);
	void destruction(std::vector<int>& erased_jobs, int d);
	void destruction_block(std::vector<int>& erased_jobs, int block_size);
	[[nodiscard]] int get_job(int index) const
	{
		return job_sequence[index];
	}
	[[nodiscard]] int get_index(int job_id) const
	{
		return std::find(job_sequence.begin(), job_sequence.end(), job_id) - job_sequence.begin();;
	}
	void insert_job_at_position(int inserted_job_id, int position)
	{
		job_sequence.insert(job_sequence.begin() + position, inserted_job_id);
	}
	void insert_block_at_position(const std::vector<int>& erased_jobs, int position)
	{
		job_sequence.insert(job_sequence.begin() + position, erased_jobs.begin(), erased_jobs.end());
	}
	void remove_one_job_by_index(int index)
	{
		job_sequence.erase(job_sequence.begin() + index);
	}
	int remove_one_job_by_id(int job_id)
	{
		auto it = std::find(job_sequence.begin(), job_sequence.end(), job_id);
		int position = it - job_sequence.begin();
		job_sequence.erase(it);
		return position;
	}
	void swap_job_by_index(int index1, int index2)
	{
		std::swap(job_sequence[index1], job_sequence[index2]);
	}
	void emplace_back_one_job(int inserted_job_id)
	{
		job_sequence.emplace_back(inserted_job_id);
	}
	void remove_last_job()
	{
		job_sequence.pop_back();
	}
	[[nodiscard]] const std::vector<int>& get_job_sequence() const
	{
		return job_sequence;
	}
	void set_job_sequence(const std::vector<int>& sequence)
	{
		job_sequence = sequence;
	}
	[[nodiscard]] int get_span() const
	{
		return make_span;
	}
	void set_span(int span)
	{
		make_span = span;
	}
	[[nodiscard]] const HFS_Problem& get_problem() const
	{
		return problem;
	}
};

class MetaheuristicAlgorithms
{
	static int m_d;
	static double m_T;
	static double m_jP;
	static int m_block_max;
	static double m_lambda;
 public:
	enum AlgorithmType
	{
		IGRS, IGGR, IGT, IGALL
	};
	static void set_parameters(double lambda, int d, double t, double jp, int block_max);
	static SolutionMA NEH(const HFS_Problem& problem, const std::vector<int>& guiding_job_sequence);
	static SolutionMA GRASP_NEH(const HFS_Problem& problem, const std::vector<int>& guiding_job_sequence, int x);
	static void GRASP(SolutionMA& solution);
	static void destruction(SolutionMA& solution, std::vector<int>& erased_jobs);
	static void destruction_block(SolutionMA& solution, std::vector<int>& erased_jobs, int block_size);
	static void construction(SolutionMA& solution, std::vector<int>& erased_jobs);
	static void construction_block(SolutionMA& solution, std::vector<int>& erased_jobs);
	static void first_improvement_insertion_local_search(SolutionMA& solution);
	static void RIS_local_search(SolutionMA& solution, const SolutionMA& best_solution);
	static void RSS_local_search(SolutionMA& solution, const SolutionMA& best_solution);
	static int IG_algorithm(const HFS_Problem& problem, AlgorithmType algorithm_type);
	static int VBIH_algorithm(const HFS_Problem& problem);
};

#endif //_METAHEURISTICALGORITHMS_H_
