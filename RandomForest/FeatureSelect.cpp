#include"FeatureSelect.h"
#include<hash_map>
#include<algorithm>
#include<cassert>

void InfoGain::setFeaturesAndData(std::vector<size_t>& fs, std::vector<size_t>& di)
{
	features.swap(fs);
	data_id.swap(di);
	splits.resize(features.size());
	data.resize(features.size());
	getSplits(); 
}

void InfoGain::clear()
{
	features.clear();
	splits.clear();
	data.clear();
	data_id.clear();
}

std::pair<bool, size_t> InfoGain::checkData(std::hash_map<int, size_t>& cls_count)const
{
	cls_count = countingClass(-1, 0, data_id.size()); //ͳ�Ƹ����ڵ��������ݼ��������
	size_t cls_value = UINT_MAX, count = 0;
	for (auto iter = cls_count.begin(); iter != cls_count.end(); ++iter)
	{//�ҳ��ĸ����ռ�����
		if (iter->second > count)
		{
			count = iter->second;
			cls_value = iter->first;
		}
	}
	return{ count == data_id.size(), cls_value }; //����ֵΪ�Ƿ�Ϊͬһ���ռ���������
}

std::hash_map<int, size_t> InfoGain::countingClass(int which, size_t first, size_t last)const
{//which������ID��ID����features[i]��i
	std::hash_map<int, size_t> cls_count;
	//which�����-1������ͳ���������ݼ��������Ϣ����������ĳ��������ĳһ��Χ�������Ϣ
	const size_t that = (which == -1 ? 0 : which);
	for (size_t i = first; i != last; ++i)
		++cls_count[data[that][i].cls];
	return std::move(cls_count);
}

double InfoGain::entropyAux(const std::hash_map<int, size_t>&cls_count, size_t size)const
{
	double epy = 0.0, prob = 0.0;
	for (auto iter = cls_count.begin(); iter != cls_count.end(); ++iter)
	{
		prob = iter->second * 1.0 / size;
		epy += -prob * log2(prob);
	}
	return epy;
}

inline double InfoGain::entropy(int which, size_t first, size_t last)const
{
	//��ͳ�����
	std::hash_map<int, size_t> cls_count = countingClass(which, first, last);
	return entropyAux(cls_count, last - first); //�ټ�����
}

inline double InfoGain::conditionalEntropy(size_t which, const split& s, size_t last)const
{
	//��which�����ָ����ݼ������
	const double less = entropy(which, 0, s.index);
	const double greater = entropy(which, s.index, last);
	double cdl_epy = less * s.index / last;
	cdl_epy += greater * (last - s.index) / last;
	return cdl_epy;
}

void InfoGain::getSplits()
{
	const std::vector<sample>& trainset = pdata->prf->train_set;

	for (size_t i = 0; i != features.size(); ++i)
	{//��ÿһ������
		data[i].reserve(data_id.size());
		for (size_t j = 0; j != data_id.size(); ++j)
		{//��ÿһ������������
			//��ȡ��{����id������ֵ�����������ֵ}����һ��FeatCls����
			data[i].emplace_back(data_id[j], trainset[data_id[j]].x[features[i]], trainset[data_id[j]].y);
		}
	}

	//ͨ��data������зָ��
	for (size_t i = 0; i != data.size(); ++i)
	{//��ÿ�������µ�data���ݼ�����ɨ��
		sort(data[i].begin(), data[i].end()); //�Ƚ�FeatCls���󼯺�data[i]�ǽ�������
		int cls = data[i][0].cls;
		splits[i].reserve(100);
		for (size_t j = 1; j != data[i].size(); ++j)
		{//�ü����е�ɨ��ÿһ������
			if (data[i][j].cls != cls)
			{//���������仯����ô˵������һ���ָ��
				//��÷ָ��ֵ
				const double temp = (data[i][j - 1].feat_value + data[i][j].feat_value) / 2.0;
				splits[i].emplace_back(j, temp); //{�ָ���������ָ��ֵ}
				cls = data[i][j].cls;
			}
		}
	}
}

std::pair<size_t, double> InfoGain::select(std::vector<std::vector<size_t>>& splited_data_id)
{
	assert(!features.empty() && !splits.empty() && !data_id.empty());
	size_t split_feat_id_id = UINT_MAX;
	//const double cur_epy = entropy(-1, 0, data_id.size()); //��ǰ���ݼ�����
	double min_cdl_epy = INT_MAX;
	split best_split(0, 0.0);
	for (size_t i = 0; i != features.size(); ++i)
	{//��ÿ������
		for (size_t j = 0; j != splits[i].size(); ++j)
		{//ɨ�������еķָ��
			//�����Ը÷ָ��ָ����ݼ����������
			const double ret = conditionalEntropy(i, splits[i][j], data[i].size());
			if (ret < min_cdl_epy)
			{//�ҳ���������С�ķָ�
				split_feat_id_id = i; //����ID��ID����features[i]�е�i
				best_split = splits[i][j];
				min_cdl_epy = ret;
			}
		}
	}
	printf("epy: %lf\t", min_cdl_epy);
	//�������ָ��ָ����ݼ�
	assert(splited_data_id.size() >= 2);
	splited_data_id[0].reserve(best_split.index);
	splited_data_id[1].reserve(data_id.size() - best_split.index);
	//�ȷָ��ֵС�����ݼ�id����
	for (size_t i = 0; i != best_split.index; ++i)
		splited_data_id[0].push_back(data[split_feat_id_id][i].sample_id);
	//�ȷָ��ֵ������ݼ�id����
	for (size_t i = best_split.index; i != data[split_feat_id_id].size(); ++i)
		splited_data_id[1].push_back(data[split_feat_id_id][i].sample_id);

	//����ֵ{��ѷָ�����ID����ѷָ��ֵ}
	return{ features[split_feat_id_id], best_split.split_value };
}
