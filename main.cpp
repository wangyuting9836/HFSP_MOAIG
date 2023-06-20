#include <iostream>
#include "HFS_Problem.h"
#include "Solution.h"
#include "PyGantt.h"

#include <iomanip>
#include <sstream>
#include <fstream>
#include <numeric>
#include <chrono>

void writeFile(const HFS_Problem &problem, const std::vector<int> &jobSequence, int fileId) {
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(5) << fileId;
    std::ofstream pFOut(std::string("../data/") + std::string("p") + oss.str() + std::string(".txt"));
    pFOut << problem.getNumOfStates() << "\t" << problem.getNumOfJobs() << std::endl;
    for (int s = 0; s < problem.getNumOfStates(); ++s) {
        pFOut << problem.getNumOfMachinesInStage(s) << std::endl;
    }
    for (const auto &job: problem.getJobs()) {
        for (int s = 0; s < problem.getNumOfStates(); ++s) {
            pFOut << job.getProcessTime(s) << "\t";
        }
        pFOut << std::endl;
    }

    std::for_each(std::begin(jobSequence), std::end(jobSequence), [&](const auto &v) {
        pFOut << v << "\t";
    });
    pFOut.close();

    std::ofstream sFOut(std::string("../data/") + std::string("p") + oss.str() + std::string("-s.txt"));
    for (const auto &job: jobSequence) {
        sFOut << job << "\t";
    }
    sFOut.close();
}

bool optimalInsertPosition(Solution &solution, Node *deleteCriticalNode) {
    //关键路径的第一个和最后一个，无需处理
    if (deleteCriticalNode->sucByMachine->jobId == -1 || deleteCriticalNode->preByMachine->jobId == -1) {
        return false;
    }

    const HFS_Problem &problem = solution.getProblem();
    Node *beforeDeleteCriticalNode = deleteCriticalNode->preByMachine;
    Node *afterDeleteCriticalNode = deleteCriticalNode->sucByMachine;

    int originalMakeSpan = solution.getMakeSpan();

    int makeSpanAfterDeleteNode = solution.deleteNodeWithRefresh(deleteCriticalNode);


    if (makeSpanAfterDeleteNode == originalMakeSpan) {
        //恢复
        solution.moveNode(deleteCriticalNode, beforeDeleteCriticalNode);
        Solution::refreshStartTimeFromNode(deleteCriticalNode);
        Solution::refreshTailTimeFromNode(deleteCriticalNode);
        solution.setMakeSpan(originalMakeSpan);
        return false;
    }

    //PyGantt::ganttOneCriticalPath(solution, { }, originalMakeSpan, false, false, "243");
    //PyGantt::ganttOneCriticalPath(solution, { }, originalMakeSpan, true, false, "244");

    //构造集合
    const auto &heads = solution.getHeadOnMachinesInStage(deleteCriticalNode->stageId);
    const auto &tails = solution.getTailOnMachinesInStage(deleteCriticalNode->stageId);

    int machineCount = problem.getNumOfMachinesInStage(deleteCriticalNode->stageId);
    for (int mid = 0; mid < machineCount; ++mid) {
        int r_num = 0;
        Node *fNode = heads[mid];
        while (fNode->sucByMachine->jobId != -1) {
            if (fNode->sucByMachine->startTime + fNode->sucByMachine->processTime > deleteCriticalNode->startTime) {
                break;
            }
            fNode = fNode->sucByMachine;
            ++r_num;
        }

        int l_num = 0;
        Node *bNode = tails[mid];
        while (bNode->preByMachine->jobId != -1) {
            if (bNode->preByMachine->tailTime + bNode->preByMachine->processTime > deleteCriticalNode->tailTime) {
                break;
            }
            bNode = bNode->preByMachine;
            ++l_num;
        }
        int jobCount = solution.getJobCount(deleteCriticalNode->stageId, mid);

        if (l_num + r_num >= jobCount) {
            //交集不为空
            int longestValue = deleteCriticalNode->startTime + deleteCriticalNode->tailTime
                               + deleteCriticalNode->processTime;
            int makeSpanAfterInsert = std::max(longestValue, makeSpanAfterDeleteNode);
            if (makeSpanAfterInsert < originalMakeSpan) {
                //找到合适位置，插入afterNode之后
                solution.moveNode(deleteCriticalNode, fNode);
                solution.setMakeSpan(makeSpanAfterInsert);
                //画图用，下面可以不需要，因为makeSpan已经计算出来
                //Solution::refreshStartTimeFromNode(deleteCriticalNode);
                return true;
            }
        } else {
            //交集为空
            Node *behindThisNode = fNode;
            while (behindThisNode != bNode) {
                int longestValue =
                        std::max(deleteCriticalNode->startTime,
                                 behindThisNode->startTime +
                                 behindThisNode->processTime)
                        +
                        std::max(deleteCriticalNode->tailTime,
                                 behindThisNode->sucByMachine->tailTime +
                                 behindThisNode->sucByMachine->processTime)
                        + deleteCriticalNode->processTime;
                int makeSpanAfterInsert = std::max(longestValue, makeSpanAfterDeleteNode);
                if (makeSpanAfterInsert < originalMakeSpan) {
                    //找到合适位置，插入afterNode之后
                    solution.moveNode(deleteCriticalNode, behindThisNode);
                    solution.setMakeSpan(makeSpanAfterInsert);
                    //画图用，下面可以不需要，因为makeSpan已经计算出来
                    //Solution::refreshStartTimeFromNode(deleteCriticalNode);
                    return true;
                }
                behindThisNode = behindThisNode->sucByMachine;
            }
        }
    }
    //不能变好，恢复
    solution.moveNode(deleteCriticalNode, beforeDeleteCriticalNode);
    Solution::refreshStartTimeFromNode(deleteCriticalNode);
    Solution::refreshTailTimeFromNode(deleteCriticalNode);
    solution.setMakeSpan(originalMakeSpan);
    return false;
}

bool optimalInsertPositionNoGlobalRefresh(Solution &solution, Node *deleteCriticalNode) {
    //关键路径的第一个和最后一个，无需处理
    if (deleteCriticalNode->sucByMachine->jobId == -1 || deleteCriticalNode->preByMachine->jobId == -1) {
        return false;
    }

    const HFS_Problem &problem = solution.getProblem();
    Node *beforeDeleteCriticalNode = deleteCriticalNode->preByMachine;
    Node *afterDeleteCriticalNode = deleteCriticalNode->sucByMachine;

    int originalMakeSpan = solution.getMakeSpan();
    solution.deleteNodeWithoutRefresh(deleteCriticalNode);

    //构造集合
    const auto &heads = solution.getHeadOnMachinesInStage(deleteCriticalNode->stageId);
    const auto &tails = solution.getTailOnMachinesInStage(deleteCriticalNode->stageId);

    int machineCount = problem.getNumOfMachinesInStage(deleteCriticalNode->stageId);
    for (int mid = 0; mid < machineCount; ++mid) {
        int r_num = 0;
        Node *fNode = heads[mid];
        while (fNode->sucByMachine->jobId != -1) {
            if (fNode->sucByMachine->startTime + fNode->sucByMachine->processTime > deleteCriticalNode->startTime) {
                break;
            }
            fNode = fNode->sucByMachine;
            ++r_num;
        }

        int l_num = 0;
        Node *bNode = tails[mid];
        while (bNode->preByMachine->jobId != -1) {
            if (bNode->preByMachine->tailTime + bNode->preByMachine->processTime > deleteCriticalNode->tailTime) {
                break;
            }
            bNode = bNode->preByMachine;
            ++l_num;
        }
        int jobCount = solution.getJobCount(deleteCriticalNode->stageId, mid);

        if (l_num + r_num >= jobCount) {
            //交集不为空
            int longestValue = deleteCriticalNode->startTime + deleteCriticalNode->tailTime
                               + deleteCriticalNode->processTime;
            if (longestValue < originalMakeSpan) {
                //找到合适位置，插入startNode之后
                solution.moveNode(deleteCriticalNode, fNode);
                if (beforeDeleteCriticalNode->machineId != mid) {
                    Solution::refreshStartTimeFromNode(afterDeleteCriticalNode);
                    Solution::refreshStartTimeFromNode(deleteCriticalNode);
                    //Solution::refreshTailTimeFromNode(beforeDeleteCriticalNode);
                    //Solution::refreshTailTimeFromNode(deleteCriticalNode);
                } else {
                    if (fNode->machineIndex > beforeDeleteCriticalNode->machineIndex) {
                        Solution::refreshStartTimeFromNode(afterDeleteCriticalNode);
                        //Solution::refreshTailTimeFromNode(deleteCriticalNode);
                    } else if (fNode->machineIndex < beforeDeleteCriticalNode->machineIndex) {
                        Solution::refreshStartTimeFromNode(deleteCriticalNode);
                        //Solution::refreshTailTimeFromNode(beforeDeleteCriticalNode);
                    } else {
                        std::cout << "error";
                    }
                }
                solution.reCalculateMakeSpan();
                return true;
            }
        } else {
            //交集为空
            if (deleteCriticalNode->machineId != mid) {
                Node *behindThisNode = fNode;
                while (behindThisNode != bNode) {
                    int longestValue =
                            std::max(deleteCriticalNode->startTime,
                                     behindThisNode->startTime +
                                     behindThisNode->processTime)
                            +
                            std::max(deleteCriticalNode->tailTime,
                                     behindThisNode->sucByMachine->tailTime +
                                     behindThisNode->sucByMachine->processTime)
                            + deleteCriticalNode->processTime;
                    if (longestValue < originalMakeSpan) {
                        //找到合适位置，插入startNode之后，计算makeSpan
                        solution.moveNode(deleteCriticalNode, behindThisNode);
                        Solution::refreshStartTimeFromNode(afterDeleteCriticalNode);
                        Solution::refreshStartTimeFromNode(deleteCriticalNode);
                        //Solution::refreshTailTimeFromNode(beforeDeleteCriticalNode);
                        //Solution::refreshTailTimeFromNode(deleteCriticalNode);
                        solution.reCalculateMakeSpan();
                        return true;
                    }
                    behindThisNode = behindThisNode->sucByMachine;
                }
            } else {
                if (deleteCriticalNode->machineIndex < fNode->machineIndex) {
                    Node *startNode = afterDeleteCriticalNode;
                    int startTime = beforeDeleteCriticalNode->startTime;
                    while (startNode != fNode) {
                        startTime = std::max(startTime + startNode->preByMachine->processTime,
                                             startNode->preByStage->startTime + startNode->preByStage->processTime);
                        startNode = startNode->sucByMachine;
                    }

                    while (startNode != bNode) {
                        startTime = std::max(startTime + startNode->preByMachine->processTime,
                                             startNode->preByStage->startTime + startNode->preByStage->processTime);

                        int longestValue =
                                std::max(deleteCriticalNode->startTime, startTime + startNode->processTime)
                                +
                                std::max(deleteCriticalNode->tailTime,
                                         startNode->sucByMachine->tailTime +
                                         startNode->sucByMachine->processTime)
                                + deleteCriticalNode->processTime;
                        if (longestValue < originalMakeSpan) {
                            //找到合适位置，插入afterNode之后
                            solution.moveNode(deleteCriticalNode, startNode);
                            Solution::refreshStartTimeFromNode(afterDeleteCriticalNode);
                            //Solution::refreshTailTimeFromNode(deleteCriticalNode);
                            solution.reCalculateMakeSpan();
                            return false;
                        }
                        startNode = startNode->sucByMachine;
                    }
                } else if (deleteCriticalNode->machineIndex > bNode->machineIndex) {
                    Node *startNode = beforeDeleteCriticalNode;
                    int tailTime = afterDeleteCriticalNode->tailTime;
                    while (startNode != bNode) {
                        tailTime = std::max(tailTime + startNode->sucByMachine->processTime,
                                            startNode->sucByStage->tailTime + startNode->sucByStage->processTime);
                        startNode = startNode->preByMachine;
                    }

                    while (startNode != fNode) {
                        tailTime = std::max(tailTime + startNode->sucByMachine->processTime,
                                            startNode->sucByStage->tailTime + startNode->sucByStage->processTime);

                        int longestValue =
                                std::max(deleteCriticalNode->tailTime, tailTime + startNode->processTime)
                                +
                                std::max(deleteCriticalNode->startTime,
                                         startNode->preByMachine->startTime +
                                         startNode->preByMachine->processTime)
                                + deleteCriticalNode->processTime;
                        if (longestValue < originalMakeSpan) {
                            //找到合适位置，插入afterNode之后
                            solution.moveNode(deleteCriticalNode, startNode->preByMachine);
                            Solution::refreshStartTimeFromNode(deleteCriticalNode);
                            //Solution::refreshTailTimeFromNode(beforeDeleteCriticalNode);
                            solution.reCalculateMakeSpan();
                            return false;
                        }
                        startNode = startNode->preByMachine;
                    }
                } else {
                    Node *startNode;

                    startNode = afterDeleteCriticalNode;
                    int startTime = beforeDeleteCriticalNode->startTime;
                    while (startNode != bNode) {
                        startTime = std::max(startTime + startNode->preByMachine->processTime,
                                             startNode->preByStage->startTime + startNode->preByStage->processTime);

                        int longestValue =
                                std::max(deleteCriticalNode->startTime, startTime + startNode->processTime)
                                +
                                std::max(deleteCriticalNode->tailTime,
                                         startNode->sucByMachine->tailTime +
                                         startNode->sucByMachine->processTime)
                                + deleteCriticalNode->processTime;
                        if (longestValue < originalMakeSpan) {
                            //找到合适位置，插入startNode之后
                            solution.moveNode(deleteCriticalNode, startNode);
                            Solution::refreshStartTimeFromNode(afterDeleteCriticalNode);
                            //Solution::refreshTailTimeFromNode(deleteCriticalNode);
                            solution.reCalculateMakeSpan();
                            return true;
                        }
                        startNode = startNode->sucByMachine;
                    }

                    startNode = beforeDeleteCriticalNode;
                    int tailTime = afterDeleteCriticalNode->tailTime;
                    while (startNode != fNode) {
                        tailTime = std::max(tailTime + startNode->sucByMachine->processTime,
                                            startNode->sucByStage->tailTime + startNode->sucByStage->processTime);

                        int longestValue =
                                std::max(deleteCriticalNode->tailTime, tailTime + startNode->processTime)
                                +
                                std::max(deleteCriticalNode->startTime,
                                         startNode->preByMachine->startTime +
                                         startNode->preByMachine->processTime)
                                + deleteCriticalNode->processTime;
                        if (longestValue < originalMakeSpan) {
                            //找到合适位置，插入startNode之后
                            solution.moveNode(deleteCriticalNode, startNode->preByMachine);
                            Solution::refreshStartTimeFromNode(deleteCriticalNode);
                            //Solution::refreshTailTimeFromNode(beforeDeleteCriticalNode);
                            solution.reCalculateMakeSpan();
                            return true;
                        }
                        startNode = startNode->preByMachine;
                    }
                }
            }
        }
    }
    //没有合适位置，恢复
    solution.moveNode(deleteCriticalNode, beforeDeleteCriticalNode);
    Solution::refreshStatTailTimeForNode(deleteCriticalNode);
    return false;
}

int main() {
    std::chrono::time_point<std::chrono::steady_clock> st;
    std::chrono::time_point<std::chrono::steady_clock> et;

    double duration1 = 0;
    double duration2 = 0;
    double duration3 = 0;
    double duration4 = 0;
    int betterCount1 = 0;
    int betterCount2 = 0;
    int betterCount3 = 0;
    int betterCount4 = 0;
    int totalDecrease1 = 0;
    int totalDecrease2 = 0;
    int totalDecrease3 = 0;
    int totalDecrease4 = 0;
    std::ofstream resultFile("result.txt");
    for (int i = 1; i <= 10; ++i) {
        HFS_Problem problem(10, 4);
        std::vector<int> jobSequence(problem.getNumOfJobs());
        std::iota(std::begin(jobSequence), std::end(jobSequence), 0);
        std::shuffle(std::begin(jobSequence), std::end(jobSequence), rand_generator());

        //std::ostringstream oss;
        //oss << std::setfill('0') << std::setw(5) << i;
        //HFS_Problem problem(std::string("../data/") + std::string("p") + oss.str() + std::string(".txt"));

        //第一种策略
        st = std::chrono::steady_clock::now();
        Solution solution1(problem, jobSequence);
        //Solution solution1(problem, std::string("../data/") + std::string("p") + oss.str() + std::string("-s.txt"));

        solution1.decode();
        int originalMakeSpan1 = solution1.getMakeSpan();

        std::vector<Node *> oneCriticalPath1;
        //同机器优先，从后往前产生关键路径
        solution1.findOneCriticalPathMachineFirst(oneCriticalPath1);

        //正向甘特图
        //PyGantt::ganttOneCriticalPath(solution1, oneCriticalPath1, "Instance" + std::to_string(i), originalMakeSpan1,
        //		false, true);
        //逆向甘特图
        //PyGantt::ganttOneCriticalPath(solution1, oneCriticalPath, "Instance" + std::to_string(i), originalMakeSpan1,
        // 		true, true);

        bool re1 = false;
        //在关键路径oneCriticalPath中尝试移动工序，逆向遍历
        for (auto deleteCriticalNode: oneCriticalPath1) {
            //对关键路径criticalPath中的deleteCriticalNode进行最优插入，删除deleteCriticalNode时计算startTime和tailTime
            bool result = optimalInsertPosition(solution1, deleteCriticalNode);
            if (result) {
                //正向甘特图
                //PyGantt::ganttOneCriticalPath(solution1, oneCriticalPath1, "Instance" + std::to_string(i),
                //		originalMakeSpan1, false, true);
                //逆向甘特图
                //PyGantt::ganttOneCriticalPath(solution1, oneCriticalPath1, "Instance" + std::to_string(i),
                // 		originalMakeSpan1, true, true);
                re1 = true;
                break;
            }
        }
        et = std::chrono::steady_clock::now();
        //持续时间
        duration1 += std::chrono::duration<double, std::milli>(et - st).count();
        if (re1) {
            ++betterCount1;
            totalDecrease1 += originalMakeSpan1 - solution1.getMakeSpan();
            resultFile << originalMakeSpan1 << "\t" << 1 << "\t" << solution1.getMakeSpan() << "\t";
            //std::cout << i << ":makeSpan become better." << std::endl;
        } else {
            resultFile << originalMakeSpan1 << "\t" << 0 << "\t" << solution1.getMakeSpan() << "\t";
            //std::cout << i << ":makeSpan not become better." << std::endl;
        }

        //第二种策略
        st = std::chrono::steady_clock::now();
        Solution solution2(problem, jobSequence);
        //Solution solution2(problem, std::string("../data/") + std::string("p") + oss.str() + std::string("-s.txt"));
        solution2.decode();
        int originalMakeSpan2 = solution2.getMakeSpan();

        std::vector<Node *> oneCriticalPath2;
        //工序优先，从后往前产生关键路径
        solution2.findOneCriticalPathJobFirst(oneCriticalPath2);

        //正向甘特图
        //PyGantt::ganttOneCriticalPath(solution2, oneCriticalPath2, "Instance" + std::to_string(i), originalMakeSpan2,
        //		false, true);
        //逆向甘特图
        //PyGantt::ganttOneCriticalPath(solution2, oneCriticalPath2, "Instance" + std::to_string(i), originalMakeSpan2,
        // 		true, true);

        bool re2 = false;
        //在关键路径oneCriticalPath中尝试移动工序，逆向遍历
        for (auto deleteCriticalNode: oneCriticalPath2) {
            //对关键路径criticalPath中的deleteCriticalNode进行最优插入，删除deleteCriticalNode时不计算startTime和tailTime
            bool result = optimalInsertPosition(solution2, deleteCriticalNode);
            if (result) {
                //正向甘特图
                //PyGantt::ganttOneCriticalPath(solution2, oneCriticalPath2, "Instance" + std::to_string(i),
                //		originalMakeSpan2, false, true);
                //逆向甘特图
                //PyGantt::ganttOneCriticalPath(solution2, oneCriticalPath2, "Instance" + std::to_string(i),
                // 		originalMakeSpan2, true, true);
                re2 = true;
                break;
            }
        }
        et = std::chrono::steady_clock::now();
        //持续时间
        duration2 += std::chrono::duration<double, std::milli>(et - st).count();
        if (re2) {
            ++betterCount2;
            totalDecrease2 += originalMakeSpan2 - solution2.getMakeSpan();
            resultFile << originalMakeSpan2 << "\t" << 1 << "\t" << solution2.getMakeSpan() << "\t";
            //std::cout << i << ":makeSpan become better." << std::endl;
        } else {
            resultFile << originalMakeSpan2 << "\t" << 0 << "\t" << solution2.getMakeSpan() << "\t";
            //std::cout << i << ":makeSpan not become better." << std::endl;
        }

        //第三种策略
        st = std::chrono::steady_clock::now();
        Solution solution3(problem, jobSequence);
        //Solution solution3(problem, std::string("../data/") + std::string("p") + oss.str() + std::string("-s.txt"));
        solution3.decode();
        int originalMakeSpan3 = solution3.getMakeSpan();

        //找到所有关键路径上的节点，画图用
        std::vector<Node*> criticalNodes;
        solution3.findAllCriticalNodes(criticalNodes);

        //找到所有关键路径图的割点
        std::vector<Node *> cutNodes1;
        solution3.findCutNodes(cutNodes1);

        //正向甘特图
        PyGantt::ganttAllCriticalNodes(solution3, {}, cutNodes1, "Instance" + std::to_string(i), originalMakeSpan3,
        		false, true);
        //逆向甘特图
        PyGantt::ganttAllCriticalNodes(solution3, {}, cutNodes1, "Instance" + std::to_string(i), originalMakeSpan3,
         		true, true);

        bool re3 = false;
        //在所有割点中尝试移动工序，逆向遍历
        for (auto it = std::rbegin(cutNodes1); it != std::rend(cutNodes1); ++it) {
            auto deleteCriticalNode = *it;
            //对关键路径criticalPath中的第pos个工序进行最优插入
            bool result = optimalInsertPositionNoGlobalRefresh(solution3, deleteCriticalNode);

            if (result) {
                //正向甘特图
                //PyGantt::ganttAllCriticalNodes(solution3, {}, cutNodes1, "Instance" + std::to_string(i),
                //		originalMakeSpan3, false, true);
                //逆向甘特图
                //PyGantt::ganttAllCriticalNodes(solution3, {}, cutNodes1, "Instance" + std::to_string(i),
                // 		originalMakeSpan3, true, true);
                re3 = true;
                break;
            }
        }
        et = std::chrono::steady_clock::now();
        //持续时间
        duration3 += std::chrono::duration<double, std::milli>(et - st).count();
        if (re3) {
            ++betterCount3;
            totalDecrease3 += originalMakeSpan3 - solution3.getMakeSpan();
            resultFile << originalMakeSpan3 << "\t" << 1 << "\t" << solution3.getMakeSpan() << "\t";
            //std::cout << i << ":makeSpan become better." << std::endl;
        } else {
            resultFile << originalMakeSpan3 << "\t" << 0 << "\t" << solution3.getMakeSpan() << "\t";
            //std::cout << i << ":makeSpan not become better." << std::endl;
        }

        //第四种策略
        st = std::chrono::steady_clock::now();
        Solution solution4(problem, jobSequence);
        //Solution solution3(problem, std::string("../data/") + std::string("p") + oss.str() + std::string("-s.txt"));
        solution4.decode();
        int originalMakeSpan4 = solution4.getMakeSpan();

        //找到所有关键路径上的节点，画图用
        //std::vector<Node*> criticalNodes;
        //solution3.findAllCriticalNodes(criticalNodes);

        //找到所有关键路径图的割点
        std::vector<Node *> cutNodes2;
        solution4.findCutNodes(cutNodes2);

        //正向甘特图
        //PyGantt::ganttAllCriticalNodes(solution4, {}, cutNodes2, "Instance" + std::to_string(i), originalMakeSpan3,
        //		false, true);
        //逆向甘特图
        //PyGantt::ganttAllCriticalNodes(solution4, {}, cutNodes2, "Instance" + std::to_string(i), originalMakeSpan3,
        // 		true, true);

        bool re4 = false;
        //在所有割点中尝试移动工序，正向遍历
        for (auto deleteCriticalNode: cutNodes2) {
            //对关键路径criticalPath中的第pos个工序进行最优插入
            bool result = optimalInsertPositionNoGlobalRefresh(solution4, deleteCriticalNode);

            if (result) {
                //正向甘特图
                //PyGantt::ganttAllCriticalNodes(solution4, {}, cutNodes2, "Instance" + std::to_string(i),
                //		originalMakeSpan4, false, true);
                //逆向甘特图
                //PyGantt::ganttAllCriticalNodes(solution4, {}, cutNodes2, "Instance" + std::to_string(i),
                // 		originalMakeSpan4, true, true);
                re4 = true;
                break;
            }
        }
        et = std::chrono::steady_clock::now();
        //持续时间
        duration4 += std::chrono::duration<double, std::milli>(et - st).count();
        if (re4) {
            ++betterCount4;
            totalDecrease4 += originalMakeSpan4 - solution4.getMakeSpan();
            resultFile << originalMakeSpan4 << "\t" << 1 << "\t" << solution4.getMakeSpan() << "\t";
            //std::cout << i << ":makeSpan become better." << std::endl;
        } else {
            resultFile << originalMakeSpan4 << "\t" << 0 << "\t" << solution4.getMakeSpan() << "\t";
            //std::cout << i << ":makeSpan not become better." << std::endl;
        }
        resultFile << std::endl;
    }

    resultFile << duration1 << "\t" << duration2 << "\t" << duration3 << "\t" << duration4 << "\t" << std::endl;
    resultFile << betterCount1 << "\t" << betterCount2 << "\t" << betterCount3 << "\t" << betterCount4 << "\t"
               << std::endl;
    resultFile << totalDecrease1 << "\t" << totalDecrease2 << "\t" << totalDecrease3 << "\t" << totalDecrease4 << "\t"
               << std::endl;
    resultFile.close();
    std::cout << duration1 << "\t" << duration2 << "\t" << duration3 << "\t" << duration4 << "\t" << std::endl;
    std::cout << betterCount1 << "\t" << betterCount2 << "\t" << betterCount3 << "\t" << betterCount4 << "\t"
              << std::endl;
    std::cout << totalDecrease1 << "\t" << totalDecrease2 << "\t" << totalDecrease3 << "\t" << totalDecrease4 << "\t"
              << std::endl;
    /*for (int i = 24; i <= 24; ++i)
    {
        std::ostringstream oss;
        oss << std::setfill('0') << std::setw(5) << i;
        HFS_Problem problem(std::string("../data/") + std::string("p") + oss.str() + std::string(".txt"));
        Solution solution(problem, std::string("../data/") + std::string("p") + oss.str() + std::string("-s.txt"));
        solution.decode();
        int originalMakeSpan = solution.getMakeSpan();
        solution.print();
    }*/

    return 0;
}
