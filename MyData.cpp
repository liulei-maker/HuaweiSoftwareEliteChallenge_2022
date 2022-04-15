#include"MyData.h"

string demandFilePath = "/data/demand.csv";
string QoSFilePath = "/data/qos.csv";
string siteBandwidthFilePath = "/data/site_bandwidth.csv";
string iniPath = "/data/config.ini";
string outFilePath = "/output/solution.txt";




int ReadCSVData(vector<CustomerNode*>& _customerVec,	//�ͻ��ڵ�ID
    vector<EdgeNode*>& _edgeVec,			//��Ե�ڵ�ID
    int& _timeCnt,							//ʱ��������
    int _qosConstraint) { 						//ʱ��������
//��ȡdata���ݼ�
    ifstream demandFile(demandFilePath, ios::in);
    ifstream QoSFile(QoSFilePath, ios::in);
    ifstream siteBandwidthFile(siteBandwidthFilePath, ios::in);

    if (!demandFile) {
        cout << "demand.csv ��ʧ�ܣ�" << endl;
        return 1;
    }
    if (!QoSFile) {
        cout << "qos.csv ��ʧ�ܣ�" << endl;
        return 1;
    }
    if (!siteBandwidthFile) {
        cout << "site_bandwidth.csv ��ʧ�ܣ�" << endl;
        return 1;
    }
    unordered_map<string, CustomerNode*> customerMap;
    unordered_map<string, EdgeNode*> edgeMap;
    vector<string> custNameVec;
    vector<string> edgeNameVec;
    /******************************************************************************************/
    //��ȡdemand csv��head����
    string line;
    string str;
    getline(demandFile, line);
    istringstream sin(line);
    getline(sin, str, ',');
    int indexID = 0;
    while (getline(sin, str, ',')) {
        if (str.back() == '\n' || str.back() == '\r')
            str.pop_back();
        _customerVec.push_back(new CustomerNode(str, indexID));
        customerMap[str] = _customerVec[indexID];
        indexID++;
    }
    //�������һ�������еĻ���
    if (_customerVec.back()->nameID.back() == '\n' || _customerVec.back()->nameID.back() == '\r')
        _customerVec.back()->nameID.pop_back();

    //��ȡdemand csv��body����
    while (getline(demandFile, line)) {     //��ȡһ��������
        istringstream sin(line);            //�������ַ���line���뵽�ַ�����istringstream��
        getline(sin, str, ',');
        int index = 0;
        while (getline(sin, str, ',')) { //���ַ�����sin�е��ַ����뵽field�ַ����У��Զ���Ϊ�ָ���
            _customerVec[index]->demandsVec.push_back(atoi(str.c_str())); //���ոն�ȡ���ַ�����ӵ�����fields��
            index++;
        }
    }

    //�����ʱ������ֵ
    if (_customerVec.size())
        _timeCnt = _customerVec[0]->demandsVec.size();

    /******************************************************************************************/
    //��ȡsiteBandwidth csv��head����
    getline(siteBandwidthFile, line);
    //��ȡsiteBandwidth csv��body����
    indexID = 0;
    while (getline(siteBandwidthFile, line)) {     //��ȡһ��������
        string nameStr;
        int bandwidth = 0;
        istringstream sin(line);            //�������ַ���line���뵽�ַ�����istringstream��
        getline(sin, nameStr, ',');
        getline(sin, str, ',');
        bandwidth = atoi(str.c_str());
        _edgeVec.push_back(new EdgeNode(nameStr, bandwidth, _timeCnt, indexID));
        edgeMap[nameStr] = _edgeVec[indexID];
        indexID++;
    }


    /******************************************************************************************/
    //��ȡQoS csv������
    getline(QoSFile, line);              //��ȡ��ͷ����ͷ����Ҫ
    istringstream sinqos(line);
    getline(sinqos, str, ',');
    while (getline(sinqos, str, ',')) {
        if (str.back() == '\n' || str.back() == '\r')
            str.pop_back();
        custNameVec.push_back(str);
    }
    int edgeIndex = 0;
    while (getline(QoSFile, line)) {     //��ȡһ��������
        istringstream sin(line);         //�������ַ���line���뵽�ַ�����istringstream��
        getline(sin, str, ',');
        string edgeName = str;
        int custIndex = 0;
        while (getline(sin, str, ',')) { //���ַ�����sin�е��ַ����뵽field�ַ����У��Զ���Ϊ�ָ���
            if (atoi(str.c_str()) < _qosConstraint) {
                customerMap[custNameVec[custIndex]]->qosEdgeVec.push_back(edgeMap[edgeName]);
                edgeMap[edgeName]->qosCustVec.push_back(customerMap[custNameVec[custIndex]]);
            }
            custIndex++;
        }
        edgeIndex++;
    }

    //��������   1����ÿһ����Ե�ڵ�����qos���û��ڵ㰴�������ӵı�Ե�ڵ���Ŀ��������
    //           2����ʼ��ÿһ����Ե�ڵ�����������
    for (auto edge : _edgeVec) {
        sort(edge->qosCustVec.begin(), edge->qosCustVec.end(), [&](CustomerNode*& cust1, CustomerNode*& cust2) {
            return cust1->qosEdgeVec.size() < cust2->qosEdgeVec.size(); });
    }

    //��ʼ����Ե�ڵ�Ľڵ�� ����

    for (auto edge : _edgeVec) {
        for (int timeIndex = 0; timeIndex < _timeCnt; timeIndex++) {
            int edgeDemandSum = 0;
            for (auto qosCust : edge->qosCustVec) {
                edgeDemandSum += qosCust->demandsVec[timeIndex];
            }
            edge->edgeDemandSum.push_back(edgeDemandSum);
        }
    }
    for (auto cust : _customerVec) {
        for (int timeIndex = 0; timeIndex < _timeCnt; timeIndex++) {
            int demandMarginSum = 0;
            for (auto qosEdge : cust->qosEdgeVec) {
                demandMarginSum += qosEdge->demandMarginVec[timeIndex];
            }
            cust->demandMarginSumVec.push_back(demandMarginSum);
        }
    }

    //�رմ򿪵�csv�ļ�
    demandFile.close();
    QoSFile.close();
    siteBandwidthFile.close();
}

#include"CMyINI.h"

void ReadINIConfig(int& _qosConstraint) {
    //��ȡini�����ļ�
    CMyINI* p = new CMyINI(iniPath);
    _qosConstraint = atoi(p->GetValue("config", "qos_constraint").c_str());
}


void ReadEdge2Graph(vector<CustomerNode*>& _customerVec, vector<EdgeNode*>& _edgeVec, vector<Graph*>& graphVec) {
    if (_customerVec.size() == 0)
        return;
    int timeCnt = _customerVec[0]->demandsVec.size();

    //�����еı�Ե�ڵ���û��ڵ㽨������ͨͼ
    set<EdgeNode*> edgeSet;
    for (auto edge : _edgeVec)
        edgeSet.insert(edge);
    while (!edgeSet.empty()) {
        vector<EdgeNode*> edgeVec;
        vector<CustomerNode*>customerVec;
        unordered_map<string, bool> custNodeMap;
        unordered_map<string, bool> edgeNodeMap;
        queue<EdgeNode*>edgeQue;
        queue<CustomerNode*>custQue;
        edgeQue.push(*edgeSet.begin());
        edgeNodeMap[(*edgeSet.begin())->nameID] = true;
        while (edgeQue.size()) {
            while (edgeQue.size()) {
                auto frontEdge = edgeQue.front();
                for (auto qosCust : frontEdge->qosCustVec) {
                    if (custNodeMap[qosCust->nameID] == false) {
                        custNodeMap[qosCust->nameID] = true;
                        custQue.push(qosCust);
                    }
                }
                edgeVec.push_back(frontEdge);
                edgeQue.pop();
                edgeSet.erase(frontEdge);
            }
            while (custQue.size()) {
                auto frontCust = custQue.front();
                for (auto qosEdge : frontCust->qosEdgeVec) {
                    if (edgeNodeMap[qosEdge->nameID] == false) {
                        edgeNodeMap[qosEdge->nameID] = true;
                        edgeQue.push(qosEdge);
                    }
                }
                customerVec.push_back(frontCust);
                custQue.pop();
            }
        }
        graphVec.push_back(new Graph(edgeVec, customerVec, timeCnt));
    }
}

//�������solution.txt�ļ�
void OutPutMatrix(vector<vector<vector<int>>>& allocMatrix, vector<CustomerNode*> customerVec, vector<EdgeNode*> edgeVec) {
    if (customerVec.size() == 0)
        return;
    int timeCnt = customerVec[0]->demandsVec.size();

    fstream f(outFilePath, ios::out);
    if (f.bad()) {
        return;
    }
    string str;
    for (int timeIndex = 0; timeIndex < timeCnt; timeIndex++) {
        for (int custIndex = 0; custIndex < customerVec.size(); custIndex++) {
            str = customerVec[custIndex]->nameID + ":";
            for (int edgeIndex = 0; edgeIndex < edgeVec.size(); edgeIndex++) {
                if (allocMatrix[edgeIndex][custIndex][timeIndex] > 0) {
                    str += "<" + edgeVec[edgeIndex]->nameID + "," + to_string(allocMatrix[edgeIndex][custIndex][timeIndex]) + ">,";
                }
            }
            if (str.back() == ',') str.pop_back();
            f << str << endl;
        }
    }
    f.close();
}
void OutPutMatrix1(vector<vector<vector<int>>>& allocMatrix, vector<CustomerNode*> customerVec, vector<EdgeNode*> edgeVec) {
    if (customerVec.size() == 0)
        return;
    int timeCnt = customerVec[0]->demandsVec.size();

    fstream f(outFilePath, ios::out);
    if (f.bad()) {
        return;
    }
    string str;
    for (int edgeIndex = 0; edgeIndex < edgeVec.size(); edgeIndex++) {
        for (int timeIndex = 0; timeIndex < timeCnt; timeIndex++) {
            str = "time: " + to_string(timeIndex) + "    name: " + edgeVec[edgeIndex]->nameID + ":";
            for (int custIndex = 0; custIndex < customerVec.size(); custIndex++) {
                if (allocMatrix[edgeIndex][custIndex][timeIndex] > 0) {
                    str += "<" + customerVec[custIndex]->nameID + "," + to_string(allocMatrix[edgeIndex][custIndex][timeIndex]) + ">,";
                }
            }
            if (str.back() == ',') str.pop_back();
            f << str << endl;
        }
    }
    f.close();
}

//��֤�������
void VerifyMatrix(vector<vector<vector<int>>>& allocMatrix, vector<CustomerNode*> customerVec, vector<EdgeNode*> edgeVec, double percentNum) {
    vector<vector<int>> edegeBandwidthSeqVec(allocMatrix.size(), vector<int>(allocMatrix[0][0].size(), 0));//��Ե�ڵ��������
    vector<vector<int>> cusrBandwidthSeqVec(allocMatrix[0].size(), vector<int>(allocMatrix[0][0].size(), 0));//�û��ڵ��������

    /**********************************************************************************************************************/
    if (customerVec.size() == 0)
        return;
    int timeCnt = customerVec[0]->demandsVec.size();
    int index95Percent = ceil(timeCnt * 0.95) - 1;
    for (int timeIndex = 0; timeIndex < timeCnt; timeIndex++) {
        for (int edgeIndex = 0; edgeIndex < edgeVec.size(); edgeIndex++) {
            int demandSum = 0;
            for (int custIndex = 0; custIndex < customerVec.size(); custIndex++) {
                demandSum += allocMatrix[edgeIndex][custIndex][timeIndex];
            }
            edegeBandwidthSeqVec[edgeIndex][timeIndex] = demandSum;
            if (demandSum > edgeVec[edgeIndex]->bandwidth)
                cout << "��Ե�ڵ�" << edgeVec[edgeIndex]->nameID << "���䲻�Ϸ�" << endl;
        }
    }

    /**********************************************************************************************************************/
    for (int timeIndex = 0; timeIndex < timeCnt; timeIndex++) {
        for (int custIndex = 0; custIndex < customerVec.size(); custIndex++) {
            int demandSum = 0;
            for (int edgeIndex = 0; edgeIndex < edgeVec.size(); edgeIndex++) {
                demandSum += allocMatrix[edgeIndex][custIndex][timeIndex];
            }
            cusrBandwidthSeqVec[custIndex][timeIndex] = demandSum;
            //if (demandSum !=  customerVec[custIndex]->demandsVecTmp[timeIndex])
            //    cout << "�û��ڵ�" << customerVec[custIndex]->nameID << "���䲻�Ϸ�" << endl;
            if (customerVec[custIndex]->demandsVec[timeIndex] > 0) {
                cout << "�û��ڵ�δ������  << �û��ڵ㣺" << customerVec[custIndex]->nameID << ", ʣ������" << customerVec[custIndex]->demandsVec[timeIndex] << endl;
            }
        }
    }
    /**********************************************************************************************************************/
    fstream f1(".//output//custtomer.txt", ios::out);
    if (f1.bad()) {
        return;
    }
    for (int j = 0; j < customerVec.size(); j++) {
        f1 << customerVec[j]->nameID << ":\t";
        for (int k = 0; k < timeCnt; k++) {
            f1 << to_string(cusrBandwidthSeqVec[j][k]) << "\t";
        }
        f1 << endl;
    }
    f1.close();

    /**********************************************************************************************************************/
    fstream f2(".//output//custtomer.txt", ios::out);
    if (f2.bad()) {
        return;
    }
    for (int j = 0; j < customerVec.size(); j++) {
        f2 << customerVec[j]->nameID << ":\t";
        for (int k = 0; k < timeCnt; k++) {
            f2 << to_string(cusrBandwidthSeqVec[j][k]) << "\t";
        }
        f2 << endl;
    }
    f2.close();

    /**********************************************************************************************************************/
    fstream f3(".//output//edgeSequence_sort.txt", ios::out);
    if (f3.bad()) {
        return;
    }
    int cost = 0;
    for (int i = 0; i < edegeBandwidthSeqVec.size(); i++) {
        sort(edegeBandwidthSeqVec[i].begin(), edegeBandwidthSeqVec[i].end());
        cost += edegeBandwidthSeqVec[i][index95Percent];

        f3 << edgeVec[i]->indexID << "  :  ";
        f3 << edegeBandwidthSeqVec[i][index95Percent] << endl;
        for (auto n : edegeBandwidthSeqVec[i]) {
            f3 << n << "  ";
        }
        f3 << endl;
    }
    f3 << endl << "cost: " << to_string(cost) << endl;
    f3.close();
    /**********************************************************************************************************************/
    //fstream f3(".//output//vertify3.txt", ios::out);
    //if (f3.bad()) {
    //    return;
    //}
    //for (auto edge : edgeVec) {
    //    sort(edge->bandwidthLoadVec.begin(), edge->bandwidthLoadVec.end(), [=](pair<int, int>& p1, pair<int, int>& p2) {
    //        return p1.second < p2.second; });
    //    f3 << edge->nameID << ":  ";
    //    for (auto load : edge->bandwidthLoadVec) {
    //        f3 << "<" << load.first << ", " << load.second << ">, ";
    //    }
    //    f3 << endl << endl << endl;
    //}
    //f3.close();


    cout << "cost: " << to_string(cost) << endl;
}


//��ʾ����
void ShowData(vector<vector<int>>& QoSVec) {
    cout << "QoSVec:" << endl;
    for (auto row : QoSVec) {
        for (auto num : row) {
            cout << num << "  ";
        }
        cout << endl;
    }
}

void ShowData(unordered_map<string, CustomerNode*> _customerMap) {
    for (auto cus : _customerMap) {
        cout << "nameID: " << cus.second->nameID << endl;
        for (auto n : cus.second->demandsVec)
            cout << n << "  ";
        cout << endl << endl;
    }
}

void ShowData(unordered_map<string, EdgeNode*> _edgeMap) {
    for (auto edg : _edgeMap) {
        cout << "nameID: " << edg.second->nameID
            << "\timeIndex\tbandwidth: " << edg.second->bandwidth
            << endl;
    }
}