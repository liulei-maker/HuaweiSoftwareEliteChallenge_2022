#pragma once
#include<iostream>
#include<vector>
#include<string>
#include<fstream>
#include <sstream>
#include<map>
#include<unordered_map>
#include<algorithm>
#include<math.h>
#include<set>
#include<queue>
#include<time.h>
#include <math.h>

using namespace std;
struct EdgeNode;
//�����û��ڵ�ṹ��
struct CustomerNode {
	CustomerNode(string _nameID, int _indexID) {
		nameID = _nameID;
		indexID = _indexID;
	};

	string nameID;			//�û��ڵ��string ID
	int indexID;			//�û��ڵ��index ID
	vector<int> demandsVec;			//�û��ڵ���ʱ��ά���ϵĴ�������ֲ�
	vector<int> demandMarginSumVec;	//���û��ڵ���Χ��Ե�ڵ���ʣ�������ܺ�
	vector<EdgeNode*> qosEdgeVec;	//���û����Ե�ڵ�ͨ��С��qosConstraint,��ʾ���û����Ժ���Щ��Ե�ڵ㽨���������
};

//�����Ե�ڵ�ṹ��
struct EdgeNode {
	EdgeNode(string _nameID, int _bandwidth, int _timeCnt, int _indexID) {
		nameID = _nameID;
		bandwidth = _bandwidth;
		demandMarginVec = vector<int>(_timeCnt, _bandwidth);
		bandwidthLoad = vector<int>(_timeCnt, 0);
		maxDemandNum = 0;
		isFullFlag = vector<bool>(_timeCnt, false);
		indexID = _indexID;
		edgeCost = 0;
		predictCost = 0;
	};
	string nameID;		//��Ե�ڵ��string ID
	int indexID;		//��Ե�ڵ��index ID
	int bandwidth;		//��Ե�ڵ��ܴ�������
	int maxDemandNum;	//����������������ڱ��5%
	int edgeCost;		//�ýڵ�ĳɱ�
	long predictCost;	//�ýڵ��Ԥ��ɱ�
	vector<bool> isFullFlag;			//��¼��Щʱ��ڵ㱻�����
	vector<int> bandwidthLoad;		//��Ե�ڵ������ 
	vector<int> demandMarginVec;	//�û��ڵ��ڴ�������֮�󣬱�Ե�ڵ����ʣ��
	vector<int> edgeDemandSum;		//��ǰ��Ե�ڵ����������ܺ�
	vector<CustomerNode*> qosCustVec;//�ñ�Ե���û��ڵ�ͨ��С��qosConstraint,��ʾ�ñ�Ե���Ժ���Щ�û��ڵ㽨���������
};

class Graph {
public:
	Graph(vector<EdgeNode*> _edgeVec,
		vector<CustomerNode*>_customerVec,
		int _timeCnt) {
		this->timeCnt = _timeCnt;
		edgeVec = _edgeVec;
		customerVec = _customerVec;
		for (int timeIndex = 0; timeIndex < _timeCnt; timeIndex++) {
			int rowDemandSum = 0;
			for (auto cust : this->customerVec) {
				rowDemandSum += cust->demandsVec[timeIndex];
			}
			this->rowDemandSum.push_back(rowDemandSum);
		}
		rowFullEdgeNum = vector<int>(_timeCnt, 0);
	};
	~Graph() {};
	int timeCnt;					 //ͼ�����е�ʱ�̵�
	vector<EdgeNode*> edgeVec;		 //��ͼ�����еı�Ե�ڵ�����
	vector<CustomerNode*>customerVec;//��ͼ�����е��û��ڵ�����
	vector<int> rowDemandSum;		 //��ǰ���������б�Ե�ڵ㱻������ܺ�
	vector<int> rowFullEdgeNum;	 //ÿһ����д������������
private:
};

struct RowAverDemandMax {
	RowAverDemandMax(int _averDemand, int _timeIndex) {
		averDemand = _averDemand;
		timeIndex = _timeIndex;
	};
	int averDemand;	//��ǰ���������б�Ե�ڵ㱻�����ƽ����
	int timeIndex;	//��ǰʱ���

	//�Զ����������
	//�����������
	bool operator<(const RowAverDemandMax& a) const {
		return averDemand < a.averDemand; //�󶥶�
	}
};

struct EdgeDemandMax {
	EdgeDemandMax(EdgeNode* _edgeNode, int _edgeDemandSum, int _timeIndex) {
		edgeNode = _edgeNode;
		edgeDemandSum = _edgeDemandSum;
		timeIndex = _timeIndex;
	};
	EdgeNode* edgeNode;
	int edgeDemandSum;	//��ǰ��Ե�ڵ����������ܺ�
	int timeIndex;		//��ǰʱ���

	//�Զ����������
	//�����������������ı�Ե�ڵ�����
	bool operator<(const EdgeDemandMax& a) const {
		return edgeDemandSum < a.edgeDemandSum; //�󶥶�
	}
};

struct EdgeDemandMin {
	EdgeDemandMin(EdgeNode* _edgeNode, int _timeIndex) {
		edgeNode = _edgeNode;
		priority = _edgeNode->edgeDemandSum[_timeIndex] / (double)(_edgeNode->demandMarginVec[_timeIndex]);
	};
	EdgeNode* edgeNode;
	double priority;
	vector<bool> edgeFlag;
	bool operator<(const EdgeDemandMin& a) const {
		return priority > a.priority; //С����
	}
};

//��������
int ReadCSVData(vector<CustomerNode*>& _customerVec,	//�ͻ��ڵ�ID
	vector<EdgeNode*>& _edgeVec,			//��Ե�ڵ�ID
	int& _timeCnt,							//ʱ��������
	int _qosConstraint);
void ReadINIConfig(int& qosConstraint);
void ReadEdge2Graph(vector<CustomerNode*>& _customerVec, vector<EdgeNode*>& _edgeVec, vector<Graph*>& graphVec);

void OutPutMatrix(vector<vector<vector<int>>>& allocMatrix, vector<CustomerNode*> customerVec, vector<EdgeNode*> edgeVec); //����ļ�Ϊfile
void OutPutMatrix1(vector<vector<vector<int>>>& allocMatrix, vector<CustomerNode*> customerVec, vector<EdgeNode*> edgeVec);
void VerifyMatrix(vector<vector<vector<int>>>& allocMatrix, vector<CustomerNode*> customerVec, vector<EdgeNode*> edgeVec, double percentNum = 0.95); //��֤�������

//��ʾ����
void ShowData(vector<vector<int>>& QoSVec);
void ShowData(unordered_map<string, CustomerNode*> customerMap);
void ShowData(unordered_map<string, EdgeNode*> edgeMap);
