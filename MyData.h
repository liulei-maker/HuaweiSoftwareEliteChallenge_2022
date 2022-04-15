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
//定义用户节点结构体
struct CustomerNode {
	CustomerNode(string _nameID, int _indexID) {
		nameID = _nameID;
		indexID = _indexID;
	};

	string nameID;			//用户节点的string ID
	int indexID;			//用户节点的index ID
	vector<int> demandsVec;			//用户节点在时间维度上的带宽请求分布
	vector<int> demandMarginSumVec;	//该用户节点周围边缘节点所剩的余量总和
	vector<EdgeNode*> qosEdgeVec;	//该用户与边缘节点通信小于qosConstraint,表示该用户可以和哪些边缘节点建立带宽分配
};

//定义边缘节点结构体
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
	string nameID;		//边缘节点的string ID
	int indexID;		//边缘节点的index ID
	int bandwidth;		//边缘节点总带宽能力
	int maxDemandNum;	//最大请求数量，用于标记5%
	int edgeCost;		//该节点的成本
	long predictCost;	//该节点的预测成本
	vector<bool> isFullFlag;			//记录哪些时间节点被分配过
	vector<int> bandwidthLoad;		//边缘节点带宽负载 
	vector<int> demandMarginVec;	//用户节点在带宽请求之后，边缘节点带宽剩余
	vector<int> edgeDemandSum;		//当前边缘节点带宽被请求的总和
	vector<CustomerNode*> qosCustVec;//该边缘与用户节点通信小于qosConstraint,表示该边缘可以和哪些用户节点建立带宽分配
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
	int timeCnt;					 //图中所有的时刻点
	vector<EdgeNode*> edgeVec;		 //该图中所有的边缘节点数据
	vector<CustomerNode*>customerVec;//该图中所有的用户节点数据
	vector<int> rowDemandSum;		 //当前所在行所有边缘节点被请求的总和
	vector<int> rowFullEdgeNum;	 //每一行填写最大请求的数量
private:
};

struct RowAverDemandMax {
	RowAverDemandMax(int _averDemand, int _timeIndex) {
		averDemand = _averDemand;
		timeIndex = _timeIndex;
	};
	int averDemand;	//当前所在行所有边缘节点被请求的平均数
	int timeIndex;	//当前时间点

	//自定义排序规则
	//优先排最大行
	bool operator<(const RowAverDemandMax& a) const {
		return averDemand < a.averDemand; //大顶堆
	}
};

struct EdgeDemandMax {
	EdgeDemandMax(EdgeNode* _edgeNode, int _edgeDemandSum, int _timeIndex) {
		edgeNode = _edgeNode;
		edgeDemandSum = _edgeDemandSum;
		timeIndex = _timeIndex;
	};
	EdgeNode* edgeNode;
	int edgeDemandSum;	//当前边缘节点带宽被请求的总和
	int timeIndex;		//当前时间点

	//自定义排序规则
	//行中优先排最大被请求的边缘节点数据
	bool operator<(const EdgeDemandMax& a) const {
		return edgeDemandSum < a.edgeDemandSum; //大顶堆
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
		return priority > a.priority; //小顶堆
	}
};

//函数定义
int ReadCSVData(vector<CustomerNode*>& _customerVec,	//客户节点ID
	vector<EdgeNode*>& _edgeVec,			//边缘节点ID
	int& _timeCnt,							//时刻总数量
	int _qosConstraint);
void ReadINIConfig(int& qosConstraint);
void ReadEdge2Graph(vector<CustomerNode*>& _customerVec, vector<EdgeNode*>& _edgeVec, vector<Graph*>& graphVec);

void OutPutMatrix(vector<vector<vector<int>>>& allocMatrix, vector<CustomerNode*> customerVec, vector<EdgeNode*> edgeVec); //输出文件为file
void OutPutMatrix1(vector<vector<vector<int>>>& allocMatrix, vector<CustomerNode*> customerVec, vector<EdgeNode*> edgeVec);
void VerifyMatrix(vector<vector<vector<int>>>& allocMatrix, vector<CustomerNode*> customerVec, vector<EdgeNode*> edgeVec, double percentNum = 0.95); //验证输出矩阵

//显示数据
void ShowData(vector<vector<int>>& QoSVec);
void ShowData(unordered_map<string, CustomerNode*> customerMap);
void ShowData(unordered_map<string, EdgeNode*> edgeMap);
