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
	sample(U&& u, int y_) :x(std::forward<U>(u)), y(y_){}
};

struct RFParams;

//ÿ��������������õ����ݼ�
struct TreeDataSet
{
	std::vector<size_t> train_data; //ѵ����
	std::vector<size_t> oob; //�������ݼ������洢����id
	const std::shared_ptr<const RFParams> prf;

	TreeDataSet(const std::shared_ptr<const RFParams>& p) :prf(p){}
	//����bagging�㷨���ѵ����
	void bagging(size_t);
	//�����ѵ�����ݼ��Ĳ�����Ϊ�������ݣ�ȫ��Ϊ���ɭ�ֵ�ѵ�����ݼ�
	const std::vector<size_t>& oobData();
	bool contains(size_t)const;
};

//��ֹ�����ṹ�����õ�����Ϊ0
struct Termcriteria
{
	const double eps; //����ֵ
	const size_t num; //���ݼ���С��ֵ
	const size_t depth; //��������
	const size_t iter; //����������ֵ
	Termcriteria(double e, size_t n, size_t d, size_t i) :
		eps(e), num(n), depth(d), iter(i){}
};

struct RFParams
{
	std::vector<sample> train_set;
	std::vector<sample> test_set;
	const size_t cls_num;
	const size_t D;
	const size_t F;
	const size_t N;
	Termcriteria tc;
	const bool calc_fte_importance;

	template <typename U>
	RFParams(U&& ts1, U&& ts2, size_t d, size_t cn, size_t f, size_t n, bool c,
		const Termcriteria& t) :train_set(std::forward<U>(ts1)), test_set(std::forward<U>(ts2)),
		cls_num(cn), D(d), F(f), N(n), tc(t), calc_fte_importance(c){}
};

extern unsigned int prev_seed;
//��������ͬ�����������X(i+1) = {X(i) * A + C} mod B,�˴�A = 16807,C = 0,B = 2147483647(2^31 - 1)
using lce = std::linear_congruential_engine < unsigned long, 16807, 0, 2147483647 >;

extern std::mt19937 getMt19937(); //�������ͬ�����������������������ӳ�ʼ��

//�зŻط�ʽ�������һ������
extern void samplingWithReplacement(size_t, size_t, std::vector<size_t>&);

//�޷Żط�ʽ�������һ������
extern void samplingNoReplacement(size_t, size_t, std::vector<size_t>&);

//��������Ϊѵ���������Լ��������Ŀ������ά�ȣ��������ڵ�ʱ��ѡȡ��������������������
//ѵ����ֹ�������Ƿ����������Ҫ��
extern std::shared_ptr<RFParams> newRFParams
(	const std::vector<sample>&,
	const std::vector<sample>&,
	size_t, size_t, size_t, size_t,
	const Termcriteria&,
	bool
);

#endif