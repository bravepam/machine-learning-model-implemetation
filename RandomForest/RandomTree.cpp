#include"RandomTree.h"
#include"util.h"
#include<vector>
#include<utility>
#include<cassert>
#include<algorithm>

struct RandomTree::node
{
	int fte_id_or_cls; //�ڽڵ㣬���Ƿָ�����ID��Ҷ�ӽڵ㣬�������ֵ
	double split_value;
	bool leaf;
	node* less = nullptr;
	node* greater = nullptr;
	node(int fc, double fv, bool l = false) :fte_id_or_cls(fc), split_value(fv), leaf(l){}
};

std::vector<size_t> RandomTree::randomSelectFeatures(size_t slt_num, size_t feat_num)
{
	std::vector<size_t> features;
	samplingNoReplacement(slt_num, feat_num, features);
	return std::move(features);
}

void RandomTree::create(std::vector<size_t>& data_id, node*& r)
{
	ig.clear(); //���ǰһ��create������������Ϣ
	//��������������
	std::vector<size_t> features = randomSelectFeatures(prf->F, prf->D);
	ig.setFeaturesAndData(features, data_id);
	std::hash_map<int, size_t> cls_count;
	//��鵱ǰ�������ݼ�������ֵ{{�Ƿ�ͬһ���ռ���������}�����ͳ����Ϣ}
	std::pair<bool, size_t> ret = ig.checkData(cls_count);

	++height;
	//�ĸ�������ֹ��������������1�����ݼ�Ϊͬһ���2�����ݼ�̫С��3����ֵ̫С����
	//������λͬһ���4�����Ѿ��㹻�����ïʢ
	if (ret.first || 
		(prf->tc.num > 0 && data_id.size() <= prf->tc.num) ||
		(prf->tc.eps > 0.0 && ig.entropyAux(cls_count, data_id.size()) <= prf->tc.eps) ||
		(prf->tc.depth > 0 && height >= prf->tc.depth))
	{
		r = new node(ret.second, 0.0, true); //��ռ�����������Ϊ��Ҷ�ӵ����ֵ
		return;
	}

	std::vector<std::vector<size_t>> splited_data_id(2);
	//���򣬰�����Ϣ����׼��ѡȡ��ѷָ����ֵ{{��ѷָ���������ѷָ��ֵ}���ָ����������ݼ�}
	std::pair<size_t, double> slt = ig.select(splited_data_id);
	used_features[slt.first] = true;
	r = new node(slt.first, slt.second);
	//����������
	create(splited_data_id[0], r->less);
	create(splited_data_id[1], r->greater);
}

void RandomTree::clear(node* r)
{
	if (r->less)
		clear(r->less);
	if (r->greater)
		clear(r->greater);
	delete r;
}

int RandomTree::predict(const sample& s)const
{
	assert(!empty());
	node* cur = root;
	while (!cur->leaf)
	{
		if (s.x[cur->fte_id_or_cls] < cur->split_value)
			cur = cur->less;
		else
			cur = cur->greater;
	}
	return cur->fte_id_or_cls;
}

double RandomTree::oobError()
{
	if (oob_err > 0.0)
		return oob_err;
	const auto& raw_data = prf->train_set;
	size_t err = 0;
	for (size_t i = 0; i != tree_data.oob.size(); ++i)
	{
		const int pred = predict(raw_data[tree_data.oob[i]]);
		err += static_cast<size_t>(pred != raw_data[tree_data.oob[i]].y);
	}
	oob_err = err * 1.0 / tree_data.oob.size();
	return oob_err;
}

double RandomTree::permutedOobError(size_t which)
{
	const auto& raw_data = prf->train_set;
	const size_t size = tree_data.oob.size();
	std::vector<double> which_value;
	which_value.reserve(size);
	for (size_t i = 0; i != size; ++i) //������ݼ���which������Ӧ������ֵ
		which_value.push_back(raw_data[tree_data.oob[i]].x[which]);
	//�������
	std::random_shuffle(which_value.begin(), which_value.end());

	size_t err = 0;
	for (size_t i = 0; i != size; ++i)
	{
		node* cur = root;
		while (!cur->leaf)
		{
			if (cur->fte_id_or_cls == which)
			{//�����ǰ���ڵ�ķָ������Ǹո���������Ŷ�������
				//�����Ŷ����ֵ�ж�
				if (which_value[i] < cur->split_value)
					cur = cur->less;
				else cur = cur->greater;
			}
			else
			{//���������ж�
				if (raw_data[tree_data.oob[i]].x[cur->fte_id_or_cls] < cur->split_value)
					cur = cur->less;
				else
					cur = cur->greater;
			}
		}
		err += static_cast<size_t>(raw_data[tree_data.oob[i]].y != cur->fte_id_or_cls);
	}
	const double oob_permuted_err = err * 1.0 / size;
	return oob_permuted_err;
}