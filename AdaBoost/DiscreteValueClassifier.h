//Boost������������ֵ����ɢ��

#ifndef DISCRETEVALUECLASSIFIER_H
#define DISCRETEVALUECLASSIFIER_H

#include<vector>
#include"sample.h"

using namespace std;

//��ɢ����ֵ����������
class DiscreteValueBaseClassifier
{
private:
	int fte_id;		//��ѡ����
	int fte_value;		//��Ӧ������ĳ����ֵ
	bool equal;		//��������ξ���
	double a;		//������ϵ������Ȩ��
public:
	DiscreteValueBaseClassifier(int fi, int fv, double _a, bool loe) :
		fte_id(fi), fte_value(fv), a(_a), equal(loe){}
	double decide(const vector<int> &x)const
	{//�û����������ľ��ߺ���
		int tag = 0;
		if (equal) //���ڣ������Ϊ1
			tag = x[fte_id] == fte_value ? 1 : -1;
		else  //�����ڣ������Ϊ1
			tag = x[fte_id] != fte_value ? 1 : -1;
		return a * tag;
	}
};

//��ɢֵ������������ѡ��׼��
class DiscreteClassifierFeatureSelection
{
private:
	vector<sample<int>> data;		//��������
	vector<double> weight;		//����������Ȩ��
	vector<int> fte_values;		//ÿ��������ȡֵ����
private:
	pair<double, int> selectFeatureValue(int, bool&);
public:
	DiscreteClassifierFeatureSelection(vector<sample<int>> &d, vector<int> &fv)
		:data(move(d)), fte_values(move(fv)), weight()
	{
		double w = 1.0 / data.size();		//��ʼ��ÿ��������Ȩ�ؾ�Ϊ1 / N
		weight.resize(data.size());
		fill_n(weight.begin(), data.size(), w);
	}

	bool hasFeature()const
	{//�ж��Ƿ�����������
		for (int j = 0; j != fte_values.size(); ++j)
			if (fte_values[j] != 0) return true;
		return false;
	}

	void zeroFeature(int fte_id)
	{//����������ã�����0
		fte_values[fte_id] = 0;
	}

	void updateWeight(const DiscreteValueBaseClassifier &bc)
	{//ͨ����ǰ�Ļ�����������������Ȩ��
		double zm = 0.0;
		for (int i = 0; i != data.size(); ++i)
			zm += weight[i] * pow(e, -1.0 * data[i].y * bc.decide(data[i].x));
		for (int j = 0; j != weight.size(); ++j)
			weight[j] *= pow(e, -1.0 * data[j].y * bc.decide(data[j].x)) / zm;
	}

	pair<int, int> select(double&, bool&);
};

//����ѡ�񣬷���ֵ����Ϊ��ѡ����������ֵ����С��������ʺʹﵽ�������ʱ�ķ������boolֵ
pair<int, int> DiscreteClassifierFeatureSelection::select(double &min_error, bool &equal)
{
	int fte_id = -1, fte_value = -1;
	min_error = INT_MAX;
	for (int i = 0; i != fte_values.size(); ++i)
	{//����ÿһ������
		if (fte_values[i] == 0) continue;

		//����ѡ���ܹ�ʹ������������С������ֵ����Ϣ
		pair<double, int> slt = selectFeatureValue(i, equal);
		if (slt.first < min_error)
		{//�������������ʱȵ�ǰ��С�������
			min_error = slt.first;
			fte_id = i;
			fte_value = slt.second;
		}
	}
	return pair<int, int>(fte_id, fte_value);
}

//��������fte_id��ѡ����ʹ������������С������ֵ������ֵ����Ϊ��������ʡ�����ֵ�Լ�����boolֵ
pair<double, int> DiscreteClassifierFeatureSelection::selectFeatureValue(int fte_id, bool &equal)
{
	double min_error = INT_MAX;
	int temp_fte_value = -1;
	for (int j = 0; j != fte_values[fte_id]; ++j)
	{//���ڸ�������ÿһ��ȡֵ
		double error_equal = 0.0, error_not_equal = 0.0;
		for (int i = 0; i != data.size(); ++i)
		{//ɨ��ÿһ������
			if (data[i].x[fte_id] == j) //����������Ӧֵ���ڸ�����ֵ
				//�����Ϊ1���ۼӷ��������
				error_equal += static_cast<double>(data[i].y != 1) * weight[i];
			else
				error_equal += static_cast<double>(data[i].y != -1) * weight[i];

			if (data[i].x[fte_id] != j) //����������Ӧֵ�����ڸ�����ֵ
				//�����Ϊ1���ۼӷ��������
				error_not_equal += static_cast<double>(data[i].y != 1) * weight[i];
			else
				error_not_equal += static_cast<double>(data[i].y != -1) * weight[i];
		}
		//�Ƚ���������µķ�������ʺ͵�ǰ��С��������ʣ��Ի����Сֵ�����������Ϣ
		if (error_equal < min_error)
		{
			min_error = error_equal;
			temp_fte_value = j;
			equal = true;
		}
		if (error_not_equal < min_error)
		{
			min_error = error_not_equal;
			temp_fte_value = j;
			equal = false;
		}
	}
	return pair<double, int>(min_error, temp_fte_value);
}

//��ɢֵBoost������
class DicreteValueClassifier
{
private:
	vector<DiscreteValueBaseClassifier> dvbc;		//��������������
	DiscreteClassifierFeatureSelection dcfs;		//����ѡ����
	double eps;		//���ȣ����ƺ�ʱѵ��ֹͣ
public:
	DicreteValueClassifier(vector<sample<int>> &data, vector<int> &fv, double _e)
		:dvbc(), dcfs(data, fv), eps(_e){}
	void train();
	int decide(const vector<int> &d)const
	{//���ߺ���
		double res = 0.0;
		for (int i = 0; i != dvbc.size(); ++i)
			res += dvbc[i].decide(d); //�ۼ�ÿ���������ļ�Ȩ����ֵ
		if (res > 0.0) return 1;
		else return -1;
	}
};

//ѵ��������
void DicreteValueClassifier::train()
{
	double prev_em = INT_MAX; //ǰһ�εķ��������
	while (true)
	{
		if (!dcfs.hasFeature()) return; //û����������ʱ������
		double em = 0.0;
		bool equal = false;
		pair<int, int> slt = dcfs.select(em, equal); //ѡ��������ʹ�����������С
		if (em > prev_em) return; //����÷��������û���½������˳�
		else prev_em = em;
		double a = 0.5 * log((1 - em) / em); //���������Ȩ��
		DiscreteValueBaseClassifier bc(slt.first, slt.second, a, equal); //�������������
		dvbc.push_back(bc);
		dcfs.updateWeight(bc);		//��������Ȩֵ
		dcfs.zeroFeature(slt.first);		//��0��ǰ��������ʾ����
		if (em < eps) return;
	}
}


#endif