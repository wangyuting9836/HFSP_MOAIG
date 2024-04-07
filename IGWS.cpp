//
// Created by wangy on 2024/1/13.
//

#include <numeric>
#include <algorithm>
#include <iostream>

#include "IGWS.h"
#include "HFS_Problem.h"
#include "Solution.h"
#include "utils.h"

double IGWS::m_lambda = 0.0;
double IGWS::m_T = 0.0;
int IGWS::m_d = 0;

int IGWS::NEH_f(const std::vector<int>& seed_job_sequence, Solution& solution)
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

int IGWS::iterative_improvement_insertion(Solution& solution, int old_span)
{
	const HFS_Problem& problem = solution.getProblem();

	std::vector<int> old_sequence;

	bool improvement = true;
	while (improvement)
	{
		improvement = false;
		old_sequence = solution.getJobSequence();
		for(int job_id : old_sequence)
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

void IGWS::destruction(Solution& solution, std::vector<int>& erased_jobs)
{
	solution.destruction(erased_jobs, m_d);
}

int IGWS::construction(Solution& solution, std::vector<int>& erased_jobs)
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

int IGWS::Evolution(const HFS_Problem& problem)
{
	int num_of_jobs = problem.getNumOfJobs();
	int num_of_stages = problem.getNumOfStages();
	int total_process_time = problem.get_total_process_time();

	double time_limit = m_lambda * num_of_jobs * num_of_jobs * num_of_jobs * num_of_stages;

	long algorithm_start_time = GetElapsedProcessTime();

	int best_span = INT32_MAX;
	int span;

	std::vector<int> seed_job_sequence(num_of_jobs);
	std::iota(std::begin(seed_job_sequence), std::end(seed_job_sequence), 0);
	std::sort(seed_job_sequence.begin(), seed_job_sequence.end(), [&](int j1, int j2)
	{
		return problem.getJobs()[j1].get_total_process_time() > problem.getJobs()[j2].get_total_process_time();
	});

	Solution solution(problem, Solution::Solution_Init_Type::Empty);

	span = NEH_f(seed_job_sequence, solution);
	if (span < best_span)
	{
		best_span = span;
	}

	span = iterative_improvement_insertion(solution, span);
	if (span < best_span)
	{
		best_span = span;
	}

	std::vector<int> erased_jobs;
	std::vector<int> original_job_sequence = solution.getJobSequence();
	int original_span = span;

	int total_iteration_count = 0;

	while (true)
	{
		erased_jobs.clear();
		destruction(solution, erased_jobs);
		span = construction(solution, erased_jobs);

		solution.decode_forward_to_graph();
		span = solution.decode_local_search_critical_nodes(span);

		if (span < best_span)
		{
			best_span = span;
			//std::cout << "\titeration" << total_iteration_count << ":\t" << best_span << std::endl;
		}

		if (span <= original_span)
		{
			original_span = span;
			original_job_sequence = solution.getJobSequence();
		}
		else
		{
			double rand_value = wyt_rand(0.0, 1.0);
			if (rand_value
				< std::exp(-(span - original_span) * num_of_jobs * num_of_stages * 10 / (m_T * total_process_time)))
			{
				original_span = span;
				original_job_sequence = solution.getJobSequence();
			}
			else
			{
				solution.set_job_sequence(original_job_sequence);
				span = original_span;
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
	return best_span;
}
