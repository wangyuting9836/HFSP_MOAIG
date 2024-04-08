//
// Created by wangy on 2024/1/17.
//
#include <vector>
#include <chrono>
#include <fstream>
#include <string>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <regex>

#include "PaperAlgorithm.h"
#include "IGWS.h"
#include "MOAIG.h"
#include "CSA.h"
#include "TCSNSA.h"
#include "MetaheuristicAlgorithms.h"
#include "TemporyStorage.h"

namespace fs = std::filesystem;

void sort_instances(std::vector<std::tuple<int, int, int, std::string, std::string>>& instances)
{
	std::sort(instances.begin(), instances.end(), [](const auto& ins1, const auto& ins2)
	{
		if (std::get<0>(ins1) < std::get<0>(ins2))
		{
			return true;
		}
		else if (std::get<0>(ins1) == std::get<0>(ins2))
		{
			if (std::get<1>(ins1) < std::get<1>(ins2))
			{
				return true;
			}
			else if (std::get<1>(ins1) == std::get<1>(ins2))
			{
				return std::get<2>(ins1) < std::get<2>(ins2);
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	});
}

enum Algorithm
{
	MOAIG_ALG, IGWS_ALG, CSA_ALG, TCANSA_ALG, IGRS_ALG, IGGR_ALG, IGT_ALG, IGALL_ALG, VBIH_ALG
};

void construct_csv_head(const std::vector<std::pair<Algorithm, std::string>>& algorithm_vec, std::ofstream& o_file,
	std::ofstream& o_rpd_file)
{
	std::chrono::system_clock::time_point time_point_now = std::chrono::system_clock::now();
	std::chrono::system_clock::duration duration_since_epoch
		= time_point_now.time_since_epoch();
	time_t since_epoch = duration_since_epoch.count();

	int algorithm_count = algorithm_vec.size();

	o_file.open(std::string("../result/") + std::string("1_result") + std::to_string(since_epoch) + ".csv");
	o_file << "Instance_Name" << "," << "n" << "," << "s" << "," << "no" << ",";
	o_file << "MakeSpan" << ",";
	o_file << "algorithm" << std::endl;

	o_rpd_file.open(std::string("../result/") + std::string("0_result") + std::to_string(since_epoch) + ".csv");
	o_rpd_file << "Instance_Name" << "," << "n" << "," << "s" << "," << "no" << ",";

	for (int i = 0; i < algorithm_count; ++i)
	{
		o_rpd_file << algorithm_vec[i].second + std::string("_max_rpd") << ",";
	}
	for (int i = 0; i < algorithm_count; ++i)
	{
		o_rpd_file << algorithm_vec[i].second + std::string("_min_rpd") << ",";
	}
	for (int i = 0; i < algorithm_count; ++i)
	{
		o_rpd_file << algorithm_vec[i].second + std::string("_avg_rpd") << ",";
	}

	o_rpd_file << std::endl;
}

void write_rpd(const std::tuple<int, int, int, std::string, std::string>& instance_info,
	const std::vector<std::pair<Algorithm, std::string>>& algorithm_vec,
	const std::vector<std::vector<int>>& best_span, std::ofstream& o_rpd_file)
{
	int num_of_jobs = std::get<0>(instance_info);
	int num_of_stages = std::get<1>(instance_info);
	int no = std::get<2>(instance_info);
	std::string file_name = std::get<4>(instance_info);
	o_rpd_file << file_name << "," << num_of_jobs << "," << num_of_stages << "," << no << ",";

	int algorithm_count = algorithm_vec.size();

	int g_min_span = INT32_MAX;
	for (int i = 0; i < algorithm_count; ++i)
	{
		int min_span = INT32_MAX;
		for (int k = 0; k < best_span[i].size(); ++k)
		{
			min_span = std::min(min_span, best_span[i][k]);
		}
		g_min_span = std::min(g_min_span, min_span);
	}

	for (int i = 0; i < algorithm_count; ++i)
	{
		int max_span = INT32_MIN;
		for (int k = 0; k < best_span[i].size(); ++k)
		{
			max_span = std::max(max_span, best_span[i][k]);
		}
		double max_rpd = static_cast<double>(max_span - g_min_span) / g_min_span * 100;
		o_rpd_file << std::setprecision(6) << max_rpd << ",";
	}

	for (int i = 0; i < algorithm_count; ++i)
	{
		int min_span = INT32_MAX;
		for (int k = 0; k < best_span[i].size(); ++k)
		{
			min_span = std::min(min_span, best_span[i][k]);
		}
		double min_rpd = static_cast<double>(min_span - g_min_span) / g_min_span * 100;
		o_rpd_file << std::setprecision(6) << min_rpd << ",";
	}
	for (int i = 0; i < algorithm_count; ++i)
	{
		double sum_span = 0.0;
		for (int k = 0; k < best_span[i].size(); ++k)
		{
			sum_span += best_span[i][k];
		}

		double avg_span = sum_span / best_span[i].size();
		double avg_rpd = (avg_span - g_min_span) / g_min_span * 100;
		o_rpd_file << std::setprecision(6) << avg_rpd << ",";
	}

	o_rpd_file << std::endl;
}

void run_algorithm(const HFS_Problem& problem, std::vector<int>& best_spans, int repeat_times,
	const std::string& file_name, int num_of_jobs, int num_of_stages, int no, std::ofstream& o_file, Algorithm alg,
	const std::string& algorithm_name)
{
	for (int i = 0; i < repeat_times; ++i)
	{
		int best_span;
		switch (alg)
		{
		case MOAIG_ALG:
			best_span = MOAIG::Evolution(problem);
			break;
		case IGWS_ALG:
			best_span = IGWS::Evolution(problem);
			break;
		case CSA_ALG:
			best_span = CSA::Evolution(problem);
			break;
		case TCANSA_ALG:
			best_span = TCSNSA::Evolution(problem);
			break;
		case IGRS_ALG:
			best_span = MetaheuristicAlgorithms::IG_algorithm(problem, MetaheuristicAlgorithms::IGRS);
			break;
		case IGGR_ALG:
			best_span = MetaheuristicAlgorithms::IG_algorithm(problem, MetaheuristicAlgorithms::IGGR);
			break;
		case IGT_ALG:
			best_span = MetaheuristicAlgorithms::IG_algorithm(problem, MetaheuristicAlgorithms::IGT);
			break;
		case IGALL_ALG:
			best_span = MetaheuristicAlgorithms::IG_algorithm(problem, MetaheuristicAlgorithms::IGALL);
			break;
		case VBIH_ALG:
			best_span = MetaheuristicAlgorithms::VBIH_algorithm(problem);
			break;
		}
		best_spans.emplace_back(best_span);
		std::cout << "\t" << algorithm_name
				  << "\t" << "Rep" << i + 1 << ":" << "\t"
				  << "make_span" << "," << best_span << "\t"
				  << std::endl;

		o_file << file_name << "," << num_of_jobs << "," << num_of_stages << "," << no << ",";
		o_file << best_span << ",";
		o_file << algorithm_name << std::endl;
	}
}

void experimental_ls_small_instance()
{
	std::chrono::system_clock::time_point time_point_now = std::chrono::system_clock::now();
	std::chrono::system_clock::duration duration_since_epoch
		= time_point_now.time_since_epoch();
	time_t since_epoch = duration_since_epoch.count();

	std::ofstream o_file;
	std::ofstream o_rpd_file;

	o_file.open(std::string("../result/") + std::string("1_ls_result") + std::to_string(since_epoch) + ".csv");
	o_file << "Instance_Name" << "," << "n" << "," << "s" << "," << "Instance" << ",";
	o_file << "MOEIALS_Span" << "," << "MOEIALS_Time0" << "," << std::fixed << "MOEIALS_Time1" << ","
		   << "LSWS_span" << "," << "LSWS_Time0" << "," << std::fixed << "LSWS_Time1" << ","
		   << "LSR_span" << "," << "LSR_Time0" << "," << std::fixed << "LSR_Time1" << ","
		   << "LS_span" << "," << "LS_Time0" << "," << std::fixed << "LS_Time1"
		   << std::endl;

	std::vector<std::tuple<int, int, int, std::string, std::string>> instances;
	for (const auto& file_path : fs::directory_iterator("../benchmark/Small_Size_instances"))
	{
		std::string file_name = file_path.path().stem().string();
		std::string reg_string = std::regex_replace(file_name, std::regex("_"), " ");
		std::istringstream ss(reg_string);
		std::string temp;
		int num_of_jobs, num_of_stages, no;
		ss >> temp >> num_of_jobs >> num_of_stages >> no;
		instances.emplace_back(num_of_jobs, num_of_stages, no, file_path.path().string(), file_name);
	}

	sort_instances(instances);

	int k = 0;
	for (const auto& ins : instances)
	{
		int num_of_jobs = std::get<0>(ins);
		int num_of_stages = std::get<1>(ins);
		int no = std::get<2>(ins);
		std::string file_path = std::get<3>(ins);
		std::string file_name = std::get<4>(ins);

		o_file << file_name << "," << num_of_jobs << "," << num_of_stages << "," << no << ",";

		HFS_Problem problem(file_path);
		TS::init_storage(problem);
		std::cout << k << ":" << file_name << ":" << "\t"
				  << problem.getNumOfJobs() << "*" << problem.getNumOfStages()
				  << std::endl;

		std::vector<std::tuple<int, int, double>> result;
		MOAIG::Evolution_local_search(problem, result);
		o_file << std::get<0>(result[0]) << "," << std::get<1>(result[0]) << "," << std::get<2>(result[0]) << ","
			   << std::get<0>(result[1]) << "," << std::get<1>(result[1]) << "," << std::get<2>(result[1]) << ","
			   << std::get<0>(result[2]) << "," << std::get<1>(result[2]) << "," << std::get<2>(result[2]) << ","
			   << std::get<0>(result[3]) << "," << std::get<1>(result[3]) << "," << std::get<2>(result[3])
			   << std::endl;
		++k;
	}
	o_file.close();
}

void experimental_ls_big_instance()
{
	std::chrono::system_clock::time_point time_point_now = std::chrono::system_clock::now();
	std::chrono::system_clock::duration duration_since_epoch
		= time_point_now.time_since_epoch();
	time_t since_epoch = duration_since_epoch.count();

	std::ofstream o_file;
	std::ofstream o_rpd_file;

	o_file.open(std::string("../result/") + std::string("1_ls_result") + std::to_string(since_epoch) + ".csv");
	o_file << "Instance_Name" << "," << "n" << "," << "s" << "," << "Instance" << ",";
	o_file << "MOEIALS_Span" << "," << "MOEIALS_Time0" << "," << std::fixed << "MOEIALS_Time1" << ","
		   << "LSWS_span" << "," << "LSWS_Time0" << "," << std::fixed << "LSWS_Time1" << ","
		   << "LSR_span" << "," << "LSR_Time0" << "," << std::fixed << "LSR_Time1" << ","
		   << "LS_span" << "," << "LS_Time0" << "," << std::fixed << "LS_Time1"
		   << std::endl;

	std::vector<std::tuple<int, int, int, std::string, std::string>> instances;
	for (const auto& file_path : fs::directory_iterator("../benchmark/Big_Size_instances"))
	{
		std::string file_name = file_path.path().stem().string();
		std::string reg_string = std::regex_replace(file_name, std::regex("_"), " ");
		std::istringstream ss(reg_string);
		std::string temp;
		int num_of_jobs, num_of_stages, no;
		ss >> temp >> num_of_jobs >> num_of_stages >> no;
		instances.emplace_back(num_of_jobs, num_of_stages, no, file_path.path().string(), file_name);
	}

	sort_instances(instances);

	int k = 0;
	for (const auto& ins : instances)
	{
		int num_of_jobs = std::get<0>(ins);
		int num_of_stages = std::get<1>(ins);
		int no = std::get<2>(ins);
		std::string file_path = std::get<3>(ins);
		std::string file_name = std::get<4>(ins);

		o_file << file_name << "," << num_of_jobs << "," << num_of_stages << "," << no << ",";

		HFS_Problem problem(file_path);
		TS::init_storage(problem);
		std::cout << k << ":" << file_name << ":" << "\t"
				  << problem.getNumOfJobs() << "*" << problem.getNumOfStages()
				  << std::endl;

		std::vector<std::tuple<int, int, double>> result;
		MOAIG::Evolution_local_search(problem, result);
		o_file << std::get<0>(result[0]) << "," << std::get<1>(result[0]) << "," << std::get<2>(result[0]) << ","
			   << std::get<0>(result[1]) << "," << std::get<1>(result[1]) << "," << std::get<2>(result[1]) << ","
			   << std::get<0>(result[2]) << "," << std::get<1>(result[2]) << "," << std::get<2>(result[2]) << ","
			   << std::get<0>(result[3]) << "," << std::get<1>(result[3]) << "," << std::get<2>(result[3])
			   << std::endl;
		++k;
	}
	o_file.close();
}

void experimental_small_instance_parameter_setting(double lambda, double jp, int d)
{
	int repeat_times = 5;

	MOAIG::set_parameters(lambda, 0.4, jp, d);
	IGWS::set_parameters(lambda, 0.4, 4);
	CSA::set_parameters(lambda, 3.0, 3500, 0.99, 0.5);
	TCSNSA::set_parameters(lambda, 3.0, 0.95, 3500, 10);
	MetaheuristicAlgorithms::set_parameters(lambda, 2, 0.5, 0.4, 8);

	std::vector<std::pair<Algorithm, std::string>> algorithm_vec
		= {{ MOAIG_ALG, "MOAIG" }};

	int algorithm_count = algorithm_vec.size();
	std::ofstream o_file_vec;
	std::ofstream o_rpd_file;

	construct_csv_head(algorithm_vec, o_file_vec, o_rpd_file);

	std::vector<std::tuple<int, int, int, std::string, std::string>> instances;
	for (const auto& file_path : fs::directory_iterator("../benchmark/Small_Size_instances"))
	{
		std::string file_name = file_path.path().stem().string();
		std::string reg_string = std::regex_replace(file_name, std::regex("_"), " ");
		std::istringstream ss(reg_string);
		std::string temp;
		int num_of_jobs, num_of_stages, no;
		ss >> temp >> num_of_jobs >> num_of_stages >> no;
		instances.emplace_back(num_of_jobs, num_of_stages, no, file_path.path().string(), file_name);
	}

	sort_instances(instances);

	int k = 0;
	for (const auto& ins : instances)
	{
		int num_of_jobs = std::get<0>(ins);
		int num_of_stages = std::get<1>(ins);
		int no = std::get<2>(ins);
		std::string file_path = std::get<3>(ins);
		std::string file_name = std::get<4>(ins);

		HFS_Problem problem(file_path);
		TS::init_storage(problem);
		std::cout << k << ":" << file_name << ":" << "\t"
				  << problem.getNumOfJobs() << "*" << problem.getNumOfStages()
				  << std::endl;

		std::vector<std::vector<int>> best_span(algorithm_count);
		for (int i = 0; i < algorithm_count; ++i)
		{
			run_algorithm(problem, best_span[i], repeat_times, file_name, num_of_jobs, num_of_stages, no, o_file_vec,
				algorithm_vec[i].first, algorithm_vec[i].second);
		}

		write_rpd(ins, algorithm_vec, best_span, o_rpd_file);
		++k;
	}

	o_file_vec.close();

	o_rpd_file.close();
}

void experimental_big_instance_parameter_setting(double lambda, double jp, int d)
{
	int repeat_times = 5;

	MOAIG::set_parameters(lambda, 0.4, jp, d);
	IGWS::set_parameters(lambda, 0.4, 4);
	CSA::set_parameters(lambda, 3.0, 3500, 0.99, 0.5);
	TCSNSA::set_parameters(lambda, 3.0, 0.95, 3500, 10);
	MetaheuristicAlgorithms::set_parameters(lambda, 2, 0.5, 0.4, 8);

	std::vector<std::pair<Algorithm, std::string>> algorithm_vec
		= {{ MOAIG_ALG, "MOAIG" }};

	int algorithm_count = algorithm_vec.size();
	std::ofstream o_file_vec;
	std::ofstream o_rpd_file;

	construct_csv_head(algorithm_vec, o_file_vec, o_rpd_file);

	std::vector<std::tuple<int, int, int, std::string, std::string>> instances;
	for (const auto& file_path : fs::directory_iterator("../benchmark/Big_Size_instances"))
	{
		std::string file_name = file_path.path().stem().string();
		std::string reg_string = std::regex_replace(file_name, std::regex("_"), " ");
		std::istringstream ss(reg_string);
		std::string temp;
		int num_of_jobs, num_of_stages, no;
		ss >> temp >> num_of_jobs >> num_of_stages >> no;
		instances.emplace_back(num_of_jobs, num_of_stages, no, file_path.path().string(), file_name);
	}

	sort_instances(instances);

	int k = 0;
	for (const auto& ins : instances)
	{
		int num_of_jobs = std::get<0>(ins);
		int num_of_stages = std::get<1>(ins);
		int no = std::get<2>(ins);
		std::string file_path = std::get<3>(ins);
		std::string file_name = std::get<4>(ins);

		HFS_Problem problem(file_path);
		TS::init_storage(problem);
		std::cout << k << ":" << file_name << ":" << "\t"
				  << problem.getNumOfJobs() << "*" << problem.getNumOfStages()
				  << std::endl;

		std::vector<std::vector<int>> best_span(algorithm_count);
		for (int i = 0; i < algorithm_count; ++i)
		{
			run_algorithm(problem, best_span[i], repeat_times, file_name, num_of_jobs, num_of_stages, no, o_file_vec,
				algorithm_vec[i].first, algorithm_vec[i].second);
		}

		write_rpd(ins, algorithm_vec, best_span, o_rpd_file);
		++k;
	}

	o_file_vec.close();

	o_rpd_file.close();
}

void experimental_Carlier_benchmark_parameter_setting(double lambda, double jp, int d)
{
	int repeat_times = 5;

	MOAIG::set_parameters(lambda, 0.4, jp, d);
	IGWS::set_parameters(lambda, 0.4, 4);
	CSA::set_parameters(lambda, 3.0, 3500, 0.99, 0.5);
	TCSNSA::set_parameters(lambda, 3.0, 0.95, 3500, 10);
	MetaheuristicAlgorithms::set_parameters(lambda, 2, 0.5, 0.4, 8);

	std::vector<std::pair<Algorithm, std::string>> algorithm_vec
		= {{ MOAIG_ALG, "MOAIG" }};

	int algorithm_count = algorithm_vec.size();
	std::ofstream o_file_vec;
	std::ofstream o_rpd_file;

	construct_csv_head(algorithm_vec, o_file_vec, o_rpd_file);

	std::vector<std::tuple<int, int, int, std::string, std::string>> instances;
	for (const auto& file_path : fs::directory_iterator("../benchmark/Carlier_benchmark"))
	{
		std::string file_name = file_path.path().stem().string();
		std::string reg_string = std::regex_replace(file_name, std::regex("[a-zA-Z]"), " ");
		std::istringstream ss(reg_string);
		int num_of_jobs, num_of_stages, no;
		ss >> num_of_jobs >> num_of_stages >> no;
		instances.emplace_back(num_of_jobs, num_of_stages, no, file_path.path().string(), file_name);
	}

	sort_instances(instances);

	for (int k = 0; k < instances.size(); ++k)
	{
		std::string file_name = std::get<4>(instances[k]);
		std::cout << std::setw(3) << k << ": " << std::setw(18) << file_name << ", ";
		if ((k + 1) % 4 == 0)
		{
			std::cout << std::endl;
		}
	}

	int k = 0;
	for (const auto& ins : instances)
	{
		int num_of_jobs = std::get<0>(ins);
		int num_of_stages = std::get<1>(ins);
		int no = std::get<2>(ins);
		std::string file_path = std::get<3>(ins);
		std::string file_name = std::get<4>(ins);

		HFS_Problem problem(file_path);
		TS::init_storage(problem);
		std::cout << k << ":" << file_name << ":" << "\t"
				  << problem.getNumOfJobs() << "*" << problem.getNumOfStages()
				  << std::endl;

		std::vector<std::vector<int>> best_span(algorithm_count);
		for (int i = 0; i < algorithm_count; ++i)
		{
			run_algorithm(problem, best_span[i], repeat_times, file_name, num_of_jobs, num_of_stages, no, o_file_vec,
				algorithm_vec[i].first, algorithm_vec[i].second);
		}

		write_rpd(ins, algorithm_vec, best_span, o_rpd_file);
		++k;
	}

	o_file_vec.close();
	o_rpd_file.close();
}

void parameter_setting()
{
	std::vector<double> jp_vec = { 0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0 };
	std::vector<int> d_vec = { 2, 3, 4, 5, 6 };
	std::vector<std::pair<double, int>> parameters;
	for (auto jp : jp_vec)
	{
		for (auto d : d_vec)
		{
			parameters.emplace_back(jp, d);
		}
	}
	int index = 0;
	for (auto& para : parameters)
	{
		std::cout << "No. " << std::setw(2) << index + 1 << ": (d " << std::setw(3) << std::setprecision(1)
				  << para.first << ", jp " << para.second << ") ";
		++index;
		if (index % 5 == 0)
		{
			std::cout << std::endl;
		}
	}

	for (auto & para : parameters)
	{
		//under the termination condition of 30?n?s
		experimental_small_instance_parameter_setting(0.006, para.first, para.second);
		experimental_big_instance_parameter_setting(0.006, para.first, para.second);
	}
}

void experimental_small_instance(double lambda, double jp, int d)
{
	int repeat_times = 5;

	MOAIG::set_parameters(lambda, 0.4, jp, d);
	IGWS::set_parameters(lambda, 0.4, 4);
	CSA::set_parameters(lambda, 3.0, 3500, 0.99, 0.5);
	TCSNSA::set_parameters(lambda, 3.0, 0.95, 3500, 10);
	MetaheuristicAlgorithms::set_parameters(lambda, 2, 0.5, 0.4, 8);

	std::vector<std::pair<Algorithm, std::string>> algorithm_vec
		= {
			{ IGWS_ALG, "IGWS" },
			{ IGT_ALG, "IGT" },
			{ CSA_ALG, "CSA" },
			{ TCANSA_ALG, "TCANSA" },
			{ IGRS_ALG, "IGRS" },
			{ IGGR_ALG, "IGGR" },
			{ IGALL_ALG, "IGALL" },
			{ VBIH_ALG, "VBIH" },
			{ MOAIG_ALG, "MOAIG" },
		};

	int algorithm_count = algorithm_vec.size();
	std::ofstream o_file_vec;
	std::ofstream o_rpd_file;

	construct_csv_head(algorithm_vec, o_file_vec, o_rpd_file);

	std::vector<std::tuple<int, int, int, std::string, std::string>> instances;
	for (const auto& file_path : fs::directory_iterator("../benchmark/Small_Size_Instances"))
	{
		std::string file_name = file_path.path().stem().string();
		std::string reg_string = std::regex_replace(file_name, std::regex("_"), " ");
		std::istringstream ss(reg_string);
		std::string temp;
		int num_of_jobs, num_of_stages, no;
		ss >> temp >> num_of_jobs >> num_of_stages >> no;
		instances.emplace_back(num_of_jobs, num_of_stages, no, file_path.path().string(), file_name);
	}

	sort_instances(instances);

	int k = 0;
	for (const auto& ins : instances)
	{
		int num_of_jobs = std::get<0>(ins);
		int num_of_stages = std::get<1>(ins);
		int no = std::get<2>(ins);
		std::string file_path = std::get<3>(ins);
		std::string file_name = std::get<4>(ins);

		HFS_Problem problem(file_path);
		TS::init_storage(problem);
		std::cout << k << ":" << file_name << ":" << "\t"
				  << problem.getNumOfJobs() << "*" << problem.getNumOfStages()
				  << std::endl;

		std::vector<std::vector<int>> best_span(algorithm_count);
		for (int i = 0; i < algorithm_count; ++i)
		{
			run_algorithm(problem, best_span[i], repeat_times, file_name, num_of_jobs, num_of_stages, no, o_file_vec,
				algorithm_vec[i].first, algorithm_vec[i].second);
		}

		write_rpd(ins, algorithm_vec, best_span, o_rpd_file);
		++k;
	}

	o_file_vec.close();
	o_rpd_file.close();
}

void experimental_big_instance(double lambda, double jp, int d)
{
	int repeat_times = 5;

	MOAIG::set_parameters(lambda, 0.4, jp, d);
	IGWS::set_parameters(lambda, 0.4, 4);
	CSA::set_parameters(lambda, 3.0, 3500, 0.99, 0.5);
	TCSNSA::set_parameters(lambda, 3.0, 0.95, 3500, 10);
	MetaheuristicAlgorithms::set_parameters(lambda, 2, 0.5, 0.4, 8);

	std::vector<std::pair<Algorithm, std::string>> algorithm_vec
		= {
			{ IGWS_ALG, "IGWS" },
			{ IGT_ALG, "IGT" },
			{ CSA_ALG, "CSA" },
			{ TCANSA_ALG, "TCANSA" },
			{ IGRS_ALG, "IGRS" },
			{ IGGR_ALG, "IGGR" },
			{ IGALL_ALG, "IGALL" },
			{ VBIH_ALG, "VBIH" },
			{ MOAIG_ALG, "MOAIG" },
		};
	int algorithm_count = algorithm_vec.size();
	std::ofstream o_file_vec;
	std::ofstream o_rpd_file;

	construct_csv_head(algorithm_vec, o_file_vec, o_rpd_file);

	std::vector<std::tuple<int, int, int, std::string, std::string>> instances;
	for (const auto& file_path : fs::directory_iterator("../benchmark/Big_Size_Instances"))
	{
		std::string file_name = file_path.path().stem().string();
		std::string reg_string = std::regex_replace(file_name, std::regex("_"), " ");
		std::istringstream ss(reg_string);
		std::string temp;
		int num_of_jobs, num_of_stages, no;
		ss >> temp >> num_of_jobs >> num_of_stages >> no;
		instances.emplace_back(num_of_jobs, num_of_stages, no, file_path.path().string(), file_name);
	}

	sort_instances(instances);

	int k = 0;
	for (const auto& ins : instances)
	{
		int num_of_jobs = std::get<0>(ins);
		int num_of_stages = std::get<1>(ins);
		int no = std::get<2>(ins);
		std::string file_path = std::get<3>(ins);
		std::string file_name = std::get<4>(ins);

		HFS_Problem problem(file_path);
		TS::init_storage(problem);
		std::cout << k << ":" << file_name << ":" << "\t"
				  << problem.getNumOfJobs() << "*" << problem.getNumOfStages()
				  << std::endl;

		std::vector<std::vector<int>> best_span(algorithm_count);
		for (int i = 0; i < algorithm_count; ++i)
		{
			run_algorithm(problem, best_span[i], repeat_times, file_name, num_of_jobs, num_of_stages, no, o_file_vec,
				algorithm_vec[i].first, algorithm_vec[i].second);
		}

		write_rpd(ins, algorithm_vec, best_span, o_rpd_file);
		++k;
	}

	o_file_vec.close();

	o_rpd_file.close();
}

void experimental_Carlier_benchmark(double lambda, double jp, int d)
{
	int repeat_times = 5;

	MOAIG::set_parameters(lambda, 0.4, jp, d);
	IGWS::set_parameters(lambda, 0.4, 4);
	CSA::set_parameters(lambda, 3.0, 3500, 0.99, 0.5);
	TCSNSA::set_parameters(lambda, 3.0, 0.95, 3500, 10);
	MetaheuristicAlgorithms::set_parameters(lambda, 2, 0.5, 0.4, 8);

	std::vector<std::pair<Algorithm, std::string>> algorithm_vec
		= {
			{ IGWS_ALG, "IGWS" },
			{ IGT_ALG, "IGT" },
			{ CSA_ALG, "CSA" },
			{ TCANSA_ALG, "TCANSA" },
			{ IGRS_ALG, "IGRS" },
			{ IGGR_ALG, "IGGR" },
			{ IGALL_ALG, "IGALL" },
			{ VBIH_ALG, "VBIH" },
			{ MOAIG_ALG, "MOAIG" },
		};
	int algorithm_count = algorithm_vec.size();
	std::ofstream o_file_vec;
	std::ofstream o_rpd_file;

	construct_csv_head(algorithm_vec, o_file_vec, o_rpd_file);

	std::vector<std::tuple<int, int, int, std::string, std::string>> instances;
	for (const auto& file_path : fs::directory_iterator("../benchmark/Carlier_benchmark"))
	{
		std::string file_name = file_path.path().stem().string();
		std::string reg_string = std::regex_replace(file_name, std::regex("[a-zA-Z]"), " ");
		std::istringstream ss(reg_string);
		int num_of_jobs, num_of_stages, no;
		ss >> num_of_jobs >> num_of_stages >> no;
		instances.emplace_back(num_of_jobs, num_of_stages, no, file_path.path().string(), file_name);
	}

	sort_instances(instances);

	for (int k = 0; k < instances.size(); ++k)
	{
		std::string file_name = std::get<4>(instances[k]);
		std::cout << std::setw(3) << k << ": " << std::setw(18) << file_name << ", ";
		if ((k + 1) % 4 == 0)
		{
			std::cout << std::endl;
		}
	}

	int k = 0;
	for (const auto& ins : instances)
	{
		int num_of_jobs = std::get<0>(ins);
		int num_of_stages = std::get<1>(ins);
		int no = std::get<2>(ins);
		std::string file_path = std::get<3>(ins);
		std::string file_name = std::get<4>(ins);

		HFS_Problem problem(file_path);
		TS::init_storage(problem);
		std::cout << k << ":" << file_name << ":" << "\t"
				  << problem.getNumOfJobs() << "*" << problem.getNumOfStages()
				  << std::endl;

		std::vector<std::vector<int>> best_span(algorithm_count);
		for (int i = 0; i < algorithm_count; ++i)
		{
			run_algorithm(problem, best_span[i], repeat_times, file_name, num_of_jobs, num_of_stages, no, o_file_vec,
				algorithm_vec[i].first, algorithm_vec[i].second);
		}

		write_rpd(ins, algorithm_vec, best_span, o_rpd_file);
		++k;
	}

	o_file_vec.close();
	o_rpd_file.close();
}

