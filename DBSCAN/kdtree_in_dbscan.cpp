#include<algorithm>
#include<vector>
#include<queue>
#include<stack>
#include"kdtree_in_dbscan.h"

void kdtree::create(std::vector<size_t>& data_indexes, size_t start, size_t end, node*& r, size_t h)
{
	const size_t dim = h % m_D;//���ڵ�Ӧ�����ĸ�ά�����ݽ��зָ�
	size_t index = splitData(data_indexes, dim, start, end);//��÷ָ�������
	r = new node(data_indexes[index], dim);//��������
	if (start < index)//����߻�������
		create(data_indexes, start, index - 1, r->left, h + 1);//��ݹ鴴��������
	if (index < end)
		create(data_indexes, index + 1, end, r->right, h + 1);
}

size_t kdtree::splitData(std::vector<size_t>& data_indexes, size_t dim, size_t start, size_t end)
{
	std::vector<double> pivots;
	pivots.reserve(end - start + 1);
	for (size_t i = start; i <= end; ++i)//��ȡdimά����
		pivots.emplace_back(pdb->m_points[data_indexes[i]].m_value[dim]);
	nth_element(pivots.begin(), pivots.begin() + pivots.size() / 2, pivots.end());
	double pivot = *(pivots.begin() + pivots.size() / 2);//�����ֵ������Ϊ����
	size_t mid = (end - start + 1) / 2 + start;
	bool meet_pivot = false;
	while (start < mid && end > mid)
	{//�ָ�㼯
		while (start < mid && pdb->m_points[data_indexes[start]].m_value[dim] < pivot) ++start;
		if (abs(pdb->m_points[data_indexes[start]].m_value[dim] - pivot)
			<= std::numeric_limits<double>::epsilon() && !meet_pivot)
		{
			std::swap(data_indexes[start], data_indexes[mid]);
			meet_pivot = true;
			continue;
		}
		if (start == mid) break;
		while (mid < end && pdb->m_points[data_indexes[end]].m_value[dim] > pivot) --end;
		std::swap(data_indexes[start], data_indexes[end]);
	}
	return mid;//��÷ָ������
}

void kdtree::knn(node* r, size_t dot_index, size_t k,
	std::priority_queue<double>& kdists, double& maxdist)const
{
	std::stack<node*> path;
	node *pnrt = nullptr, *curr = nullptr;
	searchToLeaf(r, dot_index, path);
	while (!path.empty())
	{
		curr = path.top();
		path.pop();
		const double dist = pdb->distance(pdb->m_points[dot_index], pdb->m_points[curr->m_dot_index]);
		if (k > 0)
		{//����õĵ㲻��k��ʱ��ֱ�Ӳ��룬�����ж�
			kdists.emplace(dist);
			maxdist = maxdist > dist ? maxdist : dist;//ͬʱ������Щ���е�������
			--k;
		}
		else if (dist < maxdist)
		{//��������ǰ��ľ����������С
			kdists.pop();
			kdists.emplace(dist);//���뵱ǰ��
			maxdist = kdists.top();//����������
		}
		if (k > 0 || abs(pdb->m_points[curr->m_dot_index].m_value[curr->m_split_dim] -
			pdb->m_points[dot_index].m_value[curr->m_split_dim]) < maxdist)
		{//DΪ2����2ά����£������ƹ㵽nά�����Բ�ѯ��dotΪԲ�ģ���ǰ��С����distΪ�뾶��Բ��
			//�жϽڵ�curr���������split_dimά��ƽ���Ƿ����Բ�ཻ�����ཻ����˵��curr����һ�ӿռ���ܴ�
			//�ڸ����������㣬��������
			if (pdb->m_points[dot_index].m_value[curr->m_split_dim] <
				pdb->m_points[curr->m_dot_index].m_value[curr->m_split_dim] && curr->right)
				knn(curr->right, dot_index, k, kdists, maxdist);
			if (pdb->m_points[dot_index].m_value[curr->m_split_dim] >=
				pdb->m_points[curr->m_dot_index].m_value[curr->m_split_dim] && curr->left)
				knn(curr->left, dot_index, k, kdists, maxdist);
		}
	}
}

void kdtree::knnInRadius(node* r, size_t dot_index, double radius, std::vector<size_t>& neighbors)const
{
	std::stack<node*> path;
	searchToLeaf(r, dot_index, path);
	node* curr = nullptr;
	while (!path.empty())
	{
		curr = path.top();
		path.pop();
		const double dist = pdb->distance(pdb->m_points[dot_index], pdb->m_points[curr->m_dot_index]);
		if (dist <= radius && curr->m_dot_index != dot_index) //���ܰ��Լ�����
			neighbors.emplace_back(curr->m_dot_index);
		if (abs(pdb->m_points[curr->m_dot_index].m_value[curr->m_split_dim] -
			pdb->m_points[dot_index].m_value[curr->m_split_dim]) < radius)
		{//ͬ��
			if (pdb->m_points[dot_index].m_value[curr->m_split_dim] <
				pdb->m_points[curr->m_dot_index].m_value[curr->m_split_dim] && curr->right)
				knnInRadius(curr->right, dot_index, radius, neighbors);
			if (pdb->m_points[dot_index].m_value[curr->m_split_dim] >=
				pdb->m_points[curr->m_dot_index].m_value[curr->m_split_dim] && curr->left)
				knnInRadius(curr->left, dot_index, radius, neighbors);
		}
	}
}

void kdtree::searchToLeaf(node* r, size_t dot_index, std::stack<node*>& path)const
{
	while (r != nullptr)
	{//�Ӹ���ʼ��һֱ��Ѱ��Ҷ�ӣ��ҵ�dot���ڵ���С�ӿռ�
		path.emplace(r);
		if (pdb->m_points[dot_index].m_value[r->m_split_dim] <
			pdb->m_points[r->m_dot_index].m_value[r->m_split_dim])
			r = r->left;
		else r = r->right;
	}
}

void kdtree::clear(node* r)
{
	if (!r)return;
	if (r->left)
		clear(r->left);
	if (r->right)
		clear(r->right);
	delete r;
}