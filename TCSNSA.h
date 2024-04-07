//
// Created by wangy on 2024/1/24.
//

/*
 *Kuang, Y., Wu, X., Chen, Z., & Li, W. (2024). A two-stage cross-neighborhood search algorithm bridging different solution representation spaces for solving the hybrid flow shop scheduling problem. Swarm and Evolutionary Computation, 84, 101455. https://doi.org/10.1016/j.swevo.2023.101455
*/

#ifndef _TCSNSA_H_
#define _TCSNSA_H_

#include <vector>
#include "HFS_Problem.h"

class NeighborhoodSearch
{
 public:
	static void insertion(std::vector<int>& sequence);
	static void swap(std::vector<int>& sequence);
	static void pairwise_exchange(std::vector<int>& sequence);
};

class SolutionTCSNSA;
class SolutionU11;
class SolutionU12;
class SolutionU21;
class SolutionU22;

class SolutionTCSNSA
{
 protected:
	const HFS_Problem& problem;
	int make_span;
 public:
	explicit SolutionTCSNSA(const HFS_Problem& problem);
	[[nodiscard]] virtual int evaluate() const = 0;
	[[nodiscard]] virtual std::vector<int>& get_job_sequence(int stage_id) = 0;
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
	virtual void set_job_sequence(int stage_id, const std::vector<int>& sequence) = 0;
};

class SolutionU11 : public SolutionTCSNSA
{
	std::vector<int> job_sequence;
 public:
	explicit SolutionU11(const HFS_Problem& problem);
	SolutionU11(const SolutionU11& solution) = default;
	SolutionU11& operator=(const SolutionU11& solution);
	[[nodiscard]] int evaluate() const override;
	[[nodiscard]] const std::vector<int>& get_job_sequence() const
	{
		return job_sequence;
	}
	[[nodiscard]] std::vector<int>& get_job_sequence(int stage_id) override
	{
		return job_sequence;
	}
	void set_job_sequence(int stage_id, const std::vector<int>& sequence) override
	{
		job_sequence = sequence;
	}
};

class SolutionU12 : public SolutionTCSNSA
{
	std::vector<int> job_sequence;
 public:
	explicit SolutionU12(const HFS_Problem& problem);
	SolutionU12(const SolutionU12& solution) = default;
	SolutionU12& operator=(const SolutionU12& solution);
	[[nodiscard]] int evaluate() const override;
	[[nodiscard]] const std::vector<int>& get_job_sequence() const
	{
		return job_sequence;
	}
	[[nodiscard]] std::vector<int>& get_job_sequence(int stage_id) override
	{
		return job_sequence;
	}
	void set_job_sequence(int stage_id, const std::vector<int>& sequence) override
	{
		job_sequence = sequence;
	}
};

class SolutionU21 : public SolutionTCSNSA
{
	std::vector<std::vector<int>> job_sequence_at_each_stage;
 public:
	explicit SolutionU21(const HFS_Problem& problem);
	SolutionU21(const SolutionU21& solution) = default;
	SolutionU21& operator=(const SolutionU21& solution);
	[[nodiscard]] int evaluate() const override;
	void generate_expert(SolutionU22& solution_u22_e) const;
	[[nodiscard]] const std::vector<std::vector<int>>& get_job_sequence() const
	{
		return job_sequence_at_each_stage;
	}
	[[nodiscard]] const std::vector<int>& get_job_sequence(int stage_id) const
	{
		return job_sequence_at_each_stage[stage_id];
	}
	[[nodiscard]] std::vector<int>& get_job_sequence(int stage_id) override
	{
		return job_sequence_at_each_stage[stage_id];
	}
	void set_job_sequence(int stage_id, const std::vector<int>& sequence) override
	{
		job_sequence_at_each_stage[stage_id] = sequence;
	}
};

class SolutionU22 : public SolutionTCSNSA
{
	std::vector<std::vector<int>> job_sequence_at_each_stage;
 public:
	explicit SolutionU22(const HFS_Problem& problem);
	SolutionU22(const SolutionU22& solution) = default;
	SolutionU22& operator=(const SolutionU22& solution);
	[[nodiscard]] int evaluate() const override;
	void generate_expert(SolutionU21& solution_u21_e) const;
	[[nodiscard]] const std::vector<std::vector<int>>& get_job_sequence() const
	{
		return job_sequence_at_each_stage;
	}
	[[nodiscard]] const std::vector<int>& get_job_sequence(int stage_id) const
	{
		return job_sequence_at_each_stage[stage_id];
	}
	[[nodiscard]] std::vector<int>& get_job_sequence(int stage_id) override
	{
		return job_sequence_at_each_stage[stage_id];
	}
	void set_job_sequence(int stage_id, const std::vector<int>& sequence) override
	{
		job_sequence_at_each_stage[stage_id] = sequence;
	}
};

class TCSNSA
{
	static double m_lambda;
	static double m_gamma;
	static double m_alpha;
	static double m_beta;
	static int m_num_NS;
 public:
	static void set_parameters(double lambda, double gamma, double alpha, double beta, int num_NS);
	static int Evolution(const HFS_Problem& problem);

	//static SolutionU11 local_search_critical_nodes(int iteration_count, const SolutionU11& solution);
	//static SolutionU12 local_search_critical_nodes(int iteration_count, const SolutionU12& solution);
	//static SolutionU21 local_search_critical_nodes(int iteration_count, const SolutionU21& solution);
	//static SolutionU22 local_search_critical_nodes(int iteration_count, const SolutionU22& solution);

	static SolutionU21 FSSE(const SolutionU11& solution_u11);
	static SolutionU22 BSSE(const SolutionU12& solution_u12);

	static void CNSA(int iteration_count, SolutionU21& solution_u21_prime, SolutionU22& solution_u22_prime,
		SolutionU22& solution_u22_e, SolutionU21& solution_u21_e);
 private:
	static void local_search_inner(int iteration_count, SolutionTCSNSA& solution);
};

#endif //_TCSNSA_H_
