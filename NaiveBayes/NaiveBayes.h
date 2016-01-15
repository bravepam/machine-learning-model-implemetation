#ifndef NAIVEBAYES_H
#define NAIVEBAYES_H

#include<vector>

using namespace std;

class NaiveBayes
{/*���ر�Ҷ˹ģ��
 *k��Ϊ���ĵ�k��ȡֵ
 *j��ʾ��j������
 *v��ʾĳ�������ĵ�v��ȡֵ������k,j,v = 0,1....K-1,J-1,Sj-1
 *Sj��ʾ��j��������ȡֵ����
 */
private:
	/*probability�洢�ڵ�k�����ֵ������£���j������ȡ��v��ֵ�ĸ��ʣ�
	*����P(Xj = xjv | Y = k)��������Ĵ�СΪK*(S0+S1+...+Sj+...),j = 0,1...J-1
	*/
	vector<vector<vector<double>>> probability;
	vector<double> cls;//cls[k]��ʾȡ��k�����ֵ�ĸ���
	double lambda;//��Ҷ˹���Ʋ��õĲ���������ĳЩ�����Ϊ0
public:
	NaiveBayes() :lambda(0.0), cls(), probability(){}
	void create(vector<int>&, int, double);//����ģ��
	void train(vector<vector<int>>&, vector<int>&);//ѵ��ģ��
	int compute(vector<int>&, double&);//����ĳ���������������
};

void NaiveBayes::create(vector<int> &fvn, int clsn, double _lambda)
{
	/*����ģ�ͣ���Ҫ�Ƿ���ռ�
	*fvn����feature-value-number��fvn[j]��ʾ��j�������м���ȡֵ
	*clsn����class-number����ʾ�м������ֵ
	*/
	lambda = _lambda;
	cls.resize(clsn);
	probability.resize(clsn);
	int feature_number = fvn.size();
	for (int k = 0; k != probability.size(); ++k)
	{
		probability[k].resize(feature_number);
		for (int j = 0; j != probability[k].size(); ++j)
			probability[k][j].resize(fvn[j]);
	}
}

void NaiveBayes::train(vector<vector<int>> &smpps, vector<int> &smpvs)
{
	/*ѵ��ģ��
	*smpps,��sample points����ʾ������
	*smpvs,��sample values����ʾ��Ӧ������ֵ
	*/
	//�ۼ�����ֵ
	int sample_number = smpvs.size();
	for (int si = 0; si != sample_number; ++si)
	{
		++cls[smpvs[si]];//����ֵΪsmpvs[si]����������1
		//�����������ֵΪsmpvs[si]��ǰ���£���j������ȡֵΪsmpps[si][j]��������1
		for (int j = 0; j != probability[smpvs[si]].size(); ++j)
			++probability[smpvs[si]][j][smpps[si][j]];
	}
	//�������
	for (int k = 0; k != cls.size(); ++k)
	{
		for (int j = 0; j != probability[k].size(); ++j)
			for (int v = 0; v != probability[k][j].size(); ++v)
				probability[k][j][v] = //����P(Xj = xjv | Y = k)
				(probability[k][j][v] + lambda) / (cls[k] + probability[k][j].size() * lambda);
		cls[k] = (cls[k] + lambda) / (sample_number + cls.size() * lambda);//����P(Y = k)
	}
}

int NaiveBayes::compute(vector<int> &data, double &pro)
{//�������ݵ�data�������������Ӧ�ĸ���
	int data_cls = -1;
	pro = 0.0;
	for (int k = 0; k != cls.size(); ++k)
	{
		//������ȡ��k�����ֵ������£�data�и����������������Ӧ��ȡֵ�ĸ��ʵ�
		//�˻����ٳ������Ϊkth�����õĽ������Ϊ������ʣ���ʽ�ɱ�Ҷ˹��ʽ�Ƶ�
		//��P(Y=k)P(Xj=xj | Y=k),j = 0,1...Sj-1
		double temp_pro = cls[k];
		for (int j = 0; j != data.size(); ++j)
			temp_pro *= probability[k][j][data[j]];
		if (temp_pro > pro)
		{//ȡ���������
			data_cls = k;
			pro = temp_pro;
		}
	}
	return data_cls;
}

#endif