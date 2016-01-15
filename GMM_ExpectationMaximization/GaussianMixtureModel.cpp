#include<vector>
#include<iostream>

using namespace std;

const double E = 2.7182818284;
const double PI = 3.1415926;

class GMM
{//��˹���ģ��
private:
	struct GM
	{//��˹ģ��
		double weight;
		double expectation;
		double variance;
		GM() :weight(0.), expectation(0.), variance(1.0){}
		GM(double w, double e, double v) :weight(w), expectation(e), variance(v){}
		double compute(double y)const
		{//�����Ȩ����
			double temp1 = weight / (sqrt(2 * PI) * variance);
			double temp2 = pow(E, -(y - expectation) * (y - expectation) / (2 * variance));
			return temp1 * temp2;
		}
		double error(const vector<double> &v)const
		{//����ǰ������ģ�Ͳ���������
			double temp = pow(weight - v[0], 2) + pow(expectation - v[1], 2)
				+ pow(variance - v[2], 2);
			return sqrt(temp);
		}
		double setGM(const vector<double> &v)
		{//����ģ�Ͳ���
			double err = error(v);
			weight = v[0], expectation = v[1], variance = v[2];
			return err;
		}
	};
private:
	vector<GM> gmm;
	double eps;//����
private:
	void initGMM(const vector<double> &data)
	{//��ʼ��GMM
		const int k = gmm.size(), n = data.size();
		vector<double> sum(k);
		double data_sum = 0.0;
		for (int j = 0; j != n; ++j)
		{
			sum[j % k] += data[j];
			data_sum += data[j];
		}
		for (int i = 0; i != k; ++i)
		{
			gmm[i].expectation = sum[i] / (n / k);
			gmm[i].variance = (gmm[i].expectation - data_sum / n) *
				(gmm[i].expectation - data_sum / n);
		}
	}
public:
	GMM(int k, double _eps) :gmm(k), eps(_eps)
	{
		double w = 1.0 / k;
		for (int i = 0; i != gmm.size(); ++i)
			gmm[i].weight = w;
	}
	void train(const vector<double>&);
	double compute(double y)const
	{
		double sum = 0.0;
		for (int i = 0; i != gmm.size(); ++i)
			sum += gmm[i].compute(y);
		return sum;
	}
	void print()const
	{
		printf("weight         expectation   variance\n");
		for (int i = 0; i != gmm.size(); ++i)
			printf("%-15lf%-15lf%-15lf\n", gmm[i].weight, gmm[i].expectation, gmm[i].variance);
	}
};

void GMM::train(const vector<double> &data)
{//ѵ��ģ��
	initGMM(data);
	const int k = gmm.size(), n = data.size();
	vector<vector<double>> response(n, vector<double>(k + 1, 0.));
	while (true)
	{
		for (int i = 0; i != n; ++i)
		{//��ÿ������
			response[i][k] = 0.0;
			for (int j = 0; j != k; ++j)
			{//���������˹ģ�ͷ�������Ӧ
				response[i][j] = gmm[j].compute(data[i]);
				response[i][k] += response[i][j];
			}
			for (int jj = 0; jj != k; ++jj)
				response[i][jj] /= response[i][k]; //������Ӧ��
		}
		bool is_continue = false;
		for (int j = 0; j != k; ++j)
		{//����ÿ����˹ģ�ͷ������²���
			double sum_e = 0., sum_v = 0., sum_w = 0.;
			for (int i = 0; i != n; ++i)
			{
				sum_e += response[i][j] * data[i]; //��ֵ
				sum_v += response[i][j] * (data[i] - gmm[j].expectation) *
					(data[i] - gmm[j].expectation); //����
				sum_w += response[i][j];//Ȩ��
			}
			double err = gmm[j].setGM({ sum_w / n, sum_e / sum_w, sum_v / sum_w });
			if (err >= eps) is_continue = true;
		}
		if (!is_continue) break;
	}
}

int main()
{
	vector<double> data = { -67, -48, 6, 8, 14, 16, 23, 24, 28, 29, 41, 49, 56, 60, 75 };
	GMM gmm(3, 0.001);
	gmm.train(data);
	gmm.print();
	cout << gmm.compute(24) << endl;
	getchar();
	return 0;
}