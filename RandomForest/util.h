#ifndef UTIL_H
#define UTIL_H

#include<vector>
#include<random>
#include<memory>

//�����ṹ
struct sample
{
	std::vector<double> x;
	int y;

	template <typename U>
	sample(U&& u, int y_) :x(std::forward<std::vector<double>>(u)), y(y_){}
};

struct RFParams;

//ÿ��������������õ����ݼ�
struct TreeDataSet
{
	std::vector<size_t> train_data; //ѵ����
	std::vector<size_t> oob; //�������ݼ������洢����id
	const RFParams* const prf;

	TreeDataSet(const RFParams* const p) :prf(p){}
	//����bagging�㷨���ѵ����
	void bagging(size_t);
	//����������ݼ�
	const std::vector<size_t>& getOobData();
};

struct TreeDataSets
{
	std::vector<TreeDataSet*> datasets;

};

//��ֹ�����ṹ
struct Termcriteria
{
	const double eps; //����ֵ
	const size_t iter; //����������ֵ
	const size_t num; //���ݼ���С��ֵ
	Termcriteria(double e, size_t n, size_t i) :
		eps(e), num(n), iter(i){}
};

struct RFParams
{
	std::vector<sample> train_set;
	std::vector<sample> test_set;
	const size_t cls_num;
	const size_t D;
	const size_t F;
	const size_t N;
	const size_t depth;
	Termcriteria tc;
	const bool calc_var_importance;

	template <typename U>
	RFParams(U&& ts1, U&& ts2, size_t d, size_t cn, size_t f, size_t n, size_t dep, bool c,
		double e, size_t num, size_t i) :train_set(std::forward<std::vector<sample>>(ts1)),
		test_set(std::forward<sample>(ts2)),
		cls_num(cn), D(d), F(f), N(n), depth(dep), tc(e, num, i), calc_var_importance(c){}
};

extern void randomVectorEngine(size_t, size_t, std::vector<size_t>&);

#endif