//ID3�㷨��C4.5�㷨���ɵľ�����

#ifndef _ID3_C45_GINI_DTREE_H
#define _ID3_C45_GINI_DTREE_H

#include<vector>
#include"FeatureSelectionCriterion.h"

using namespace std;

struct basenode
{//ID3���ڵ�����
	//����ID��������ڽڵ㣬��ô�ǰ��ո�ֵ��������������еķָ
	//�����Ҷ�ӽڵ㣬��û�зָ��ˣ���ʱ��ֵΪ-1.
	int fte_id;

	//�ýڵ����������������ڽڵ㣬��û�������������ֵΪ-1��
	//�����Ҷ�ӽڵ㣬��ֵ���ʾ�������
	int cls_value;

	//����ýڵ����ڽڵ㣬��ᰴ������fte_id���зָ��������ֵ�Ĳ�ͬ���������
	//���ɸ����ӣ�������������ֵ���������������Ҷ�ӽڵ㣬��Ϊ�ա�
	vector<basenode*> child;
	basenode(int _fte_id, int _cls_value) :fte_id(_fte_id), cls_value(_cls_value), child(){}
	virtual ~basenode(){}
};

struct C45_node :public basenode
{//C45���ڵ����ͣ��̳���ID3���ڵ㣬������һ����Ϣ��ʧ��
	double loss;//��������ýڵ�����к��Ӻ����Ϣ��ʧ
	C45_node(int _fte_id, int _cls_value) :basenode(_fte_id, _cls_value), loss(0.0){}
	virtual ~C45_node(){}
};

class ID3tree
{//ID3������
protected:
	vector<int> data_id;
	basenode *root = nullptr;
	static InfoGain *pig;//����ѡ��׼��
	double eps;//��ֵ�����׼����������Ϣ���棨�ȣ����ڸ���ֵ�����ٷָ�ڵ㣬������ΪҶ�ӽڵ�
private:
	basenode* create(const vector<int>&);
	void clear(basenode*);
public:
	ID3tree(int samples_num, double _e) :data_id(samples_num), eps(_e)
	{
		for (int i = 0; i != data_id.size(); ++i)
			data_id[i] = i;
	}
	static void initCriterion(const vector<sample<int>> &d, const vector<int> &fv, int cn)
	{
		pig = new InfoGain(d, fv, cn);
	}
	void create()
	{
		root = create(data_id);
	}
	int compute(const vector<int>&);//���Ҹ����������������
	void clear()
	{//�����
		clear(root);
		delete pig;
		root = nullptr;
	}
	virtual ~ID3tree()
	{
		clear();
	}
};

InfoGain *ID3tree::pig;//�����׼��

basenode* ID3tree::create(const vector<int> &data_id)
{
	vector<int> cls_count;
	pair<bool, int> ret = pig->checkData(data_id, cls_count);//������ݼ�
	if (ret.first || !pig->hasFeature())
	{//�����ͬһ�����߲�������������
		basenode *p = new basenode(-1, ret.second);//����ռ���������ֵ����Ҷ�ӽڵ�
		return p;
	}
	vector<vector<int>> splited_data;
	//������ݵ�ǰ���ݼ�����ѡ��������ѡ�����ŷָ����splited_data���洢�ָ��������Ӽ�
	//����ֵΪ������Ϣ��������ŷָ�����ID
	pair<double, int> slt = pig->select(data_id, splited_data);
	if (slt.first < eps)
	{//�����Ϣ����ϵ�
		basenode *p = new basenode(-1, ret.second);//���ٷָ��ռ���������ֵ����Ҷ�ӽڵ�
		return p;
	}
	//���򴴽��ڽڵ�
	basenode *p = new basenode(slt.second, -1);
	p->child.resize(pig->getSpecificFeatureValues(slt.second));//��������ֵ�������亢��
	pig->zeroSpecificFeatureValues(slt.second);//������ֵ������0��ʾ���������ٿ���
	for (int i = 0; i != splited_data.size(); ++i)
		p->child[i] = create(splited_data[i]);//�м�������ֵ���ͻ��м��������Ӽ�����һ��������
	return p;
}

int ID3tree::compute(const vector<int> &data)
{//�������������������
	basenode *curr = root;
	while (curr->fte_id != -1)//���ݷָǰ�ڵ������ID�����������ֵ���жϸ����ĸ����ӽڵ���Ѱ
		curr = curr->child[data[curr->fte_id]];
	return curr->cls_value;//ֱ��Ҷ�ӣ����ظ�Ҷ����𼴿�
}

void ID3tree::clear(basenode *r)
{
	for (int i = 0; i != r->child.size(); ++i)
		clear(r->child[i]);
	//cout << r << ' ' << r->fte_id << ' ' << r->cls_value << endl;
	delete r;
}

class C45tree:public ID3tree
{//C45���������̳���ID3��
private:
	double a;//a���ڿ���Ԥ�����������Ӷ�ƽ��Ĳ���
	basenode* create(const vector<int>&);
public:
	C45tree(int samples_num, double _e, double _a) :ID3tree(samples_num, _e), a(_a){}

	//�ض���������������
	static void initCriterion(const vector<sample<int>> &d, const vector<int> &fv, int cn)
	{
		pig = new InfoGainRatio(d, fv, cn);
	}
	void create()
	{
		root = create(data_id);
	}
	virtual ~C45tree()
	{
		
	}
};

basenode* C45tree::create(const vector<int> &data_id)
{
	vector<int> cls_count;
	pair<bool, int> ret = pig->checkData(data_id, cls_count);//������ݼ�
	//���ýڵ���ΪҶ�Ӷ���������ʧ
	double temp_loss = data_id.size() * 1.0 * pig->entropyAux(data_id.size(), cls_count) + a;
	if (ret.first || !pig->hasFeature())
	{//�����ͬһ�����߲�������������
		C45_node *p = new C45_node(-1, ret.second);//����ռ���������ֵ����Ҷ�ӽڵ�
		p->loss = temp_loss;
		return p;
	}
	vector<vector<int>> splited_data;

	//������ݵ�ǰ���ݼ�����ѡ��������ѡ�����ŷָ����splited_data���洢�ָ��������Ӽ�
	//����ֵΪ������Ϣ����Ⱥ����ŷָ�����ID
	pair<double, int> slt = pig->select(data_id, splited_data);
	if (slt.first < eps)
	{//�����Ϣ����Ƚϵ�
		C45_node *p = new C45_node(-1, ret.second);//���ٷָ��ռ���������ֵ����Ҷ�ӽڵ�
		p->loss = temp_loss;
		return p;
	}

	//���򴴽��ڽڵ�
	C45_node *p = new C45_node(slt.second, -1);
	p->loss = temp_loss;
	p->child.resize(pig->getSpecificFeatureValues(slt.second));//��������ֵ�������亢��
	pig->zeroSpecificFeatureValues(slt.second);//������ֵ������0��ʾ���������ٿ���
	for (int i = 0; i != splited_data.size(); ++i)
		p->child[i] = create(splited_data[i]);//�м�������ֵ���ͻ��м��������Ӽ�����һ��������

	double sum_loss = 0.0;
	for (int j = 0; j != p->child.size(); ++j)
		sum_loss += static_cast<C45_node*>(p->child[j])->loss;//�ۼӺ��ӽڵ����ʧ
	if (sum_loss >= p->loss)
	{//�������֦����ʧ�����˼�֦�ģ����֦
		for (int k = 0; k != p->child.size(); ++k)
			delete p->child[k];//��ôҪ���м�֦
		p->child.clear();
		p->fte_id = -1, p->cls_value = ret.second;//�޸ĸýڵ�ΪҶ�ӽڵ�
	}
	else p->loss = sum_loss;//���򲻼�����������ʧΪ����֦����ʧ
	return p;
}

#endif