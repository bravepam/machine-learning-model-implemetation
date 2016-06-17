#ifndef RANDOM_TREE_H
#define RANDOM_TREE_H

#include"FeatureSelect.h"
#include"util.h"
#include<vector>

//���������
class RandomTree
{
private:
	struct node;
private:
	node* root;
	TreeDataSet tree_data; //�������ݼ�
	double oob_err = 0.0; //�������
	double oob_permuted_err = 0.0; //�������ĳһ��������ֵ��Ĵ������
	size_t height = 0;
	InfoGain ig;
	const RFParams* const prf;
private:
	//���ѡȡ����
	static std::vector<size_t> randomSelectFeatures(size_t, size_t);
	void create(std::vector<size_t>&, node*&);
	void clear(node*);
public:
	RandomTree(const RFParams* const p) :root(nullptr), tree_data(p), ig(&tree_data),
		prf(p){}
	void create()
	{
		tree_data.bagging(prf->train_set.size());
		tree_data.getOobData();
		create(tree_data.train_data, root);
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
	int predict(const sample&)const;
	void computeOobErr();
	//��������Ŷ���Ĵ������
	void permuteFeature(size_t);
};

#endif