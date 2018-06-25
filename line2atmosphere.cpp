//#include"stdafx.h"
#include"Ge0_xyz.h"
#include<math.h>

#ifndef PI
#define PI
#define pi 3.1415926535897932384626433832795
#endif

#ifndef GEOEARTH_PARAMETER
#define GEOEARTH_PARAMETER
//��������˹������
#define CZ_Beijing54 6378245.000
//#define DZ_Beijing54 6356863.0188
#define DZ_Beijing54 6356863.018773034
#define e2_54 0.00669342162297



//1975��������
#define CZ_XIAN80 6378140.000
#define DZ_XIAN80 6356755.2882
#define e2_1975  0.00669438499959




#define CZ_NewBeijing54 6378137.000
#define DZ_NewBeijing54 6356752.31420


#define CZ_CGCS2000 6378137.000
//#define DZ_CGCS2000 6356752.31420	
#define DZ_CGCS2000 6356752.314140358
#define e2_CGCS2000 0.00669438002290

//WGS_84ϵ����
#define CZ_WGS84 6378137.000
#define DZ_WGS84 6356752.3142
#endif


bool fequal(double value1, double value2, double precision)
{
	if (abs(value1-value2)<=precision)
	{
		return true;
	}
	else
	{
		return false;
	}
}

/*����˵�� line2atmosphereLBH
       ���룺viewpoint_L double��  ����������ӵ�ľ��� ��λΪ��
	         viewpoint_B  double��  ����������ӵ��γ�� ��λΪ��
			  viewponit_H double��  ����������ӵ�ĸ߶� ��λΪ��
	            target_L  double��  ���������Ŀ����ߵر�ľ��� ��λΪ��
                 target_B double��  ���������Ŀ����ߵر��γ�� ��λΪ��
			       target double��   ���������Ŀ����ߵر�ĸ߶� ��λΪ��
				    atmos_H double��   ��������´����㶥�ĸ߶� ��λΪ�� һ��Ϊ99000
	   �����atmos_L double*��  ��������´����㶥�ľ��� ��λΪ��
	         atmos_B double*��  ��������´����㶥��γ�� ��λΪ��

			 ״̬���� 1  Ϊ����������㶥�н���
			         -1 Ϊ����������㶥û�л��߲�ֻһ������
*/

int line2atmosphereLBH(double viewpoint_L, double viewpoint_B, double viewponit_H,
	double target_L, double target_B, double target_H, double atmos_H,
	double *atmos_L, double *atmos_B, int axis_type)
{ 
	double DZ, CZ;
	switch (axis_type)
	{

	case 1:
	{
		CZ= CZ_Beijing54;
		DZ= DZ_Beijing54;
		break;
	}

	case 2:
	{
		CZ = CZ_XIAN80;
		DZ = DZ_XIAN80;
		break;
	}

	case 3:
	{
		CZ = CZ_NewBeijing54;
		DZ = DZ_NewBeijing54;
		break;
	}



	case 4:
	{
		CZ = CZ_CGCS2000;
		DZ = DZ_CGCS2000;
		break;
	}



	case 5:
	{
		CZ = CZ_WGS84;
		DZ = DZ_WGS84;
		break;
	}

	default:
		break;
	}


	double target_x,target_y,target_z, viewpoint_x, viewpoint_y, viewpoint_z, atmos_x, atmos_y, atmos_z;
	geo2xyz(viewpoint_L, viewpoint_B, viewponit_H, &viewpoint_x, &viewpoint_y, &viewpoint_z, axis_type);
	geo2xyz(target_L, target_B, target_H, &target_x, &target_y, &target_z, axis_type);
	double Ta, Tb, Tc,t1,t2,t,H_compute;
	double m,n,p;
	m = viewpoint_x - target_x;
	n = viewpoint_y - target_y;
	p = viewpoint_z - target_z;
	Ta = (DZ + atmos_H)*(DZ + atmos_H)*(m*m+ n*n)+ (CZ + atmos_H)*(CZ + atmos_H)*p*p;
	Tb = 2 *(DZ + atmos_H)*(DZ + atmos_H) * (target_x*m+ target_y*n)+ 2*(CZ + atmos_H)*(CZ + atmos_H)*target_z*p;
	Tc = (DZ + atmos_H)*(DZ + atmos_H)*(target_x*target_x + target_y*target_y)
		+ (CZ + atmos_H)*(CZ + atmos_H)*target_z*target_z
		- (CZ + atmos_H)*(CZ + atmos_H)*(DZ + atmos_H)*(DZ + atmos_H);

	double deltaT = Tb*Tb - 4 * Ta*Tc;
	if ((deltaT>=0)&(Ta != 0))
	{
		t1 = (-Tb + sqrt(deltaT)) / 2 / Ta;
		t2 = (-Tb - sqrt(deltaT)) / 2 / Ta;

   if (t1*t1 < t2*t2)
   {
	   t = t1;
   }
   else
   {
	   t = t2;
   }

   atmos_x = target_x + (viewpoint_x - target_x)*t;
   atmos_y = target_y + (viewpoint_y - target_y)*t;
   atmos_z = target_z + (viewpoint_z - target_z)*t;
   xyz2geo(atmos_x, atmos_y, atmos_z, atmos_L, atmos_B, &H_compute, axis_type);
   return 1;
	}
	else
	{
		return - 1;
	}
	

}



/*����˵�� line2atmosphereXYZ
���룺
Nearpoint_x  double��  ���ӵ����ֱ������ϵ��x������ֵ ��λΪ��
Nearpoint_y  double��  ���ӵ����ֱ������ϵ��x������ֵ ��λΪ��
Nearpoint_z double��  ���ӵ����ֱ������ϵ��x������ֵ ��λΪ��
Farpoint_x  double��  ���ӵ����ֱ������ϵ��x������ֵ ��λΪ��
Farpoint_y double��  ���ӵ����ֱ������ϵ��x������ֵ ��λΪ��
Farpoint_z double��   ���ӵ����ֱ������ϵ��x������ֵ ��λΪ��
atmos_H  double��   ����������ĸ߶ȣ�99kmΪ�����㶥��0kmΪ�ر�

�����
atmos_L double*��  ���߶�Ӧ�Ĵ�������´����㶥�ľ��� ��λΪ��
atmos_B double*��       ���߶�Ӧ�Ĵ�������´����㶥��γ�� ��λΪ��
land_L double*��  ��������´����㶥�ľ��� ��λΪ��
land_B double*��  ��������´����㶥��γ�� ��λΪ��

״̬���� flag 0  Ϊ���������û�н���
              1 Ϊ����������㶥ֻ��һ������
			  2  ���������������������ر�û�н���
			  3 ���������������������ر�һ������
			  4 �������������������ر���������
 **ע�⣺NearpointҪ��֤����ʵ�����������ʵ�֣���������ӵ�λ��ʵ���㷨
*/

int line2atmosphereXYZ(double Nearpoint_x, double Nearpoint_y, double Nearpoint_z,
	double Farpoint_x, double Farpoint_y, double Farpoint_z,double atmos_H,
	double *atmos_L, double *atmos_B, 
	double *Land_L,double *Land_B,
	int axis_type)
{
	double DZ, CZ;
	switch (axis_type)
	{

	case 1:
	{
		CZ = CZ_Beijing54;
		DZ = DZ_Beijing54;
		break;
	}

	case 2:
	{
		CZ = CZ_XIAN80;
		DZ = DZ_XIAN80;
		break;
	}

	case 3:
	{
		CZ = CZ_NewBeijing54;
		DZ = DZ_NewBeijing54;
		break;
	}



	case 4:
	{
		CZ = CZ_CGCS2000;
		DZ = DZ_CGCS2000;
		break;
	}



	case 5:
	{
		CZ = CZ_WGS84;
		DZ = DZ_WGS84;
		break;
	}

	default:
		break;
	}

	*atmos_L = 0;
	*atmos_B = 0;
	*Land_L = 0;
	*Land_B = 0;

	double atmos_x, atmos_y, atmos_z;
	double Ta, Tb, Tc;
	double Ta_l, Tb_l, Tc_l;

	double	t1, t2, t, H_compute;
	double m, n, p;

	m = Farpoint_x - Nearpoint_x;
	n = Farpoint_y - Nearpoint_y;
	p = Farpoint_z -Nearpoint_z;

	double DZ_atmos = DZ + atmos_H;
	double CZ_atmos = CZ + atmos_H;

	double DZ_earth = DZ;
	double CZ_earth = CZ;

	Ta = DZ_atmos*DZ_atmos*(m*m + n*n) + CZ_atmos*CZ_atmos*p*p;
	Tb = 2 * DZ_atmos*DZ_atmos * (Nearpoint_x*m + Nearpoint_y*n) + 2 * CZ_atmos*CZ_atmos*Nearpoint_z*p;
	Tc = DZ_atmos*DZ_atmos*(Nearpoint_x*Nearpoint_x + Nearpoint_y*Nearpoint_y)
		+ CZ_atmos*CZ_atmos*Nearpoint_z*Nearpoint_z
		- CZ_atmos*CZ_atmos*DZ_atmos*DZ_atmos;




	Ta_l= DZ_earth*DZ_earth*(m*m + n*n) + CZ_earth*CZ_earth*p*p;
	Tb_l= 2 * DZ_earth*DZ_earth * (Nearpoint_x*m + Nearpoint_y*n) + 2 * CZ_earth*CZ_earth*Nearpoint_z*p;
	Tc_l= DZ_earth*DZ_earth*(Nearpoint_x*Nearpoint_x + Nearpoint_y*Nearpoint_y)
		+ CZ_earth*CZ_earth*Nearpoint_z*Nearpoint_z
		- CZ_earth*CZ_earth*DZ_earth*DZ_earth;



	double deltaT = Tb*Tb - 4 * Ta*Tc;
	double deltaT_l = Tb_l*Tb_l - 4 * Ta_l*Tc_l;
	double flag;


	if (deltaT<0)
	{
		flag = 0;//�����û�н���
	}

	if (deltaT == 0)
	{
		flag = 1;//�������һ������
		t = (-Tb ) / 2 / Ta;
		atmos_x = Nearpoint_x + m*t;
		atmos_y = Nearpoint_y + n*t;
		atmos_z = Nearpoint_z + p*t;
		xyz2geo(atmos_x, atmos_y, atmos_z, atmos_L, atmos_B, &H_compute, axis_type);


	}
	if (deltaT >0&&deltaT_l<0)
	{
		flag = 2;//���������������½��û�н���

		t1 = (-Tb + sqrt(deltaT)) / 2 / Ta;
		t2 = (-Tb - sqrt(deltaT)) / 2 / Ta;

		if (abs(t1)<abs(t2))
		{

			t = t1;
		}
		atmos_x = Nearpoint_x + m*t;
		atmos_y = Nearpoint_y + n*t;
		atmos_z = Nearpoint_z + p*t;

		xyz2geo(atmos_x, atmos_y, atmos_z, atmos_L, atmos_B, &H_compute, axis_type);
	}

	if ((deltaT>0) && (deltaT_l == 0))
	{
		flag = 3;//���������������½��һ������



		t1 = (-Tb + sqrt(deltaT)) / 2 / Ta;
		t2 = (-Tb - sqrt(deltaT)) / 2 / Ta;

		if (abs(t1)<abs(t2))
		{

			t = t1;
		}
		else
		{
			t = t2;
		}

		atmos_x = Nearpoint_x + m*t;
		atmos_y = Nearpoint_y + n*t;
		atmos_z = Nearpoint_z + p*t;

		xyz2geo(atmos_x, atmos_y, atmos_z, atmos_L, atmos_B, &H_compute, axis_type);


		t = (-Tb_l) / 2 / Ta_l;
		atmos_x = Nearpoint_x + m*t;
		atmos_y = Nearpoint_y + n*t;
		atmos_z = Nearpoint_z + p*t;
		xyz2geo(atmos_x, atmos_y, atmos_z, Land_L, Land_B, &H_compute, axis_type);


	}

	if ((deltaT>0) && (deltaT_l >0))
	{
		flag = 4;//���������������½����������


		t1 = (-Tb + sqrt(deltaT)) / 2 / Ta;
		t2 = (-Tb - sqrt(deltaT)) / 2 / Ta;

		if (abs(t1)<abs(t2))
		{

			t = t1;
		}
		else
		{
			t = t2;
		}

		atmos_x = Nearpoint_x + m*t;
		atmos_y = Nearpoint_y + n*t;
		atmos_z = Nearpoint_z + p*t;

		xyz2geo(atmos_x, atmos_y, atmos_z, atmos_L, atmos_B, &H_compute, axis_type);


		t1 = (-Tb_l + sqrt(deltaT_l)) / 2 / Ta_l;
		t2 = (-Tb_l- sqrt(deltaT_l)) / 2 / Ta_l;

		if (abs(t1)<abs(t2))
		{

			t = t1;
		}
		else
		{
			t = t2;
		}

		atmos_x = Nearpoint_x + m*t;
		atmos_y = Nearpoint_y + n*t;
		atmos_z = Nearpoint_z + p*t;
		xyz2geo(atmos_x, atmos_y, atmos_z, Land_L, Land_B, &H_compute, axis_type);


	}


	return flag;


}



/*����˵�� viewpointline2atmosphereXYZ
���룺
viewpoint_L double��  ����������ӵ�ľ��� ��λΪ��
viewpoint_B  double��  ����������ӵ��γ�� ��λΪ��
viewponit_H double��  ����������ӵ�ĸ߶� ��λΪ��
Farpoint_x  double��  ���ӵ����ֱ������ϵ��x������ֵ ��λΪ��
Farpoint_y double��  ���ӵ����ֱ������ϵ��x������ֵ ��λΪ��
Farpoint_z double��   ���ӵ����ֱ������ϵ��x������ֵ ��λΪ��
atmos_H  double��   ����������ĸ߶ȣ�99kmΪ�����㶥��0kmΪ�ر�

�����
atmos_L double*��  ���߶�Ӧ�Ĵ�������´����㶥�ľ��� ��λΪ��
atmos_B double*��       ���߶�Ӧ�Ĵ�������´����㶥��γ�� ��λΪ��
land_L double*��  ��������´����㶥�ľ��� ��λΪ��
land_B double*��  ��������´����㶥��γ�� ��λΪ��

״̬���� flag 0  Ϊ���������û�н���
1 Ϊ����������㶥ֻ��һ������
2  ���������������������ر�û�н���
3 ���������������������ر�һ������
4 �������������������ر���������
*/

int viewpointline2atmosphereXYZ(double viewpoint_L, double viewpoint_B, double viewponit_H,
	double Farpoint_x, double Farpoint_y, double Farpoint_z, double atmos_H,
	double *atmos_L, double *atmos_B,
	double *Land_L, double *Land_B,
	int axis_type)
{

	double viewpoint_x, viewpoint_y, viewpoint_z, atmos_x, atmos_y, atmos_z;
	geo2xyz(viewpoint_L, viewpoint_B, viewponit_H, &viewpoint_x, &viewpoint_y, &viewpoint_z, axis_type);
	int flag;
	flag = line2atmosphereXYZ(viewpoint_x, viewpoint_y, viewpoint_z,
		Farpoint_x,  Farpoint_y, Farpoint_z, atmos_H,atmos_L, atmos_B,Land_L,Land_B,axis_type);
	return flag;


}
