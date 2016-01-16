
/*******************************************
* Author: bravepam
*
* E-mail:1372120340@qq.com
*******************************************
*/


#ifndef CONTINUOUSVALUECLASSIFIER_H
#define CONTINUOUSVALUECLASSIFIER_H

#include<vector>
#include"sample.h"

using namespace std;

//����ֵ���������������Ժ���ɢֵ������������ɼ̳й�ϵ
class ContinuousValueBaseCalssifier
{
private:
	int fte_id;
	double fte_value;	//����ֵ
	bool less;		//���ڷ������
	double a;
public:
	ContinuousValueBaseCalssifier(int fi, double fv, double _a, bool _less) :
		fte_id(fi), fte_value(fv), less(_less), a(_a){}

	double decide(const vector<double> &x)const
	{//�û����������ľ��ߺ���
		int tag = 0;
		if (less) //Ϊ�棬��С�ڷ���Ϊ1
			tag = x[fte_id] < fte_value ? 1 : -1;
		else //Ϊ�٣���С�ڷ���Ϊ1
			tag = !(x[fte_id] < fte_value) ? 1 : -1;
		return a * tag;
	}
};

//����ֵ����������ѡ��׼��
class ContinuousClassifierFeatureSelection
{
private:
	vector<sample<double>> data; 
	vector<double> weight;		//����Ȩ��
private:
	pair<double, double> selectFeatureValue(int, bool&);
public:
	ContinuousClassifierFeatureSelection(vector<sample<double>> &d) :
		data(move(d)), weight()
	{
		double w = 1.0 / data.size();		//��ʼ����Ȩֵ
		weight.resize(data.size());
		fill_n(weight.begin(), weight.size(), w);
	}

	pair<double, int> select(double&, bool&);
	void updateWeight(const ContinuousValueBaseCalssifier&);
};

//��������fte_id��ѡ����ʹ��������ʴﵽ��С������ֵ������ֵΪ����ֵ����������ʺͷ������boolֵ
pair<double, double> ContinuousClassifierFeatureSelection::selectFeatureValue(int fte_id, bool &less)
{
	double min_error = INT_MAX, fte_value = 0.0;
	for (int i = 0; i != data.size(); ++i)
	{//���ڸ�������ÿһ��ֵ
		double error_less = 0.0, error_not_less = 0.0;
		for (int j = 0; j != data.size(); ++j)
		{//��ÿ����������ɸѡ
			if (data[j].x[fte_id] < data[i].x[fte_id]) //��С�ڣ������Ϊ1
				error_less += static_cast<double>(data[j].y != 1) * weight[j]; //�ۼƷ��������
			else error_less += static_cast<double>(data[j].y != -1) * weight[j];

			if (data[j].x[fte_id] >= data[i].x[fte_id]) //����С�ڣ������Ϊ1
				error_not_less += static_cast<double>(data[j].y != 1) * weight[j];
			else error_not_less += static_cast<double>(data[j].y != -1) * weight[j];
		}
		//�Ƚ���������µķ�������ʺ͵�ǰ��С��������ʣ��Ի����С��������ʵ���Ϣ��������
		if (error_less < min_error)
		{
			min_error = error_less;
			fte_value = data[i].x[fte_id];
			less = true;
		}
		if (error_not_less < min_error)
		{
			min_error = error_not_less;
			fte_value = data[i].x[fte_id];
			less = false;
		}
	}
	return pair<double, double>(fte_value, min_error);
}

//ѡ����ʹ�����������С������������ֵ����Ϊ����ֵ����������С��������ʺͷ������boolֵ
pair<double, int> ContinuousClassifierFeatureSelection::select(double &min_error, bool &less)
{
	int fte_values = data[0].x.size(), fte_id = -1;
	double fte_value = 0.0;
	min_error = INT_MAX;
	for (int j = 0; j != fte_values; ++j)
	{//��ÿһ������
		pair<double, double> slt = selectFeatureValue(j, less); //����ɸѡ
		if (slt.second < min_error)
		{//���ڵõ���ʹ�����������С����������Ϣ
			min_error = slt.second;
			fte_value = slt.first;
			fte_id = j;
		}
	}
	return pair<double, int>(fte_value, fte_id);
}

//���õ�ǰ�Ļ�����������������Ȩ��
void ContinuousClassifierFeatureSelection::updateWeight(const ContinuousValueBaseCalssifier &cvbc)
{
	double zm = 0.0;
	for (int i = 0; i != data.size(); ++i)
		zm += weight[i] * pow(e, -1.0 * data[i].y * cvbc.decide(data[i].x));
	for (int j = 0; j != weight.size(); ++j)
		weight[j] *= pow(e, -1.0 * data[j].y * cvbc.decide(data[j].x)) / zm;
}

//����ֵ������
class ContinuousValueClassifier
{
private:
	vector<ContinuousValueBaseCalssifier> cvbc; //��������������
	ContinuousClassifierFeatureSelection ccfs;	//����ѡ����
	double eps;		//����
public:
	ContinuousValueClassifier(vector<sample<double>> &data, double _e) :
		cvbc(), ccfs(data), eps(_e){}

	void train();
	int decide(const vector<double> &x)const
	{//���ߺ���
		double res = 0.0;
		for (int i = 0; i != cvbc.size(); ++i)
			res += cvbc[i].decide(x);	//�ۼƾ���ֵ
		if (res > 0.0) return 1;
		else return -1;
	}
};

//ѵ��������
void ContinuousValueClassifier::train()
{
	double prev_em = INT_MAX;		//ǰһ�η��������
	while (true)
	{
		double em = 0.0;
		bool less = false;
		pair<double, int> slt = ccfs.select(em, less); //ѡ�����ʺϵ�����
		if (em > prev_em) return; //����ǰһ����ȣ������û���½������˳�
		else prev_em = em;
		double a = 0.5 * log((1.0 - em) / em); //���������Ȩ��
		ContinuousValueBaseCalssifier bc(slt.second, slt.first, a, less); //�������������
		ccfs.updateWeight(bc); //��������Ȩ��
		cvbc.push_back(bc);
		if (em < eps) return; //�ﵽ���ȣ����˳�
	}
}


#endif