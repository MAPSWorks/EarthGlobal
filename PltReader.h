#ifndef PLTREADER_H
#define PLTREADER_H

#include <QtCore/QString>
#include <QtCore/QList>
#include <osg/Node>

struct JetPosInfo
{
	double dx;
	double dy;
	double dz;

	double dPitch;
	double dYaw;
	double dRoll;
};

struct TargetDataPara
{
	bool bTransToRadio;

	QString strIndexPath;
	QString strValuePath;

	QString strJetPath;

	QString strPltPath;

	QList<JetPosInfo> listJetPosInfo;

	//�Ƿ����Զ��巽ʽ����β�档��������Զ��壬�Ͱ���֮ǰ�ĸ�β�棨ÿ��β��������Ƭ״ģ�͹��ɣ������ء�
	bool bCustomJet;

	double dx;
	double dy;
	double dz;

	double dstart;
	double dend;
	double dfen;
	double dfa;

	//ŷ����
	double dAngleX;
	double dAngleY;
	double dAngleZ;

	// 	osg::Node* pJet1;
	// 	osg::Node* pJet2;
	// 	osg::Node* pJet3;
	// 	osg::Node* pJet4;

	TargetDataPara()
	{
		//pJet1 = NULL;
		//pJet2 = NULL;
		//pJet3 = NULL;
		//pJet4 = NULL;

		dx = 0.0;
		dy = 0.0;
		dz = 0.0;

		dstart = 0.0;
		dend = 0.0;
		dfen = 0.0;
		dfa = 0.0;

		dAngleX = 0.0;
		dAngleY = 0.0;
		dAngleZ = 0.0;

		bCustomJet = true;
	}
};

class PltReader
{
public:
	PltReader();
	~PltReader();

	osg::Geode* GetPltNode(const QString& strFilePath, const TargetDataPara& para);

private:
	
};

#endif // PLTREADER_H
