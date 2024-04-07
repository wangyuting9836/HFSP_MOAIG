//
// Created by wangy on 2024/1/23.
//

/*
 *Lin, S.-W., Cheng, C.-Y., Pourhejazy, P., Ying, K.-C., & Lee, C.-H. (2021). New benchmark algorithm for hybrid flowshop scheduling with identical machines. Expert Systems with Applications, 183, 115422. https://doi.org/10.1016/j.eswa.2021.115422
*/

#ifndef _CSA_H_
#define _CSA_H_

#include <vector>
#include "HFS_Problem.h"
class SolutionCSA
{
	const HFS_Problem& problem;
	std::vector<std::vector<int>> job_sequence_at_each_stage;
	int d_code;
	int os_code;

 public:
	explicit SolutionCSA(const HFS_Problem& problem);
	SolutionCSA& operator=(const SolutionCSA& solutionCsa);
	int list_scheduling_forward();
	int permutation_scheduling_forward();
	int non_permutation_scheduling_forward();
	int list_scheduling_backward();
	int permutation_scheduling_backward();
	int non_permutation_scheduling_backward();
	void swap(int stage_index);
	void insertion(int stage_index);
	void reversion(int stage_index);
	void neighborhood_search();
	void mutation_d_code();
	void mutation_os_code();
	[[nodiscard]] int evaluate();
	void set_d_code(int code)
	{
		d_code = code;
	}
	void set_os_code(int code)
	{
		os_code = code;
	}
	void set_job_sequence(int stage_id, const std::vector<int>& sequence)
	{
		job_sequence_at_each_stage[stage_id] = sequence;
	}
};

class CSA
{
	static double m_t_initial;
	static int m_i_iter;
	static double m_alpha;
	static double m_lambda;
	static double m_c_factor;
 public:
	static void set_parameters(double lambda, double t_initial, int i_iter, double alpha, double c_factor);

	static int Evolution(const HFS_Problem& problem);
};

#endif //_CSA_H_
