#include"RandomForest.h"
#include"util.h"
#include<hash_map>
#include<algorithm>
#include<cassert>
#include<string>
#include<fstream>

void RandomForest::setParams(const RFParams* p)
{
	assert(p);
	prf.reset(p);
	rf.reserve(p->N);
	datasets.reserve(p->N);
	fis.reserve(p->D);
}

void RandomForest::train()
{
	assert(!prf->train_set.empty());
	for (size_t i = 0; i != prf->N; ++i)
	{
		//����һ�������
		std::shared_ptr<RandomTree> prt = std::make_shared<RandomTree>(prf);
		//ѵ���������ѵ����Ϻ󽫷������õ����ݼ�ָ��
		const TreeDataSet* ptds = prt->create();
		rf.emplace_back(prt); //������ӵ�����
		datasets.push_back(ptds); //���ݼ�Ҳ��
	}
	if (prf->calc_fte_importance) //�����Ҫ����������Ҫ��
		FeatureImportance();
}

int RandomForest::predict(const sample& s)const
{
	assert(!rf.empty());
	std::hash_map<int, size_t> cls_count;
	for (size_t i = 0; i != rf.size(); ++i)
	{//���ÿһ��������������һ��Ԥ�����
		++cls_count[rf[i]->predict(s)];
	}

	std::pair<int, size_t> max{ INT_MIN, 0 };
	for_each(cls_count.begin(), cls_count.end(),
		[&max](const std::hash_map<int,size_t>::value_type& item)
	{//Ȼ�����Щ���ͳ����Ϣ���ҳ�ռ�����������Ϊ���
		if (item.second > max.second)
			max = item;
	});
	return max.first;
}

double RandomForest::testError()
{
	assert(!prf->test_set.empty());
	if (test_err > 0.0) //�Ѽ��㣬��ֱ�ӷ��أ���ͬ
		return test_err;
	size_t error = 0;
	for_each(prf->test_set.begin(), prf->test_set.end(), [&error, this](const sample& s)
	{
		const int pred = predict(s);
		error += static_cast<size_t>(pred != s.y);
	});
	test_err = error * 1.0 / prf->test_set.size();
	return test_err;
}

//���㷺�������ô���������
double RandomForest::generalizationError()
{
	if (gen_err > 0.0)
		return gen_err;
	std::vector<double> oob_errors; //��¼ÿ�����������
	double sum_error = 0.0;
	const std::vector<sample>& train = prf->train_set;
	oob_errors.reserve(train.size());
	for (size_t i = 0; i != train.size(); ++i)
	{//����ѵ�����е�ÿ������
		size_t error = 0, howmany_trees = 0;
		for (size_t j = 0; j != rf.size(); ++j)
		{//�����е����������Ѱ
			if (!datasets[j]->contains(i))
			{//����������õ�ѵ������û���������������Ǹ����Ĵ�������
				const int pred = rf[j]->predict(train[i]); //��ô������Ԥ��
				error += static_cast<size_t>(pred != train[i].y);
				++howmany_trees;
			}
		}
		const double temp = error * 1.0 / howmany_trees; //�����������
		sum_error += temp; //�������
		oob_errors.push_back(temp);
	}
	gen_err = sum_error / oob_errors.size(); //���ķ������
	return gen_err;
}

double RandomForest::avgOobErrorOfTree()
{
	if (oob_err > 0.0)
		return oob_err;
	double sum_error = 0.0;
	for_each(rf.begin(), rf.end(), [&sum_error](const std::shared_ptr<RandomTree>& rt)
	{
		sum_error += rt->oobError();
	});
	oob_err = sum_error / rf.size();
	return oob_err;
}

//����f����Ҫ�Զ����������·�ʽ���㣺����ÿ��ʹ��������f���������������У�Ҳ���Լ�������
//���Ҹ�������ѵ�����ݼ�������f��ֵ��Ȼ���ټ�����������ż�ȥ��������µĴ�������
//����������f����Ҫ�Զ��������������������������f��Ҫ�Եľ�ֵ��Ϊ��������Ҫ��
const std::vector<std::pair<size_t, double>>& RandomForest::FeatureImportance()
{
	assert(prf->calc_fte_importance);
	if (!fis.empty())
		return fis;
	for (size_t i = 0; i != prf->D; ++i)
	{
		double sum_ooberror_of_trees_used_fte_i = 0.0,
			sum_pererror_of_trees_used_fte_i = 0.0;
		for (size_t j = 0; j != rf.size(); ++j)
		{
			if (rf[i]->usedFeature(i)) //��������������������ʹ���˸�����
			{
				sum_ooberror_of_trees_used_fte_i += rf[i]->oobError();
				sum_pererror_of_trees_used_fte_i += rf[i]->permutedOobError(i);
			}
		}
		fis.emplace_back(i, sum_pererror_of_trees_used_fte_i - sum_ooberror_of_trees_used_fte_i);
	}
	//����������Ҫ�Է���������
	std::sort(fis.begin(), fis.end(), [](const std::pair<size_t, double>& lhs,
		const std::pair<size_t, double>& rhs)->bool
	{
		return lhs.second > rhs.second;
	});
	return fis;
}

std::vector<sample> RandomForest::loadData(const std::string& filename, size_t D, size_t& size)
{
	std::ifstream infile(filename);
	assert(infile);
	std::vector<sample> data;
	data.reserve(size);
	double fte_val = 0.0;
	int cls_val = 0;
	std::vector<double> x;
	while (true)
	{
		x.reserve(D);
		while (infile >> fte_val)
		{
			x.push_back(fte_val);
			if (x.size() == D)
			{
				infile >> cls_val; //�Ѷ�ȡ��һ������
				data.emplace_back(std::move(x), cls_val);
				break;
			}
		}
		if (!infile) break;
	}
	size = data.size(); //���ؼ��ص�������
	return std::move(data);
}