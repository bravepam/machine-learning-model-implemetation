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
protected:
	const vector<sample<int>> samples;//��������
	vector<int> feature_values;//ÿ����������ȡ������ֵ�ĸ���
	const int cls_num;//������

protected:
	//�ܱ����Ĺ��캯����ֻ����̳У��������ⲿʹ��
	criterion(const vector<sample<int>> &d, const vector<int> &fv, int cn) :
		samples(move(d)), feature_values(move(fv)), cls_num(cn){}
};

//-------------------------------------------------------------------------------------------------------
class InfoGain :public criterion
{//������Ϣ���������ѡ�񣬼̳���׼�����
	//��ʵ�����е��麯��
protected:
	vector<int> countingClass(const vector<int>&, int)const;
public:
	InfoGain(const vector<sample<int>> &d, const vector<int> &fv, int cn) :criterion(d, fv, cn){}

	//�Զ�����ĸ�����
	//��鵱ǰ���ݼ��Ƿ���ͬһ�������ǣ��򷵻�true�����ֵ��
	//������ǣ��򷵻�false��ռ���������ֵ
	pair<bool, int> checkData(const vector<int>&, vector<int>&)const;
	inline bool hasFeature()const;//�Ƿ��п�������
	int getSpecificFeatureValues(int fte_id)const
	{
		return feature_values[fte_id];
	}
	void zeroSpecificFeatureValues(int fte_id)
	{
		feature_values[fte_id] = 0;
	}
	virtual double entropyAux(int, const vector<int>&)const;

	//�������ݼ������������
	double entropy(const vector<int>&, int)const;
	//�������ݼ������ĳһ������������
	double conditionalEntropy(const vector<int>&, int, vector<vector<int>>&)const;
	//ѡ�����ŷָ�����
	virtual pair<double, int> select(const vector<int>&, vector<vector<int>>&)const;
	virtual ~InfoGain(){}
};

vector<int> InfoGain::countingClass(const vector<int> &samples_id,int fte_id)const
{
	int value_num = (fte_id == -1 ? cls_num : feature_values[fte_id]);
	vector<int> specific_fte_value_count(value_num, 0);//������ֵ����ռ������
	for (int i = 0; i != samples_id.size(); ++i)
	{//ͳ�Ƹ�����ֵ��������
		if (fte_id == -1)
			++specific_fte_value_count[samples[samples_id[i]].y];
		else
			++specific_fte_value_count[samples[samples_id[i]].x[fte_id]];
	}
	return specific_fte_value_count;
}

double InfoGain::entropyAux(int size,const vector<int> &specific_fte_value_count)const
{
	double epy = 0.0, prob = 0.0;
	for (int k = 0; k != specific_fte_value_count.size(); ++k)
	{//������
		if (specific_fte_value_count[k] > 0)
		{//�����Ӽ��������0
			prob = specific_fte_value_count[k] * 1.0 / size;
			epy += -prob * log2(prob);
		}
	}
	return epy;
}

pair<bool, int> InfoGain::checkData(const vector<int> &samples_id, vector<int> &cls_count)const
{//������ݼ�
	cls_count = countingClass(samples_id, -1);//ͳ�Ƹ�����Ŀ
	int count = 0, cls_value = -1;
	for (int k = 0; k != cls_count.size(); ++k)
	{
		if (cls_count[k] > count)
		{//����ռ���������
			count = cls_count[k];
			cls_value = k;
		}
	}
	return pair<bool, int>(count == samples_id.size(), cls_value);
}

bool InfoGain::hasFeature()const
{//����Ƿ�����������
	for (int i = 0; i != feature_values.size(); ++i)
	{
		//�������ȡֵ������Ϊ0����˵������
		if (feature_values[i] != 0) return true;
	}
	return false;
}

//fte_id��Ϊ����ID
double InfoGain::entropy(const vector<int> &samples_id, int fte_id)const
{
	vector<int> specific_fte_value_count = countingClass(samples_id, fte_id);
	return entropyAux(samples_id.size(), specific_fte_value_count);
}

/*������ĳһ�����µ�������
*fte_id�Ǹ�����ID��splited_data_id�Ƿָ��������ݼ�����
*/
double InfoGain::conditionalEntropy(const vector<int> &samples_id, int fte_id, vector<vector<int>> &splited_data_id)const
{
	for (int i = 0; i != samples_id.size(); ++i)
	{
		//��������fte_id�ĸ���ȡֵ�ָ����ݼ�
		splited_data_id[samples[samples_id[i]].x[fte_id]].push_back(samples_id[i]);
	}
	double cdl_epy = 0.0, prob = 0.0;
	for (int k = 0; k != splited_data_id.size(); ++k)
	{//����������
		if (!splited_data_id[k].empty())
		{
			prob = splited_data_id[k].size() * 1.0 / samples_id.size();//�������Ӽ���ռ�����ݼ���Ƶ��
			cdl_epy += prob * entropy(splited_data_id[k], -1);//������
		}
	}
	return cdl_epy;
}

/*ѡ�����ŵķָ�����
*splited_data_id�����ŷָ��������ݼ�����
*/
pair<double, int> InfoGain::select(const vector<int> &samples_id, vector<vector<int>> &splited_data_id)const
{
	int split_fte_id = -1;//�ָ������������
	//�����������
	double cls_epy = entropy(samples_id, -1), min_cdl_epy = INT_MAX;
	for (int j = 0; j != feature_values.size(); ++j)
	{//��ÿһ������
		if (feature_values[j] == 0) continue;//��ʾ�������Ѿ����ù���
		vector<vector<int>> temp_splited_data(feature_values[j]);
		//����������������
		double ret = conditionalEntropy(samples_id, j, temp_splited_data);
		if (ret < min_cdl_epy)
		{//���ҳ�������С��������
			min_cdl_epy = ret;
			split_fte_id = j;//���µ�ǰ���ŷָ�����
			splited_data_id.swap(temp_splited_data);//�Լ��ָ������ݼ�����
		}
	}
	return pair<double, int>(cls_epy - min_cdl_epy, split_fte_id);//���������Ϣ����ͷָ�����
}

//-------------------------------------------------------------------------------------------------------
class InfoGainRatio :public InfoGain
{//������Ϣ����ȵ�����ѡ�񣬼̳�����Ϣ����׼��
private:
	//�Զ����˽�г�Ա����
	double infoGainRatio(const double&, const vector<int>&, int, vector<vector<int>>&)const;
public:
	InfoGainRatio(const vector<sample<int>> &d, const vector<int> &fv, int cn) :
		InfoGain(d, fv, cn){}
	
	//��д���ŷָ�����ѡ����
	pair<double, int> select(const vector<int>&, vector<vector<int>>&)const;
	virtual ~InfoGainRatio(){}
};

/*������Ϣ�����
*fte_id����ID
*splited_data_idΪ�ָ��������ݼ�����
*/
double InfoGainRatio::infoGainRatio(const double &cls_epy, const vector<int> &samples_id, int fte_id, vector<vector<int>> &splited_data_id)const
{
	//ͬ����ʹ�û����Ա������fte_id�����µ�������
	double ret = conditionalEntropy(samples_id, fte_id, splited_data_id);
	double info_gain = cls_epy - ret;//�õ���Ϣ����
	double fte_id_epy = entropy(samples_id, fte_id);//���������fte_id��������
	return info_gain / fte_id_epy;//�õ���Ϣ�����
}

/*ѡ�����ŵķָ�����
*splited_data_id�����ŷָ��������ݼ�����
*/
pair<double, int> InfoGainRatio::select(const vector<int> &samples_id, vector<vector<int>> &splited_data_id)const
{
	double max_info_gain_ratio = 0.0, cls_epy = entropy(samples_id, -1);
	int split_fte_id = -1;//�ָ�����
	for (int j = 0; j != feature_values.size(); ++j)
	{//��ÿһ������
		if (feature_values[j] == 0) continue;//��ʾ�������Ѿ����ù���
		vector<vector<int>> temp_splited_data(feature_values[j]);
		//����������
		double info_gain_ratio = infoGainRatio(cls_epy, samples_id, j, temp_splited_data);
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

	double entropyAux(int, const vector<int>&)const;
	pair<double, int> select(const vector<int>&, vector<vector<int>>&)const;
	virtual ~Gini(){}
};

/*ѡ�����ŵķָ�����
*splited_data_id�����ŷָ��������ݼ�����
*/
pair<double, int> Gini::select(const vector<int> &samples_id, vector<vector<int>> &splited_data_id)const
{
	double min_gini_index = INT_MAX;
	int split_fte_id = -1;
	for (int j = 0; j != feature_values.size(); ++j)
	{//��ÿһ������
		if (feature_values[j] == 0) continue;//��ʾ�������Ѿ����ù���
		vector<vector<int>> temp_splited_data(feature_values[j]);
		//����������С����ָ������Ӧ������ȡֵ
		double ret = conditionalEntropy(samples_id, j, temp_splited_data);
		if (ret < min_gini_index)
		{//��ȡ��
			min_gini_index = ret;
			split_fte_id = j;//���µ�ǰ�ָ�����
			splited_data_id.swap(temp_splited_data);//�Լ��ָ�������ݼ�����
		}
	}
	//��InfoGain��ȣ�Gini���ֻ�ڵ�һ������ֵ��ǰ������Ϣ���棬�����ǻ���ָ�������Ҫ���ص��ǻ���ָ���
	//��ú����ɸ���
	return pair<double, int>(min_gini_index,split_fte_id);//����ȫ����С�Ļ���ָ�������ŷָ�����
}

//�������ָ�������������ˣ���ʱfte_idֻ������-1�������
double Gini::entropyAux(int size, const vector<int> &specific_fte_value_count)const
{
	double gini_index = 1.0, temp = 0.0;
	for (int k = 0; k != specific_fte_value_count.size(); ++k)
	{
		temp = specific_fte_value_count[k] * 1.0 / size;
		gini_index -= temp * temp;
	}
	return gini_index;
}

//-------------------------------------------------------------------------------------------------------
class LeastSquareError
{//��С��������ѡ��׼�����ڻع�����������ݵ㶼�Ǹ�����
private:
	vector<sample<double>> samples;//��������
private:
	double squareError(vector<int>&, int, double);
	pair<double, double> specificFeatureMinSquareError(vector<int>&, int, vector<vector<int>>&);
public:
	LeastSquareError(const vector<sample<double>> &d) :samples(move(d)){}
	pair<double, int> select(vector<int>&, vector<vector<int>>&);
	double average(vector<int> &samples_id)
	{
		double sum = 0.0;
		for (int i = 0; i != samples_id.size(); ++i)
			sum += samples[samples_id[i]].y;
		return sum / samples_id.size();
	}
};

/*��������j�µ�ƽ����֮��
*j������ID
*avg�Ǹ������µ�ƽ��ֵ
*/
double LeastSquareError::squareError(vector<int> &samples_id, int j, double avg)
{
	double se = 0.0;
	for (int i = 0; i != samples_id.size(); ++i)
		se += (samples[samples_id[i]].x[j] - avg) * (samples[samples_id[i]].x[j] - avg);
	return se;
}

/*Ѱ��������j�����ŵķָ��
*splited_data_id�����ŷָ��������ݼ�����
*/
pair<double, double> LeastSquareError::specificFeatureMinSquareError(vector<int> &samples_id, int j,
	vector<vector<int>> &splited_data_id)
{
	double min_se = INT_MAX, min_split_value = INT_MAX;
	vector<int> L, R;
	for (int i = 0; i != samples_id.size(); ++i)
	{//��ÿһ������j��ȡֵ
		//split_value���¸�ֵ
		double split_value = samples[samples_id[i]].x[j], left_avg = 0.0, right_avg = 0.0;
		for (int k = 0; k != samples_id.size(); ++k)
		{
			if (samples[samples_id[k]].x[j] <= split_value)
			{//ͳ�Ʋ����ڸ�ֵ������
				L.push_back(samples_id[k]);
				left_avg += samples[samples_id[k]].x[j];
			}
			else
			{//�Լ����ڸ�ֵ������
				R.push_back(samples_id[k]);
				right_avg += samples[samples_id[k]].x[j];
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
pair<double, int> LeastSquareError::select(vector<int> &samples_id, vector<vector<int>> &splited_data_id)
{
	double min_se = INT_MAX, min_split_value = INT_MAX;
	int splited_fte_id = -1, fte_num = samples[samples_id[0]].x.size();//�ָ���������������
	for (int j = 0; j != fte_num; ++j)
	{//��ÿһ������
		vector<vector<int>> temp_splited_data(2);
		//�������������ŷָ���Լ���Ӧ����Сƽ����
		pair<double, double> se = specificFeatureMinSquareError(samples_id, j, temp_splited_data);
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