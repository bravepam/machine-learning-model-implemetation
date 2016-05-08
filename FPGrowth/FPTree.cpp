#include<iostream>
#include<sstream>
#include<string>
#include<unordered_map>
#include<algorithm>
#include<vector>

using namespace std;

//FP��������ʵ��FP-growth�㷨����Ч�ھ�Ƶ����

struct node
{//�����ڵ�
	string key;
	int count;
	node(const string& v, int cnt) :key(move(v)), count(cnt){}

	struct comparer
	{//node�ڵ�Ƚ���
		struct less
		{
			bool operator()(const node& lhs, const node& rhs)const
			{
				if (lhs.count == rhs.count) //�������ֵ���
					return lhs.key < rhs.key; //��Ƚϼ�ֵ
				return lhs.count < rhs.count; //����
			}
		};

		struct greater
		{
			bool operator()(const node& lhs, const node& rhs)const
			{
				if (lhs.count == rhs.count)
					return lhs.key > rhs.key;
				return lhs.count > rhs.count;
			}	
		};
	};
};

struct treeNode :public node
{//���ڵ����ͣ��̳���node
	treeNode *parent;
	treeNode *next = nullptr;
	unordered_map<string, treeNode*> children; //�����б����ֵ䷽�����
	treeNode(const string& v, int cnt, treeNode *par = nullptr) :
		node(v, cnt), parent(par), children(){}
};

struct headNode
{//ͷָ���б�ڵ�
	int count;
	treeNode *ptreenode = nullptr;//�������о�����ͬ��ֵ�����ڵ�
	treeNode *tail = nullptr; //����β�ڵ�
	headNode(int cnt) :count(cnt){}
};

class FPTree
{//FP��
private:
	unordered_map<string, headNode*> head; //ͷָ���б�
	treeNode *root;
	int eps; //������ֵ
private:
	void updateTree(const vector<string>&, int, treeNode*, int); 
	void mineTree(string&, unordered_map<string, int>&);
	void findPrefixPath(const string&,unordered_map<string,int>&);
	void ascendTree(treeNode*, string&)const;
	void print(treeNode*, int)const;
public:
	FPTree(int e) :root(new treeNode(string(), 0)), eps(e){}
	unordered_map<string, int> loadData(const string&)const;
	void create(const unordered_map<string, int>&);
	unordered_map<string, int> mineTree()
	{
		string prefix("(");
		unordered_map<string, int> frequent;
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
void FPTree::create(const unordered_map<string, int> &data)
{
	unordered_map<string, int> umkey_count;//��¼���еļ�����ÿ����¼��Լ�����
	string k;
	for (auto iter1 = data.begin(); iter1 != data.end(); ++iter1) //��һ��ɨ���������ݼ�
	{//ÿ����¼�����ַ������󣬲��ÿո�ֿ�
		istringstream stream(iter1->first); //��ʼ��һ���ַ���������
		while (stream >> k)
		{//��һ��ü�¼��
			if (umkey_count.find(k) == umkey_count.end()) //�����������ͷָ���б������
				umkey_count.emplace(move(k), iter1->second);
			else //����ֻ��Ҫ�ۼӼ���ֵ����
				umkey_count[k] += iter1->second;
		}
	}

	for (auto iter2 = umkey_count.begin(); iter2 != umkey_count.end(); ++iter2)
	{//ɨ��ÿ����¼��
		if (iter2->second >= eps) //�ѽϸ�Ƶ���ļ�¼������
		{
			head.emplace(iter2->first, new headNode(iter2->second));
		}
	}

	if (head.empty()) return; //���û�г��ִ���������ֵ�ļ�¼����˳�
	for (auto iter3 = data.begin(); iter3 != data.end(); ++iter3)
	{//�ڶ���ɨ���������ݼ�
		//����ÿ����¼�����ݼ�¼��ͳ��ֵ��ܴ�������ڵ㣬Ȼ����뵽����
		vector<node> key_count; 
		istringstream stream(iter3->first);
		while (stream >> k)
		{//���ڸ�����¼�е�ÿ����¼��
			if (head.find(k) != head.end()) //ֻ�г��ִ����ϸ�
				key_count.emplace_back(node(k, head[k]->count)); //�Żᱻ���뵽������
		}
		if (!key_count.empty())
		{//�����ո�����¼���г��ִ����϶����
			sort(key_count.begin(), key_count.end(), node::comparer::greater());
			vector<string> order;
			//���ճ��ִ����Ӹߵ��͵�˳����ȡ����
			for_each(key_count.begin(), key_count.end(), [&order](const node &arg){order.emplace_back(arg.key); });
			updateTree(order, 0, root, iter3->second);//���ڸ�����
		}
	}
}

//����order��Ԫ�ص�˳�򣬴ӽڵ�r��ʼ���¸�����
void FPTree::updateTree(const vector<string> &order, int index, treeNode *r, int count)
{
	if (r->children.find(order[index]) != r->children.end()) //�����ǰ����r�ĺ���
		r->children[order[index]]->count += count;//��ֱ�Ӹ��¼���ֵ
	else
	{
		treeNode *p = new treeNode(order[index], count, r); //������ڵ�
		r->children.emplace(order[index], p); //���뵽�����б�
		if (!(head[order[index]]->ptreenode))//ͬʱ����ͷָ���б�
			head[order[index]]->ptreenode = p;
		else 
			head[order[index]]->tail->next = p;
		head[order[index]]->tail = p;
	}
	if ((static_cast<int>(order.size()) - index) > 1) //���order�ܻ�������
		updateTree(order, index + 1, r->children[order[index]], count);//������ݹ����¸�����
}

//�ӹ���õ������ھ�Ƶ���
void FPTree::mineTree(string &prefix, unordered_map<string,int> &frequent)
{
	//ͷָ���б�head�д洢��ʵ���ǵ�Ƶ���������ֵ�ͼ���ֵ����ڵ㣬���뵽����
	//��ͬ�ڽ���ʱ����ʱ�ǰ��ռ���ֵ�ӵ͵���˳������
	vector<node> key_count;
	for (auto iter = head.begin(); iter != head.end(); ++iter)
		key_count.emplace_back(node(iter->first, iter->second->count));
	sort(key_count.begin(), key_count.end(), node::comparer::less());
	for (auto iter = key_count.begin(); iter != key_count.end(); ++iter)
	{//����ÿһ����Ƶ����
		prefix.append(iter->key);
		frequent.emplace(prefix + ')',iter->count); //����һ��Ƶ����
		unordered_map<string, int> subdata;

		//�Ե�ǰ��Ƶ����Ϊβ�������������ھ���������Ҳ�������ѳ���prefix��������ھ��¼
		findPrefixPath(iter->key, subdata); 
		FPTree subtree(eps);
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
void FPTree::findPrefixPath(const string& tail, unordered_map<string, int> &paths)
{
	treeNode *first = head[tail]->ptreenode;
	while (first)
	{//���ڵ�ǰ��Ϊtail�Ľڵ�
		string prefix;
		ascendTree(first, prefix);//�ɴ����ݵ������������
		if (!prefix.empty())//�������������
			paths.emplace(move(prefix), first->count);
		first = first->next;
	}
}

//�Ե�ǰ�ڵ㿪ʼ���ݵ�������˳����½ڵ��ֵ������һ��������
void FPTree::ascendTree(treeNode *curr, string &prefix)const
{
	treeNode *par = curr->parent; //��������������ǰ�ڵ�
	bool is_int = is_same<string, int>::value;
	while (par->parent)
	{
		prefix.append(par->key);
		prefix.push_back(' ');
		par = par->parent;
	}
	if (!prefix.empty()) //ɾ��ĩβ�Ŀո�
		prefix.resize(prefix.size() - 1);
}

//��ӡFP��
void FPTree::print(treeNode *r, int depth = 1)const
{
	string right_indent(depth, ' '); //�������������ֳ����Ĳ�νṹ
	cout << right_indent << r->key << ":" << r->count << endl;
	for (auto iter = r->children.begin(); iter != r->children.end(); ++iter)
		print(iter->second, depth + 1);
}

//�����
void FPTree::clear()
{
	for (auto iter = head.begin(); iter != head.end(); ++iter)
	{//����ͷָ���б���һ���������ϣ�ÿ�������Ǽ�ֵ��ͬ�Ľڵ�ļ��ϣ��������ͨ��
		//�ͷ������������ﵽ�������Ŀ��
		treeNode *first = iter->second->ptreenode, *r = nullptr;
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
unordered_map<string, int> FPTree::loadData(const string& filename)const
{
	unordered_map<string, int> data;
	const size_t SIZE = 32 * 1024 * 1024;
	char* buf = new char[SIZE];
	FILE *fp;
	fopen_s(&fp, filename.c_str(), "rb");
	size_t len = fread_s(buf, SIZE, 1, SIZE, fp);
	buf[len] = '\0';
	string line;
	for (char* ptr = buf, *first = buf; static_cast<size_t>(ptr - buf) <= len; ++ptr)
	{
		if (*ptr == '\n' || (buf + len) == ptr)
		{
			if (*(ptr - 1) == '\r')
				line.append(first, ptr - 1);
			else line.append(first, ptr);
			//�����ݼ�ת��Ϊ�ֵ䣬��������Ҫ��Ϊ�˷����ھ�Ƶ����ʱ�ݹ鹹����FP��
			data.emplace(move(line), 1);
			first = ptr + 1;
		}
	}
	delete buf;
	return data;
}

const string filename = "data.txt";
//�����ݼ�������
//const string filename = "C:\\Users\\png\\Desktop\\machinelearninginaction\\Ch12\\kosarak.dat";

int main()
{
	//FPTree tree(100000);
	FPTree tree(3);
	unordered_map<string, int> data = tree.loadData(filename);
	tree.create(data);
	tree.print();
	unordered_map<string, int> frequent = tree.mineTree();
	for (auto i = frequent.begin(); i != frequent.end(); ++i)
		cout << i->first << "----------------" << i->second << endl;
	getchar();
	return 0;
}