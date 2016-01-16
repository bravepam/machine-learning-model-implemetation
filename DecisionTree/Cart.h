
/*******************************************
* Author: bravepam
*
* E-mail:1372120340@qq.com
*******************************************
*/


//CART�����������û���ָ����Ϊ����ѡ��׼��

#ifndef _CART_H
#define _CART_H

#include<vector>
#include"FeatureSelectionCriterion.h"

using namespace std;

struct node
{//�ڵ�
	int fte_id;//������ڽڵ㣬��ô�Ǹýڵ�ķָ���������Ҷ����Ϊ-1
	int fte_cls_value;//������ڽڵ㣬��������fte_id��ȡֵ�����������ȡֵ
	node *yes = nullptr;//������fte_id��fte_cls_valueֵ�ָ���ڸ�ֵ�����ɵ�����
	node *no = nullptr;//�����ڸ�ֵ�����ɵ�����
	node(int fi, int fcv) :fte_id(fi), fte_cls_value(fcv){}
};

class Cart
{//������
private:
	node *root;
	Gini gini;//���û���ָ����Ϊ����ѡ��׼��
	vector<int> data_id;//������id�ż���
	int e;//��ֵ��������С��Ŀ
	double g;//��ֵ������ָ����Сֵ��С����������ֵ����һ�������ʾ���ٷָ���
private:
	void create(vector<int>&, node*&);
	void clear(node*);
public:
	Cart(const vector<sample<int>> &d, const vector<int> &fv, int cn, double _g, int _e) :
		gini(d, fv, cn), g(_g), e(_e)
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
	~Cart()
	{
		clear();
	}
	int compute(vector<int>&);//��������������
};

void Cart::create(vector<int> &data_id, node *&r)
{//����Cart������
	pair<bool, int> ret = gini.checkData(data_id);//������ݼ�
	if (ret.first || !gini.hasFeature() || gini.entropy(data_id, -1) <= g)
	{//���������ͬһ�࣬���߲����п������������߸����ݼ��Ļ���ָ��С����ֵ
		r = new node(-1, ret.second);//�򴴽�Ҷ�ӽڵ㣬��ռ���������Ϊ�ýڵ����
		return;//����
	}
	//���򣬽����зָ�
	vector<vector<int>> splited_data_id(2);//��¼�ָ��������Ӽ�����
	pair<double, int> slt = gini.select(data_id, splited_data_id);//ѡ�����ŷָ���������ȡֵ
	r = new node((int)slt.first, slt.second);//������ֻ��������
	gini.zeroSpecificFeatureValues((int)slt.first);//��������ȡֵ������0����ʾ���ٿ���
	create(splited_data_id[0], r->yes);//�ݹ鴴�����ǡ�����
	create(splited_data_id[1], r->no);//�ݹ鴴����������
}

void Cart::clear(node *r)
{//�����
	if (r == nullptr) return;
	clear(r->yes);
	clear(r->no);
	delete r;
}

int Cart::compute(vector<int> &x)
{//����x�����
	node *curr = root;
	while (curr->fte_id != -1)
	{
		if (x[curr->fte_id] == curr->fte_cls_value)
			curr = curr->yes;
		else curr = curr->no;
	}
	return curr->fte_cls_value;
}

#endif