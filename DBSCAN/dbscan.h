#ifndef _DBSCAN_H
#define _DBSCAN_H

#include<vector>
#include<cmath>
#include<cassert>
#include<string>
#include<fstream>
#include<sstream>
#include<algorithm>
#include<queue>
#include<functional>
//#include<iostream>

//DBSCAN�����㷨
class dbscan
{
private:
	enum datatype{ CORE, BOUNDARY, NOISE }; //�����ͣ��ֱ�Ϊ���ĵ㡢�߽�㡢������
	//���ݵ�
	struct point
	{
		const std::vector<double> m_value; //��ֵ
		std::vector<size_t> m_neighbors; //�����ڵ������㣬ֻ������
		datatype m_type = NOISE;
		int m_cluster_id = -1;
		bool m_visited = false;
		point(std::vector<double>&& v) :m_value(std::move(v)){}
	};

	//���������
	struct clusters
	{
		std::vector<std::vector<int>> m_clusters; //��
		std::vector<size_t> m_boundary; //�߽�㼯
		std::vector<size_t> m_noise; //�����㼯
		std::vector<double> m_kdists; //ÿ�����ݵ��k-���룬����k����С����

		//�������������ǽ�������д���ļ���Ȼ�����python������л�ͼ
		//�Կ��ӻ���������ͨ���ı�writeKDists��Kֵ��dbscan�е�m_minpts�������ҵ���ѵ���ֵ�Ͱ뾶

		//��������д���ļ�����д�ش�С����д�����ĵ�
		void writeClusters(const std::string&, const dbscan&)const;
		//��k-���������д���ļ�
		void writeKDists(const std::string&);
	};
private:
	friend struct clusters;
	std::vector<point> m_points; //���ݵ㼯
	const double m_radius; //����뾶
	const size_t m_minpts; //�����ڵ�����ֵ
	const size_t m_dim; //���ݵ�ά��
	size_t m_cluster_num; //���մ���
	clusters m_res; //������

private:
	//�������ݵ���룬Ҳ������������������
	double distance(const point& lhs, const point& rhs)const
	{
		assert(lhs.m_value.size() == rhs.m_value.size());
		double temp = 0.0;
		for (int i = 0; i != lhs.m_value.size(); ++i)
			temp += (lhs.m_value[i] - rhs.m_value[i]) * (lhs.m_value[i] - rhs.m_value[i]);
		return sqrt(temp);
	}

	//���ڵ�ǰ���ĵ�ݹ����
	void coreClustering(int, int);
	//ɸѡ���ĵ�
	void findCorePoints();

	dbscan(const dbscan&);
	dbscan& operator=(const dbscan&);
public:
	//�ù��캯��ֻ����Ѱ����Ѱ뾶��kֵʱʹ��
	dbscan(size_t k, size_t d) :m_radius(0.), m_minpts(k), m_dim(d){}
	dbscan(double r, size_t m, size_t d) :m_radius(r), m_minpts(m), m_dim(d){}
	size_t getClustersNum()const
	{
		return m_cluster_num;
	}
	size_t getPointsNum()const
	{
		return m_points.size();
	}
	void readPoints(const std::string&);
	//���������������ڼ���k-���룬Ȼ�����python������ƣ����ҳ���ѵ�k�Ͱ뾶
	void computeKdists();
	void drawKdists();

	void clustering();
	//�����յľ�����д���ļ������յ�ID��������ID�����ݵ�ֵ�����������ݵ�������˳��
	void writePoints(const std::string&);
	//���ƾ���Ľ��
	void drawClusters();
};

void dbscan::readPoints(const std::string& filename)
{
	std::ifstream infile(filename);
	assert(infile);
	int count = 0;
	double temp;
	std::vector<double> vtemp;
	while (infile >> temp)
	{
		++count;
		vtemp.emplace_back(temp);
		if (count == m_dim)
		{
			m_points.emplace_back(point(std::move(vtemp)));
			count = 0;
		}
	}
	infile.close();
}

void dbscan::computeKdists()
{
	m_res.m_kdists.reserve(getPointsNum());
	double dist = 0.0;
	for (int i = 0; i != m_points.size(); ++i)
	{
		int m = m_minpts;
		std::priority_queue<double> priq; //������ȼ�����
		for (int j = 0; j != m_points.size(); ++j)
		{
			if (i == j) continue;
			dist = distance(m_points[i], m_points[j]);
			--m;
			if (m >= 0)
				priq.emplace(dist);
			else if (dist < priq.top())
			{
				priq.pop();
				priq.emplace(dist);
			}
		}
		m_res.m_kdists.emplace_back(priq.top());//k-����
	}
}

void dbscan::drawKdists()
{
	const std::string filename = "k-dists.txt";//k-������������ļ�
	assert(!m_res.m_kdists.empty());
	m_res.writeKDists(filename);
	const std::string command = "python drawKDists.py";
	system(command.c_str());
}

void dbscan::drawClusters()
{
	const std::string filename = "clusters.txt";//����������ļ�
	assert(!m_res.m_clusters.empty());
	m_res.writeClusters(filename, *this);
	const std::string command = "python drawClusters.py";
	system(command.c_str());
}

void dbscan::findCorePoints()
{
	double dist = 0.0;
	for (int i = 0; i != m_points.size(); ++i)
	{
		for (int j = 0; j != m_points.size(); ++j)
		{
			if (i == j)
				continue;
			dist = distance(m_points[i], m_points[j]);
			if (dist <= m_radius)//����ڰ뾶�ڣ���˵���ڵ�i��������
				m_points[i].m_neighbors.emplace_back(j);//��������
		}
		if (m_points[i].m_neighbors.size() >= m_minpts)//����������ֵ
			m_points[i].m_type = CORE;//��Ϊ���ĵ�
	}
}

void dbscan::clustering()
{
	findCorePoints();
	int cluster_id = 0;
	for (int i = 0; i != m_points.size(); ++i)
	{
		if (m_points[i].m_type == CORE && !m_points[i].m_visited)
		{//�õ�Ϊ���ĵ㣬��û�з��ʹ���û�б������κ�һ���أ�
			coreClustering(i, cluster_id);//��Ըôؽ�����չ
			++cluster_id;
		}
	}
	m_cluster_num = cluster_id;//�õ�����
}

void dbscan::coreClustering(int pid, int cluster_id)
{
	if (m_points[pid].m_visited)
		return;
	else if (m_points[pid].m_type == NOISE)
	{//coreClustering�ʼ�ĵ������ɺ��ĵ���õģ�����������ĳ��ΪNOISE����˵���õ�
		//�Ǳ߽�㣬��Ϊ��������ĳ���ĵ������ڣ����Լ��ֲ��Ǻ��ĵ�
		m_points[pid].m_type = BOUNDARY;
		//return;
	}
	m_points[pid].m_cluster_id = cluster_id;
	m_points[pid].m_visited = true;
	if (m_points[pid].m_type != CORE)
		return;
	for (int i = 0; i != m_points[pid].m_neighbors.size(); ++i)
	{
		coreClustering(m_points[pid].m_neighbors[i], cluster_id);//�����������
	}
}

void dbscan::writePoints(const std::string& filename)
{
	std::ofstream outfile(filename);
	assert(outfile);
	m_res.m_clusters.resize(m_cluster_num);//���þ������еĴش�С
	outfile << "cluster numbers: " << m_cluster_num << std::endl;
	for (int i = 0; i != m_points.size(); ++i)
	{
		std::ostringstream ostream;
		ostream << "ID: " << i << " cluster ID: " << m_points[i].m_cluster_id
			<< " Type: ";
		switch (m_points[i].m_type)
		{
		case CORE:
			ostream << "core";
			m_res.m_clusters[m_points[i].m_cluster_id].emplace_back(i);
			break;
		case BOUNDARY:
			ostream << "boundary";
			m_res.m_clusters[m_points[i].m_cluster_id].emplace_back(i);//�߽��Ҳ����ĳ��
			m_res.m_boundary.emplace_back(i);
			break;
		case NOISE:
			ostream << "noise";
			m_res.m_noise.emplace_back(i);
			break;
		default:
			ostream << "unknown";
			break;
		}
		ostream << " [";
		for (int j = 0; j != m_points[i].m_value.size(); ++j)
			ostream << m_points[i].m_value[j] << ',';
		ostream << ']' << std::endl << "neighbors: ";
		for (int k = 0; k != m_points[i].m_neighbors.size(); ++k)
			ostream << m_points[i].m_neighbors[k] << '\t';
		ostream << "\n\n";
		outfile << ostream.str();
	}
	outfile.close();
}

void dbscan::clusters::writeKDists(const std::string& filename)
{
	std::ofstream outfile(filename);
	assert(outfile);
	sort(m_kdists.begin(), m_kdists.end()); //������
	//double sum = 0.0;
	//for_each(m_kdists.begin(), m_kdists.end(), [&sum](const double& v){sum += v; });
	//std::cout << sum << std::endl;
	for (int i = 0; i != m_kdists.size(); ++i)
		outfile << m_kdists[i] << '\t';//��д
	outfile.close();
}

void dbscan::clusters::writeClusters(const std::string& filename, const dbscan& db)const
{
	std::ofstream outfile(filename);
	assert(outfile);
	//��д������
	for (int i = 0; i != m_clusters.size(); ++i)
	{
		outfile << m_clusters[i].size() << std::endl;//��д�ش�С
		for (int j = 0; j != m_clusters[i].size(); ++j)//��д�ô��������ݵ�
		{
			auto& p = db.m_points[m_clusters[i][j]].m_value;
			for (int k = 0; k != p.size(); ++k)
				outfile << p[k] << '\t';
			outfile << std::endl;
		}
	}

	//д�߽�㼯
	outfile << m_boundary.size() << std::endl;
	for (int i = 0; i != m_boundary.size(); ++i)
	{
		auto& p = db.m_points[m_boundary[i]].m_value;
		for (int k = 0; k != p.size(); ++k)
			outfile << p[k] << '\t';
		outfile << std::endl;
	}

	//д�����㼯
	outfile << m_noise.size() << std::endl;
	for (int i = 0; i != m_noise.size(); ++i)
	{
		auto& p = db.m_points[m_noise[i]].m_value;
		for (int k = 0; k != p.size(); ++k)
			outfile << p[k] << '\t';
		outfile << std::endl;
	}
	outfile.close();
}

#endif