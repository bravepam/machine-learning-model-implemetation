
/*******************************************
* Author: bravepam
*
* E-mail:1372120340@qq.com
*******************************************
*/


//CART�ع�����������С������Ϊ����ѡ��׼�����ɵ�����Ϊ��С���˻ع���

#ifndef LEASTSQUAREREGRESSIONTREE_H
#define LEASTSQUAREREGRESSIONTREE_H

#include<vector>
#include"FeatureSelectionCriterion.h"

using namespace std;

struct node
{//�ڵ�
	int split_fte;//������ڽڵ㣬���Ƿָ������������Ҷ�ӽڵ㣬����-1
	double value;//������fte_id�£��õ����ŷָ�ʱ��ȡ��ֵ
	node *small = nullptr;//С�ڻ���ڸ�ֵ������
	node *large = nullptr;//���ڸ�ֵ������
	node(int sf, double sv) :split_fte(sf), value(sv){}
};

class Rtree
{//�ع���
private:
	node *root = nullptr;
	LeastSquareError lse;//������С������Ϊ����ѡ��׼��
	int e;//��ֵ������������Сֵ
	vector<int> data_id;//������ID����
private:
	void create(vector<int>&, node*&);
	void clear(node*);
public:
	Rtree(const vector<sample<double>> &d, int _e) :e(_e), lse(d)
	{
		for (int i = 0; i != d.size(); ++i)
			data_id.push_back(i);
	}
	void create()
	{
		create(data_id, root);
	}
	void clear()
	{
		clear(root);
		root = nullptr;
	}
	double compute(vector<double>&);//������������ֵ
	~Rtree()
	{
		clear();
	}
};

void Rtree::create(vector<int> &data_id, node *&r)
{//�����ع���
	if ((int)data_id.size() <= e)
	{//���������̫С
		r = new node(-1, lse.average(data_id));//���ٷָ��������ֵ�ľ�ֵ����Ҷ�ӽڵ�
		return;
	}

	//����ָ�ڵ�
	vector<vector<int>> splited_data_id;
	pair<double, int> ret = lse.select(data_id, splited_data_id);//�ҵ����ŷָ������ͷָ��
	r = new node(ret.second, ret.first);//���䴴���ڵ�
	create(splited_data_id[0], r->small);//�ݹ鴴���������ڡ�����
	create(splited_data_id[1], r->large);//�ݹ鴴�������ڡ�����
}

double Rtree::compute(vector<double> &x)
{//����������x�Ļع�ֵ
	node *curr = root;
	while (curr->split_fte != -1)
	{
		if (x[curr->split_fte] <= curr->value)
			curr = curr->small;
		else curr = curr->large;
	}
	return curr->value;
}

void Rtree::clear(node *r)
{//�����
	if (r == nullptr) return;
	clear(r->small);
	clear(r->large);
	delete r;
}

#endif