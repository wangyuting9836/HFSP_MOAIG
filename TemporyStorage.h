//
// Created by wangy on 2024/2/6.
//

#ifndef _TEMPORYSTORAGE_H_
#define _TEMPORYSTORAGE_H_

#include <vector>

#include "HFS_Problem.h"
class TemporyStorage
{
 public:
	static std::vector<std::vector<int>> c_time;
	static std::vector<std::vector<int>> m_idle_time;
	static std::vector<int> sequence_of_other_stage;

	static std::vector<int> U;
	static std::vector<int> CF;
	static std::vector<int> RCL;

	static void init_storage(const HFS_Problem& problem);
	static void zero_storage();
};

typedef TemporyStorage TS;

#endif //_TEMPORYSTORAGE_H_
