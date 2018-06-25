
#include "Temp2radiance.h"

#include <QtCore/QtMath>
#ifndef PLANK_PARAMETERS
#define PLANK_PARAMETERS FFFF
#define h  6.626E-34    //���ʿ˳��� J.s
#define C  2.997825E8        //���� m/s
#define pi 3.141592  
#endif
#ifndef BOLTZMAN_Parameter
#define  BOLTZMAN_Parameter FFFF
#define K 1.3806E-23   //���������� ��λ J/K
#endif // 


/*
���������ʿ˺���˵��
���룺
��1�� �¶ȣ�temp      ��λΪ     K        double��
��2�� ������v         ��λΪ   1/cm       double��

�����
(3)  ������ BvT ��λ w/m2/sr/cm         double��
*/
double plankfunction(double temp, double v)
{
	double vlt = v*100;
	double parameter1 = 2 * h*vlt*vlt*vlt*C*C;
	double parameter2 = h*vlt *C / K / temp;
	double parameter3 = qExp(parameter2) - 1;
	double BvT = 100*parameter1 / parameter3;
	return BvT;
}



/*����˵��
���룺
��1���¶ȣ�temperature ��λΪK double��
��2��ģ����ʼ���Σ�wavelengthstart         ��λΪ��m       double��
��3��ģ���ն˲��Σ�wavelengthend           ��λΪ��m       double��
��4��ģ�Ⲩ���׷ֱ��ʣ�spetrumresolution   ��λΪcm^-1    double��
 (5)�����ʣ�       emissitivity           �޵�λ         double��

 �����
 ��1����������      radiance          ��λΪ w/sr/m2  double��
*/
double Temp2radiance(double temperature, double wavelengthstart, 
	double wavelengthend, double spetrumresolution, 
	double emissitivity)
{
	double v_start = 10000/ wavelengthstart;   //  ��λΪ1/cm
	double v_end = 10000 / wavelengthend;       // ��λΪ1/cm
	double v = v_end;
	double BvT = 0;
	double radiance = 0;
	while (v <=v_start)
	{
		BvT = plankfunction(temperature, v);
		radiance = radiance + BvT;
		v = v + spetrumresolution;
	}

	radiance = radiance*spetrumresolution*emissitivity;
	return radiance;

}



