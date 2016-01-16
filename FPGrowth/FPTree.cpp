
/*******************************************
* Author: bravepam
*
* E-mail:1372120340@qq.com
*******************************************
*/


#include<iostream>
#include<sstream>
#include<fstream>
#include<string>
#include<unordered_map>
#include<set>
#include<unordered_set>
#include<algorithm>
#include<vector>
#include<functional>

using namespace std;

//FP��������ʵ��FP-growth�㷨����Ч�ھ�Ƶ����

template <typename Key>
struct t_node
{//�����ڵ�
	Key key;
	int count;
	t_node(Key v, int cnt) :key(v), count(cnt){}
};

template <typename Comp>
struct nodeComparer
{//t_node�ڵ�Ƚ���
	template <typename Key>
	bool operator()(const t_node<Key>&lhs, const t_node<Key> &rhs)const
	{
		if (lhs.count == rhs.count) //�������ֵ���
			return Comp()(lhs.key, rhs.key); //��Ƚϼ�ֵ
		else return Comp()(lhs.count, rhs.count); //����
	}
};

template <typename Key>
struct t_treeNode:public t_node<Key>
{//���ڵ����ͣ��̳���t_node
	t_treeNode *parent;
	t_treeNode *next = nullptr;
	unordered_map<char, t_treeNode*> children; //�����б����ֵ䷽�����
	t_treeNode(Key v, int cnt, t_treeNode *par = nullptr) :t_node<Key>(v, cnt), parent(par), children(){}
};

template <typename Key>
struct t_headNode
{//ͷָ���б�ڵ�
	int count;
	t_treeNode<Key> *next = nullptr;//�������о�����ͬ��ֵ�����ڵ�
	t_headNode(int cnt) :count(cnt){}
};

template <typename Key>
class FPTree
{//FP��
	typedef t_node<Key>						node;
	typedef t_headNode<Key>					headNode;
	typedef t_treeNode<Key>					treeNode;
private:
	unordered_map<Key, headNode*> head; //ͷָ���б�
	treeNode *root;
	int eps; //������ֵ
private:
	void updateTree(const vector<Key>&, int, treeNode*, int); 
	void updateHead(treeNode*, treeNode*);
	void mineTree(string&, vector<string>&);
	void findPrefixPath(Key,unordered_map<string,int>&);
	void ascendTree(treeNode*, string&)const;
	void print(treeNode*, int)const;
public:
	FPTree(int e) :root(new treeNode(Key(), 0)), eps(e){}
	unordered_map<string, int> loadData(istream&)const;
	void create(const unordered_map<string, int>&);
	vector<string> mineTree()
	{
		string prefix("(");
		vector<string> frequent;
		mineTree(prefix, frequent);
		return frequent;
	}
	bool empty()const
	{
		return root->children.empty();
	}
	void clear();
	void print(int depth = 1)const
	{
		print(root, depth);
	}
	~FPTree()
	{
		clear();
		delete root;
	}
};

//�������ݼ�����FP��
template <typename Key> 
void FPTree<Key>::create(const unordered_map<string, int> &data)
{
	unordered_set<Key> keys; //��¼���еļ�����ÿ����¼��
	for (auto iter1 = data.begin(); iter1 != data.end(); ++iter1) //��һ��ɨ���������ݼ�
	{//ÿ����¼�����ַ������󣬲��ÿո�ֿ�
		istringstream stream(iter1->first); //��ʼ��һ���ַ���������
		Key k;
		while (stream >> k)
		{//��һ��ü�¼��
			if (head.find(k) == head.end()) //�����������ͷָ���б������
				head.insert(make_pair(k, new headNode(iter1->second)));
			else //����ֻ��Ҫ�ۼӼ���ֵ����
				head[k]->count += iter1->second;
			keys.insert(k);
		}
	}

	for (auto iter2 = keys.begin(); iter2 != keys.end(); ++iter2)
	{//ɨ��ÿ����¼��
		if (head[*iter2]->count < eps) //�������ֵ�ϵͣ���ɾ��֮
			head.erase(*iter2);
	}

	if (head.empty()) return; //���û�г��ִ���������ֵ�ļ�¼����˳�
	for (auto iter3 = data.begin(); iter3 != data.end(); ++iter3)
	{//�ڶ���ɨ���������ݼ�
		//����ÿ����¼�����ݼ�¼��ͳ��ֵ��ܴ�������ڵ㣬Ȼ����뵽���򼯺�
		multiset<node,nodeComparer<greater<int>>> key_count; 
		istringstream stream(iter3->first);
		Key k;
		while (stream >> k)
		{//���ڸ�����¼�е�ÿ����¼��
			if (head.find(k) != head.end()) //ֻ�г��ִ����ϸ�
				key_count.insert(node(k, head[k]->count)); //�Żᱻ���뵽���򼯺���
		}
		if (!key_count.empty())
		{//�����ո�����¼���г��ִ����϶����
			vector<Key> order;
			//���ճ��ִ����Ӹߵ��͵�˳����ȡ����
			for_each(key_count.begin(), key_count.end(), [&](const node &arg){order.push_back(arg.key); });
			updateTree(order, 0, root, iter3->second);//���ڸ�����
		}
	}
}

//����order��Ԫ�ص�˳�򣬴ӽڵ�r��ʼ���¸�����
template <typename Key>
void FPTree<Key>::updateTree(const vector<Key> &order, int index, treeNode *r, int count)
{
	if (r->children.find(order[index]) != r->children.end()) //�����ǰ����r�ĺ���
		r->children[order[index]]->count += count;//��ֱ�Ӹ��¼���ֵ
	else
	{
		treeNode *p = new treeNode(order[index], count, r); //������ڵ�
		r->children.insert(make_pair(order[index], p)); //���뵽�����б�
		if (head[order[index]]->next)//ͬʱ����ͷָ���б�
			updateHead(head[order[index]]->next, p);
		else
			head[order[index]]->next = p;
	}
	if ((static_cast<int>(order.size()) - index) > 1) //���order�ܻ�������
		updateTree(order, index + 1, r->children[order[index]], count);//������ݹ����¸�����
}

//����ͷָ���б�ʵ���Ͼ����ڵ�����β�˲���һ���ڵ�
template <typename Key>
void FPTree<Key>::updateHead(treeNode *h, treeNode *cur)
{
	while (h->next)
		h = h->next;
	h->next = cur;
}

//�ӹ���õ������ھ�Ƶ���
template <typename Key>
void FPTree<Key>::mineTree(string &prefix, vector<string> &frequent)
{
	//ͷָ���б�head�д洢��ʵ���ǵ�Ƶ���������ֵ�ͼ���ֵ����ڵ㣬���뵽���򼯺�
	//��ͬ�ڽ���ʱ����ʱ�ǰ��ռ���ֵ�ӵ͵���˳������
	set <node, nodeComparer<less<int>>> key_count;
	for (auto iter = head.begin(); iter != head.end(); ++iter)
		key_count.insert(node(iter->first, iter->second->count));
	for (auto iter = key_count.begin(); iter != key_count.end(); ++iter)
	{//����ÿһ����Ƶ����
		if (string(typeid(iter->key).name()) == string("int"))
		{//�����ֵ������int
			char ch[12];
			prefix += _itoa(iter->key, ch, 10); //����ҪתΪstring���ͣ�Ȼ����뵽ǰ׺��
		}
		else prefix.push_back(iter->key); //�����������ʵ����ֻ����char��
		frequent.push_back(prefix + ')'); //����һ��Ƶ����
		unordered_map<string, int> subdata;

		//�Ե�ǰ��Ƶ����Ϊβ�������������ھ���������Ҳ�������ѳ���prefix��������ھ��¼
		findPrefixPath(iter->key, subdata); 
		FPTree<Key> subtree(eps);
		subtree.create(subdata); //�����ھ򵽵ļ�¼����������ݼ�������FP�����������ھ�����ӵ�Ƶ����
		if (!subtree.empty())
		{//��������գ������ڸ����ӵ�Ƶ����
			prefix += ' ';//���ڷָ���¼��
			subtree.mineTree(prefix, frequent); //�����ݹ��ھ�
			prefix.pop_back(); //ɾ���ո�
		}
		int index = prefix.rfind(' ');
		if (index == string::npos) prefix.resize(1); //ɾ��iter->key
		else prefix.resize(index + 1);
	}
}

//�ھ���������ʵ�����ǣ�������·��
template <typename Key>
void FPTree<Key>::findPrefixPath(Key tail, unordered_map<string, int> &paths)
{
	treeNode *first = head[tail]->next; //��Ϊtail�ڵ㹹��һ��������
	while (first)
	{//���ڵ�ǰ��Ϊtail�Ľڵ�
		string prefix;
		ascendTree(first, prefix);//�ɴ����ݵ������������
		if (!prefix.empty())//�������������
			paths.insert(make_pair(prefix, first->count)); 
		first = first->next; //��һ���ڵ�
	}
}

//�Ե�ǰ�ڵ㿪ʼ���ݵ�������˳����½ڵ��ֵ������һ��������
template <typename Key>
void FPTree<Key>::ascendTree(treeNode *curr, string &prefix)const
{
	treeNode *par = curr->parent; //��������������ǰ�ڵ�
	while (par->parent)
	{
		if (string(typeid(par->key).name()) == string("int"))
		{//���������Ϊint
			char ch[12];
			prefix += _itoa(par->key, ch, 10);
		}
		else prefix.push_back(par->key); //���Ϊchar
		prefix.push_back(' '); //�ָ���¼��
		par = par->parent;
	}
	if (!prefix.empty()) //ɾ��ĩβ�Ŀո�
		prefix.resize(prefix.size() - 1);
}

//��ӡFP��
template <typename Key>
void FPTree<Key>::print(treeNode *r, int depth = 1)const
{
	string right_indent(depth, ' '); //�������������ֳ����Ĳ�νṹ
	cout << right_indent << r->key << ":" << r->count << endl;
	for (auto iter = r->children.begin(); iter != r->children.end(); ++iter)
		print(iter->second, depth + 1);
}

//�����
template <typename Key>
void FPTree<Key>::clear()
{
	for (auto iter = head.begin(); iter != head.end(); ++iter)
	{//����ͷָ���б���һ���������ϣ�ÿ�������Ǽ�ֵ��ͬ�Ľڵ�ļ��ϣ��������ͨ��
		//�ͷ������������ﵽ�������Ŀ��
		treeNode *first = iter->second->next, *r = nullptr;
		while (first)
		{
			r = first;
			first = first->next;
			//cout << "delete tree " << r->key << ' ' << r->count << endl;
			delete r;
		}
		//cout << "delete head " << iter->first << ' ' << iter->second->count << endl;
		delete iter->second; //�ͷ�����ͷ
	}
}

//�������ݼ�
template <typename Key>
unordered_map<string, int> FPTree<Key>::loadData(istream &infile)const
{
	unordered_map<string, int> data;
	string line;
	while (getline(infile, line))
		data.insert(make_pair(line, 1)); //�����ݼ�ת��Ϊ�ֵ䣬��������Ҫ��Ϊ�˷����ھ�Ƶ����ʱ�ݹ鹹����FP��
	return data;
}

int main()
{
	//FPTree<int> tree(100000);
	//�����ݼ�������
	//ifstream infile("C:\\Users\\png\\Desktop\\machinelearninginaction\\Ch12\\kosarak.dat");
	FPTree<char> tree(3);
	ifstream infile("data.txt");
	unordered_map<string, int> data = tree.loadData(infile);
	tree.create(data);
	tree.print();
	vector<string> frequent = tree.mineTree();
	for (int i = 0; i != frequent.size(); ++i)
		cout << frequent[i] << endl;
	getchar();
	return 0;
}