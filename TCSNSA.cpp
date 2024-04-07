//
// Created by wangy on 2024/1/24.
//

#include <iostream>
#include <numeric>
#include "TCSNSA.h"
#include "utils.h"
#include "TemporyStorage.h"

void NeighborhoodSearch::insertion(std::vector<int>& sequence)
{
	int count = sequence.size();
	int p1 = wyt_rand(count);
	int p2;
	do
	{
		p2 = wyt_rand(count);
	} while (p1 == p2);
	int job_id = sequence[p1];
	sequence.erase(sequence.begin() + p1);
	sequence.insert(sequence.begin() + p2, job_id);
}

void NeighborhoodSearch::swap(std::vector<int>& sequence)
{
	int count = sequence.size();
	int p1 = wyt_rand(count);
	int p2;
	if (p1 == count - 1)
	{
		p2 = p1 - 1;
	}
	else
	{
		p2 = p1 + 1;
	}
	std::swap(sequence[p1], sequence[p2]);
}

void NeighborhoodSearch::pairwise_exchange(std::vector<int>& sequence)
{
	int count = sequence.size();
	int p1 = wyt_rand(count);
	int p2;
	do
	{
		p2 = wyt_rand(count);
	} while (p1 == p2 || p2 == p1 + 1 || p2 == p1 - 1);
	std::swap(sequence[p1], sequence[p2]);
}

SolutionTCSNSA::SolutionTCSNSA(const HFS_Problem& problem)
	: problem(problem), make_span(0)
{

}

SolutionU11::SolutionU11(const HFS_Problem& problem)
	: SolutionTCSNSA(problem)
{
	job_sequence.resize(problem.getNumOfJobs());
	std::iota(std::begin(job_sequence), std::end(job_sequence), 0);
	std::shuffle(std::begin(job_sequence), std::end(job_sequence), rand_generator());
	//make_span = evaluate();
}

SolutionU11& SolutionU11::operator=(const SolutionU11& solution)
{
	if (this == &solution)
	{
		return *this;
	}
	job_sequence = solution.job_sequence;
	make_span = solution.make_span;
	return *this;
}

int SolutionU11::evaluate() const
{
	int num_of_stages = problem.getNumOfStages();
	const std::vector<Job>& jobs = problem.getJobs();

	TS::zero_storage();
	//first stage
	for (auto job_id : job_sequence)
	{
		int mt = min_element(TS::m_idle_time[0].begin(), TS::m_idle_time[0].end()) - TS::m_idle_time[0].begin();
		TS::c_time[job_id][0] = TS::m_idle_time[0][mt] + jobs[job_id].getProcessTime(0);
		TS::m_idle_time[0][mt] = TS::c_time[job_id][0];
	}

	TS::sequence_of_other_stage = job_sequence;
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
				+ jobs[first_come_job_id].getProcessTime(s);
			TS::m_idle_time[s][mt] = TS::c_time[first_come_job_id][s];
		}
	}
	return *std::max_element(TS::m_idle_time[num_of_stages - 1].begin(), TS::m_idle_time[num_of_stages - 1].end());
}

SolutionU12::SolutionU12(const HFS_Problem& problem)
	: SolutionTCSNSA(problem)
{
	job_sequence.resize(problem.getNumOfJobs());
	std::iota(std::begin(job_sequence), std::end(job_sequence), 0);
	std::shuffle(std::begin(job_sequence), std::end(job_sequence), rand_generator());
	//make_span = evaluate();
}

SolutionU12& SolutionU12::operator=(const SolutionU12& solution)
{
	if (this == &solution)
	{
		return *this;
	}
	job_sequence = solution.job_sequence;
	make_span = solution.make_span;
	return *this;
}

int SolutionU12::evaluate() const
{
	int num_of_stages = problem.getNumOfStages();
	const std::vector<Job>& jobs = problem.getJobs();

	TS::zero_storage();
	//最后阶段
	for (auto it = job_sequence.rbegin(); it != job_sequence.rend(); ++it)
	{
		int job_id = *it;
		int mt = min_element(TS::m_idle_time[num_of_stages - 1].begin(), TS::m_idle_time[num_of_stages - 1].end())
				 - TS::m_idle_time[num_of_stages - 1].begin();
		TS::c_time[job_id][num_of_stages - 1] =
			TS::m_idle_time[num_of_stages - 1][mt] + jobs[job_id].getProcessTime(num_of_stages - 1);
		TS::m_idle_time[num_of_stages - 1][mt] = TS::c_time[job_id][num_of_stages - 1];
	}

	TS::sequence_of_other_stage = job_sequence;
	//other stage. FIFO and FAM
	for (int s = num_of_stages - 2; s >= 0; --s)
	{
		std::sort(TS::sequence_of_other_stage.begin(), TS::sequence_of_other_stage.end(), [&](int job1, int job2)
		{
			return TS::c_time[job1][s + 1] > TS::c_time[job2][s + 1];
		});

		for (auto it = TS::sequence_of_other_stage.rbegin(); it != TS::sequence_of_other_stage.rend(); ++it)
		{
			int first_come_job_id = *it;

			int mt = min_element(TS::m_idle_time[s].begin(), TS::m_idle_time[s].end()) - TS::m_idle_time[s].begin();
			TS::c_time[first_come_job_id][s] =
				std::max(TS::m_idle_time[s][mt], TS::c_time[first_come_job_id][s + 1])
				+ jobs[first_come_job_id].getProcessTime(s);
			TS::m_idle_time[s][mt] = TS::c_time[first_come_job_id][s];
		}
	}
	return *std::max_element(TS::m_idle_time[0].begin(), TS::m_idle_time[0].end());
}

SolutionU21::SolutionU21(const HFS_Problem& problem)
	: SolutionTCSNSA(problem)
{
	job_sequence_at_each_stage.resize(problem.getNumOfStages());
	for (auto& seq : job_sequence_at_each_stage)
	{
		seq.resize(problem.getNumOfJobs());
		//std::iota(std::begin(seq), std::end(seq), 0);
		//std::shuffle(std::begin(seq), std::end(seq), rand_generator());
	}
	make_span = 0;
}

SolutionU21& SolutionU21::operator=(const SolutionU21& solution)
{
	if (this == &solution)
	{
		return *this;
	}
	job_sequence_at_each_stage = solution.job_sequence_at_each_stage;
	make_span = solution.make_span;
	return *this;
}

int SolutionU21::evaluate() const
{
	int num_of_stages = problem.getNumOfStages();
	const std::vector<Job>& jobs = problem.getJobs();

	TS::zero_storage();
	//first stage
	for (auto job_id : job_sequence_at_each_stage[0])
	{
		int mt = min_element(TS::m_idle_time[0].begin(), TS::m_idle_time[0].end()) - TS::m_idle_time[0].begin();
		TS::c_time[job_id][0] = TS::m_idle_time[0][mt] + jobs[job_id].getProcessTime(0);
		TS::m_idle_time[0][mt] = TS::c_time[job_id][0];
	}

	//other stage. FIFO and FAM
	for (int s = 1; s < num_of_stages; ++s)
	{
		for (int job_id : job_sequence_at_each_stage[s])
		{
			int mt = min_element(TS::m_idle_time[s].begin(), TS::m_idle_time[s].end()) - TS::m_idle_time[s].begin();
			TS::c_time[job_id][s] =
				std::max(TS::m_idle_time[s][mt], TS::c_time[job_id][s - 1])
				+ jobs[job_id].getProcessTime(s);
			TS::m_idle_time[s][mt] = TS::c_time[job_id][s];
		}
	}
	return *std::max_element(TS::m_idle_time[num_of_stages - 1].begin(), TS::m_idle_time[num_of_stages - 1].end());
}

void SolutionU21::generate_expert(SolutionU22& solution_u22_e) const
{
	//SolutionU22 solution_u22_e(problem);

	int num_of_stages = problem.getNumOfStages();
	const std::vector<Job>& jobs = problem.getJobs();

	TS::zero_storage();
	//first stage
	for (auto job_id : job_sequence_at_each_stage[0])
	{
		int mt = min_element(TS::m_idle_time[0].begin(), TS::m_idle_time[0].end()) - TS::m_idle_time[0].begin();
		TS::c_time[job_id][0] = TS::m_idle_time[0][mt] + jobs[job_id].getProcessTime(0);
		TS::m_idle_time[0][mt] = TS::c_time[job_id][0];
	}

	//other stage. FIFO and FAM
	for (int s = 1; s < num_of_stages; ++s)
	{
		for (int job_id : job_sequence_at_each_stage[s])
		{
			int mt = min_element(TS::m_idle_time[s].begin(), TS::m_idle_time[s].end()) - TS::m_idle_time[s].begin();
			TS::c_time[job_id][s] =
				std::max(TS::m_idle_time[s][mt], TS::c_time[job_id][s - 1])
				+ jobs[job_id].getProcessTime(s);
			TS::m_idle_time[s][mt] = TS::c_time[job_id][s];
		}
	}

	for (int s = num_of_stages - 1; s >= 0; --s)
	{
		solution_u22_e.set_job_sequence(s, job_sequence_at_each_stage[s]);
		std::sort(solution_u22_e.get_job_sequence(s).begin(), solution_u22_e.get_job_sequence(s).end(),
			[&](int job1, int job2)
			{
				return TS::c_time[job1][s] < TS::c_time[job2][s];
			}
		);
	}

	int span = solution_u22_e.evaluate();
	solution_u22_e.set_span(span);

	//return solution_u22_e;
}

SolutionU22::SolutionU22(const HFS_Problem& problem)
	: SolutionTCSNSA(problem)
{
	job_sequence_at_each_stage.resize(problem.getNumOfStages());
	for (auto& seq : job_sequence_at_each_stage)
	{
		seq.resize(problem.getNumOfJobs());
		//std::iota(std::begin(seq), std::end(seq), 0);
		//std::shuffle(std::begin(seq), std::end(seq), rand_generator());
	}
	make_span = 0;
}

SolutionU22& SolutionU22::operator=(const SolutionU22& solution)
{
	if (this == &solution)
	{
		return *this;
	}
	job_sequence_at_each_stage = solution.job_sequence_at_each_stage;
	make_span = solution.make_span;
	return *this;
}

int SolutionU22::evaluate() const
{
	int num_of_stages = problem.getNumOfStages();
	const std::vector<Job>& jobs = problem.getJobs();

	TS::zero_storage();
	//最后阶段
	for (auto it = job_sequence_at_each_stage[num_of_stages - 1].rbegin();
		 it != job_sequence_at_each_stage[num_of_stages - 1].rend(); ++it)
	{
		int job_id = *it;
		int mt = min_element(TS::m_idle_time[num_of_stages - 1].begin(), TS::m_idle_time[num_of_stages - 1].end())
				 - TS::m_idle_time[num_of_stages - 1].begin();
		TS::c_time[job_id][num_of_stages - 1] =
			TS::m_idle_time[num_of_stages - 1][mt] + jobs[job_id].getProcessTime(num_of_stages - 1);
		TS::m_idle_time[num_of_stages - 1][mt] = TS::c_time[job_id][num_of_stages - 1];
	}

	//other stage. FIFO and FAM
	for (int s = num_of_stages - 2; s >= 0; --s)
	{
		for (auto it = job_sequence_at_each_stage[s].rbegin(); it != job_sequence_at_each_stage[s].rend(); ++it)
		{
			int first_come_job_id = *it;

			int mt = min_element(TS::m_idle_time[s].begin(), TS::m_idle_time[s].end()) - TS::m_idle_time[s].begin();
			TS::c_time[first_come_job_id][s] =
				std::max(TS::m_idle_time[s][mt], TS::c_time[first_come_job_id][s + 1])
				+ jobs[first_come_job_id].getProcessTime(s);
			TS::m_idle_time[s][mt] = TS::c_time[first_come_job_id][s];
		}
	}
	return *std::max_element(TS::m_idle_time[0].begin(), TS::m_idle_time[0].end());
}

void SolutionU22::generate_expert(SolutionU21& solution_u21_e) const
{
	//SolutionU21 solution_u21_e(problem);

	int num_of_jobs = problem.getNumOfJobs();
	int num_of_stages = problem.getNumOfStages();
	const std::vector<Job>& jobs = problem.getJobs();

	//各工序的逆向完工时间
	std::vector<std::vector<int>> c_time(num_of_jobs, std::vector<int>(num_of_stages));
	//每个阶段每个机器的可用时间
	std::vector<std::vector<int>> m_idle_time(num_of_stages);
	for (int s = 0; s < problem.getNumOfStages(); s++)
	{
		m_idle_time[s].resize(problem.getNumOfMachinesInStage(s));
	}

	//最后阶段
	for (auto it = job_sequence_at_each_stage[num_of_stages - 1].rbegin();
		 it != job_sequence_at_each_stage[num_of_stages - 1].rend(); ++it)
	{
		int job_id = *it;
		int mt = min_element(m_idle_time[num_of_stages - 1].begin(), m_idle_time[num_of_stages - 1].end())
				 - m_idle_time[num_of_stages - 1].begin();
		c_time[job_id][num_of_stages - 1] =
			m_idle_time[num_of_stages - 1][mt] + jobs[job_id].getProcessTime(num_of_stages - 1);
		m_idle_time[num_of_stages - 1][mt] = c_time[job_id][num_of_stages - 1];
	}

	//other stage. FIFO and FAM
	for (int s = num_of_stages - 2; s >= 0; --s)
	{
		for (auto it = job_sequence_at_each_stage[s].rbegin(); it != job_sequence_at_each_stage[s].rend(); ++it)
		{
			int first_come_job_id = *it;

			int mt = min_element(m_idle_time[s].begin(), m_idle_time[s].end()) - m_idle_time[s].begin();
			c_time[first_come_job_id][s] =
				std::max(m_idle_time[s][mt], c_time[first_come_job_id][s + 1])
				+ jobs[first_come_job_id].getProcessTime(s);
			m_idle_time[s][mt] = c_time[first_come_job_id][s];
		}
	}

	for (int s = 0; s < num_of_stages; ++s)
	{
		solution_u21_e.set_job_sequence(s, job_sequence_at_each_stage[s]);
		std::sort(solution_u21_e.get_job_sequence(s).begin(), solution_u21_e.get_job_sequence(s).end(),
			[&](int job1, int job2)
			{
				return c_time[job1][s] > c_time[job2][s];
			}
		);
	}
	int span = solution_u21_e.evaluate();
	solution_u21_e.set_span(span);

	//return solution_u21_e;
}

double TCSNSA::m_lambda;
double TCSNSA::m_gamma;
double TCSNSA::m_alpha;
double TCSNSA::m_beta;
int TCSNSA::m_num_NS;

void TCSNSA::set_parameters(double lambda, double gamma, double alpha, double beta, int num_NS)
{
	m_lambda = lambda;
	m_gamma = gamma;
	m_alpha = alpha;
	m_beta = beta;
	m_num_NS = num_NS;
}

void TCSNSA::local_search_inner(int iteration_count, SolutionTCSNSA& solution)
{
	const HFS_Problem& problem = solution.get_problem();
	int num_of_stages = problem.getNumOfStages();
	int stage_id = wyt_rand(num_of_stages);

	std::vector<int>& sequence = solution.get_job_sequence(stage_id);
	std::vector<int> original_sequence = sequence;
	int r = wyt_rand(3);
	switch (r)
	{
	case 0:
		NeighborhoodSearch::insertion(sequence);
		break;
	case 1:
		NeighborhoodSearch::swap(sequence);
		break;
	case 2:
		NeighborhoodSearch::pairwise_exchange(sequence);
		break;
	default:
		break;
	}
	int span = solution.evaluate();
	if (span <= solution.get_span())
	{
		solution.set_span(span);
	}
	else
	{
		double rand = wyt_rand(0.0, 1.0);
		if (rand < exp(-(span - solution.get_span()) / (m_gamma * std::pow(m_alpha, iteration_count / m_beta))))
		{
			solution.set_span(span);
		}
		else
		{
			solution.set_job_sequence(stage_id, original_sequence);
		}
	}
}

/*SolutionU11 TCSNSA::local_search(int iteration_count, const SolutionU11& solution)
{
	SolutionU11 solution_prime = solution;
	local_search_inner(iteration_count, solution_prime);
	return solution_prime;
}

SolutionU12 TCSNSA::local_search(int iteration_count, const SolutionU12& solution)
{
	SolutionU12 solution_prime = solution;
	local_search_inner(iteration_count, solution_prime);
	return solution_prime;
}

SolutionU21 TCSNSA::local_search(int iteration_count, const SolutionU21& solution)
{
	SolutionU21 solution_prime = solution;
	local_search_inner(iteration_count, solution_prime);
	return solution_prime;
}

SolutionU22 TCSNSA::local_search_critical_nodes(int iteration_count, const SolutionU22& solution)
{
	SolutionU22 solution_prime = solution;
	local_search_inner(iteration_count, solution_prime);
	return solution_prime;
}*/

SolutionU21 TCSNSA::FSSE(const SolutionU11& solution_u11)
{
	const HFS_Problem& problem = solution_u11.get_problem();
	int num_of_stages = problem.getNumOfStages();
	const std::vector<Job>& jobs = problem.getJobs();

	SolutionU21 solution_u21(problem);

	TS::zero_storage();

	//first stage
	solution_u21.set_job_sequence(0, solution_u11.get_job_sequence());
	for (auto job_id : solution_u11.get_job_sequence())
	{
		int mt = min_element(TS::m_idle_time[0].begin(), TS::m_idle_time[0].end()) - TS::m_idle_time[0].begin();
		TS::c_time[job_id][0] = TS::m_idle_time[0][mt] + jobs[job_id].getProcessTime(0);
		TS::m_idle_time[0][mt] = TS::c_time[job_id][0];
	}

	TS::sequence_of_other_stage = solution_u11.get_job_sequence();
	//other stage. FIFO and FAM
	for (int s = 1; s < num_of_stages; ++s)
	{
		std::sort(TS::sequence_of_other_stage.begin(), TS::sequence_of_other_stage.end(), [&](int job1, int job2)
		{
			return TS::c_time[job1][s - 1] < TS::c_time[job2][s - 1];
		});
		solution_u21.set_job_sequence(s, TS::sequence_of_other_stage);
		for (int first_come_job_id : TS::sequence_of_other_stage)
		{
			int mt = min_element(TS::m_idle_time[s].begin(), TS::m_idle_time[s].end()) - TS::m_idle_time[s].begin();
			TS::c_time[first_come_job_id][s] =
				std::max(TS::m_idle_time[s][mt], TS::c_time[first_come_job_id][s - 1])
				+ jobs[first_come_job_id].getProcessTime(s);
			TS::m_idle_time[s][mt] = TS::c_time[first_come_job_id][s];
		}
	}
	int span = *std::max_element(TS::m_idle_time[num_of_stages - 1].begin(), TS::m_idle_time[num_of_stages - 1].end());
	solution_u21.set_span(span);
	return solution_u21;
}

SolutionU22 TCSNSA::BSSE(const SolutionU12& solution_u12)
{
	const HFS_Problem& problem = solution_u12.get_problem();
	int num_of_stages = problem.getNumOfStages();
	const std::vector<Job>& jobs = problem.getJobs();

	SolutionU22 solution_u22(problem);

	TS::zero_storage();
	//最后阶段
	solution_u22.set_job_sequence(num_of_stages - 1, solution_u12.get_job_sequence());
	for (auto it = solution_u12.get_job_sequence().rbegin(); it != solution_u12.get_job_sequence().rend(); ++it)
	{
		int job_id = *it;
		int mt = min_element(TS::m_idle_time[num_of_stages - 1].begin(), TS::m_idle_time[num_of_stages - 1].end())
				 - TS::m_idle_time[num_of_stages - 1].begin();
		TS::c_time[job_id][num_of_stages - 1] =
			TS::m_idle_time[num_of_stages - 1][mt] + jobs[job_id].getProcessTime(num_of_stages - 1);
		TS::m_idle_time[num_of_stages - 1][mt] = TS::c_time[job_id][num_of_stages - 1];
	}

	TS::sequence_of_other_stage = solution_u12.get_job_sequence();
	//other stage. FIFO and FAM
	for (int s = num_of_stages - 2; s >= 0; --s)
	{
		std::sort(TS::sequence_of_other_stage.begin(), TS::sequence_of_other_stage.end(), [&](int job1, int job2)
		{
			return TS::c_time[job1][s + 1] > TS::c_time[job2][s + 1];
		});
		solution_u22.set_job_sequence(s, TS::sequence_of_other_stage);
		for (auto it = TS::sequence_of_other_stage.rbegin(); it != TS::sequence_of_other_stage.rend(); ++it)
		{
			int first_come_job_id = *it;

			int mt = min_element(TS::m_idle_time[s].begin(), TS::m_idle_time[s].end()) - TS::m_idle_time[s].begin();
			TS::c_time[first_come_job_id][s] =
				std::max(TS::m_idle_time[s][mt], TS::c_time[first_come_job_id][s + 1])
				+ jobs[first_come_job_id].getProcessTime(s);
			TS::m_idle_time[s][mt] = TS::c_time[first_come_job_id][s];

		}
	}
	int span = *std::max_element(TS::m_idle_time[0].begin(), TS::m_idle_time[0].end());
	solution_u22.set_span(span);
	return solution_u22;
}

void TCSNSA::CNSA(int iteration_count, SolutionU21& solution_u21_prime, SolutionU22& solution_u22_prime,
	SolutionU22& solution_u22_e, SolutionU21& solution_u21_e)
{
	local_search_inner(iteration_count, solution_u21_prime);
	local_search_inner(iteration_count, solution_u22_prime);
	if (iteration_count % m_num_NS == 0)
	{
		solution_u21_prime.generate_expert(solution_u22_e);
		solution_u22_prime.generate_expert(solution_u21_e);
		if (solution_u21_e.get_span() <= solution_u21_prime.get_span())
		{
			solution_u21_prime = solution_u21_e;
		}
		if (solution_u22_e.get_span() <= solution_u22_prime.get_span())
		{
			solution_u22_prime = solution_u22_e;
		}
	}
}

int TCSNSA::Evolution(const HFS_Problem& problem)
{
	int num_of_jobs = problem.getNumOfJobs();
	int num_of_stages = problem.getNumOfStages();

	double time_limit = m_lambda * num_of_jobs * num_of_jobs * num_of_jobs * num_of_stages;

	long algorithm_start_time = GetElapsedProcessTime();

	SolutionU11 solution_u11(problem);
	int span11 = solution_u11.evaluate();
	solution_u11.set_span(span11);
	SolutionU11 best_solution_u11 = solution_u11;

	SolutionU12 solution_u12(problem);
	int span12 = solution_u12.evaluate();
	solution_u12.set_span(span12);
	SolutionU12 best_solution_u12 = solution_u12;

	SolutionU11 solution_u11_prime(problem);
	SolutionU12 solution_u12_prime(problem);

	int total_iteration_count1 = 0;
	while (true)
	{
		++total_iteration_count1;
		solution_u11_prime = solution_u11;
		local_search_inner(total_iteration_count1, solution_u11_prime);
		solution_u12_prime = solution_u12;
		local_search_inner(total_iteration_count1, solution_u12_prime);

		if (solution_u11_prime.get_span() <= best_solution_u11.get_span())
		{
			best_solution_u11 = solution_u11_prime;
			solution_u11 = solution_u11_prime;
		}

		if (solution_u12_prime.get_span() <= best_solution_u12.get_span())
		{
			best_solution_u12 = solution_u12_prime;
			solution_u12 = solution_u12_prime;
		}

		long cur_time = GetElapsedProcessTime();
		long ElapsedTime = cur_time - algorithm_start_time;

		if (ElapsedTime > time_limit / 2)
		{
			break;
		}
	}

	SolutionU21 solution_u21 = FSSE(best_solution_u11);
	SolutionU21 best_solution_u21 = solution_u21;
	SolutionU22 solution_u22 = BSSE(best_solution_u12);
	SolutionU22 best_solution_u22 = solution_u22;

	SolutionU21 solution_u21_prime(problem);
	SolutionU22 solution_u22_prime(problem);

	SolutionU22 solution_u22_e(problem);
	SolutionU21 solution_u21_e(problem);

	int total_iteration_count2 = 0;
	while (true)
	{
		++total_iteration_count2;
		solution_u21_prime = solution_u21;
		solution_u22_prime = solution_u22;

		CNSA(total_iteration_count2, solution_u21_prime, solution_u22_prime, solution_u22_e, solution_u21_e);

		if (solution_u21_prime.get_span() <= best_solution_u21.get_span())
		{
			best_solution_u21 = solution_u21_prime;
			solution_u21 = solution_u21_prime;
		}
		if (solution_u22_prime.get_span() <= best_solution_u22.get_span())
		{
			best_solution_u22 = solution_u22_prime;
			solution_u22 = solution_u22_prime;
		}

		long cur_time = GetElapsedProcessTime();
		long ElapsedTime = cur_time - algorithm_start_time;

		if (ElapsedTime > time_limit)
		{
			break;
		}
	}
	std::cout << "\titeration," << total_iteration_count1 + total_iteration_count2;
	return std::min(best_solution_u21.get_span(), best_solution_u22.get_span());
}


