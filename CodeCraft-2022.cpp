#include"MyData.h"


//存放分配矩阵，第一维度：边缘节点   第二维度：用户节点   第三节点：时间
vector<vector<vector<int>>> allocMatrix;

//将用户节点的带宽请求分配给边缘节点
//参数1 graph:     图
//参数2 edge：     对应的边缘节点
//参数3 cust：     对应的用户节点
//参数4 demandTmp：分配的带宽
//参数5 timeIndex：对应的时间
void AllocatedDemand(Graph* graph, EdgeNode* edge, CustomerNode* cust, int demandTmp, int timeIndex, bool isCost = true) {
    //可以避免某一个用户节点没有边缘节点可以分配
    //for (auto qosCust : edge->qosCustVec) {
    //    if (qosCust != cust)
    //        demandTmp = min(demandTmp, qosCust->demandMarginSumVec[timeIndex] - qosCust->demandsVec[timeIndex]);
    //}

    if (demandTmp < 0) return;

    //防止超限
    demandTmp = min(demandTmp, cust->demandsVec[timeIndex]);
    demandTmp = min(demandTmp, edge->demandMarginVec[timeIndex]);

    //对应的用户节点带宽需求减去demandTmp
    cust->demandsVec[timeIndex] -= demandTmp;
    //对应的边缘节点带宽负载增加demandTmp
    edge->bandwidthLoad[timeIndex] += demandTmp;
    if (isCost)
        edge->edgeCost = max(edge->edgeCost, edge->bandwidthLoad[timeIndex]);
    //对应的边缘节点在timeIndex时刻，带宽余量减去demandTmp
    edge->demandMarginVec[timeIndex] -= demandTmp;

    for (auto qosEdge : cust->qosEdgeVec) {
        qosEdge->edgeDemandSum[timeIndex] -= demandTmp;
    }
    for (auto qosCust : edge->qosCustVec) {
        qosCust->demandMarginSumVec[timeIndex] -= demandTmp;
    }
    //图对应时间带宽总和减去demandTmp
    graph->rowDemandSum[timeIndex] -= demandTmp;
    //将数据写到分配矩阵中
    allocMatrix[edge->indexID][cust->indexID][timeIndex] += demandTmp;
}

//将用户节点分配给边缘节点的带宽请求撤回
void TakeBackDemand(Graph* graph, EdgeNode* edge, CustomerNode* cust, int demandTmp, int timeIndex) {
    //对应的用户节点带宽需求加上demandTmp
    cust->demandsVec[timeIndex] += demandTmp;
    //对应的边缘节点带宽负载减去demandTmp
    edge->bandwidthLoad[timeIndex] -= demandTmp;
    //对应的边缘节点在timeIndex时刻，带宽余量加上demandTmp
    edge->demandMarginVec[timeIndex] += demandTmp;

    for (auto qosEdge : cust->qosEdgeVec) {
        qosEdge->edgeDemandSum[timeIndex] += demandTmp;
    }
    for (auto qosCust : edge->qosCustVec) {
        qosCust->demandMarginSumVec[timeIndex] += demandTmp;
    }
    //图对应时间带宽总和加上demandTmp
    graph->rowDemandSum[timeIndex] += demandTmp;
    //将数据写到分配矩阵中
    allocMatrix[edge->indexID][cust->indexID][timeIndex] -= demandTmp;
}

//将一个边缘节点的带宽负载分给另外一个边缘节点
void ReallocatedDemand(Graph* graph, EdgeNode* sendEdge, EdgeNode* receiveEdge, int demandTmp, int timeIndex) {
    unordered_map<CustomerNode*, bool> qosCustMap;
    vector<CustomerNode*> qosCustVec;
    for (auto qosCust : receiveEdge->qosCustVec) {
        qosCustMap[qosCust] = true;
    }
    int demandSum = 0;//可以搬移的最大数据
    for (auto qosCust : sendEdge->qosCustVec) {
        if (qosCustMap[qosCust]) {
            qosCustVec.push_back(qosCust);
            demandSum += allocMatrix[sendEdge->indexID][qosCust->indexID][timeIndex];
        }
    }
    if (demandSum == 0) return;
    //如果需要的数据 > 可以搬移的最大数据，则全部搬去
    if (demandTmp > demandSum) {
        for (auto qosCust : qosCustVec) {
            int demandPerNode = allocMatrix[sendEdge->indexID][qosCust->indexID][timeIndex];
            TakeBackDemand(graph, sendEdge, qosCust, demandPerNode, timeIndex);
            AllocatedDemand(graph, receiveEdge, qosCust, demandPerNode, timeIndex, false);
        }
    }
    else {//如果需要的数据 < 可以搬移的最大数据
        double scale = demandTmp / (double)demandSum;
        for (auto qosCust : qosCustVec) {
            int demandPerNode = (int)(scale * allocMatrix[sendEdge->indexID][qosCust->indexID][timeIndex]);
            TakeBackDemand(graph, sendEdge, qosCust, demandPerNode, timeIndex);
            AllocatedDemand(graph, receiveEdge, qosCust, demandPerNode, timeIndex, false);
        }
    }
}

//指定需求填入边缘节点
void AssignAlloc2Edge(Graph* graph, EdgeNode* edge, int demandTmp, int timeIndex) {
    ////************************************  新版本  没有问题了  ***************************************
    //抽取边缘节点所连接的用户节点，将边缘节点填满
    //按照用户节点从大到小抽取，使得剩余的用户节点尽量平均
    sort(edge->qosCustVec.begin(), edge->qosCustVec.end(), [&](CustomerNode* cust1, CustomerNode* cust2) {
        return cust1->demandsVec[timeIndex] < cust2->demandsVec[timeIndex]; });

    //***************************************  打印测试  ************************************************
    //cout << "抽取之前：    edge name: " << edge->nameID << "  time: " << timeIndex << "  demandTmp: " << demandTmp << endl;
    //for (auto qosCust : edge->qosCustVec) {
    //    cout << "<" << qosCust->nameID << "," << qosCust->demandsVec[timeIndex] << ">, ";
    //}
    //cout << endl;

    int index = 0;
    int edgeDemandSum = edge->edgeDemandSum[timeIndex];
    int edgeDemandSumTmp = edgeDemandSum;
    for (index = 0; index < edge->qosCustVec.size(); index++) {
        if (index == 0)
            edgeDemandSumTmp -= (edge->qosCustVec.size() - index) * edge->qosCustVec[index]->demandsVec[timeIndex];
        else
            edgeDemandSumTmp -= (edge->qosCustVec.size() - index) * (edge->qosCustVec[index]->demandsVec[timeIndex] - edge->qosCustVec[index - 1]->demandsVec[timeIndex]);
        if (edgeDemandSumTmp < demandTmp)
            break;
        edgeDemandSum = edgeDemandSumTmp;
    }
    if (index == edge->qosCustVec.size())
        index = edge->qosCustVec.size() - 1;
    int averNum = 0;
    if (edgeDemandSum > demandTmp) {
        if (index != 0) {
            averNum = (edgeDemandSum - demandTmp) / (edge->qosCustVec.size() - index) + edge->qosCustVec[index - 1]->demandsVec[timeIndex];
        }
        else {
            averNum = (edgeDemandSum - demandTmp) / edge->qosCustVec.size();
        }
    }
    else {
        averNum = 0;
    }

    for (index; index < edge->qosCustVec.size(); index++) {
        int demandTmp = min(edge->demandMarginVec[timeIndex], edge->qosCustVec[index]->demandsVec[timeIndex] - averNum);
        AllocatedDemand(graph, edge, edge->qosCustVec[index], demandTmp, timeIndex);

    }

    //***************************************  打印测试  ************************************************
    //cout << "抽取之后：    edge name: " << edge->nameID << "  Load: " << edge->bandwidthLoad[timeIndex] << endl;
    //for (auto qosCust : edge->qosCustVec) {
    //    cout << "<" << qosCust->nameID << "," << qosCust->demandsVec[timeIndex] << ">, ";
    //}
    //cout << endl << endl;
}

//填满一个边缘节点，限制最高点
void FullAlloc2EdgeLimitTop(Graph* graph, EdgeNode* edge, int timeIndex) {
    ////************************************  新版本  没有问题了  ***************************************
    //如果当前边缘节点总的请求大于边缘节点的余量
    int edgeDemandSum = edge->edgeDemandSum[timeIndex];
    //抽取边缘节点所连接的用户节点，将边缘节点填满
    //按照用户节点从大到小抽取，使得剩余的用户节点尽量平均
    sort(edge->qosCustVec.begin(), edge->qosCustVec.end(), [&](CustomerNode* cust1, CustomerNode* cust2) {
        return cust1->demandsVec[timeIndex] < cust2->demandsVec[timeIndex]; });
    int index = 0;
    int edgeDemandSumTmp = edgeDemandSum;
    for (index = 0; index < edge->qosCustVec.size(); index++) {
        if (index == 0)
            edgeDemandSumTmp -= (edge->qosCustVec.size() - index) * edge->qosCustVec[index]->demandsVec[timeIndex];
        else
            edgeDemandSumTmp -= (edge->qosCustVec.size() - index) * (edge->qosCustVec[index]->demandsVec[timeIndex] - edge->qosCustVec[index - 1]->demandsVec[timeIndex]);
        if (edgeDemandSumTmp < edge->demandMarginVec[timeIndex])
            break;
        edgeDemandSum = edgeDemandSumTmp;
    }
    if (index == edge->qosCustVec.size())
        index = edge->qosCustVec.size() - 1;
    int averNum = 0;
    if (edgeDemandSum > edge->demandMarginVec[timeIndex]) {
        if (index != 0) {
            averNum = (edgeDemandSum - edge->demandMarginVec[timeIndex]) / (edge->qosCustVec.size() - index) + edge->qosCustVec[index - 1]->demandsVec[timeIndex];
        }
        else {
            averNum = (edgeDemandSum - edge->demandMarginVec[timeIndex]) / edge->qosCustVec.size();
        }
    }
    else {
        averNum = 0;
    }

    for (index; index < edge->qosCustVec.size(); index++) {
        int demandTmp = min(edge->demandMarginVec[timeIndex], edge->qosCustVec[index]->demandsVec[timeIndex] - averNum);
        AllocatedDemand(graph, edge, edge->qosCustVec[index], demandTmp, timeIndex, false);
            
    }
}

//填满一个边缘节点，限制最大点
//有点不好用的样子，先暂时不用他
void FullAlloc2EdgeLimitMax(Graph* graph, EdgeNode* edge, int timeIndex) {
    sort(edge->qosCustVec.begin(), edge->qosCustVec.end(), [&](CustomerNode* cust1, CustomerNode* cust2) {
        return cust1->demandsVec[timeIndex] > cust2->demandsVec[timeIndex]; });
    for (auto qosCust : edge->qosCustVec) {
        int demandTmp = min(edge->demandMarginVec[timeIndex], qosCust->demandsVec[timeIndex]);
        AllocatedDemand(graph, edge, qosCust, demandTmp, timeIndex);
    }
}

//在这一个时间内，将所有的用户节点带宽需求做平均
void AverAlloc2Edge(Graph* graph, int timeIndex) {
    sort(graph->customerVec.begin(), graph->customerVec.end(), [&](CustomerNode* p1, CustomerNode* p2) {
        return p1->demandMarginSumVec[timeIndex] < p2->demandMarginSumVec[timeIndex]; });
    for (auto cust : graph->customerVec) {
        //检查当前用户节点是否已经被分配
        while (cust->demandsVec[timeIndex] > 0) {
            vector<EdgeNode*> edgeVecTmp;
            long qosEdgeCostSum = 0;
            long edgeLoadSum = 0;
            for (auto qosEdge : cust->qosEdgeVec) {
                if (qosEdge->demandMarginVec[timeIndex]) {
                    edgeVecTmp.push_back(qosEdge);
                    qosEdgeCostSum += qosEdge->edgeCost;
                    edgeLoadSum += qosEdge->bandwidthLoad[timeIndex];
                }
            }
            if (edgeVecTmp.size() == 0) continue;
            if (cust->demandsVec[timeIndex] + edgeLoadSum >= qosEdgeCostSum) {
                sort(edgeVecTmp.begin(), edgeVecTmp.end(), [&](EdgeNode* edge1, EdgeNode* edge2) {
                    return edge1->edgeCost < edge2->edgeCost; });
                int index = 0;
                long edgeLoadSumTmp = 0;
                long sum = 0;
                for (index = 0; index < edgeVecTmp.size(); index++) {
                    edgeLoadSumTmp += edgeVecTmp[index]->bandwidthLoad[timeIndex];
                    if (index < edgeVecTmp.size() - 1)
                        sum = edgeVecTmp[index + 1]->bandwidthLoad[timeIndex] * (index + 1);
                    else
                        break;
                    if (sum - edgeLoadSumTmp >= cust->demandsVec[timeIndex])
                        break;
                }
                long averNum = 1 + (cust->demandsVec[timeIndex] + edgeLoadSumTmp) / (index + 1);
                int lastNum = 0;
                for (auto qosEdge : edgeVecTmp) {
                    int demandTmp = averNum - qosEdge->bandwidthLoad[timeIndex] + lastNum;
                    demandTmp = min(qosEdge->demandMarginVec[timeIndex], demandTmp);
                    demandTmp = min(demandTmp, cust->demandsVec[timeIndex]);
                    lastNum += (averNum - qosEdge->bandwidthLoad[timeIndex] - demandTmp);
                    /*分配节点数据*/
                    AllocatedDemand(graph, qosEdge, cust, demandTmp, timeIndex);
                    if (cust->demandsVec[timeIndex] == 0)
                        break;
                }
            }
            else {//填进去之后不会增加成本，则等比例放进去
                if (qosEdgeCostSum == edgeLoadSum)
                    break;
                double scale =  cust->demandsVec[timeIndex] / (double)(qosEdgeCostSum - edgeLoadSum);
                for (auto qosEdge : edgeVecTmp) {
                    int demandTmp = 1 + (int)(scale * (qosEdge->edgeCost - qosEdge->bandwidthLoad[timeIndex]));
                    demandTmp = min(qosEdge->demandMarginVec[timeIndex], demandTmp);
                    demandTmp = min(demandTmp, cust->demandsVec[timeIndex]);
                    /*分配节点数据*/
                    AllocatedDemand(graph, qosEdge, cust, demandTmp, timeIndex);
                    if (cust->demandsVec[timeIndex] == 0)
                        break;
                }
            }
        }
    }
}

//分配top 5%的数据节点
int AllocTopPercentNode(Graph* graph, double percentNum) {
    //95分位的下标索引
    int timeCnt = graph->timeCnt;
    int index95Percent = ceil(timeCnt * percentNum) - 1;
    int count5Percent = timeCnt - 1 - index95Percent;

    //创建每一行在每一个时刻的最大请求，push到优先队列中
    priority_queue<RowAverDemandMax> rowMaxDemandQue;
    for (int timeIndex = 0; timeIndex < timeCnt; timeIndex++) {
        int edgeCnt = graph->edgeVec.size() - graph->rowFullEdgeNum[timeIndex];
        if (edgeCnt > 0) {
            int averDemand = graph->rowDemandSum[timeIndex] / edgeCnt;
            rowMaxDemandQue.push(RowAverDemandMax(averDemand, timeIndex));
        }
    }
    while (!rowMaxDemandQue.empty()) {
        bool isExit = true;
        //如果边缘节点时间序列上Top %5都填满，则退出当前循环
        for (auto edge : graph->edgeVec) {
            isExit = isExit && edge->maxDemandNum == count5Percent;
        }
        if (isExit) break;

        //获取最大行的时间
        auto rowDemandMax = rowMaxDemandQue.top();
        rowMaxDemandQue.pop();
        int timeIndex = rowDemandMax.timeIndex;

        //创建每一个边缘节点在每一个时刻的最大请求，push到优先队列中
        priority_queue<EdgeDemandMax> edgeMaxDemandQue;
        for (auto edge : graph->edgeVec) {
            if (edge->isFullFlag[timeIndex] == false && edge->maxDemandNum < count5Percent) {
                edgeMaxDemandQue.push(EdgeDemandMax(edge, edge->edgeDemandSum[timeIndex] + 1 * edge->bandwidth, timeIndex));
            }
        }
        if (edgeMaxDemandQue.empty())
            continue;
        //取出当前最合适的节点
        EdgeNode* edge = edgeMaxDemandQue.top().edgeNode;
        //cout << "time:" << timeIndex << "   MaxDemand:" << edge->edgeDemandSum[timeIndex] << endl;
        FullAlloc2EdgeLimitTop(graph, edge, timeIndex);

        graph->rowFullEdgeNum[timeIndex]++;
        int edgeCnt = graph->edgeVec.size() - graph->rowFullEdgeNum[timeIndex];
        if (edgeCnt > 0) {
            int averDemand = graph->rowDemandSum[timeIndex] / edgeCnt;
            rowMaxDemandQue.push(RowAverDemandMax(averDemand, timeIndex));
        }
        edge->isFullFlag[timeIndex] = true;
        edge->maxDemandNum++;
    }
    return 0;
}

//回环扫查，实现合理分配
void LoopbanckCheck(Graph* graph, int iterate = 1) {
    int timeCnt = graph->timeCnt;
    //所有时间，合理分配用户节点的需求
    for (int timeIndex = 0; timeIndex < graph->timeCnt; timeIndex++) {
        vector<EdgeNode*> edgeVecTmp;
        for (auto edge : graph->edgeVec) {
            if (edge->isFullFlag[timeIndex] == false) {
                edgeVecTmp.push_back(edge);
            }
        }
        if (edgeVecTmp.size() == 0)continue;

        sort(edgeVecTmp.begin(), edgeVecTmp.end(), [&](EdgeNode* n1, EdgeNode* n2) {
            return n1->bandwidthLoad[timeIndex] < n2->bandwidthLoad[timeIndex]; });
        //for (auto edge : edgeVecTmp) {
        //    cout << "< name:" << edge->nameID << "  load:" << edge->bandwidthLoad[timeIndex] << ">,   ";
        //}
        //cout << endl << endl;
        int _iterate = iterate;
        while (_iterate) {
            int receiveIndex = 0, sendIndex = edgeVecTmp.size() - 1;
            while (receiveIndex < sendIndex) {
                EdgeNode* sendEdge = edgeVecTmp[sendIndex];
                EdgeNode* receiveEdge = edgeVecTmp[receiveIndex];
                int demandTmp = (sendEdge->bandwidthLoad[timeIndex] - receiveEdge->bandwidthLoad[timeIndex]) / 2;
                ReallocatedDemand(graph, sendEdge, receiveEdge, demandTmp, timeIndex);
                sendIndex--;
                receiveIndex++;
            }
            _iterate--;
        }

        //for (auto edge : edgeVecTmp) {
        //    cout << "< name:" << edge->nameID << "  load:" << edge->bandwidthLoad[timeIndex] << ">,   ";
        //}
        //cout << endl << endl;
    }
}

//全局做到平均分配
int GlobalAverageAlloc(Graph* graph) {
    int timeCnt = graph->timeCnt;
    //所有时间，合理分配用户节点的需求
    for (int timeIndex = 0; timeIndex < timeCnt; timeIndex++) {
        int edgeCnt = graph->edgeVec.size() - graph->rowFullEdgeNum[timeIndex];
        if (edgeCnt == 0) continue;
        int averDemand = 1 + graph->rowDemandSum[timeIndex] / edgeCnt;
        for (int edgeIndex = 0; edgeIndex < graph->edgeVec.size(); edgeIndex++) {
            //创建每一个边缘节点在每一个时刻的最大请求，push到优先队列中
            priority_queue<EdgeDemandMin> edgeAverDemandQue;
            for (auto edge : graph->edgeVec) {
                if (edge->edgeDemandSum[timeIndex] > 0 &&
                    edge->bandwidthLoad[timeIndex] < 0.9 * averDemand &&
                    edge->demandMarginVec[timeIndex] > 0 &&
                    edge->isFullFlag[timeIndex] == false) {
                    edgeAverDemandQue.push(EdgeDemandMin(edge, timeIndex));
                }
            }
            if (edgeAverDemandQue.empty()) break;
            int times = graph->edgeVec.size() * 0.05;
            while (!edgeAverDemandQue.empty()) {
                //取出当前最合适的节点
                auto demandQueTop = edgeAverDemandQue.top();
                edgeAverDemandQue.pop();
                EdgeNode* edge = demandQueTop.edgeNode;

                //计算当前edge服务器被请求的最大的带宽                
                int demandSum = min(edge->edgeDemandSum[timeIndex], edge->demandMarginVec[timeIndex]);
                if (demandSum == 0) continue;
                int curDemand = averDemand - edge->bandwidthLoad[timeIndex];
                //cout << "1:  " << graph->demandSumVec[timeIndex] << "  " << edgeVecTmp.size() << "  " << demandSum << endl;
                ////如果平均分配的请求 >= 当前节点的可分配最大需求，则将将当前节点的需求分配完
                
                
                
                //*************************************************  新版本  ***************************************************
                //int demandTmp = min(edge->demandMarginVec[timeIndex], curDemand);
                //AssignAlloc2Edge(graph, edge, demandTmp, timeIndex);
                
                
                //*************************************************  旧版本  ***************************************************
                if (curDemand >= demandSum) {
                    for (auto qosCust : edge->qosCustVec) {
                        int demandTmp = min(edge->demandMarginVec[timeIndex], qosCust->demandsVec[timeIndex]);
                        AllocatedDemand(graph, edge, qosCust, demandTmp, timeIndex);
                    }
                }
                else {//如果平均分配的请求 <= 当前节点的可分配最大需求，则表示可以全部放得下平均需求
                    double scale = min(curDemand / (double)edge->edgeDemandSum[timeIndex], 1.0);
                    int demandSum = 0;
                    for (auto qosCust : edge->qosCustVec) {
                        int demandTmp = min(edge->demandMarginVec[timeIndex], (int)ceil(scale * qosCust->demandsVec[timeIndex]));
                        demandSum += demandTmp;
                        AllocatedDemand(graph, edge, qosCust, demandTmp, timeIndex);
                    }
                }
                
                times--;
                if (times <= 0)break;
            }

        }
    }
    return 0;
}

//在用户基础上做平均分配
int UserAverageAlloc(Graph* graph) {
    //所有时间，合理分配用户节点的需求
    int timeCnt = graph->timeCnt;
    for (int timeIndex = 0; timeIndex < timeCnt; timeIndex++) {
        AverAlloc2Edge(graph, timeIndex);
    }
    return 0;
}

//栈顶数据，平均值遍历生成
int TopStackAverageAlloc_planB(Graph* graph) {
    int timeCnt = graph->timeCnt;       //总时间
    int statisticTimes = (int)ceil(timeCnt / 1.0);   //统计次数
    if (statisticTimes == 0) return 0;
    for (int timeIndex = 0; timeIndex < statisticTimes; timeIndex++) {
        for (auto cust : graph->customerVec) {
            //检查当前用户节点是否已经被分配
            for (auto qosEdge : cust->qosEdgeVec) {
                qosEdge->predictCost += 1 + cust->demandsVec[timeIndex] / cust->qosEdgeVec.size();
            }
        }
    }
    for (auto edge : graph->edgeVec) {
        edge->predictCost = 1 * (edge->predictCost / statisticTimes);
    }

    //所有时间，合理分配用户节点的需求
    for (int timeIndex = 0; timeIndex < timeCnt; timeIndex++) {
        sort(graph->edgeVec.begin(), graph->edgeVec.end(), [&](EdgeNode* n1, EdgeNode* n2) {
            return  n1->edgeDemandSum[timeIndex] < n2->edgeDemandSum[timeIndex]; });
        for (auto edge : graph->edgeVec) {
            //计算当前edge服务器被请求的最大的带宽
            int demandSum = min(edge->edgeDemandSum[timeIndex], edge->demandMarginVec[timeIndex]);
            if (demandSum == 0) continue;
            int curDemand = edge->predictCost - edge->bandwidthLoad[timeIndex];
            if (curDemand <= 0)  continue;

            ////*************************************************  新版本  ***************************************************
            //int demandTmp = min(edge->demandMarginVec[timeIndex], curDemand);
            //AssignAlloc2Edge(graph, edge, demandTmp, timeIndex);

            //*************************************************  旧版本  ***************************************************
            //cout << "1:  " << graph->demandSumVec[timeIndex] << "  " << edgeVecTmp.size() << "  " << demandSum << endl;
            //如果平均分配的请求 >= 当前节点的可分配最大需求，则将将当前节点的需求分配完
            if (curDemand >= demandSum) {
                for (auto qosCust : edge->qosCustVec) {
                    int demandTmp = min(edge->demandMarginVec[timeIndex], qosCust->demandsVec[timeIndex]);
                    AllocatedDemand(graph, edge, qosCust, demandTmp, timeIndex);
                }
            }
            else {//如果平均分配的请求 <= 当前节点的可分配最大需求，则表示可以全部放得下平均需求
                double scale = min(curDemand / (double)edge->edgeDemandSum[timeIndex], 1.0);
                int demandSum = 0;
                sort(edge->qosCustVec.begin(), edge->qosCustVec.end(), [&](CustomerNode* p1, CustomerNode* p2) {
                    return p1->demandMarginSumVec[timeIndex] < p2->demandMarginSumVec[timeIndex]; });
                for (auto qosCust : edge->qosCustVec) {
                    int demandTmp = min(edge->demandMarginVec[timeIndex], (int)ceil(scale * qosCust->demandsVec[timeIndex]));
                    demandSum += demandTmp;
                    AllocatedDemand(graph, edge, qosCust, demandTmp, timeIndex);
                }
            }
        }
    }
    return 0;
}

//填满某些节点
void FullSomeEdgeAlloc(Graph* graph, double fullEdgePercent) {
    int timeCnt = graph->timeCnt;
    if (timeCnt == 0) return;
    int timeIndex = 0;
    vector<EdgeNode*> fullEdgeVec;
    int fullEdgeCnt = fullEdgePercent * graph->edgeVec.size();
    for (int t = 0; t < fullEdgeCnt; t++) {
        priority_queue<EdgeDemandMax> edgeMaxDemandQue;
        for (auto edge : graph->edgeVec) {
            if (std::find(fullEdgeVec.begin(), fullEdgeVec.end(), edge) == fullEdgeVec.end()) {
                edgeMaxDemandQue.push(EdgeDemandMax(edge, edge->edgeDemandSum[timeIndex], timeIndex));
            }
        }
        if (edgeMaxDemandQue.empty())
            return;
        //取出当前最合适的节点
        EdgeNode* edge = edgeMaxDemandQue.top().edgeNode;
        fullEdgeVec.push_back(edge);
        FullAlloc2EdgeLimitTop(graph, edge, timeIndex);
    }
    AverAlloc2Edge(graph, timeIndex);


    for (timeIndex = 1; timeIndex < timeCnt; timeIndex++) {
        for (auto fullEdge : fullEdgeVec) {
            FullAlloc2EdgeLimitTop(graph, fullEdge, timeIndex);
        }
        AverAlloc2Edge(graph, timeIndex);
    }
}


int main() {
    double percent95Num = 0.95;
    int qosConstraint = 0;
    int timeCnt = 0;
    vector<CustomerNode*> customerVec;                  //客户节点Vec       
    vector<EdgeNode*> edgeVec;                          //边缘节点Vec
    //读取data
    ReadINIConfig(qosConstraint);
    ReadCSVData(customerVec, edgeVec, timeCnt, qosConstraint);
    allocMatrix = vector<vector<vector<int>>>(edgeVec.size(), vector<vector<int>>(customerVec.size(), vector<int>(timeCnt, 0)));

    vector<Graph*> graphVec;
    ReadEdge2Graph(customerVec, edgeVec, graphVec);
    for (auto graph : graphVec) {
        if (graph->edgeVec.size() == 0 || graph->customerVec.size() == 0)
            continue;
        AllocTopPercentNode(graph, percent95Num);
        GlobalAverageAlloc(graph);
        
        UserAverageAlloc(graph);

    }
    
    //输出分配矩阵
    OutPutMatrix(allocMatrix, customerVec, edgeVec);
    //检查分配矩阵
    //VerifyMatrix(allocMatrix, customerVec, edgeVec, 0.95);

    return 0;
}