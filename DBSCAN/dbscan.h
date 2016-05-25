#ifndef _DBSCAN_H
#define _DBSCAN_H

#include<vector>
#include<cassert>
#include"kdtree_in_dbscan.h"

enum datatype{ CORE, BOUNDARY, NOISE }; //�����ͣ��ֱ�Ϊ���ĵ㡢�߽�㡢������

//DBSCAN�����㷨
class dbscan
{
private:
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
		std::vector<std::vector<size_t>> m_clusters; //��
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
	friend class kdtree;
	std::vector<point> m_points; //���ݵ㼯
	const double m_radius; //����뾶
	const size_t m_minpts; //�����ڵ�����ֵ
	const size_t m_dim; //���ݵ�ά��
	size_t m_cluster_num; //���մ���
	clusters m_res; //������
	kdtree* pkdt = nullptr;

private:
	//�������ݵ���룬Ҳ������������������
	double dbscan::distance(const point& lhs, const point& rhs)const
	{
		assert(lhs.m_value.size() == rhs.m_value.size());
		double temp = 0.0;
		for (int i = 0; i != lhs.m_value.size(); ++i)
			temp += (lhs.m_value[i] - rhs.m_value[i]) * (lhs.m_value[i] - rhs.m_value[i]);
		return sqrt(temp);
	}

	//���ڵ�ǰ���ĵ�ݹ����
	void coreClustering(size_t, size_t);
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
	~dbscan();
};

#endif