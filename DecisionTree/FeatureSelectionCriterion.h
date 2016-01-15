/**************************************************************************************
 *��ͷ�ļ���������Ҫ���Ǿ���������ѡ���׼�������֣��ṹ���£�
 *1��criterion�ǰ����������麯���ĳ�����࣬��Ϊ����׼����ĸ���ʹ�ã�
 *2��InfoGain��������Ϣ������ѡ���������̳���criterion���ض������������麯����
 *   �����Լ������ݳ�Ա��
 *3��InfoGainRatio��������Ϣ�������ѡ���������̳���InfoGain����һ���Լ�
 *   ��˽�к������ض�����select�麯����
 *4��Gini�����ڻ���ָ����ѡ���������̳���InfoGain���ض������������麯����
 *5��LeastSquareError��������Сƽ������ѡ���������������ɻع���������ѡ���޻���
 **************************************************************************************/


#ifndef FEATURESELECTIONCRITERION_H
#define FEATURESELECTIONCRITERION_H

#include<iostream>
#include<utility>
#include<vector>

using namespace std;

template<typename T>
struct sample
{//����
	vector<T> x;//�����㣬������
	T y;//����ȡֵ�����������
	sample(vector<T> &_x, T _y) :x(move(_x)), y(_y){}
	void print()const
	{
		cout << '(';
		for (int i = 0; i != x.size(); ++i)
		{
			cout << x[i];
			if (i != x.size() - 1)
				cout << ',';
		}
		cout << ") " << y << endl;
	}
};

//-------------------------------------------------------------------------------------------------------
class criterion
{//����ѡ��׼�������
public:
	virtual double entropy(vector<int>&, int) = 0;//������
	virtual pair<double, int> conditionalEntropy(vector<int>&, int, vector<vector<int>>&) = 0;//����������
	//ѡ�����ŷָ�����
	virtual pair<double, int> select(vector<int>&, vector<vector<int>>&) = 0;
};

//-------------------------------------------------------------------------------------------------------
class InfoGain :public criterion
{//������Ϣ���������ѡ�񣬼̳���׼�����
	//��ʵ�����е��麯��
protected:
	vector<sample<int>> data;//��������
	vector<int> feature_values;//ÿ����������ȡ������ֵ�ĸ���
	int cls_num;//������
public:
	InfoGain(const vector<sample<int>> &d, const vector<int> &fv, int cn) :
		data(move(d)), feature_values(move(fv)), cls_num(cn){}

	//�Զ������������
	//��鵱ǰ���ݼ��Ƿ���ͬһ�������ǣ��򷵻�true�����ֵ��
	//������ǣ��򷵻�false��ռ���������ֵ
	pair<bool, int> checkData(vector<int>&);
	bool hasFeature();//�Ƿ��п�������
	int getSpecificFeatureValues(int fte_id)
	{
		return feature_values[fte_id];
	}
	void zeroSpecificFeatureValues(int fte_id)
	{
		feature_values[fte_id] = 0;
	}

	//�������ݼ������������
	virtual double entropy(vector<int>&, int);
	//�������ݼ������ĳһ������������
	virtual pair<double, int> conditionalEntropy(vector<int>&, int, vector<vector<int>>&);
	virtual pair<double, int> select(vector<int>&, vector<vector<int>>&);
	virtual ~InfoGain(){}
};

pair<bool, int> InfoGain::checkData(vector<int> &data_id)
{//������ݼ�
	vector<int> cls_count(cls_num, 0);//����ÿ������������
	for (int i = 0; i != data_id.size(); ++i)
	{
		for (int j = 0; j != cls_count.size(); ++j)
			if (data[data_id[i]].y == j)
			{
				++cls_count[j];
				break;
			}
	}
	int count = 0, cls_value = -1;
	for (int k = 0; k != cls_count.size(); ++k)
		if (cls_count[k] > count)
		{//����ռ���������
			count = cls_count[k];
			cls_value = k;
		}
	return pair<bool, int>(count == data_id.size(), cls_value);
}

bool InfoGain::hasFeature()
{//����Ƿ�����������
	bool has = false;
	for (int i = 0; i != feature_values.size(); ++i)
	{
		has = feature_values[i] != 0;//�������ȡֵ������Ϊ0����˵������
		if (has) return true;
	}
	return false;
}

//fte_id��Ϊ����ID
double InfoGain::entropy(vector<int> &data_id, int fte_id)
{
	int value_num = (fte_id == -1 ? cls_num : feature_values[fte_id]);
	vector<int> specific_fte_value_count(value_num, 0);//������ֵ����ռ������
	for (int i = 0; i != data_id.size(); ++i)
	{//ͳ�Ƹ�����ֵ��������
		for (int j = 0; j != value_num; ++j)
			if ((fte_id == -1 ? data[data_id[i]].y : data[data_id[i]].x[fte_id]) == j)
			{
				++specific_fte_value_count[j];
				break;
			}
	}
	double epy = 0.0, prob = 0.0;
	for (int k = 0; k != specific_fte_value_count.size(); ++k)
	{//������
		if (specific_fte_value_count[k] > 0)
		{//�����Ӽ��������0
			prob = specific_fte_value_count[k] * 1.0 / data_id.size();
			epy += -prob * log2(prob);
		}
	}
	return epy;
}

/*������ĳһ�����µ�������
*fte_id�Ǹ�����ID��splited_data_id�Ƿָ��������ݼ�����
*/
pair<double, int> InfoGain::conditionalEntropy(vector<int> &data_id, int fte_id, 
	vector<vector<int>> &splited_data_id)
{
	for (int i = 0; i != data_id.size(); ++i)
	{//��ÿ������
		for (int j = 0; j != feature_values[fte_id]; ++j)
			if (data[data_id[i]].x[fte_id] == j)
			{//ͳ���������������ݼ�
				splited_data_id[j].push_back(data_id[i]);
				break;
			}
	}
	double cdl_epy = 0.0, prob = 0.0;
	for (int k = 0; k != splited_data_id.size(); ++k)
	{//����������
		if (!splited_data_id[k].empty())
		{
			prob = splited_data_id[k].size() * 1.0 / data_id.size();//�������Ӽ���ռ�����ݼ���Ƶ��
			cdl_epy += prob * entropy(splited_data_id[k], -1);//������
		}
	}
	return pair<double, int>(cdl_epy, int());//ֻ��Ҫ������
}

/*ѡ�����ŵķָ�����
*splited_data_id�����ŷָ��������ݼ�����
*/
pair<double, int> InfoGain::select(vector<int> &data_id, vector<vector<int>> &splited_data_id)
{
	int split_fte_id = -1;//�ָ������������
	//�����������
	double cls_epy = entropy(data_id, -1), max_info_gain = 0.0;
	for (int j = 0; j != feature_values.size(); ++j)
	{//��ÿһ������
		if (feature_values[j] == 0) continue;//��ʾ�������Ѿ����ù���
		vector<vector<int>> temp_splited_data(feature_values[j]);
		//����������������
		pair<double, int> ret = conditionalEntropy(data_id, j, temp_splited_data);
		if (cls_epy - ret.first > max_info_gain)
		{//���ҳ�����������Ϣ����
			max_info_gain = cls_epy - ret.first;
			split_fte_id = j;//���µ�ǰ���ŷָ�����
			splited_data_id.swap(temp_splited_data);//�Լ��ָ������ݼ�����
		}
	}
	return pair<double, int>(max_info_gain, split_fte_id);//���������Ϣ����ͷָ�����
}

//-------------------------------------------------------------------------------------------------------
class InfoGainRatio :public InfoGain
{//������Ϣ����ȵ�����ѡ�񣬼̳�����Ϣ����׼��
private:
	//�Զ����˽�г�Ա����
	double infoGainRatio(vector<int>&, int, vector<vector<int>>&);
public:
	InfoGainRatio(const vector<sample<int>> &d, const vector<int> &fv, int cn) :
		InfoGain(d, fv, cn){}
	//�������ŷָ�����ѡ��������ʵ���������޸ľͿ���Ϊ������
	virtual pair<double, int> select(vector<int>&, vector<vector<int>>&);
	virtual ~InfoGainRatio(){}
};

/*������Ϣ�����
*fte_id����ID
*splited_data_idΪ�ָ��������ݼ�����
*/
double InfoGainRatio::infoGainRatio(vector<int> &data_id, int fte_id,vector<vector<int>> &splited_data_id)
{
	double cls_epy = entropy(data_id, -1);//���ü̳еĻ����Ա���������������
	//ͬ����ʹ�û����Ա������fte_id�����µ�������
	pair<double, int> ret = conditionalEntropy(data_id, fte_id, splited_data_id);
	double info_gain = cls_epy - ret.first;//�õ���Ϣ����
	double fte_id_epy = entropy(data_id, fte_id);//���������fte_id��������
	return info_gain / fte_id_epy;//�õ���Ϣ�����
}

/*ѡ�����ŵķָ�����
*splited_data_id�����ŷָ��������ݼ�����
*/
pair<double, int> InfoGainRatio::select(vector<int> &data_id, vector<vector<int>> &splited_data_id)
{
	double max_info_gain_ratio = 0.0;
	int split_fte_id = -1;//�ָ�����
	for (int j = 0; j != feature_values.size(); ++j)
	{//��ÿһ������
		if (feature_values[j] == 0) continue;//��ʾ�������Ѿ����ù���
		vector<vector<int>> temp_splited_data(feature_values[j]);
		//����������
		double info_gain_ratio = infoGainRatio(data_id, j, temp_splited_data);
		if (info_gain_ratio > max_info_gain_ratio)
		{//�Եõ���������
			max_info_gain_ratio = info_gain_ratio;
			split_fte_id = j;//���µ�ǰ���ŷָ�����
			splited_data_id.swap(temp_splited_data);//�ͷָ����ݼ�
		}
	}
	return pair<double, int>(max_info_gain_ratio, split_fte_id);//�����������Ⱥͷָ�����
}

//-------------------------------------------------------------------------------------------------------
class Gini :public InfoGain
{//���ڻ���ָ��������ѡ��׼�򣬼̳���InfoGain
public:
	Gini(const vector<sample<int>> &d, const vector<int> &fv, int cn) :
		InfoGain(d, fv, cn){}
	virtual double entropy(vector<int>&, int);
	virtual pair<double, int> conditionalEntropy(vector<int>&, int, vector<vector<int>>&);

	virtual pair<double, int> select(vector<int>&, vector<vector<int>>&);
	virtual ~Gini(){}
};

/*ѡ�����ŵķָ�����
*splited_data_id�����ŷָ��������ݼ�����
*/
pair<double, int> Gini::select(vector<int> &data_id, vector<vector<int>> &splited_data_id)
{
	double min_gini_index = INT_MAX;
	//�ָ������ͷָ�������ȡֵ
	int split_fte_id = -1, split_fte_value = -1;
	for (int j = 0; j != feature_values.size(); ++j)
	{//��ÿһ������
		if (feature_values[j] == 0) continue;//��ʾ�������Ѿ����ù���
		vector<vector<int>> temp_splited_data(2);
		//����������С����ָ������Ӧ������ȡֵ
		pair<double, int> ret = conditionalEntropy(data_id, j, temp_splited_data);
		if (ret.first < min_gini_index)
		{//��ȡ��ȫ����С�Ļ���ָ��
			min_gini_index = ret.first;
			split_fte_id = j;//���µ�ǰ�ָ�����
			split_fte_value = ret.second;//������ȡֵ
			splited_data_id.swap(temp_splited_data);//�Լ��ָ�������ݼ�����
		}
	}
	return pair<double, int>(split_fte_id, split_fte_value);//�������ŷָ�����������ȡֵ
}

//�������ָ�������������ˣ���ʱfte_idֻ������-1�������
double Gini::entropy(vector<int> &data_id, int fte_id)
{
	vector<int> specific_fte_value_count(cls_num, 0);
	for (int i = 0; i != data_id.size(); ++i)
	{//ͳ��ÿ�����ֵ��������
		for (int j = 0; j != cls_num; ++j)
			if (data[data_id[i]].y == j)
			{
				++specific_fte_value_count[j];
				break;
			}
	}
	double gini_index = 1.0;
	for (int k = 0; k != specific_fte_value_count.size(); ++k)
	{
		double temp = specific_fte_value_count[k] * 1.0 / data_id.size();
		gini_index -= temp * temp;
	}
	return gini_index;
}

//����������fte_id�ָ��µĻ���ָ��
pair<double, int> Gini::conditionalEntropy(vector<int> &data_id,int fte_id, 
	vector<vector<int>> &splited_data_id)
{
	vector<int> yes, no;
	double min_gini_index = INT_MAX, gini_index = 0.0;
	int split_value = -1;//�ָ�������ȡֵ
	for (int j = 0; j != feature_values[fte_id]; ++j)
	{//��ÿһ������ȡֵ
		for (int i = 0; i != data_id.size(); ++i)
		{//ͳ�Ƶ��ںͲ����ڸ�ֵ��������
			if (data[data_id[i]].x[fte_id] == j)
				yes.push_back(data_id[i]);
			else no.push_back(data_id[i]);
		}
		gini_index = (yes.size() * 1.0 / data_id.size()) * entropy(yes, -1)
			+ (no.size() * 1.0 / data_id.size()) * entropy(no, -1);//�Եõ�����ָ��
		if (gini_index < min_gini_index)
		{//�Ӷ���ø���������С�Ļ���ָ��
			min_gini_index = gini_index;
			split_value = j;//����
			splited_data_id[0].swap(yes);//�ָ�������ݼ�ֻ������
			splited_data_id[1].swap(no);
		}
		yes.clear();
		no.clear();
	}
	return pair<double, int>(min_gini_index, split_value);//���ظ���������������ȡֵ����Ӧ�Ļ���ָ��
}

//-------------------------------------------------------------------------------------------------------
class LeastSquareError
{//��С��������ѡ��׼�����ڻع�����������ݵ㶼�Ǹ�����
private:
	vector<sample<double>> data;//��������
private:
	double squareError(vector<int>&, int, double);
	pair<double, double> specificFeatureMinSquareError(vector<int>&, int, vector<vector<int>>&);
public:
	LeastSquareError(const vector<sample<double>> &d) :data(move(d)){}
	pair<double, int> select(vector<int>&, vector<vector<int>>&);
	double average(vector<int> &data_id)
	{
		double sum = 0.0;
		for (int i = 0; i != data_id.size(); ++i)
			sum += data[data_id[i]].y;
		return sum / data_id.size();
	}
};

/*��������j�µ�ƽ����֮��
*j������ID
*avg�Ǹ������µ�ƽ��ֵ
*/
double LeastSquareError::squareError(vector<int> &data_id, int j, double avg)
{
	double se = 0.0;
	for (int i = 0; i != data_id.size(); ++i)
		se += (data[data_id[i]].x[j] - avg) * (data[data_id[i]].x[j] - avg);
	return se;
}

/*Ѱ��������j�����ŵķָ��
*splited_data_id�����ŷָ��������ݼ�����
*/
pair<double, double> LeastSquareError::specificFeatureMinSquareError(vector<int> &data_id, int j,
	vector<vector<int>> &splited_data_id)
{
	double min_se = INT_MAX, min_split_value = INT_MAX;
	vector<int> L, R;
	for (int i = 0; i != data_id.size(); ++i)
	{//��ÿһ������j��ȡֵ
		//split_value���¸�ֵ
		double split_value = data[data_id[i]].x[j], left_avg = 0.0, right_avg = 0.0;
		for (int k = 0; k != data_id.size(); ++k)
		{
			if (data[data_id[k]].x[j] <= split_value)
			{//ͳ�Ʋ����ڸ�ֵ������
				L.push_back(data_id[k]);
				left_avg += data[data_id[k]].x[j];
			}
			else
			{//�Լ����ڸ�ֵ������
				R.push_back(data_id[k]);
				right_avg += data[data_id[k]].x[j];
			}
		}
		if (L.empty() || R.empty()) continue;
		left_avg /= L.size();//�����ֵ
		right_avg /= R.size();
		double se = squareError(L, j, left_avg) + squareError(R, j, right_avg);//���ƽ����֮��
		if (se < min_se)
		{//����ǵ�ǰ��С��ƽ����
			min_se = se;
			min_split_value = split_value;//����·ָ�ֵ
			splited_data_id[0].swap(L);//�Լ��ָ��Ӽ�
			splited_data_id[1].swap(R);
		}
		L.clear(); R.clear();
	}
	return pair<double, double>(min_se, min_split_value);//���ظ������µ���Сƽ�����Լ��ָ��
}

//�����ݼ����ҵ����ŷָ����������ŷָ��
pair<double, int> LeastSquareError::select(vector<int> &data_id, vector<vector<int>> &splited_data_id)
{
	double min_se = INT_MAX, min_split_value = INT_MAX;
	int splited_fte_id = -1, fte_num = data[data_id[0]].x.size();//�ָ���������������
	for (int j = 0; j != fte_num; ++j)
	{//��ÿһ������
		vector<vector<int>> temp_splited_data(2);
		//�������������ŷָ���Լ���Ӧ����Сƽ����
		pair<double, double> se = specificFeatureMinSquareError(data_id, j, temp_splited_data);
		if (se.first < min_se)
		{//����ǵ�ǰ����
			min_se = se.first;
			splited_fte_id = j;//����¸�����
			min_split_value = se.second;//�ͷָ��
			splited_data_id.swap(temp_splited_data);//�Լ��ָ��Ӽ�
		}
	}
	return pair<double, int>(min_split_value, splited_fte_id);//�������ŷָ����������ŷָ��
}

#endif