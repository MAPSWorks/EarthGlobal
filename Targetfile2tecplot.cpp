#include <QtCore/QFile>
#include <QtCore/QTextStream>

#include   <fstream>
#include   <iostream> 
#include   <stdexcept> 
#include   <fstream>
#include <iomanip>
#include "Targetfile2tecplot.h"
using namespace std;


/*����������tempAverage
           ���룺tmpInDir CString��ʽ  .tmp�ļ�·��
		         tmpFileName CString��ʽ .tmp�ļ���
				   triInDir  CString��ʽ  .tri�ļ�·��
				    triFileName  CString��ʽ  .tri�ļ���
		  �����outDir CString��ʽ   .plt�ļ����·��
		        outFileName  CString��ʽ .plt�ļ���
		      
		        

*/
void tempAverage(const QString& tmpFileName,
	const QString& triFileName,
	const QString& outFileName)
{
	int temp,n,e,num;
	double **NT, **NPP;

	QFile inTmpfile(tmpFileName);
	if (!inTmpfile.open(QIODevice::ReadOnly | QIODevice::Text))
		return;

	QFile inTrifile(triFileName);
	if (!inTrifile.open(QIODevice::ReadOnly | QIODevice::Text))
		return;

	QFile file(outFileName);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		return;

	QTextStream ofile(&file);

	QByteArray lineTri = inTrifile.readAll();
	QTextStream textLineTri(lineTri);

	QByteArray lineTmp = inTmpfile.readAll();
	QTextStream textLineTmp(lineTmp);

	textLineTri >> e;				        //��Ԫ��Ŀ
	textLineTri >> n;				        //����Ŀ
	NT = new double *[e];
	for (size_t i = 0; i < e; i++)
	{
		NT[i] = new double[4];//��->���㡢�¶�
	}

	NPP = new double *[n];
	for (size_t i = 0; i < n; i++)
	{
		NPP[i] = new double[5];
	}

	for (int i = 0; i < n; i++)
	{
		NPP[i][0] = 0;
		NPP[i][1] = 0;
		NPP[i][2] = 0;
		NPP[i][3] = 0;
		NPP[i][4] = 0;
	}

	for (int i = 0; i < e; i++)
	{
		NT[i][0] = 0;
		NT[i][1] = 0;
		NT[i][2] = 0;
		NT[i][3] = 0;
	}

	textLineTmp >> temp;					//������Ԫ��Ӧ�¶�
	for (int i = 0;i < e; i++)
	{
		textLineTmp >> temp;
		textLineTmp >> NT[temp - 1][3];
	}

	textLineTri >> num;

	for (int i = 0; i < 3 * num; i++)
	{
		textLineTri >> temp;
	}
	for (int i = 0; i < n; i++)		//�������XYZ
	{
		textLineTri >> temp;
		textLineTri >> NPP[temp - 1][0];
		textLineTri >> NPP[temp - 1][1];
		textLineTri >> NPP[temp - 1][2];
	}
	for (int i = 0; i < e; i++)		//�ۼ�ÿ������¶�
	{
		textLineTri >> temp;
		textLineTri >> NT[temp - 1][0];
		if (NT[temp-1][0]<=n)
		{
			NPP[(int)NT[temp-1][0]-1][3]+=NT[temp-1][3];
			NPP[(int)NT[temp-1][0]-1][4]++;
		}
		textLineTri >> NT[temp - 1][1];
		if (NT[temp-1][1]<=n)
		{
			NPP[(int)NT[temp-1][1]-1][3]+=NT[temp-1][3];
			NPP[(int)NT[temp-1][1]-1][4]++;
		}		
		textLineTri >> NT[temp - 1][2];
		if (NT[temp-1][2]<=n)
		{
			NPP[(int)NT[temp-1][2]-1][3]+=NT[temp-1][3];
			NPP[(int)NT[temp-1][2]-1][4]++;
		}
	}
// 	inTmpfile.close();
// 	inTrifile.close();

	for (int i = 0; i < n; i++)
	{
		if (NPP[i][3] == 0)				//���280
			NPP[i][3] = 288.0;
		if (NPP[i][4]> 0)
			NPP[i][3]/=NPP[i][4];		//�¶�ƽ��
	}

	QString strTemp = QString::fromLocal8Bit("Ŀ��");

	ofile << "Title=\"" + strTemp + "\"\n";
	ofile << "Variable=\"x\",\"y\",\"z\",\"T\"\n";
	ofile << "Zone N=" <<n << ",";
	ofile << "E=" <<e << ","<< "F=fepoint,ET=triangle\n";			//��ʱֱ��д��n��e
	for (int i = 0; i < n; i++)
	{
		ofile << fixed << NPP[i][0] << " ";			//�����ʽfixed
		ofile << NPP[i][1] << " ";
		ofile << NPP[i][2] << " ";
		ofile << NPP[i][3] << "\n";
	}
	for (int i = 0; i < e; i++)
	{
		ofile << /*setprecision(0) << */(int)(NT[i][0]) << " ";	//�������Ϊ�� ��ȥС������0
		ofile << NT[i][1] << " ";
		ofile << NT[i][2] << "\n";
	}
	//ofile.close();

	for (size_t i = 0; i < e; i++)
	{
		delete [] NT[i];//��->���㡢�¶�
	}
	delete[] NT;

	for (size_t i = 0; i < n; i++)
	{
	delete []	NPP[i];
	}
	delete[] NPP;

}
