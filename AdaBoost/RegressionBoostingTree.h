#ifndef REGRESSIONBOOSTINGTREE_H
#define REGRESSIONBOOSTINGTREE_H

#include<utility>
#include<vector>
#include"sample.h"

using namespace std;

//�ع������Boost����������������������׮
class RegressionBaseClassifier
{
private:
	int fte_id;
	double fte_value;
	double less;		//С������ֵ����ȡ��ֵ
	double great;		//��С�ڣ���ȡ��ֵ
public:
	RegressionBaseClassifier(pair<pair<double, double>, pair<double, int>> &args) :
		fte_id(args.second.second), fte_value(args.second.first),
		less(args.first.first), great(args.first.second){}

    double predict(const vector<double> &x)const
	{//����
		return x[fte_id] < fte_value ? less : great;
	}
};

//�ع���������ѡ��׼��
class RegressionFeatureSelection
{
private:
	vector<sample<double>> data;	//����
	//vector<double> residual;		//ÿ�λ���µ�������֮���Ԥ��вֱ���������и���
private:
	double squareError(double, const vector<int>&);
	pair<pair<double, double>, pair<double, double>> selectFeatureValue(int);

public:
	RegressionFeatureSelection(vector<sample<double>> &d) :data(move(d)){}

	pair<pair<double, double>, pair<double, int>> select(double&);
	vector<sample<double>>& getData()
	{
		return data;
	}
};

//ѡ����ʹԤ�������С����������Ϣ������ֵ����Ϊ<less,great>��<����ֵ������>�Լ���СԤ�����ƽ����
pair<pair<double, double>, pair<double, int>> RegressionFeatureSelection::select(double &min_error)
{
	int fte_values = data[0].x.size();
	pair<double, double> min_avg;
	pair<double, int> fte;
	min_error = INT_MAX;
	for (int i = 0; i != fte_values; ++i)
	{//��ÿ������
		//ѡ����ʹԤ�����ƽ������С������ֵ����Ϣ
		pair<pair<double, double>, pair<double, double>> slt = selectFeatureValue(i);
		if (slt.second.second < min_error)
		{//���ȵ�ǰԤ�����ƽ���͸�С�������
			min_error = slt.second.second;
			min_avg = slt.first;
			fte = make_pair(slt.second.first, i);
		}
	}
	return make_pair(min_avg, fte);
}

//����ĳһ�������ݵķ����Ԥ�����ƽ����
double RegressionFeatureSelection::squareError(double avg, const vector<int> &data_id)
{
	double error = 0.0;
	for (int i = 0; i != data_id.size(); ++i)
		error += (data[data_id[i]].y - avg) * (data[data_id[i]].y - avg);
	return error;
}

//��������fte_id��ѡ�����ʺϵ�����ֵ������ֵΪ<less��great>��<����ֵ����СԤ�����ƽ����>
pair<pair<double, double>, pair<double, double>> RegressionFeatureSelection::selectFeatureValue(int fte_id)
{
	pair<double, double> avg, value_error = make_pair(-1.0, INT_MAX);
	for (int i = 0; i != data.size(); ++i)
	{
		vector<int> less, not_less;
		double less_avg = 0.0, not_less_avg = 0.0;
		for (int j = 0; j != data.size(); ++j)
		{
			if (data[j].x[fte_id] < data[i].x[fte_id])
			{
				less.push_back(j);
				less_avg += data[j].y;
			}
			else
			{
				not_less.push_back(j);
				not_less_avg += data[j].y;
			}
		}
		less_avg /= (less.empty() ? 1 : less.size());
		not_less_avg /= (not_less.empty() ? 1 : not_less.size());
		double error = squareError(less_avg, less) + squareError(not_less_avg, not_less);
		if (error < value_error.second)
		{
			value_error = make_pair(data[i].x[fte_id], error);
			avg = make_pair(less_avg, not_less_avg);
		}
	}
	return make_pair(avg, value_error);
}

//�ع�����������
class BoostingTree
{
private:
	vector<RegressionBaseClassifier> vrbc;	//��׮����
	RegressionFeatureSelection rfs;			//����ѡ����
	double eps;			//��������
	const int iter;		//����������Ĭ�ϲ�����
private:
	void updataWeight(const RegressionBaseClassifier&, vector<sample<double>>&);
public:
	BoostingTree(vector<sample<double>> &d, double _e, int _iter = -1) :
		vrbc(), rfs(d), eps(_e), iter(_iter){}

	void train();
	double predict(const vector<double>&);
};

//Ԥ���������ֵ
double BoostingTree::predict(const vector<double> &x)
{
	double res = 0.0;
	for (int i = 0; i != vrbc.size(); ++i)
		res += vrbc[i].predict(x);  //ÿ����׮��Ԥ��ֵ֮��
	return res;
}

//ѵ��������
void BoostingTree::train()
{
	int i = 0; //��������
	while (true)
	{
		++i;
		double min_error = 0.0;
		//ѡ����ʹԤ�����ƽ������С����������Ϣ
		pair<pair<double, double>, pair<double, int>> slt = rfs.select(min_error);
		RegressionBaseClassifier rbc(slt); //������׮
		vrbc.push_back(rbc);
		updataWeight(rbc, rfs.getData()); //����Ԥ�����������м���һ��Ԥ��
		if (min_error < eps || i == iter) return; //������һ�����˳�
	}
}

//����Ԥ�����
void BoostingTree::updataWeight(const RegressionBaseClassifier &rbc, vector<sample<double>> &data)
{
	for (int i = 0; i != data.size(); ++i)
		data[i].y -= rbc.predict(data[i].x); //��ǰֵ��ȥԤ��ֵ����Ԥ�����
}

#endif