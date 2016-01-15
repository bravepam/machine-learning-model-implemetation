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

using namespace std;

struct feature
{//���ݵ�ṹ
	vector<float> x;//��
	int cls;//���
	feature(const vector<float> &_x, int _cls) :x(_x), cls(_cls){}
	void print()const
	{
		printf("(");
		for (int i = 0; i != x.size(); ++i)
		{
			printf("%f", x[i]);
			if (i != x.size() - 1)
				printf(",");
		}
		printf(") cls: %d", cls);
	}
	float distance(const vector<float> &dot)const
	{//����֮���L2���룬��ŷ�Ͼ���
		float sum = 0.0f;
		for (int i = 0; i != x.size(); ++i)
			sum += (x[i] - dot[i]) * (x[i] - dot[i]);
		return sqrt(sum);
	}
};

struct node
{//kd���ڵ�
	feature *pf;//ָ��������
	int split_dim;//�Ӹ�������ָ�ʱ�����õ�ά��
	node *left = nullptr;
	node *right = nullptr;
	node(feature *const _pf, int sd) :pf(_pf), split_dim(sd){}
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
	int k;//kֵ�������ݵ��ά��
private:
	void create(vector<vector<float>>&, vector<int>&, int, int, node *&, int);//������
	int splitDot(vector<vector<float>>&, vector<int>&, int, int, int);
	void print(node*)const;
	void clear(node*);
	void enterSubSpace(stack<node*> &path, node *r, float dist, bool pass_root, vector<float> &dot)
	{//�жϸý���r�ڵ���ĸ��ӿռ�
		if (abs(r->pf->x[r->split_dim] - dot[r->split_dim]) < dist)
		{//kΪ2����2ά����£������ƹ㵽nά�����Բ�ѯ��dotΪԲ�ģ���ǰ��С����distΪ�뾶��Բ��
		 //�жϽڵ�r���������split_dimά��ƽ���Ƿ����Բ�ཻ�����ཻ����˵��r����һ�ӿռ���ܴ�
		 //�ڸ����������㣬��������
			if (!pass_root)
			{//û�п�����ڵ��ʱ��ֻ�����ĳһ���ӿռ�
				if (dot[r->split_dim] < r->pf->x[r->split_dim] && r->right != nullptr)
					path.push(r->right);
				if (dot[r->split_dim] > r->pf->x[r->split_dim] && r->left != nullptr)
					path.push(r->left);
			}
			else
			{//����������Ҫ��
				if (r->right != nullptr) path.push(r->right);
				if (r->left != nullptr) path.push(r->left);
			}
		}
	}
public:
	KDTree(int _k) :k(_k), root(nullptr){}
	void create(vector<vector<float>> &dots, vector<int> &category)
	{
		create(dots, category, 0, dots.size() - 1, root, 0);
	}
	void print()const
	{
		if (root != nullptr)
			print(root);
	}
	float nearest(vector<float>&, vector<float>&);
	void kNN(vector<float>&, int, vector<vector<float>>&, vector<float>&);
	void clear(){ clear(root); }
	~KDTree()
	{
		clear(root);
	}
};

void KDTree::create(vector<vector<float>> &dots, vector<int> &category, int start, int end, node *&r, int h)
{//�ݹ鴴��kd����Ҳֻ�ܵݹ鴴����
	/*
	 *dots: �����㼯
	 *category: ��Ӧ�����
	 *start,end: ��ǰ�����ڴ��������ĵ㼯��Χ
	 *r: ������
	 *h: ��ǰ�ڵ�����
	 */
	int dim = h % k;//���ڵ�Ӧ�����ĸ�ά�����ݽ��зָ�
	int index = splitDot(dots, category, dim, start, end);//��÷ָ�������
	feature *pf = new feature(dots[index], category[index]);//��������
	r = new node(pf, dim);//�Ը����������ڵ�
	if (start < index)//����߻�������
		create(dots, category, start, index - 1, r->left, h + 1);//��ݹ鴴��������
	if (index < end)//ͬ��
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

int KDTree::splitDot(vector<vector<float>> &dots, vector<int> &category, int dim, int start, int end)
{//����Ե�dimά����Ϊ���ݵķָ������
	vector<float> pivots;
	for (int i = 0; i <= end; ++i)//��ȡdimά����
		pivots.push_back(dots[i][dim]);
	//sort(pivots.begin(), pivots.end());
	nth_element(pivots.begin(), pivots.begin() + pivots.size() / 2, pivots.end());
	float pivot = *(pivots.begin() + pivots.size() / 2);//�����ֵ������Ϊ����
	int mid = (end - start + 1) / 2 + start;
	bool meet_pivot = false;
	while (start < end)
	{//�ָ�㼯
		while (start < end && dots[start][dim] < pivot) ++start;
		if (dots[start][dim] == pivot && !meet_pivot)
		{
			dots[start].swap(dots[mid]);
			std::swap(category[start], category[mid]);
			meet_pivot = true;
			continue;
		}
		while (start < end && dots[end][dim] > pivot) --end;
		dots[start].swap(dots[end]);
		std::swap(category[start], category[end]);
	}
	return mid;//��÷ָ������
}

void KDTree::print(node *r) const
{
	r->print();
	if (r->left != nullptr)
		print(r->left);
	if (r->right != nullptr)
		print(r->right);
}

float KDTree::nearest(vector<float> &dot, vector<float> &nrt)
{//������㷨�������dot����ĵ㣬����nrt��NeaResT��
	stack<node*> path;//�洢����·��
	node *r = root, *pnrt = nullptr;
	while (r != nullptr)
	{//�Ӹ���ʼ��һֱ��Ѱ��Ҷ�ӣ��ҵ�dot���ڵ���С�ӿռ�
		path.push(r);
		if (dot[r->split_dim] < r->pf->x[r->split_dim])
			r = r->left;
		else r = r->right;
	}
	float mindist = (float)INT_MAX;
	bool pass_root = false;
	while (!path.empty())
	{//����
		r = path.top();
		path.pop();
		float dist = r->pf->distance(dot);//�ڵ�r����������dot�ľ���
		if (dist < mindist)
		{//�������
			pnrt = r;//����������
			mindist = dist;
		}
		enterSubSpace(path, r, mindist, pass_root, dot);//�ж��Ƿ���Ҫ����������ӿռ�
		if (root == r) pass_root = true;
	}
	nrt = pnrt->pf->x;//��������
	return mindist;//���ؾ���
}

void KDTree::kNN(vector<float> &dot, int k, vector<vector<float>> &knn, vector<float> &dist)
{//k�����㷨���ҵ�k����dot����ĵ㣬���������Ӧ�ľ���
	stack<node*> path;
	node *r = root, *pnrt = nullptr;
	while (r != nullptr)
	{
		path.push(r);
		if (dot[r->split_dim] < r->pf->x[r->split_dim])
			r = r->left;
		else r = r->right;
	}
	multimap<float, node*> mknn;//�洢�����k����
	float maxdist = 0.0f;
	bool pass_root = false;
	while (!path.empty())
	{
		r = path.top();
		path.pop();
		float dist = r->pf->distance(dot);
		if (k > 0)
		{//����õĵ㲻��k��ʱ��ֱ�Ӳ��룬�����ж�
			mknn.insert(map<float, node*>::value_type(dist, r));
			maxdist = maxdist > dist ? maxdist : dist;//ͬʱ������Щ���е�������
			--k;
		}
		else if (dist < maxdist)
		{//��������ǰ��ľ����������С
			mknn.erase(--mknn.end());//��ɾ��k�����еľ��������
			mknn.insert(map<float, node*>::value_type(dist, r));//���뵱ǰ��
			maxdist = (--mknn.end())->first;//����������
		}
		enterSubSpace(path, r, maxdist, pass_root, dot);//�ж��Ƿ���Ҫ���������ӿռ�
		if (root == r) pass_root = true;
	}
	for (auto it = mknn.begin(); it != mknn.end(); ++it)
	{
		knn.push_back(it->second->pf->x);
		dist.push_back(it->first);
	}
}

#endif