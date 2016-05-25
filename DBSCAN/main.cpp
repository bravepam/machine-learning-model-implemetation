#include"dbscan.h"
#include<iostream>
#include<fstream>

using namespace std;

void findBestKAndRadius()
{
	system("python drawRawPoints.py");//����ԭʼ��ֲ�ͼ
	for (int k = 1;; ++k)
	{
		cout << "k = " << k << endl;
		dbscan db(k, 2);
		db.readPoints(".//cluster_data//four_clusters.txt");
		db.computeKdists();
		db.drawKdists();
	}
}

void clustering()
{
	ifstream infile("k-and-radius.txt");
	double r;
	size_t k;
	while (infile >> r >> k)
	{
		dbscan db(r, k, 2);
		db.readPoints(".//cluster_data//four_clusters.txt");
		db.clustering();
		cout << "radius = " << r << " k = " << k << " clusters'number: " << db.getClustersNum() << endl;
		db.writePoints("points-info-after-clustering.txt");
		db.drawClusters();
	}
}

int main()
{
	//findBestKAndRadius();//�����иú����ҳ�һЩ��Ѵ���İ뾶��kֵ
	clustering();//�����иú����۲��ڲ�ͬ����Ѵ����µľ���Ч��
	return 0;
}