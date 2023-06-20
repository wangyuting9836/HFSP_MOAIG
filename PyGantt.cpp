//
// Created by wangy on 2023/4/26.
//

#include <iostream>
#include <string>
#include <numeric>
#include <matplotlibcpp.h>
#include <iomanip>
#include <sstream>
#include "PyGantt.h"

namespace plt = matplotlibcpp;

std::vector<std::string> PyGantt::color_dict =
		{ "WhiteSmoke", "White", "SeaShell", "PapayaWhip", "MintCream", "LightYellow", "Ivory", "GhostWhite",
				"FloralWhite", "Cornsilk", "Azure", "AliceBlue", "LightBlue", "LightCoral", "LightCyan",
				"LightGoldenRodYellow", "LightGrey", "LightGreen", "LightPink", "LightSalmon", "LightSeaGreen",
				"LightSkyBlue", "LightSlateBlue", "LightSlateGray", "LightSteelBlue" };


void
PyGantt::ganttOneCriticalPath(const Solution& solution, const std::vector<Node*>& criticalPath,
		const std::string& title, int maxX, bool isInverse, bool isShow, const std::string& figureName)
{
	plt::figure_size(1000, 600);
	plt::title(title);
	const HFS_Problem& problem = solution.getProblem();
	std::map<std::pair<int, int>, int> s_m_yaxis;
	int yTickId = 0;
	int stageCount = problem.getNumOfStates();
	for (int stageId = 0; stageId < stageCount; ++stageId)
	{
		int machinCount = problem.getNumOfMachinesInStage(stageId);

		for (int machineId = 0; machineId < machinCount; ++machineId)
		{
			s_m_yaxis[{ stageId, machineId }] = yTickId;
			++yTickId;
		}
	}

	if (isInverse)
	{
		ganttBackward(solution, maxX);
		for (auto ptr : criticalPath)
		{
			auto xrange1 = static_cast<double>(maxX - ptr->tailTime - ptr->processTime);
			plt::broken_barh(xrange1, ptr->processTime, s_m_yaxis.at({ ptr->stageId, ptr->machineId }) + 0.25, 0.05,
					{{ "edgecolor", "blue" }, { "facecolor", "blue" }});
		}

	}
	else
	{
		ganttForward(solution, maxX);
		for (auto ptr : criticalPath)
		{
			auto xrange1 = static_cast<double>(ptr->startTime);
			plt::broken_barh(xrange1, ptr->processTime, s_m_yaxis.at({ ptr->stageId, ptr->machineId }) + 0.25, 0.05,
					{{ "edgecolor", "blue" }, { "facecolor", "blue" }});
		}
	}

	//ganttCritical(criticalPath, s_m_yaxis);

	if (isShow)
	{
		plt::show();
	}
	else
	{
		plt::save(figureName);
	}
	plt::close();
}

void PyGantt::ganttAllCriticalNodes(const Solution& solution, const std::vector<Node*>& allCriticalNodes,
		const std::vector<Node*>& cutNodes, const std::string& title, int maxX, bool isInverse, bool isShow,
		const std::string& figureName)
{
	plt::figure_size(1000, 600);
	plt::title(title);
	const HFS_Problem& problem = solution.getProblem();
	std::map<std::pair<int, int>, int> s_m_yaxis;
	int yTickId = 0;
	int stageCount = problem.getNumOfStates();
	for (int stageId = 0; stageId < stageCount; ++stageId)
	{
		int machinCount = problem.getNumOfMachinesInStage(stageId);

		for (int machineId = 0; machineId < machinCount; ++machineId)
		{
			s_m_yaxis[{ stageId, machineId }] = yTickId;
			++yTickId;
		}
	}

	if (isInverse)
	{
		ganttBackward(solution, maxX);
		for (const Node* node : allCriticalNodes)
		{
			auto xrange1 = static_cast<double>(maxX - node->tailTime - node->processTime);

			plt::broken_barh(xrange1, node->processTime,
					s_m_yaxis.at({ node->stageId, node->machineId }) + 0.25, 0.05,
					{{ "edgecolor", "blue" }, { "facecolor", "blue" }});
		}

		for (const Node* node : cutNodes)
		{
			auto xrange1 = static_cast<double>(maxX - node->tailTime - node->processTime);

			plt::broken_barh(xrange1, node->processTime,
					s_m_yaxis.at({ node->stageId, node->machineId }) + 0.25, 0.05,
					{{ "edgecolor", "red" }, { "facecolor", "red" }});
		}

	}
	else
	{
		ganttForward(solution, maxX);
		for (const Node* node : allCriticalNodes)
		{
			auto xrange1 = static_cast<double>(node->startTime);

			plt::broken_barh(xrange1, node->processTime,
					s_m_yaxis.at({ node->stageId, node->machineId }) + 0.25, 0.05,
					{{ "edgecolor", "blue" }, { "facecolor", "blue" }});
		}

		for (const Node* node : cutNodes)
		{
			auto xrange1 = static_cast<double>(node->startTime);

			plt::broken_barh(xrange1, node->processTime,
					s_m_yaxis.at({ node->stageId, node->machineId }) + 0.25, 0.05,
					{{ "edgecolor", "red" }, { "facecolor", "red" }});
		}
	}

	//ganttCritical(criticalPath, s_m_yaxis);

	if (isShow)
	{
		plt::show();
	}
	else
	{
		plt::save(figureName);
	}
	plt::close();
}

void PyGantt::ganttForward(const Solution& solution, int maxX)
{
	//int makeSpan = solution.getMakeSpan();
	std::vector<double> x(maxX + 1);
	std::iota(std::begin(x), std::end(x), 0);
	std::vector<double> y(maxX + 1);
	const HFS_Problem& problem = solution.getProblem();

//    std::vector<double> yTicks;
//    std::vector<std::string> yLabels;

	int yTickId = 0;
	int stageCount = problem.getNumOfStates();
	int stageId = 0;
	for (int sid = 0; sid < stageCount; ++sid)
	{
		const std::vector<Node*>& headOnMachines = solution.getHeadOnMachinesInStage(sid);
		int machineId = 0;
		for (const auto& head : headOnMachines)
		{
//            yTicks.emplace_back(yTickId);
//            yLabels.emplace_back("$s_{" + std::to_string(stageId + 1) + "}m_{" + std::to_string(machineId + 1) + "}$");
			Node* node = head->sucByMachine;
			while (node->jobId != -1)
			{
				auto xrange1 = static_cast<double>(node->startTime);

				plt::broken_barh(xrange1, node->processTime, yTickId - 0.4, 0.7,
						{{ "edgecolor", "gray" },
								{ "facecolor", color_dict[node->jobId % color_dict.size()] }});
				plt::text(static_cast<double>(xrange1 + node->processTime / 2.0), static_cast<double>(yTickId),
						"$j_{" + std::to_string(node->jobId) + "}$", {{ "verticalalignment", "center" },
								{ "horizontalalignment", "center" },
								{ "color", "black" },
								{ "fontsize", "10" }});
				plt::text(xrange1, static_cast<double>(yTickId - 0.4),
						std::to_string(node->startTime), {{ "verticalalignment", "bottom" },
								{ "horizontalalignment", "left" },
								{ "color", "black" },
								{ "fontsize", "7" },
								{ "fontweight", "semibold" }});
				plt::text(static_cast<double>(xrange1 + node->processTime), static_cast<double>(yTickId - 0.4),
						std::to_string(node->startTime + node->processTime), {{ "verticalalignment", "bottom" },
								{ "horizontalalignment", "right" },
								{ "color", "black" },
								{ "fontsize", "7" },
								{ "fontweight", "semibold" }});
				node = node->sucByMachine;
			}
			++machineId;
			++yTickId;

		}
		if (sid < stageCount - 1)
		{
			std::fill(std::begin(y), std::end(y), yTickId - 0.55);
			plt::plot(x, y, {{ "color", "b" },
					{ "linestyle", "--" }});
		}
	}
//    plt::yticks(yTicks, yLabels);
	plt::yticks(std::vector<double>{}, std::vector<std::string>{});
	plt::xticks(std::vector<double>{}, std::vector<std::string>{});
	plt::xlim(0, maxX);
}

void PyGantt::ganttBackward(const Solution& solution, int maxX)
{
	//int makeSpan = solution.getMakeSpan();
	std::vector<double> x(maxX + 1);
	std::iota(std::begin(x), std::end(x), 0);
	std::vector<double> y(maxX + 1);
	const HFS_Problem& problem = solution.getProblem();

//    std::vector<double> yTicks;
//    std::vector<std::string> yLabels;

	int yTickId = 0;
	int stageCount = problem.getNumOfStates();
	for (int sid = 0; sid < stageCount; ++sid)
	{
		const std::vector<Node*>& headOnMachines = solution.getHeadOnMachinesInStage(sid);
		int machineId = 0;


		for (const auto& head : headOnMachines)
		{
//            yTicks.emplace_back(yTickId);
//            yLabels.emplace_back("$s_{" + std::to_string(stageId + 1) + "}m_{" + std::to_string(machineId + 1) + "}$");
			Node* node = head->sucByMachine;
			while (node->jobId != -1)
			{
				auto xrange1 = static_cast<double>(maxX - node->tailTime - node->processTime);
				plt::broken_barh(xrange1, node->processTime, yTickId - 0.4, 0.7,
						{{ "edgecolor", "gray" },
								{ "facecolor", color_dict[node->jobId % color_dict.size()] }});
				plt::text(static_cast<double>(xrange1 + node->processTime / 2.0), static_cast<double>(yTickId),
						"$j_{" + std::to_string(node->jobId) + "}$", {{ "verticalalignment", "center" },
								{ "horizontalalignment", "center" },
								{ "color", "black" },
								{ "fontsize", "10" }});
				plt::text(xrange1, static_cast<double>(yTickId - 0.4),
						std::to_string(node->tailTime + node->processTime), {{ "verticalalignment", "bottom" },
								{ "horizontalalignment", "left" },
								{ "color", "black" },
								{ "fontsize", "7" },
								{ "fontweight", "semibold" }});
				plt::text(static_cast<double>(xrange1 + node->processTime), static_cast<double>(yTickId - 0.4),
						std::to_string(node->tailTime), {{ "verticalalignment", "bottom" },
								{ "horizontalalignment", "right" },
								{ "color", "black" },
								{ "fontsize", "7" },
								{ "fontweight", "semibold" }});
				node = node->sucByMachine;
			}
			++machineId;
			++yTickId;

		}
		if (sid < stageCount - 1)
		{
			std::fill(std::begin(y), std::end(y), yTickId - 0.55);
			plt::plot(x, y, {{ "color", "b" },
					{ "linestyle", "--" }});
		}
	}

//    plt::yticks(yTicks, yLabels);
	plt::yticks(std::vector<double>{}, std::vector<std::string>{});
	plt::xticks(std::vector<double>{}, std::vector<std::string>{});
	plt::xlim(0, maxX);
}

void
PyGantt::ganttCritical(const std::vector<Node*>& criticalPath, const std::map<std::pair<int, int>, int>& s_m_yaxis)
{
	for (auto ptr : criticalPath)
	{
		plt::broken_barh(ptr->startTime, ptr->processTime, s_m_yaxis.at({ ptr->stageId, ptr->machineId }) + 0.25, 0.05,
				{{ "edgecolor", "blue" },
						{ "facecolor", "blue" }});
	}
}