#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <map>

using namespace std;

//INI�ļ����洢�ṹ
class ININode
{
public:
	ININode(string root, string key, string value)
	{
		this->root = root;
		this->key = key;
		this->value = value;
	}
	string root;
	string key;
	string value;
};

//��ֵ�Խṹ��
class SubNode
{
public:
	void InsertElement(string key, string value)
	{
		sub_node.insert(pair<string, string>(key, value));
	}
	map<string, string> sub_node;
};

//INI�ļ�������
class CMyINI
{
public:
	CMyINI();
	CMyINI(string _path);
	~CMyINI();

public:
	int ReadINI(string path);													//��ȡINI�ļ�
	string GetValue(string root, string key);									//�ɸ����ͼ���ȡֵ
	vector<ININode>::size_type GetSize() { return map_ini.size(); }				//��ȡINI�ļ��Ľ����
	vector<ININode>::size_type SetValue(string root, string key, string value);	//���ø����ͼ���ȡֵ
	int WriteINI(string path);			//д��INI�ļ�
	void Clear() { map_ini.clear(); }	//���
	void Travel();						//������ӡINI�ļ�
private:
	map<string, SubNode> map_ini;		//INI�ļ����ݵĴ洢����
};
