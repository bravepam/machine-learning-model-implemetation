#ifndef RANDOM_TREE_H
#define RANDOM_TREE_H

#include"FeatureSelect.h"
#include"util.h"
#include<vector>
#include<memory>

//���������
class RandomTree
{
private:
	struct node;
private:
	node* root;
	TreeDataSet tree_data; //�������ݼ�
	double oob_err = 0.0; //�������
	//double oob_permuted_err = 0.0; //�������ĳһ��������ֵ��Ĵ������
	size_t height = 0;
	InfoGain ig;
	const std::shared_ptr<const RFParams> prf;
	std::vector<bool> used_features;
private:
	//���ѡȡ����
	static std::vector<size_t> randomSelectFeatures(size_t, size_t);
	void create(std::vector<size_t>&, node*&);
	void clear(node*);
public:
	RandomTree(const std::shared_ptr<const RFParams>& p) :root(nullptr), tree_data(p), ig(&tree_data),
		prf(p), used_features(prf->D, false){}
	const TreeDataSet* create()
	{
		tree_data.bagging(prf->train_set.size());
		tree_data.oobData();
		create(tree_data.train_data, root);
		oobError();
		return &tree_data;
	}
	void clear()
	{
		if (root)
		{
			clear(root);
			root = nullptr;
		}
	}
	bool empty()const
	{
		return root == nullptr;
	}
	bool usedFeature(size_t fte_id)const
	{
		return used_features[fte_id];
	}
	int predict(const sample&)const;
	double oobError();
	//��������Ŷ���Ĵ������
	double permutedOobError(size_t);
};

#endif