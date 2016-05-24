/*kd����k���ڣ�k-nearest neighbor���㷨����Ҫ�����ݽṹ
 *�����ڸ�ά�ռ������ض��������㣬������BST������ÿ���ڵ�
 *�ķָ����ݵ���������ĳһά�����ݼ��ϵ���ֵ���Ӹ���Ҷ�ӣ�����
 *ѭ�����ݸ�ά��
 *���ڽ�������Ҫ���ڲ�ѯ�������Ҫ�㷨ֻʵ��������ڵ��ѯ��nearest��
 *��k���ڵ��ѯ(k-nearest neighbor)������������ɾ�����޸ģ�����ûʵ�֡�
 */

#ifndef KDTREE_H
#define KDTREE_H

#include<vector>
#include<stack>
#include<map>
#include<algorithm>
#include<string>
#include<limits>

using namespace std;

struct feature
{//���ݵ�ṹ
	const vector<double> x;//��
	int cls;//���
	feature(const vector<double> &_x, int _cls) :x(_x), cls(_cls){}
	void print()const
	{
		printf("(");
		for (int i = 0; i != x.size(); ++i)
		{
			printf("%lf", x[i]);
			if (i != x.size() - 1)
				printf(",");
		}
		printf(") cls: %d", cls);
	}
	double distance(const vector<double> &dot)const
	{//����֮���L2���룬��ŷ�Ͼ���
		double sum = 0.0f;
		for (int i = 0; i != x.size(); ++i)
			sum += (x[i] - dot[i]) * (x[i] - dot[i]);
		return sqrt(sum);
	}
};

struct node
{//kd���ڵ�
	const feature *const pf;//ָ��������
	int split_dim;//�Ӹ�������ָ�ʱ�����õ�ά��
	node *left = nullptr;
	node *right = nullptr;
	node(const feature *const _pf, int sd) :pf(_pf), split_dim(sd){}
	void print()const
	{
		pf->print();
		printf(" split dim: %d\n", split_dim);
	}
};

class KDTree
{//kd��
private:
	node *root;
	const int D;//Dֵ�������ݵ��ά��
private:
	//�ݹ鴴��KD��
	void create(vector<vector<double>>&, vector<int>&, int, int, node *&, int);
	//�ָ�ĳһ��Χ�����ݵ㣬���طָ������
	int splitDot(vector<vector<double>>&, vector<int>&, int, int, int);
	void print(node*, int)const;
	//�ݹ��ѯ�����
	node* nearest(node*, double&, const vector<double>&)const;
	void clear(node*);
	void searchToLeaf(node*, const vector<double>&, stack<node*>&)const;
	//�ݹ��ѯknn
	void kNN(node*, multimap<double, node*>&, double&, const vector<double>&, int)const;
	//�ݹ��ѯ�Բ�ѯ��ΪԲ�ģ�һ���뾶�ڵ����е�
	void kNNInRadius(node*, vector<vector<double>>&, vector<double>&, const vector<double>&, double)const;
public:
	KDTree(int _d) :D(_d), root(nullptr){}
	void create(vector<vector<double>> &dots, vector<int> &category)
	{
		create(dots, category, 0, dots.size() - 1, root, 0);
	}
	void print()const
	{
		if (root != nullptr)
			print(root, 0);
	}
	double nearest(const vector<double>& dot, vector<double>& nrt)const
	{
		double mindist = numeric_limits<double>::max();
		const node* const pnrt = nearest(root, mindist, dot);
		nrt = pnrt->pf->x;
		return mindist;
	}
	void kNN(const vector<double>& dot, int k, vector<vector<double>>& knns, vector<double>& dists)const
	{
		multimap<double, node*> mknn;
		double maxdist = 0.0;
		kNN(root, mknn, maxdist, dot, k);
		for (auto it = mknn.begin(); it != mknn.end(); ++it)
		{
			knns.emplace_back(it->second->pf->x);
			dists.emplace_back(it->first);
		}
	}
	void kNNInRadius(const vector<double>& dot, double radius, vector<vector<double>>& knns, vector<double>& dists)const
	{
		kNNInRadius(root, knns, dists, dot, radius);
	}
	void clear(){ clear(root); }
	~KDTree()
	{
		clear(root);
	}
};

void KDTree::create(vector<vector<double>> &dots, vector<int> &category, int start, int end, node *&r, int h)
{//�ݹ鴴��kd��
	/*
	 *dots: �����㼯
	 *category: ��Ӧ�����
	 *start,end: ��ǰ�����ڴ��������ĵ㼯��Χ
	 *r: ������
	 *h: ��ǰ�ڵ�����
	 */
	const int dim = h % D;//���ڵ�Ӧ�����ĸ�ά�����ݽ��зָ�
	int index = splitDot(dots, category, dim, start, end);//��÷ָ�������
	feature *pf = new feature(dots[index], category[index]);//��������
	r = new node(pf, dim);//�Ը����������ڵ�
	if (start < index)//����߻�������
		create(dots, category, start, index - 1, r->left, h + 1);//��ݹ鴴��������
	if (index < end)
		create(dots, category, index + 1, end, r->right, h + 1);
}

void KDTree::clear(node *r)
{
	if (r == nullptr) return;
	clear(r->left);
	clear(r->right);
	delete r->pf;
	delete r;
}

int KDTree::splitDot(vector<vector<double>> &dots, vector<int> &category, int dim, int start, int end)
{//����Ե�dimά����Ϊ���ݵķָ������
	vector<double> pivots;
	pivots.reserve(end - start + 1);
	for (int i = start; i <= end; ++i)//��ȡdimά����
		pivots.emplace_back(dots[i][dim]);
	nth_element(pivots.begin(), pivots.begin() + pivots.size() / 2, pivots.end());
	double pivot = *(pivots.begin() + pivots.size() / 2);//�����ֵ������Ϊ����
	int mid = (end - start + 1) / 2 + start;
	bool meet_pivot = false;
	while (start < mid && end > mid)
	{//�ָ�㼯
		while (start < mid && dots[start][dim] < pivot) ++start;
		if (abs(dots[start][dim] - pivot) <= numeric_limits<double>::epsilon() && !meet_pivot)
		{
			dots[start].swap(dots[mid]);
			std::swap(category[start], category[mid]);
			meet_pivot = true;
			continue;
		}
		if (start == mid) break;
		while (mid < end && dots[end][dim] > pivot) --end;
		dots[start].swap(dots[end]);
		std::swap(category[start], category[end]);
	}
	return mid;//��÷ָ������
}

void KDTree::print(node *r, int indent) const
{
	for (int i = 0; i != indent; ++i)
		printf("\t");
	r->print();
	if (r->left != nullptr)
		print(r->left, indent + 1);
	if (r->right != nullptr)
		print(r->right, indent + 1);
}

node* KDTree::nearest(node* r,double& mindist,const vector<double>& dot)const
{//������㷨�������dot����ĵ�
	stack<node*> path;//�洢����·��
	node *pnrt = nullptr, *curr = nullptr;
	searchToLeaf(r, dot, path);
	while (!path.empty())
	{//����
		curr = path.top();
		path.pop();
		const double dist = curr->pf->distance(dot);//�ڵ�r����������dot�ľ���
		if (dist < mindist)
		{//�������
			pnrt = curr;//����������
			mindist = dist;
		}
		node* sub_pnrt = nullptr;
		if (abs(curr->pf->x[curr->split_dim] - dot[curr->split_dim]) < mindist)
		{//DΪ2����2ά����£����ƹ㵽nά�����Բ�ѯ��dotΪԲ�ģ���ǰ��С����distΪ�뾶��Բ��
			//�жϽڵ�curr���������split_dimά��ƽ���Ƿ����Բ�ཻ�����ཻ����˵��curr����һ�ӿռ���ܴ�
			//�ڸ�����������
			if (dot[curr->split_dim] < curr->pf->x[curr->split_dim] && curr->right)
				sub_pnrt = nearest(curr->right, mindist, dot);
			if (dot[curr->split_dim] >= curr->pf->x[curr->split_dim] && curr->left)
				sub_pnrt = nearest(curr->left, mindist, dot);
		}
		if (sub_pnrt) //��Ч��˵������һ�ӿռ�ȷʵ���ڸ����ĵ�
			pnrt = sub_pnrt;
	}
	return pnrt;
}

void KDTree::searchToLeaf(node* r, const vector<double>& dot, stack<node*>& path)const
{
	while (r != nullptr)
	{//�Ӹ���ʼ��һֱ��Ѱ��Ҷ�ӣ��ҵ�dot���ڵ���С�ӿռ�
		path.emplace(r);
		if (dot[r->split_dim] < r->pf->x[r->split_dim])
			r = r->left;
		else r = r->right;
	}
}

void KDTree::kNN(node* r, multimap<double, node*>& mknn, double& maxdist, const vector<double> &dot, int k)const
{//k�����㷨���ҵ�k����dot����ĵ㣬���������Ӧ�ľ���
	stack<node*> path;
	node *pnrt = nullptr, *curr = nullptr;
	searchToLeaf(r, dot, path);
	while (!path.empty())
	{
		curr = path.top();
		path.pop();
		const double dist = curr->pf->distance(dot);
		if (k > 0)
		{//����õĵ㲻��k��ʱ��ֱ�Ӳ��룬�����ж�
			mknn.emplace(dist, curr);
			maxdist = maxdist > dist ? maxdist : dist;//ͬʱ������Щ���е�������
			--k;
		}
		else if (dist < maxdist)
		{//��������ǰ��ľ����������С
			mknn.erase(--mknn.end());//��ɾ��k�����еľ��������
			mknn.emplace(dist, curr);//���뵱ǰ��
			maxdist = (--mknn.end())->first;//����������
		}
		if (k > 0 || abs(curr->pf->x[curr->split_dim] - dot[curr->split_dim]) < maxdist)
		{//DΪ2����2ά����£������ƹ㵽nά�����Բ�ѯ��dotΪԲ�ģ���ǰ��С����distΪ�뾶��Բ��
			//�жϽڵ�curr���������split_dimά��ƽ���Ƿ����Բ�ཻ�����ཻ����˵��curr����һ�ӿռ���ܴ�
			//�ڸ����������㣬��������
			if (dot[curr->split_dim] < curr->pf->x[curr->split_dim] && curr->right)
				kNN(curr->right, mknn, maxdist, dot, k);
			if (dot[curr->split_dim] >= curr->pf->x[curr->split_dim] && curr->left)
				kNN(curr->left, mknn, maxdist, dot, k);
		}
	}
}

void KDTree::kNNInRadius(node* r, vector<vector<double>>& knns, vector<double>& dists, const vector<double>& dot, double radius)const
{
	stack<node*> path;
	searchToLeaf(r, dot, path);
	node* curr = nullptr;
	while (!path.empty())
	{
		curr = path.top();
		path.pop();
		const double dist = curr->pf->distance(dot);
		if (dist < radius)
		{
			knns.emplace_back(curr->pf->x);
			dists.emplace_back(dist);
		}
		if (abs(curr->pf->x[curr->split_dim] - dot[curr->split_dim]) < radius)
		{
			if (dot[curr->split_dim] < curr->pf->x[curr->split_dim] && curr->right)
				kNNInRadius(curr->right, knns, dists, dot, radius);
			if (dot[curr->split_dim] >= curr->pf->x[curr->split_dim] && curr->left)
				kNNInRadius(curr->left, knns, dists, dot, radius);
		}
	}
}

#endif