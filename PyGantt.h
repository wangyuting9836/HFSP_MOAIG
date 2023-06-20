//
// Created by wangy on 2023/4/26.
//

#ifndef HFSP_CRITICAL_PATH_PYGANTT_H
#define HFSP_CRITICAL_PATH_PYGANTT_H

#include <map>
#include "Solution.h"

class PyGantt
{
	static std::vector<std::string> color_dict;

	static void ganttForward(const Solution& solution, int maxX);

	static void ganttBackward(const Solution& solution, int maxX);

	static void
	ganttCritical(const std::vector<Node*>& criticalPath, const std::map<std::pair<int, int>, int>& s_m_yaxis);

public:
	static void
	ganttOneCriticalPath(const Solution& solution, const std::vector<Node*>& criticalPath, const std::string& title,
			int maxX, bool isInverse = false, bool isShow = true, const std::string& figureName = "_tem_fig");

	static void
	ganttAllCriticalNodes(const Solution& solution, const std::vector<Node*>& allCriticalNodes,
			const std::vector<Node*>& cutNodes, const std::string& title, int maxX, bool isInverse, bool isShow,
			const std::string& figureName = "_tem_fig");
};


#endif //HFSP_CRITICAL_PATH_PYGANTT_H
