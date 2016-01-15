//������һ����������ģ�壬���ݲ�ͬ������ѡ��׼���ʵ������ͬ�ľ�����
//����ʵ����ID3�㷨��C4.5�㷨���ɵľ�����


#ifndef ID3ANDC45DTREE_H
#define ID3ANDC45DTREE_H

#include<vector>
#include"FeatureSelectionCriterion.h"

using namespace std;

struct node
{//���ڵ�����
	//����ID��������ڽڵ㣬��ô�ýڵ��ǰ��ո�ֵ��������������еķָ
	//�����Ҷ�ӽڵ㣬��û�зָ��ˣ���ʱ��ֵΪ-1.
	int fte_id;

	//�ýڵ����������������ڽڵ㣬��û�������������ֵΪ-1��
	//�����Ҷ�ӽڵ㣬��ֵ���ʾ�������
	int cls_value;

	//����ýڵ����ڽڵ㣬��ᰴ������fte_id���зָ��������ֵ�Ĳ�ͬ���������
	//���ɸ����ӣ�������������ֵ���������������Ҷ�ӽڵ㣬��Ϊ�ա�
	vector<node*> child;
	double loss;//��������ýڵ�����к��Ӻ����Ϣ��ʧ������֦ʱ����
	node(int _fte_id, int _cls_value) :fte_id(_fte_id), cls_value(_cls_value), 
		child(),loss(0.0){}
};

template<typename ctr>
class Dtree
{//�������ṹ��ģ�������ʾ����ѡ��׼������
private:
	node *root = nullptr;
	ctr *pctr;
	vector<int> data_id;
	double e;//��ֵ�����׼����������Ϣֵ���ڸ���ֵ�����ٷָ�ڵ㣬������ΪҶ�ӽڵ�
private:
	node* create(vector<int>&,bool,double);
	void clear(node*);
public:
	Dtree(const vector<sample<int>> &d, const vector<int> &fv, int cn, double _e) :
		pctr(new ctr(d, fv, cn)),e(_e)
	{
		for (int i = 0; i != d.size(); ++i)
			data_id.push_back(i);
	}
	void create(bool prune = false,double a = 0.0)
	{//������������Ĭ���ǲ���֦�ģ�a���ڿ���Ԥ�����������Ӷ�ƽ��Ĳ���
		root = create(data_id,prune,a);
	}
	int compute(const vector<int>&);//���Ҹ����������������
	void clear()
	{//�����
		clear(root);
		delete pctr;
		root = nullptr;
	}
	~Dtree()
	{
		clear();
	}
};

template <typename ctr>
node* Dtree<ctr>::create(vector<int> &data_id,bool prune,double a)
{
	pair<bool, int> ret = pctr->checkData(data_id);//������ݼ�

	//���ýڵ���ΪҶ�Ӷ���������ʧ
	double temp_loss = data_id.size() * 1.0 * pctr->entropy(data_id, -1) + a;
	if (ret.first || !pctr->hasFeature())
	{//�����ͬһ�����߲�������������
		node *p = new node(-1, ret.second);//����ռ���������ֵ����Ҷ�ӽڵ�
		if (prune) p->loss = temp_loss;//�����֦����������ʧֵ����ͬ
		return p;
	}
	vector<vector<int>> splited_data;

	//������ݵ�ǰ���ݼ�����ѡ��������ѡ�����ŷָ����splited_data���洢�ָ��������Ӽ�
	//����ֵΪ���ŷָ�����ID�����õ�����Ϣֵ
	pair<double, int> slt = pctr->select(data_id, splited_data);
	if (slt.first < e)
	{//�����Ϣֵ�ϵ�
		node *p = new node(-1, ret.second);//���ٷָ��ռ���������ֵ����Ҷ�ӽڵ�
		if (prune) p->loss = temp_loss;
		return p;
	}

	//���򴴽��ڽڵ�
	node *p = new node(slt.second, -1);
	if (prune) p->loss = temp_loss;
	p->child.resize(pctr->getSpecificFeatureValues(slt.second));//��������ֵ�������亢��
	pctr->zeroSpecificFeatureValues(slt.second);//������ֵ������0��ʾ���������ٿ���
	for (int i = 0; i != splited_data.size(); ++i)
		p->child[i] = create(splited_data[i],prune,a);//�м�������ֵ���ͻ��м��������Ӽ�����һ��������

	if (prune)
	{//��Ҫ��֦
		double sum_loss = 0.0;
		for (int j = 0; j != p->child.size(); ++j)
			sum_loss += p->child[j]->loss;//�ۼӺ��ӽڵ����ʧ
		if (sum_loss >= p->loss)
		{//�������֦����ʧ�����˼�֦�ģ����֦
			for (int k = 0; k != p->child.size(); ++k)
				delete p->child[k];//��ôҪ���м�֦
			p->child.clear();
			p->fte_id = -1, p->cls_value = ret.second;//�޸ĸýڵ�ΪҶ�ӽڵ�
		}
		else p->loss = sum_loss;//���򲻼�����������ʧΪ����֦����ʧ
	}
	return p;
}

template <typename ctr>
int Dtree<ctr>::compute(const vector<int> &data)
{//�������������������
	node *curr = root;
	while (curr->fte_id != -1)//���ݷָǰ�ڵ������ID�����������ֵ���жϸ����ĸ����ӽڵ���Ѱ
		curr = curr->child[data[curr->fte_id]];
	return curr->cls_value;//ֱ��Ҷ�ӣ����ظ�Ҷ����𼴿�
}

template <typename ctr>
void Dtree<ctr>::clear(node *r)
{
	for (int i = 0; i != r->child.size(); ++i)
		clear(r->child[i]);
	//cout << r << ' ' << r->fte_id << ' ' << r->cls_value << endl;
	delete r;
}

//����Ϣ������Ϊѡ��׼���ID3�㷨���ɵľ�����
typedef Dtree<InfoGain>				ID3_Dtree;

//����Ϣ�������Ϊѡ��׼���C4.5�㷨���ɵľ�����
typedef Dtree<InfoGainRatio>		C45_Dtree;

#endif