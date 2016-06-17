#ifndef FEATURE_SELECT_H
#define FEATURE_SELECT_H

#include<vector>
#include<utility>
#include<hash_map>
#include"util.h"

//��Ϣ��������ѡ��׼��
class InfoGain
{
private:
	//ÿ������������ֵ�Լ�����ǩ
	struct FeatCls
	{
		size_t sample_id; //��������ɭ��ѵ�����е�����
		double feat_value; //ĳ������ֵ
		int cls; //���������
		FeatCls(size_t si, double fv, int c) :sample_id(si), feat_value(fv), cls(c){}
		bool operator<(const FeatCls& rhs)const
		{
			return feat_value < rhs.feat_value;
		}
	};
	//�����ָ��
	struct split
	{
		size_t index; //�ָ����������������data��ĳ��FeatCls���󼯺���ĳԪ�ص�����
		double split_value; //�ָ�ֵ
		split(size_t i, double sv) :index(i), split_value(sv){}
	};

private:
	const TreeDataSet* const pdata; //ָ��������õ�ѵ�����ݼ�
	std::vector<size_t> features; //����ĳ���ڵ�ʱ���ѡ�������id����

	//data�Ĵ�С��featuresһ�������ߴ��ڶ�Ӧ��ϵ����D = data[i]��ľ�����F = features[i]
	//��ֵ������FeatCls����data��getSplits�����лᱻ��������ֵ��С����
	std::vector<std::vector<FeatCls>> data;

	//����ĳ���ڵ�ʱ�ָ�㼯�ϣ���Ҳ��featuresһ�����Ҵ��ڶ�Ӧ��ϵ����S = splits[i]��ľ�����
	//F = features[i]��ֵ�����ķָ�㣬��������data����getSplits�õ�
	std::vector<std::vector<split>> splits; 
	std::vector<size_t> data_id; //����ĳ���ڵ�ʱ���ݼ�id
private:
	//ͳ��ĳһ��Χ�����ݼ��������
	std::hash_map<int, size_t> countingClass(int, size_t, size_t)const;

	//��������ݼ�����
	double entropy(int, size_t, size_t)const;

	//������ĳһ�����ָ����ݼ�����أ���������
	double conditionalEntropy(size_t, const split&, size_t)const;

	//��ʼ��data����������������еķָ��
	void getSplits();
public:
	InfoGain(const TreeDataSet* const p) :pdata(p){}

	//����ÿ�����ڵ�ָ�ʱ����Ҫ�����ѡ������������ݼ�id����
	void setFeaturesAndData(std::vector<size_t>&, std::vector<size_t>&);
	void clear();
	double entropyAux(const std::hash_map<int, size_t>&, size_t)const;

	//������ݼ�һЩ����
	std::pair<bool, size_t> checkData(std::hash_map<int, size_t>&)const;

	//ѡ����ѷָ������Լ��ָ��ֵ
	std::pair<size_t, double> select(std::vector<std::vector<size_t>>&);
};

#endif