//
// Created by wangy on 2024/1/17.
//

#include <vector>
#include <numeric>
#include <iostream>
#include <chrono>
#include <string>
#include "MOAIG.h"
#include "Solution.h"
#include "utils.h"

double MOAIG::m_lambda = 0.0;
double MOAIG::m_T = 0.0;
double MOAIG::m_jP = 0.0;
int MOAIG::m_d = 0;

int MOAIG::NEH(const std::vector<int>& seed_job_sequence, Solution& solution)
{
	int span;
	for (auto job_id : seed_job_sequence)
	{
		auto re = solution.find_best_insert_position(job_id);
		span = re.first;
		solution.insert_job_at_position(job_id, re.second);
	}
	return span;
}

int MOAIG::iterative_improvement_insertion_ls(Solution& solution, int old_span)
{
	std::vector<int> old_sequence;

	bool improvement = true;
	while (improvement)
	{
		improvement = false;
		old_sequence = solution.getJobSequence();
		for (int job_id : old_sequence)
		{
			int old_pos = solution.remove_one_job_by_id(job_id);
			auto re = solution.find_best_insert_position(job_id);
			if (re.first < old_span)
			{
				solution.insert_job_at_position(job_id, re.second);
				improvement = true;
				old_span = re.first;
			}
			else
			{
				solution.insert_job_at_position(job_id, old_pos);
			}
		}
	}
	return old_span;
}

void MOAIG::destruction(Solution& solution, std::vector<int>& erased_jobs)
{
	solution.destruction(erased_jobs, m_d);
}

int MOAIG::construction(Solution& solution, std::vector<int>& erased_jobs)
{
	int span;
	for (auto job_id : erased_jobs)
	{
		auto re = solution.find_best_insert_position(job_id);
		span = re.first;
		solution.insert_job_at_position(job_id, re.second);
	}
	return span;
}

int MOAIG::RIS_local_search(Solution& solution, int old_span, const Solution& best_solution)
{
	const HFS_Problem& problem = solution.getProblem();
	int num_of_jobs = problem.getNumOfJobs();

	int iter = 0;
	int pos = 0;
	while (iter < num_of_jobs)
	{
		int job_id = best_solution.get_job(pos);
		pos = (pos + 1) % num_of_jobs;
		int old_position = solution.remove_one_job_by_id(job_id);
		auto re = solution.find_best_insert_position(job_id);
		if (re.first < old_span)
		{
			solution.insert_job_at_position(job_id, re.second);
			old_span = re.first;
			iter = 0;
		}
		else
		{
			solution.insert_job_at_position(job_id, old_position);
			++iter;
		}
	}
	return old_span;
}

int MOAIG::RSS_local_search(Solution& solution, int old_span, const Solution& best_solution)
{
	const HFS_Problem& problem = solution.getProblem();
	int num_of_jobs = problem.getNumOfJobs();

	int iter = 0;
	int pos = 0;
	while (iter < num_of_jobs)
	{
		int job_id = best_solution.get_job(pos);
		pos = (pos + 1) % num_of_jobs;

		int swapped_index = solution.get_index(job_id);
		auto re = solution.find_best_swap(swapped_index);

		if (re.first < old_span)
		{
			solution.swap_job_by_index(swapped_index, re.second);
			old_span = re.first;
			iter = 0;
		}
		else
		{
			++iter;
		}
	}
	return old_span;
}

int MOAIG::Evolution(const HFS_Problem& problem)
{
	int num_of_jobs = problem.getNumOfJobs();
	int num_of_stages = problem.getNumOfStages();
	int total_process_time = problem.get_total_process_time();

	double time_limit = m_lambda * num_of_jobs * num_of_jobs * num_of_jobs * num_of_stages;
	//double time_limit = m_lambda * num_of_jobs * num_of_stages * 1000;

	long algorithm_start_time = GetElapsedProcessTime();

	int best_span_f = INT32_MAX;
	int span_f;

	std::vector<int> seed_job_sequence_f(num_of_jobs);
	std::iota(std::begin(seed_job_sequence_f), std::end(seed_job_sequence_f), 0);
	//std::shuffle(std::begin(seed_job_sequence_f), std::end(seed_job_sequence_f), rand_generator());
	std::sort(seed_job_sequence_f.begin(), seed_job_sequence_f.end(), [&](int j1, int j2)
	{
		return problem.getJobs()[j1].get_total_process_time() > problem.getJobs()[j2].get_total_process_time();
	});

	Solution solution_f(problem, Solution::Solution_Init_Type::Empty);
	span_f = NEH(seed_job_sequence_f, solution_f);

	if (span_f < best_span_f)
	{
		best_span_f = span_f;
	}

	span_f = iterative_improvement_insertion_ls(solution_f, span_f);
	if (span_f < best_span_f)
	{
		best_span_f = span_f;
	}

	std::vector<int> erased_jobs;

	Solution best_solution_f = solution_f;
	Solution assistant_solution(problem);
	std::vector<int> original_job_sequence_f = solution_f.getJobSequence();
	int original_span_f = span_f;
	int total_iteration_count = 0;
	while (true)
	{
		erased_jobs.clear();
		destruction(solution_f, erased_jobs);
		span_f = construction(solution_f, erased_jobs);

		double r1 = wyt_rand(0.0, 1.0);
		if (r1 < m_jP)
		{
			span_f = RIS_local_search(solution_f, span_f, best_solution_f);
		}
		else
		{
			span_f = RSS_local_search(solution_f, span_f, best_solution_f);
		}

		solution_f.decode_forward_to_graph();
		Solution tem_solution = solution_f;
		int ls_span_f = solution_f.decode_local_search_mandatory_nodes(span_f);
		if (ls_span_f < best_span_f)
		{
			best_span_f = ls_span_f;
			best_solution_f = solution_f;

			std::chrono::system_clock::time_point time_point_now = std::chrono::system_clock::now();
			std::chrono::system_clock::duration duration_since_epoch
				= time_point_now.time_since_epoch();
			time_t since_epoch = duration_since_epoch.count();

			best_solution_f.write_to_file(
				std::string("../result/") + std::to_string(best_span_f) + std::string("_")+ std::to_string(since_epoch)
				+ std::string(".txt"));
		}

		if (span_f <= original_span_f)
		{
			original_span_f = span_f;
			original_job_sequence_f = solution_f.getJobSequence();
		}
		else
		{
			double rand_value = wyt_rand(0.0, 1.0);
			if (rand_value
				< std::exp(-(span_f - original_span_f) * num_of_jobs * num_of_stages * 10 / (m_T * total_process_time)))
			{
				original_span_f = span_f;
				original_job_sequence_f = solution_f.getJobSequence();
			}
			else
			{
				solution_f.set_job_sequence(original_job_sequence_f);
			}
		}
		++total_iteration_count;
		long cur_time = GetElapsedProcessTime();
		long ElapsedTime = cur_time - algorithm_start_time;

		if (ElapsedTime > time_limit)
		{
			break;
		}
	}
	std::cout << "\titeration," << total_iteration_count;
	return best_span_f;
}

void MOAIG::Evolution_local_search(const HFS_Problem& problem, std::vector<std::tuple<int, int, double>>& result)
{
	std::chrono::time_point<std::chrono::steady_clock> st;
	std::chrono::time_point<std::chrono::steady_clock> et;
	long algorithm_start_time;
	long algorithm_end_time;

	int num_of_jobs = problem.getNumOfJobs();

	int span;
	int ls_span;

	int min_span_1 = INT32_MAX;
	int min_span_2 = INT32_MAX;
	int min_span_3 = INT32_MAX;
	int min_span_4 = INT32_MAX;
	int time_1 = 0;
	int time_2 = 0;
	int time_3 = 0;
	int time_4 = 0;
	double duration_1 = 0;
	double duration_2 = 0;
	double duration_3 = 0;
	double duration_4 = 0;
	int k = 0;
	while (k < 1)
	{
		std::vector<int> seed_job_sequence_f(num_of_jobs);
		std::iota(std::begin(seed_job_sequence_f), std::end(seed_job_sequence_f), 0);
		std::shuffle(std::begin(seed_job_sequence_f), std::end(seed_job_sequence_f), rand_generator());
		//	std::sort(seed_job_sequence_f.begin(), seed_job_sequence_f.end(), [&](int j1, int j2)
		//	{
		//		return problem.getJobs()[j1].get_total_process_time() > problem.getJobs()[j2].get_total_process_time();
		//	});

		st = std::chrono::steady_clock::now();
		algorithm_start_time = GetElapsedProcessTime();
		Solution solution_1(problem, seed_job_sequence_f);
		span = solution_1.decode_forward_to_graph();
		ls_span = solution_1.decode_local_search_mandatory_nodes(span);
		algorithm_end_time = GetElapsedProcessTime();
		et = std::chrono::steady_clock::now();
		min_span_1 = std::min(ls_span, min_span_1);
		duration_1 += std::chrono::duration<double, std::milli>(et - st).count();
		time_1 += algorithm_end_time - algorithm_start_time;

		st = std::chrono::steady_clock::now();
		algorithm_start_time = GetElapsedProcessTime();
		Solution solution_2(problem, seed_job_sequence_f);
		span = solution_2.decode_forward_to_graph();
		ls_span = solution_2.decode_local_search_critical_nodes(span);
		algorithm_end_time = GetElapsedProcessTime();
		et = std::chrono::steady_clock::now();
		min_span_2 = std::min(ls_span, min_span_2);
		duration_2 += std::chrono::duration<double, std::milli>(et - st).count();
		time_2 += algorithm_end_time - algorithm_start_time;

		st = std::chrono::steady_clock::now();
		algorithm_start_time = GetElapsedProcessTime();
		Solution solution_3(problem, seed_job_sequence_f);
		span = solution_3.decode_forward_to_graph();
		ls_span = solution_3.decode_local_search_all_nodes_r(span);
		algorithm_end_time = GetElapsedProcessTime();
		et = std::chrono::steady_clock::now();
		min_span_3 = std::min(ls_span, min_span_3);
		duration_3 += std::chrono::duration<double, std::milli>(et - st).count();
		time_3 += algorithm_end_time - algorithm_start_time;

		st = std::chrono::steady_clock::now();
		algorithm_start_time = GetElapsedProcessTime();
		Solution solution_4(problem, seed_job_sequence_f);
		span = solution_4.decode_forward_to_graph();
		ls_span = solution_4.decode_local_search_all_nodes_nr(span);
		algorithm_end_time = GetElapsedProcessTime();
		et = std::chrono::steady_clock::now();
		min_span_4 = std::min(ls_span, min_span_4);
		duration_4 += std::chrono::duration<double, std::milli>(et - st).count();
		time_4 += algorithm_end_time - algorithm_start_time;
		++k;
	}
	result.emplace_back(min_span_1, time_1, duration_1);
	result.emplace_back(min_span_2, time_2, duration_2);
	result.emplace_back(min_span_3, time_3, duration_3);
	result.emplace_back(min_span_4, time_4, duration_4);
}
