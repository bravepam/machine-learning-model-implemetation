
/*******************************************
* Author: bravepam
*
* E-mail:1372120340@qq.com
*******************************************
*/


#include<vector>
#include<iostream>
#include<algorithm>

using namespace std;

class HMM
{//������Ʒ�ģ��
private:
	vector<double> a; //��ʼ״̬����
	vector<vector<double>> A; //״̬ת�Ƹ��ʾ���
	vector<vector<double>> B; //״̬���ɹ۲���ʾ���
	const int N; //״̬����
	const int M; //�۲�ֵ����
private:
	vector<int> o; //��ʱ�������õĹ۲�����
private:
	vector<vector<double>> forward; //ǰ�����
	vector<vector<double>> backward; //�������
	vector<vector<double>> when_which; //ĳʱ�̳���ĳ״̬�ĸ���
	vector<vector<vector<double>>> trans; //ĳʱ�̴�ĳ״̬ת�Ƶ���һ״̬�ĸ���
private:
	template <typename T>
	void initMatrix(vector<vector<T>> &m, int r, int c)const
	{//����ά�������ռ�
		m.resize(r);
		for (int i = 0; i != r; ++i)
			m[i].resize(c);
	}
	template <typename T>
	void initMatrix(vector<vector<vector<T>>> &m, int d1, int d2, int d3)const
	{//����ά�������ռ�
		m.resize(d1);
		for (int i = 0; i != d1; ++i)
			initMatrix(m[i], d2, d3);
	}
	void HMM::initHMM()
	{//�޼ලѧϰģ��ǰ��ģ�Ͳ����趨��ֵ
		const double trans_pro = 1.0 / N, ge_state_pro = 1.0 / M;
		fill_n(a.begin(), a.size(), trans_pro); //��ֵ״̬���ʺ�Ϊ1
		for (int i = 0; i != N; ++i)
		{
			fill_n(A[i].begin(), A[i].size(), trans_pro); //��״̬iת�Ƶ�����״̬�ĸ��ʺ�Ϊ1
			fill_n(B[i].begin(), B[i].size(), ge_state_pro); //��״̬i���ɹ۲�ֵ�ø��ʺ�Ϊ1
		}
	}
	double vectorError(const vector<double> &v1, const vector<double> &v2)const
	{//�����������ƽ����
		double sum = 0.0;
		for (int i = 0; i != v1.size(); ++i)
			sum += (v1[i] - v2[i]) * (v1[i] - v2[i]);
		return sum;
	}

	//����ģ�Ͳ���������
	double error(const vector<vector<double>>&, const vector<vector<double>>&, 
		const vector<double>&)const;
public:
	HMM(int n, int m) :a(n), A(n, vector<double>(n)), B(n, vector<double>(m)), N(n), M(m){ initHMM(); }
	HMM(vector<double> &_a, vector<vector<double>> &_A, vector<vector<double>> &_B) :
		a(move(_a)), A(move(_A)), B(move(_B)), N(a.size()), M(B[0].size()){}
	//���ù۲�����
	void setObservation(vector<int> &_o)
	{
		o = move(_o);
	}

	//probability computation
	//����ǰ������Եõ��۲����г��ֵĸ���
	double forwardPro();
	//�����������Եõ��۲����г��ֵĸ���
	double backwardPro();
	//����ĳʱ�̳���ĳ״̬�ĸ���
	double whenAndWhichStatePro(int = -1, int = -1);
	//����ĳʱ��״̬֮��ת�Ƶĸ���
	double stateTransfer(int = -1, int = -1, int = -1);
	//����ĳ״̬������
	double expectationOfstate(int);
	//�Ӿ���ĳ״̬ת�Ƶ���һ״̬������
	double expectationTransFromState(int );
	//��״̬1ת�Ƶ�״̬2������
	double expectationTransItoJ(int, int);

	//learning
	//�ලѧϰ�����ṩ���ù۲�ֵ���к�״̬����
	void supervisedLearning(const vector<vector<int>>&, const vector<vector<int>>&);
	//�޼ලѧϰ��ֻ�ṩ�۲�ֵ����
	void unsupervisedLearning(vector<int>&, double);

	//prediction
	//double optimalPathPro(vector<int>&);
	//ά�ر��㷨�������ĳһ�۲�����ʱ�����״̬���У������ظ���
	double viterbi(vector<int>&, vector<int>&);
	//�����㷨�������ĳһ�۲�����ʱ�����״̬���У������ظ���
	double approximate(vector<int>&, vector<int>&);
};

double HMM::forwardPro()
{
	const int T = o.size();
	initMatrix(forward, T, N);
	for (int i = 0; i != N; ++i)
		forward[0][i] = a[i] * B[i][o[0]]; //��ʼ0ʱ��ʱ����״̬i�ĸ���
	for (int t = 0; t != T - 1; ++t)
	{
		for (int i = 0; i != N; ++i)
		{
			for (int j = 0; j != N; ++j)
				//���ƣ�ʱ��t+1����i�ĸ��ʣ�N������ǰ�����п���ת�Ƶ�״̬i
				forward[t + 1][i] += forward[t][j] * A[j][i];
			//���Դ�״̬i���ɸ�ʱ�̹۲�ֵ�ĸ��ʣ����õ�ǰ�����
			forward[t + 1][i] *= B[i][o[t + 1]]; 
		}
	}
	double sum = 0.0;
	for (int i = 0; i != N; ++i)
		sum += forward[T - 1][i];
	return sum; //�۲����г��ֵĸ���
}

double HMM::backwardPro()
{
	const int T = o.size();
	initMatrix(backward, T, N);
	for (int i = 0; i != N; ++i)
		backward[T - 1][i] = 1.; //��ʼ��ʱ��ĩβ֮������κ�״̬���У�������Ϊ1
	for (int t = T - 2; t >= 0; --t)
	{
		for (int i = 0; i != N; ++i)
		{
			for (int j = 0; j != N; ++j)
				//���ƣ�
				backward[t][i] += A[i][j] * B[j][o[t + 1]] * backward[t + 1][j];
		}
	}
	double sum = 0.0;
	for (int i = 0; i != N; ++i)
		sum += a[i] * B[i][o[0]] * backward[0][i];
	return sum; //�۲����г��ֵĸ���
}

double HMM::whenAndWhichStatePro(int time, int state)
{
	if (!when_which.empty())
		return when_which[time][state];
	if (forward.empty() || backward.empty())
	{//����ǰ��ͺ������
		forwardPro();
		backwardPro();
	}
	const int T = o.size();
	initMatrix(when_which, T, N);
	double sum = 0.0;
	for (int j = 0; j != N; ++j)
		sum += forward[0][j] * backward[0][j]; //��ʱ��t������һ״̬�ĸ���֮�����

	for (int t = 0; t != T; ++t)
		for (int i = 0; i != N; ++i)//ʱ��t����״̬i�ĸ��ʳ���ʱ��t����״̬���ܸ��ʼ�Ϊ...
			when_which[t][i] = forward[t][i] * backward[t][i] / sum;
	if (time < 0) return 0.0;
	return when_which[time][state];
}

double HMM::stateTransfer(int time, int state1, int state2)
{
	if (!trans.empty()) return trans[time][state1][state2];
	if (forward.empty() || backward.empty())
	{
		forwardPro();
		backwardPro();
	}
	const int T = o.size();
	initMatrix(trans, T, N, N);
	for (int t = 0; t != T; ++t)
	{
		double sum = 0.0;
		for (int i = 0; i != N; ++i)
		{
			for (int j = 0; j != N; ++j)
			{
				//ʱ��t��״̬iת�Ƶ�״̬j�ĸ���
				trans[t][i][j] = forward[t][i] * A[i][j] * B[j][o[t]] * backward[t][j];
				sum += trans[t][i][j]; //ʱ��t����״̬ת�Ƶ��ܸ��ʸ���
			}
		}
		for (int i = 0; i != N; ++i)
		{
			for (int j = 0; j != N; ++j)
				trans[t][i][j] /= sum; //��ȼ�Ϊʱ��t��״̬iת�Ƶ�״̬j�ĸ��ʣ���������
		}
	}
	if (time < 0) return 0.0;
	return trans[time][state1][state2];
}

double HMM::expectationOfstate(int state)
{
	double sum = 0.0;
	const int T = o.size();
	for (int t = 0; t != T; ++t)
		sum += when_which[t][state];
	return sum;
}

double HMM::expectationTransFromState(int state)
{
	double sum = 0.0;
	const int T = o.size();
	for (int t = 0; t != T - 1; ++t)
		sum += when_which[t][state];
	return sum;
}

double HMM::expectationTransItoJ(int state_I, int state_J)
{
	double sum = 0.0;
	const int T = o.size();
	for (int t = 0; t != T - 1; ++t)
		sum += trans[t][state_I][state_J];
	return sum;
}

void HMM::supervisedLearning(const vector<vector<int>> &obs, const vector<vector<int>> &state)
{//�ලѧϰ
	const int S = obs.size(), T = obs[0].size();
	vector<double> sum_trans(N), sum_obs(N);
	for (int s = 0; s != S; ++s)
	{
		for (int t = 0; t != T - 1; ++t)
		{
			++A[state[s][t]][state[s][t + 1]];
			++B[state[s][t]][obs[s][t]];
			++sum_trans[state[s][t]];
			++sum_obs[state[s][t]];
		}
		++a[state[s][0]];
	}
	for (int i = 0; i != N; ++i)
	{
		for (int j = 0; j != N; ++j)
			A[i][j] /= sum_trans[i]; //��״̬iת�Ƶ�״̬j��Ƶ�����Դ�״̬i����ת�Ƶ�Ƶ����Ϊת�Ƹ���
	}
	for (int i = 0; i != N; ++i)
	{
		for (int k = 0; k != M; ++k)
			B[i][k] /= sum_obs[i]; //��״̬i���ɹ۲�ֵk��Ƶ�����ϴ�״̬i���ɹ۲�ֵ��Ƶ����Ϊ���ɸ���
	}
	for (int i = 0; i != N; ++i)
		a[i] /= S; //ʱ��0״̬i���ֵ�Ƶ����������������Ϊ��ʼ״̬����
}

void HMM::unsupervisedLearning(vector<int> &_o, double eps)
{//�޼ලѧϰ
	const int T = _o.size();
	setObservation(_o);
	vector<double> sum1(N), tempa(N);
	vector<vector<double>> sum2, sum3, tempA, tempB;
	initMatrix(sum2, N, N); initMatrix(sum3, N, M);
	initMatrix(tempA, N, N); initMatrix(tempB, N, M);
	while (true)
	{
		whenAndWhichStatePro(); //���ݵ�ǰģ�Ͳ�������ĳʱ�̳��˰�ĳ״̬�ĸ���
		stateTransfer(); //����״̬ת�Ƹ���
		//����EM�㷨�����µ�ģ�Ͳ���
		for (int t = 0; t != T; ++t)
		{
			for (int i = 0; i != N; ++i)
			{
				sum1[i] += when_which[t][i];
				for (int j = 0; j != N; ++j)
					sum2[i][j] += trans[t][i][j];
				for (int k = 0; k != M; ++k)
					sum3[i][k] += when_which[t][i] * static_cast<double>(k == o[t]);
			}
		}
		for (int i = 0; i != N; ++i)
		{
			for (int j = 0; j != N; ++j)
				tempA[i][j] = sum2[i][j] / sum1[i];
			for (int k = 0; k != M; ++k)
				tempB[i][k] = sum3[i][k] / sum1[i];
			tempa[i] = when_which[0][i];
			fill_n(sum2[i].begin(), sum2[i].size(), 0.0);
			fill_n(sum3[i].begin(), sum3[i].size(), 0.0);
		}
		fill_n(sum1.begin(), sum1.size(), 0.0);
		double e = error(tempA, tempB, tempa); //�Ƚ����
		A.swap(tempA), B.swap(tempB), a.swap(tempa);
		if (e < eps) break; //���ѵ��������ȣ����˳�
	}
	cout << "learning done..." << endl;
}

double HMM::error(const vector<vector<double>> &tempA, const vector<vector<double>> &tempB,
	const vector<double> &tempa)const
{
	double sum = 0.0;
	for (int i = 0; i != tempA.size(); ++i)
		sum += vectorError(tempA[i], A[i]);
	for (int j = 0; j != tempB.size(); ++j)
		sum += vectorError(tempB[j], B[j]);
	sum += vectorError(tempa, a);
	return sqrt(sum);
}

double HMM::approximate(vector<int> &_o, vector<int> &res)
{//�����㷨������۲�����o������״̬����
	setObservation(_o);
	whenAndWhichStatePro(); //��Ҫ�õ�ĳʱ�̳���ĳ״̬�ĸ��ʾ���
	const int T = o.size();
	double max = 0.0, pro = 1.0;
	int state = -1;
	for (int t = 0; t != T; ++t)
	{
		for (int i = 0; i != when_which[t].size();++i)
			if (when_which[t][i] > max)
			{//����ĳʱ�̳�������ܵ�״̬
				max = when_which[t][i];
				state = i;
			}
		pro *= max;
		res.push_back(state); //��Ϊ����״̬
	}
	return pro; //���ظ�״̬���ֵĸ���
}

double HMM::viterbi(vector<int> &_o, vector<int> &res)
{//ά�ر��㷨��������۲�����o������״̬����
	setObservation(_o);
	const int T = o.size();
	//��¼��ʱ��t״̬Ϊi������·���и��ʵ����ֵ,max_pro[t][i]
	vector<vector<double>> max_pro;
	//��¼��ʱ��t״̬Ϊi��������·���еĵ�t - 1���ڵ㣬���ܹ�������ת�Ƶ�״̬i��ĳһ״̬,node[t][i]
	vector<vector<int>> node;
	initMatrix(max_pro, T, N), initMatrix(node, T, N);
	for (int i = 0; i != N; ++i)
	{
		//��ʼʱ��ȡ��״̬i�ĸ���
		max_pro[0][i] = a[i] * B[i][o[0]]; 
		node[0][i] = -1; //��ǰ��
	}
	for (int t = 1; t != T; ++t)
	{//��ʱ��t
		for (int i = 0; i != N; ++i)
		{//״̬Ϊi��������·��
			double max = 0.0;
			int node_id = -1;
			for (int j = 0; j != N;++j) //��ÿһ������ǰ��
				if (max_pro[t - 1][j] * A[j][i] > max)
				{//������ʣ����¸�������
					max = max_pro[t - 1][j] * A[j][i];
					node_id = j;
				}
			max_pro[t][i] = max * B[i][o[t]]; //�������
			node[t][i] = node_id; //���õ�ǰ��
		}
	}
	double max = 0.0;
	int last_node = -1;
	for (int i = 0; i != N;++i)
		if (max_pro[T - 1][i] > max)
		{//������ʱ��ɨ��ÿһ������״̬�����¸�������
			max = max_pro[T - 1][i];
			last_node = i;
		}
	res.push_back(last_node); //��Ϊ״̬�������һ��״̬
	for (int t = T - 1; t > 0; --t)
	{//�����ʱ�̿�ʼ������node����õ�ÿһ��ǰ��
		res.push_back(node[t][last_node]);
		last_node = node[t][last_node];
	}
	reverse(res.begin(), res.end()); //���ã����õ�״̬����
	return max;
}

int main()
{
	//vector<vector<double>> A = { { 0.5, 0.2, 0.3 }, { 0.3, 0.5, 0.2 }, { 0.2, 0.3, 0.5 } };
	//vector<vector<double>> B = { { 0.5, 0.5 }, { 0.4, 0.6 }, { 0.7, 0.3 } };
	//vector<double> a = { 0.2, 0.4, 0.4 };
	//HMM hmm(a, A, B);
	//vector<int> o = { 0, 1, 0, 0, 1, 0, 1, 1 };
	//hmm.setObservation(o);
	//cout << hmm.forwardPro() << endl;
	//cout << hmm.backwardPro() << endl;
	//cout << hmm.whenAndWhichStatePro(4, 2) << endl;
	//cout << hmm.stateTransfer(2,1,2) << endl;

	//vector<int> o = { 0, 1, 0 }, res;
	//cout << hmm.approximate(o, res) << endl;
	//for (int i = 0; i != res.size(); ++i)
	//cout << res[i] << ' ';
	//cout << endl;
	//cout << hmm.viterbi(o, res) << endl;
	//for (int i = 0; i != res.size(); ++i)
	//cout << res[i] << ' ';
	//cout << endl;

	HMM hmm(3, 2);
	vector<int> o = { 0, 1, 0, 0, 1, 0, 1, 1 };
	hmm.unsupervisedLearning(o, 0.1);
	getchar();
	return 0;
}
