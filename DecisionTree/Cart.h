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
	Gini *pgini;//���û���ָ����Ϊ����ѡ��׼��
	vector<int> data_id;//������id�ż���
	int e;//��ֵ��������С��Ŀ
	double g;//��ֵ������ָ����Сֵ��С����������ֵ����һ�������ʾ���ٷָ���
private:
	void create(vector<int>&, node*&);
	void clear(node*);
public:
	Cart(const vector<sample<int>> &d, const vector<int> &fv, int cn, double _g, int _e) :
		data_id(d.size()), pgini(new Gini(d, fv, cn)), g(_g), e(_e)
	{
		for (int i = 0; i != data_id.size(); ++i)
			data_id[i] = i;
	}
	void create()
	{
		create(data_id, root);
	}
	void clear()
	{
		clear(root);
		delete pgini;
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
	vector<int> cls_count;
	pair<bool, int> ret = pgini->checkData(data_id, cls_count);//������ݼ�
	if (ret.first || !pgini->hasFeature() || pgini->entropyAux(data_id.size(), cls_count) <= g)
	{//���������ͬһ�࣬���߲����п������������߸����ݼ��Ļ���ָ��С����ֵ
		r = new node(-1, ret.second);//�򴴽�Ҷ�ӽڵ㣬��ռ���������Ϊ�ýڵ����
		return;//����
	}
	//���򣬽����зָ�
	vector<vector<int>> splited_data_id;//��¼�ָ��������Ӽ�����
	//ѡ�����ŷָ���������ȡֵ
	pair<double, int> slt = pgini->select(data_id, splited_data_id);
	r = new node((int)slt.first, slt.second);//������ֻ��������
	pgini->zeroSpecificFeatureValues((int)slt.first);//��������ȡֵ������0����ʾ���ٿ���
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

/*����Cart��֦�㷨�����ݡ�ͳ��ѧ�������Ľ��ܣ��ܽ��һ����ϸ���еķ������£�ûʵ�֣�
 *1���ݹ����£��ڽ�����ʱ��ÿ����һ���½ڵ㣬����������Ըõ�ΪҶ�ӽڵ��Ԥ�����C_t��
 *   ��������ϣ���ô�����ڵ���ΪҶ�ӽڵ��Ԥ�����Ҳ��������ˣ���������ǽ��ýڵ��
 *   �����������õ���Ԥ�����������ڽ����Ĺ����оͿ��Խ���֦���Ԥ���������������
 *2�����¶��ϵķ���ÿһ���ڽڵ㣬�ýڵ��֦���Ԥ�������֮ǰ�Ѿ����������������֦����
 *   ���ڵ�Ԥ������������Ϊ���������ĺ��ӽڵ��Ԥ�����֮�ͣ��ڷ��ʵ������Ҳ��������
 *   ����Ҷ�ڵ��Ԥ�����֮�ͣ���������ߵ�Ԥ�����Ĳ�ľ���ֵ��Ҷ�ӽڵ���Ŀ����ʱ
 *   Ϊ1������������Ҷ�ӣ�������һ�����ľ���ֵ�ıȣ������g_t�����£����������һ�����ϡ�Ȼ����
 *   һ�㷵�أ����ص����ݰ�����Ҷ�ӽڵ�����Ͳ���֦��Ԥ��������Ҷ�ӽڵ��Ԥ�����֮�ͣ���
 *  �Թ��ϲ�ڵ�ʹ�ã��������g_t��ֱ�����ڵ������ϣ�
 *3����ʼ��֦��
 *   (1) ��ʼ��a0 = 0����ʱ��������Ϊ���ţ���ΪT0��
 *   (2) �Ӽ���g_t��ѡ����СԪ�أ���Ϊa1�����϶��±���T0���������ĳһ�ڵ��g_tֵ����a1,��
 *       �����֦���Զ�������Ϊ�ýڵ���𣬵õ�������ΪT1������;
 *   (3) ������g_t������ѡȡ�ڶ�С��Ԫ�أ���Ϊa2�����϶��±���T1��......����ΪT2�����أ�
 *   (4) ��˷�����ֱ���õ�����Tn��һ�ŵ��ڵ�����ֻ�������������˼�֦��ϡ�
 *  ��ʱ[ai,ai+1)��Ӧ��Ti��i = 0,1,2...n������a0 = 0.�����ý�����֤�õ���������Ta��aֵ��
 */