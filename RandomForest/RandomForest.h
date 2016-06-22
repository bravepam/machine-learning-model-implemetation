#ifndef RANDOM_FOREST_H
#define RANDOM_FOREST_H

#include"RandomTree.h"
#include"util.h"
#include<memory>

//���ɭ��
class RandomForest
{
private:
	std::vector<std::shared_ptr<RandomTree>> rf; //���������
	std::shared_ptr<const RFParams> prf = nullptr; //���ɭ�ֲ���
	std::vector<const TreeDataSet*> datasets; //���������õ�bagging���ݼ�
	std::vector<std::pair<size_t, double>> fis; //������Ҫ�ԣ�{��������Ҫ�Զ���ֵ}
	double gen_err = 0.0; //�������
	double test_err = 0.0; //�������
	double oob_err = 0.0; //������ķ������
public:
	RandomForest() :rf(), datasets(){}
	void setParams(std::shared_ptr<RFParams>&);
	void train();
	int predict(const sample&)const;
	double testError();
	double generalizationError();
	//������Ĵ�������ֵ
	double avgOobErrorOfTree();
	const std::vector<std::pair<size_t, double>>& FeatureImportance();
	static std::vector<sample> loadData(const std::string&, size_t, size_t&);
};

#endif