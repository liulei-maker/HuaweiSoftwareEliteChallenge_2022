#include"MyData.h"


//��ŷ�����󣬵�һά�ȣ���Ե�ڵ�   �ڶ�ά�ȣ��û��ڵ�   �����ڵ㣺ʱ��
vector<vector<vector<int>>> allocMatrix;

//���û��ڵ�Ĵ�������������Ե�ڵ�
//����1 graph:     ͼ
//����2 edge��     ��Ӧ�ı�Ե�ڵ�
//����3 cust��     ��Ӧ���û��ڵ�
//����4 demandTmp������Ĵ���
//����5 timeIndex����Ӧ��ʱ��
void AllocatedDemand(Graph* graph, EdgeNode* edge, CustomerNode* cust, int demandTmp, int timeIndex, bool isCost = true) {
    //���Ա���ĳһ���û��ڵ�û�б�Ե�ڵ���Է���
    //for (auto qosCust : edge->qosCustVec) {
    //    if (qosCust != cust)
    //        demandTmp = min(demandTmp, qosCust->demandMarginSumVec[timeIndex] - qosCust->demandsVec[timeIndex]);
    //}

    if (demandTmp < 0) return;

    //��ֹ����
    demandTmp = min(demandTmp, cust->demandsVec[timeIndex]);
    demandTmp = min(demandTmp, edge->demandMarginVec[timeIndex]);

    //��Ӧ���û��ڵ���������ȥdemandTmp
    cust->demandsVec[timeIndex] -= demandTmp;
    //��Ӧ�ı�Ե�ڵ����������demandTmp
    edge->bandwidthLoad[timeIndex] += demandTmp;
    if (isCost)
        edge->edgeCost = max(edge->edgeCost, edge->bandwidthLoad[timeIndex]);
    //��Ӧ�ı�Ե�ڵ���timeIndexʱ�̣�����������ȥdemandTmp
    edge->demandMarginVec[timeIndex] -= demandTmp;

    for (auto qosEdge : cust->qosEdgeVec) {
        qosEdge->edgeDemandSum[timeIndex] -= demandTmp;
    }
    for (auto qosCust : edge->qosCustVec) {
        qosCust->demandMarginSumVec[timeIndex] -= demandTmp;
    }
    //ͼ��Ӧʱ������ܺͼ�ȥdemandTmp
    graph->rowDemandSum[timeIndex] -= demandTmp;
    //������д�����������
    allocMatrix[edge->indexID][cust->indexID][timeIndex] += demandTmp;
}

//���û��ڵ�������Ե�ڵ�Ĵ������󳷻�
void TakeBackDemand(Graph* graph, EdgeNode* edge, CustomerNode* cust, int demandTmp, int timeIndex) {
    //��Ӧ���û��ڵ�����������demandTmp
    cust->demandsVec[timeIndex] += demandTmp;
    //��Ӧ�ı�Ե�ڵ�����ؼ�ȥdemandTmp
    edge->bandwidthLoad[timeIndex] -= demandTmp;
    //��Ӧ�ı�Ե�ڵ���timeIndexʱ�̣�������������demandTmp
    edge->demandMarginVec[timeIndex] += demandTmp;

    for (auto qosEdge : cust->qosEdgeVec) {
        qosEdge->edgeDemandSum[timeIndex] += demandTmp;
    }
    for (auto qosCust : edge->qosCustVec) {
        qosCust->demandMarginSumVec[timeIndex] += demandTmp;
    }
    //ͼ��Ӧʱ������ܺͼ���demandTmp
    graph->rowDemandSum[timeIndex] += demandTmp;
    //������д�����������
    allocMatrix[edge->indexID][cust->indexID][timeIndex] -= demandTmp;
}

//��һ����Ե�ڵ�Ĵ����طָ�����һ����Ե�ڵ�
void ReallocatedDemand(Graph* graph, EdgeNode* sendEdge, EdgeNode* receiveEdge, int demandTmp, int timeIndex) {
    unordered_map<CustomerNode*, bool> qosCustMap;
    vector<CustomerNode*> qosCustVec;
    for (auto qosCust : receiveEdge->qosCustVec) {
        qosCustMap[qosCust] = true;
    }
    int demandSum = 0;//���԰��Ƶ��������
    for (auto qosCust : sendEdge->qosCustVec) {
        if (qosCustMap[qosCust]) {
            qosCustVec.push_back(qosCust);
            demandSum += allocMatrix[sendEdge->indexID][qosCust->indexID][timeIndex];
        }
    }
    if (demandSum == 0) return;
    //�����Ҫ������ > ���԰��Ƶ�������ݣ���ȫ����ȥ
    if (demandTmp > demandSum) {
        for (auto qosCust : qosCustVec) {
            int demandPerNode = allocMatrix[sendEdge->indexID][qosCust->indexID][timeIndex];
            TakeBackDemand(graph, sendEdge, qosCust, demandPerNode, timeIndex);
            AllocatedDemand(graph, receiveEdge, qosCust, demandPerNode, timeIndex, false);
        }
    }
    else {//�����Ҫ������ < ���԰��Ƶ��������
        double scale = demandTmp / (double)demandSum;
        for (auto qosCust : qosCustVec) {
            int demandPerNode = (int)(scale * allocMatrix[sendEdge->indexID][qosCust->indexID][timeIndex]);
            TakeBackDemand(graph, sendEdge, qosCust, demandPerNode, timeIndex);
            AllocatedDemand(graph, receiveEdge, qosCust, demandPerNode, timeIndex, false);
        }
    }
}

//ָ�����������Ե�ڵ�
void AssignAlloc2Edge(Graph* graph, EdgeNode* edge, int demandTmp, int timeIndex) {
    ////************************************  �°汾  û��������  ***************************************
    //��ȡ��Ե�ڵ������ӵ��û��ڵ㣬����Ե�ڵ�����
    //�����û��ڵ�Ӵ�С��ȡ��ʹ��ʣ����û��ڵ㾡��ƽ��
    sort(edge->qosCustVec.begin(), edge->qosCustVec.end(), [&](CustomerNode* cust1, CustomerNode* cust2) {
        return cust1->demandsVec[timeIndex] < cust2->demandsVec[timeIndex]; });

    //***************************************  ��ӡ����  ************************************************
    //cout << "��ȡ֮ǰ��    edge name: " << edge->nameID << "  time: " << timeIndex << "  demandTmp: " << demandTmp << endl;
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

    //***************************************  ��ӡ����  ************************************************
    //cout << "��ȡ֮��    edge name: " << edge->nameID << "  Load: " << edge->bandwidthLoad[timeIndex] << endl;
    //for (auto qosCust : edge->qosCustVec) {
    //    cout << "<" << qosCust->nameID << "," << qosCust->demandsVec[timeIndex] << ">, ";
    //}
    //cout << endl << endl;
}

//����һ����Ե�ڵ㣬������ߵ�
void FullAlloc2EdgeLimitTop(Graph* graph, EdgeNode* edge, int timeIndex) {
    ////************************************  �°汾  û��������  ***************************************
    //�����ǰ��Ե�ڵ��ܵ�������ڱ�Ե�ڵ������
    int edgeDemandSum = edge->edgeDemandSum[timeIndex];
    //��ȡ��Ե�ڵ������ӵ��û��ڵ㣬����Ե�ڵ�����
    //�����û��ڵ�Ӵ�С��ȡ��ʹ��ʣ����û��ڵ㾡��ƽ��
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

//����һ����Ե�ڵ㣬��������
//�е㲻���õ����ӣ�����ʱ������
void FullAlloc2EdgeLimitMax(Graph* graph, EdgeNode* edge, int timeIndex) {
    sort(edge->qosCustVec.begin(), edge->qosCustVec.end(), [&](CustomerNode* cust1, CustomerNode* cust2) {
        return cust1->demandsVec[timeIndex] > cust2->demandsVec[timeIndex]; });
    for (auto qosCust : edge->qosCustVec) {
        int demandTmp = min(edge->demandMarginVec[timeIndex], qosCust->demandsVec[timeIndex]);
        AllocatedDemand(graph, edge, qosCust, demandTmp, timeIndex);
    }
}

//����һ��ʱ���ڣ������е��û��ڵ����������ƽ��
void AverAlloc2Edge(Graph* graph, int timeIndex) {
    sort(graph->customerVec.begin(), graph->customerVec.end(), [&](CustomerNode* p1, CustomerNode* p2) {
        return p1->demandMarginSumVec[timeIndex] < p2->demandMarginSumVec[timeIndex]; });
    for (auto cust : graph->customerVec) {
        //��鵱ǰ�û��ڵ��Ƿ��Ѿ�������
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
                    /*����ڵ�����*/
                    AllocatedDemand(graph, qosEdge, cust, demandTmp, timeIndex);
                    if (cust->demandsVec[timeIndex] == 0)
                        break;
                }
            }
            else {//���ȥ֮�󲻻����ӳɱ�����ȱ����Ž�ȥ
                if (qosEdgeCostSum == edgeLoadSum)
                    break;
                double scale =  cust->demandsVec[timeIndex] / (double)(qosEdgeCostSum - edgeLoadSum);
                for (auto qosEdge : edgeVecTmp) {
                    int demandTmp = 1 + (int)(scale * (qosEdge->edgeCost - qosEdge->bandwidthLoad[timeIndex]));
                    demandTmp = min(qosEdge->demandMarginVec[timeIndex], demandTmp);
                    demandTmp = min(demandTmp, cust->demandsVec[timeIndex]);
                    /*����ڵ�����*/
                    AllocatedDemand(graph, qosEdge, cust, demandTmp, timeIndex);
                    if (cust->demandsVec[timeIndex] == 0)
                        break;
                }
            }
        }
    }
}

//����top 5%�����ݽڵ�
int AllocTopPercentNode(Graph* graph, double percentNum) {
    //95��λ���±�����
    int timeCnt = graph->timeCnt;
    int index95Percent = ceil(timeCnt * percentNum) - 1;
    int count5Percent = timeCnt - 1 - index95Percent;

    //����ÿһ����ÿһ��ʱ�̵��������push�����ȶ�����
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
        //�����Ե�ڵ�ʱ��������Top %5�����������˳���ǰѭ��
        for (auto edge : graph->edgeVec) {
            isExit = isExit && edge->maxDemandNum == count5Percent;
        }
        if (isExit) break;

        //��ȡ����е�ʱ��
        auto rowDemandMax = rowMaxDemandQue.top();
        rowMaxDemandQue.pop();
        int timeIndex = rowDemandMax.timeIndex;

        //����ÿһ����Ե�ڵ���ÿһ��ʱ�̵��������push�����ȶ�����
        priority_queue<EdgeDemandMax> edgeMaxDemandQue;
        for (auto edge : graph->edgeVec) {
            if (edge->isFullFlag[timeIndex] == false && edge->maxDemandNum < count5Percent) {
                edgeMaxDemandQue.push(EdgeDemandMax(edge, edge->edgeDemandSum[timeIndex] + 1 * edge->bandwidth, timeIndex));
            }
        }
        if (edgeMaxDemandQue.empty())
            continue;
        //ȡ����ǰ����ʵĽڵ�
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

//�ػ�ɨ�飬ʵ�ֺ������
void LoopbanckCheck(Graph* graph, int iterate = 1) {
    int timeCnt = graph->timeCnt;
    //����ʱ�䣬��������û��ڵ������
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

//ȫ������ƽ������
int GlobalAverageAlloc(Graph* graph) {
    int timeCnt = graph->timeCnt;
    //����ʱ�䣬��������û��ڵ������
    for (int timeIndex = 0; timeIndex < timeCnt; timeIndex++) {
        int edgeCnt = graph->edgeVec.size() - graph->rowFullEdgeNum[timeIndex];
        if (edgeCnt == 0) continue;
        int averDemand = 1 + graph->rowDemandSum[timeIndex] / edgeCnt;
        for (int edgeIndex = 0; edgeIndex < graph->edgeVec.size(); edgeIndex++) {
            //����ÿһ����Ե�ڵ���ÿһ��ʱ�̵��������push�����ȶ�����
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
                //ȡ����ǰ����ʵĽڵ�
                auto demandQueTop = edgeAverDemandQue.top();
                edgeAverDemandQue.pop();
                EdgeNode* edge = demandQueTop.edgeNode;

                //���㵱ǰedge����������������Ĵ���                
                int demandSum = min(edge->edgeDemandSum[timeIndex], edge->demandMarginVec[timeIndex]);
                if (demandSum == 0) continue;
                int curDemand = averDemand - edge->bandwidthLoad[timeIndex];
                //cout << "1:  " << graph->demandSumVec[timeIndex] << "  " << edgeVecTmp.size() << "  " << demandSum << endl;
                ////���ƽ����������� >= ��ǰ�ڵ�Ŀɷ�����������򽫽���ǰ�ڵ�����������
                
                
                
                //*************************************************  �°汾  ***************************************************
                //int demandTmp = min(edge->demandMarginVec[timeIndex], curDemand);
                //AssignAlloc2Edge(graph, edge, demandTmp, timeIndex);
                
                
                //*************************************************  �ɰ汾  ***************************************************
                if (curDemand >= demandSum) {
                    for (auto qosCust : edge->qosCustVec) {
                        int demandTmp = min(edge->demandMarginVec[timeIndex], qosCust->demandsVec[timeIndex]);
                        AllocatedDemand(graph, edge, qosCust, demandTmp, timeIndex);
                    }
                }
                else {//���ƽ����������� <= ��ǰ�ڵ�Ŀɷ�������������ʾ����ȫ���ŵ���ƽ������
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

//���û���������ƽ������
int UserAverageAlloc(Graph* graph) {
    //����ʱ�䣬��������û��ڵ������
    int timeCnt = graph->timeCnt;
    for (int timeIndex = 0; timeIndex < timeCnt; timeIndex++) {
        AverAlloc2Edge(graph, timeIndex);
    }
    return 0;
}

//ջ�����ݣ�ƽ��ֵ��������
int TopStackAverageAlloc_planB(Graph* graph) {
    int timeCnt = graph->timeCnt;       //��ʱ��
    int statisticTimes = (int)ceil(timeCnt / 1.0);   //ͳ�ƴ���
    if (statisticTimes == 0) return 0;
    for (int timeIndex = 0; timeIndex < statisticTimes; timeIndex++) {
        for (auto cust : graph->customerVec) {
            //��鵱ǰ�û��ڵ��Ƿ��Ѿ�������
            for (auto qosEdge : cust->qosEdgeVec) {
                qosEdge->predictCost += 1 + cust->demandsVec[timeIndex] / cust->qosEdgeVec.size();
            }
        }
    }
    for (auto edge : graph->edgeVec) {
        edge->predictCost = 1 * (edge->predictCost / statisticTimes);
    }

    //����ʱ�䣬��������û��ڵ������
    for (int timeIndex = 0; timeIndex < timeCnt; timeIndex++) {
        sort(graph->edgeVec.begin(), graph->edgeVec.end(), [&](EdgeNode* n1, EdgeNode* n2) {
            return  n1->edgeDemandSum[timeIndex] < n2->edgeDemandSum[timeIndex]; });
        for (auto edge : graph->edgeVec) {
            //���㵱ǰedge����������������Ĵ���
            int demandSum = min(edge->edgeDemandSum[timeIndex], edge->demandMarginVec[timeIndex]);
            if (demandSum == 0) continue;
            int curDemand = edge->predictCost - edge->bandwidthLoad[timeIndex];
            if (curDemand <= 0)  continue;

            ////*************************************************  �°汾  ***************************************************
            //int demandTmp = min(edge->demandMarginVec[timeIndex], curDemand);
            //AssignAlloc2Edge(graph, edge, demandTmp, timeIndex);

            //*************************************************  �ɰ汾  ***************************************************
            //cout << "1:  " << graph->demandSumVec[timeIndex] << "  " << edgeVecTmp.size() << "  " << demandSum << endl;
            //���ƽ����������� >= ��ǰ�ڵ�Ŀɷ�����������򽫽���ǰ�ڵ�����������
            if (curDemand >= demandSum) {
                for (auto qosCust : edge->qosCustVec) {
                    int demandTmp = min(edge->demandMarginVec[timeIndex], qosCust->demandsVec[timeIndex]);
                    AllocatedDemand(graph, edge, qosCust, demandTmp, timeIndex);
                }
            }
            else {//���ƽ����������� <= ��ǰ�ڵ�Ŀɷ�������������ʾ����ȫ���ŵ���ƽ������
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

//����ĳЩ�ڵ�
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
        //ȡ����ǰ����ʵĽڵ�
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
    vector<CustomerNode*> customerVec;                  //�ͻ��ڵ�Vec       
    vector<EdgeNode*> edgeVec;                          //��Ե�ڵ�Vec
    //��ȡdata
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
    
    //����������
    OutPutMatrix(allocMatrix, customerVec, edgeVec);
    //���������
    //VerifyMatrix(allocMatrix, customerVec, edgeVec, 0.95);

    return 0;
}