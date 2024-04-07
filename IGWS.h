//
// Created by wangy on 2024/1/13.
//

/*
 *Fernandez-Viagas, V. (2022). A speed-up procedure for the hybrid flow shop scheduling problem. Expert Systems with Applications, 187, 115903. https://doi.org/10.1016/j.eswa.2021.115903
*/

#ifndef _IGWS_H_
#define _IGWS_H_

#include "HFS_Problem.h"
#include "Solution.h"
class IGWS
{
	static double m_lambda;
	static double m_T;
	static int m_d;
 public:
	static int Evolution(const HFS_Problem& problem);

	static void set_parameters(double lambda, double t, int d);
 private:
	static int NEH_f(const std::vector<int>& seed_job_sequence, Solution& solution);
	static int iterative_improvement_insertion(Solution& solution, int old_span);
	static void destruction(Solution& solution, std::vector<int>& erased_jobs);
	static int construction(Solution& solution, std::vector<int>& erased_jobs);
};

inline void IGWS::set_parameters(double lambda, double t, int d)
{
	m_lambda = lambda;
	m_T = t;
	m_d = d;
}

#endif //_IGWS_H_
