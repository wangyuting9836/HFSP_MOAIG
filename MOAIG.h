//
// Created by wangy on 2024/1/17.
//

#ifndef _MOAIG_H_
#define _MOAIG_H_

#include "HFS_Problem.h"
#include "Solution.h"
class MOAIG
{
	static double m_lambda;
	static double m_T;
	static double m_jP;
	static int m_d;
 public:
	//The algorithm 4 (MOAIG) in the paper
	static int Evolution(const HFS_Problem& problem);
	//The algorithm 5 (NEH) in the paper
	static int NEH(const std::vector<int>& seed_job_sequence, Solution& solution);
	//The algorithm 6 (ImprovementInsertionLocalSearch) in the paper
	static int iterative_improvement_insertion_ls(Solution& solution, int old_span);
	//The algorithm 7 (Destruction) in the paper
	static void destruction(Solution& solution, std::vector<int>& erased_jobs);
	//The algorithm 8 (Construction) in the paper
	static int construction(Solution& solution, std::vector<int>& erased_jobs);
	//The algorithm 9 (Construction) in the paper
	static int RIS_local_search(Solution& solution, int old_span, const Solution& best_solution);
	//The algorithm 10 (RISLocalSearch) in the paper
	static int RSS_local_search(Solution& solution, int old_span, const Solution& best_solution);
	static void set_parameters(double lambda, double t, double jp, int d);

	//Testing the performance of local search (Section 6.3)
	static void Evolution_local_search(const HFS_Problem& problem, std::vector<std::tuple<int, int, double>>& result);

	[[nodiscard]] static double get_lambda();
	[[nodiscard]] static double get_T();
	[[nodiscard]] static int get_d();
};

inline void MOAIG::set_parameters(double lambda, double t, double jp, int d)
{
	m_lambda = lambda;
	m_T = t;
	m_jP = jp;
	m_d = d;
}

inline double MOAIG::get_lambda()
{
	return m_lambda;
}

inline double MOAIG::get_T()
{
	return m_T;
}

inline int MOAIG::get_d()
{
	return m_d;
}

#endif //_MOAIG_H_
