#include "MainWindow.h"
#include "ribbonimages.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QSettings>
#include <QClipboard>
#include <QDesktopServices>
#include <QTextEdit>
#include <QTextCodec>
#include <QLineEdit>
#include <QTextDocumentWriter>
#include <QAbstractTextDocumentLayout>
#include <QFileDialog>
#include <QFile>
//#include <QPrinter>
#include <QSpinBox>
//#include <QPrintDialog>
#include <QFontComboBox>
#include <QTextDocument>
#include <QLabel>
#include <QToolBar>
#include <QDockWidget>
#include <QListWidget>
//#include <QPrintPreviewDialog>
#include <QTextList>
#include <QColorDialog>
#include <QMessageBox>
#include <QFontDialog>
#include <QDockWidget>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QListWidget>
#include <QBuffer>
#include <QUrl>
#include <QtCore/QTextCodec>
#include <QtWidgets/QMdiSubWindow>
#include <QtWidgets/QMdiArea>
#include <QtWidgets/QHBoxLayout>

#include <QtnRibbonStyle.h>
#include <QtnRibbonCustomizePage.h>

#include <osgEarthQt/Common>

#include "aboutdialog.h"
//#include "ribbonpopularpage.h"
#include "RibbonWindow.h"
#include "mainwindow.h"
#include "ModelLayerReader.h"

#include <QtWidgets/QDateEdit>
#include <QtWidgets/QCalendarWidget>

#include <osgEarthFeatures/GeometryUtils>
#include <osgEarthFeatures/OgrUtils>
#include <osgEarthQt/GuiActions>

#include "TargetDataReader.h"
#include "AirDataReader.h"
#include "osgEarth/DataParaManager.h"
#include "RenderModeDlg.h"
#include "AirDataDlg.h"
#include "Target2PltDlg.h"
#include "Targetfile2tecplot.h"
#include "OutputDlg.h"
#include "AnimationSettingsDlg.h"
#include "SysUtility.h"
#include "DirectManipulatorDlg.h"

#include "RangeFeatureGenerator.h"
#include "targetdatadlg.h"
#include "osgEarth/ColorTransform.h"

#include "AnimationController.h"
#include "VideoMaker.h"
#include "osg/Matrixd"
#include "Document.h"
#include "TransDataToRadioDlg.h"
#include "ProgressNumberWidget.h"

#include "gdal_priv.h"
#include "Temp2radiance.h"

#include <QtWidgets/QProgressDialog>
#include "SetStartFrameDlg.h"
#include "JetThresholdDlg.h"
#include "PltTempDlg.h"
#include "PltReader.h"

extern osg::ref_ptr<osg::Group> s_annoGroup;
extern Qtitan::RibbonStatusBar* g_pStatusBar;

QAction* g_pActionDetectTool = NULL;

bool g_bIsFixedAngldMode = false;

extern osgEarth::Features::Feature* GetAirFeature(double dTransparent);
QProgressDialog* g_ProgressDialog = NULL;
QProgressDialog* g_ProgressDialogTxt = NULL;
QWidget* g_pMainWindow = nullptr;

extern int g_nStartFrameIndex;
extern double g_dJetValue;
extern int g_OutPutFrameCount;

using namespace Qtitan;
QString g_strLastDir = "/home";

MainWindow::MainWindow(osgEarth::QtGui::DataManager* manager, osgEarth::MapNode* mapNode, osg::Group* annotationRoot)
: _manager(manager), _mapNode(mapNode), _annoRoot(annotationRoot), _layerAdded(false), _terrainProfileDock(0L), _viewerWidget(0L)
{
	setWindowTitle(tr("Earth Global"));
	g_pMainWindow = this;

	g_pStatusBar = new Qtitan::RibbonStatusBar;
	setStatusBar(g_pStatusBar);

	createMenuFile();
	//createQuickAccessBar();
	createRibbon();

 	ribbonBar()->setFrameThemeEnabled();
	//setWindowFlags(/*Qt::FramelessWindowHint*/ /*Qt::WindowSystemMenuHint |*/windowFlags() & ~Qt::WindowMinimizeButtonHint);
 	m_stateWindow = windowState();

	QString strFilePath = QApplication::applicationFilePath();
	strFilePath = QFileInfo(strFilePath).absolutePath();

	osgEarth::Drivers::BingOptions bing;
	bing.key() = QString(strFilePath + "/data/1").toUtf8().data();
	_manager->map()->addImageLayer(new osgEarth::ImageLayer("TileImage", bing));

	m_pCameraPosDlg = NULL;

	m_pSynTimer = new QTimer(this);
	m_pSynTimer->start(200);

	m_pOutputWidget = NULL;

	g_ProgressDialogTxt = new QProgressDialog(this);
	g_ProgressDialogTxt->setValue(100);

//	resize(1600, 900);
}

MainWindow::~MainWindow()
{

}

void MainWindow::InitCameraDlg()
{
	m_pCameraPosDlg = new CameraPostionDlg(this);
	QObject::connect(m_pSynTimer, SIGNAL(timeout()), m_pCameraPosDlg, SLOT(slotRefreshInfo()));
}

void MainWindow::changeEvent(QEvent* e)
{
	//ʹ��ribbon��������С���������ڻ�������ػ���Ϣ��resize���Իָ�������
	if (e->type() == QEvent::WindowStateChange)
	{
		static int s_nTag = 0;
 		if (this->windowState() & Qt::WindowMinimized)
 		{
			s_nTag = 1;
 		}
		else
		{
			if (s_nTag == 1)
			{
				Qt::WindowStates state = this->windowState();
				if (state == 0 || state == 2)
				{
					extern QMdiSubWindow* g_pMdiSubWindow;
					extern QMdiArea* g_pMdiArea;

					g_pMdiArea->resize(QSize(g_pMdiArea->geometry().width(), g_pMdiArea->geometry().height() - 1));
					g_pMdiArea->resize(QSize(g_pMdiArea->geometry().width(), g_pMdiArea->geometry().height() + 1));

// 					g_pMdiSubWindow->resize(QSize(g_pMdiSubWindow->geometry().width(), g_pMdiSubWindow->geometry().height() - 1));
// 					g_pMdiSubWindow->resize(QSize(g_pMdiSubWindow->geometry().width(), g_pMdiSubWindow->geometry().height() + 1));
// 
// 					_viewerWidget->resize(QSize(_viewerWidget->geometry().width(), _viewerWidget->geometry().height() - 1));
// 					_viewerWidget->resize(QSize(_viewerWidget->geometry().width(), _viewerWidget->geometry().height() + 1));

					m_pDockWidgetEagleEye->resize(QSize(m_pDockWidgetEagleEye->geometry().width(), m_pDockWidgetEagleEye->geometry().height() - 1));
					m_pDockWidgetEagleEye->resize(QSize(m_pDockWidgetEagleEye->geometry().width(), m_pDockWidgetEagleEye->geometry().height() + 1));

					s_nTag = 0;
				}
			}
		}
	}

	return RibbonWindow::changeEvent(e);
}

// void MainWindow::keyPressEvent(QKeyEvent* event)
// {
// 	Qtitan::RibbonMainWindow::keyPressEvent(event);
// 	if (event->key() == Qt::Key_Escape && m_actionFullScreen->isChecked())
// 		fullScreen(false);
// }

void MainWindow::slotEscPressed()
{
	if(m_actionFullScreen->isChecked())
		fullScreen(false);
}

void MainWindow::OpenProject()
{
	Document* pDocument = Document::Instance();
	pDocument->OpenProjectXML();
}

void MainWindow::NewProject()
{
	Document* pDocument = Document::Instance();
	pDocument->CreateNewProject();
}

void MainWindow::SaveAsProject()
{
	Document* pDocument = Document::Instance();
	pDocument->SaveAsProject();
}

void MainWindow::setViewerWidget(osgEarth::QtGui::ViewerWidget* viewerWidget)
{
	_viewerWidget = viewerWidget;

	_views.clear();
	_viewerWidget->getViews(_views);

	_annotationToolbar->setActiveViews(_views);
}

void MainWindow::setTerrainProfileWidget(osgEarth::QtGui::TerrainProfileWidget* widget)
{
	if (!_terrainProfileDock)
	{
		_terrainProfileDock = new QDockWidget;
		_terrainProfileDock->setAllowedAreas(Qt::BottomDockWidgetArea);
		_terrainProfileDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
		addDockWidget(Qt::BottomDockWidgetArea, _terrainProfileDock);
		_terrainProfileDock->setVisible(_terrainProfileAction->isChecked());
	}

	_terrainProfileDock->setWidget(widget);
}

void MainWindow::loadCameraPath()
{
	QString fileName = QFileDialog::getOpenFileName(NULL, QString::fromLocal8Bit("̽�����켣�ļ�"),
		g_strLastDir,
		"Path (*.txt)");

	g_strLastDir = QFileInfo(fileName).absolutePath();

	AnimationController::Instance()->LoadCameraPath(fileName);
}

void MainWindow::loadTargetPath()
{
	QString fileName = QFileDialog::getOpenFileName(NULL, QString::fromLocal8Bit("Ŀ��켣�ļ�"),
		g_strLastDir,
		"Path (*.txt)");

	g_strLastDir = QFileInfo(fileName).absolutePath();

	AnimationController::Instance()->LoadTargetPath(fileName);
}

void MainWindow::startAnmiation()
{
	AnimationController* pAnimationController = AnimationController::Instance();
	pAnimationController->StartAnimation();
}

void MainWindow::StopAnimation()
{
	AnimationController* pAnimationController = AnimationController::Instance();
	pAnimationController->StopAnimation();
}

void MainWindow::slotSavePicAsTxt()
{
	AnimationController* pAnimationController = AnimationController::Instance();

	Document* pDocument = Document::Instance();
	if (pDocument->GetSavePath().isNull())
	{
		AnimationSettingsDlg dlg;
		dlg.exec();
	}

	if (pDocument->GetSavePath().isNull())
		return;

	QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
		"/home",
		QFileDialog::ShowDirsOnly
		| QFileDialog::DontResolveSymlinks);

	if (dir.isEmpty())
		return;

	QString strDir = pDocument->GetSavePath();
	int nCurrentImageIndex = 1;

	if (g_ProgressDialog == nullptr)
	{
		g_ProgressDialog = new QProgressDialog(g_pMainWindow);
		g_ProgressDialog->setAttribute(Qt::WA_DeleteOnClose);
		g_ProgressDialog->setWindowFlags(g_ProgressDialog->windowFlags() & ~Qt::WindowCloseButtonHint);
		g_ProgressDialog->setWindowFlags(g_ProgressDialog->windowFlags() & ~Qt::WindowContextHelpButtonHint);
		g_ProgressDialog->setModal(true);
		g_ProgressDialog->setCancelButtonText("Cancel");
	}

	CameraPath* pCameraPath = AnimationController::Instance()->GetCameraPath();
	const osg::AnimationPath::TimeControlPointMap& mapCP = pCameraPath->getTimeControlPointMap();
	g_OutPutFrameCount = mapCP.size();

	g_ProgressDialog->setWindowTitle(QString::fromLocal8Bit("����txt"));
	g_ProgressDialog->setRange(0, g_OutPutFrameCount - 1);
	g_ProgressDialog->setValue(1);
	int nCount = -1;
	
	while (1)
	{
		QString tifFullPath = strDir + "/" + QString::number(nCurrentImageIndex) + ".tif";
		QString txtFullPath = dir + "/" + QString::number(nCurrentImageIndex) + ".txt";

		g_ProgressDialog->setValue(nCurrentImageIndex);
		g_ProgressDialog->setLabelText(QString::fromLocal8Bit("ת����") + QString::number(nCurrentImageIndex) + QString::fromLocal8Bit("��"));

		if (g_ProgressDialog->wasCanceled())
		{
			g_ProgressDialog->reset();
			break;
		}

		nCurrentImageIndex++;

		GDALDataset *poSrcDS =
			(GDALDataset *)GDALOpen(tifFullPath.toLocal8Bit().data(), GA_ReadOnly);

		if (poSrcDS == NULL)
			break;

		int nWid = poSrcDS->GetRasterXSize();
		int nHei = poSrcDS->GetRasterYSize();

		int nBand = 1;
		double* pdBuffer = new double[nWid * nHei];
		poSrcDS->RasterIO(GF_Read, 0, 0, nWid, nHei, pdBuffer, nWid, nHei, GDT_Float64, 1, &nBand, 0, 0, 0);
		QFile file(txtFullPath);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
			return;

		QTextStream out(&file);

		for (int j = 0; j < nHei; j ++)
		{
			for (int i = 0; i < nWid; i ++)
			{
				double dValue = pdBuffer[j * nWid + i];
				out << dValue << ',';
			}

			out << "\n";
		}

		GDALClose((GDALDatasetH)poSrcDS);
		delete pdBuffer;
	}

	g_ProgressDialog->reset();
}

void MainWindow::OutputAnimation()
{
	AnimationController* pAnimationController = AnimationController::Instance();

	Document* pDocument = Document::Instance();
	if(pDocument->GetSavePath().isNull())
	{
		AnimationSettingsDlg dlg;
		dlg.exec();
	}

	if(pDocument->GetSavePath().isNull())
		return;

// 	SetStartFrameDlg dlg;
// 	dlg.exec();
//	g_nStartFrameIndex = dlg.ui.spinBox->value(
	if (g_ProgressDialog == nullptr)
	{
		g_ProgressDialog = new QProgressDialog(this);
		g_ProgressDialog->setAttribute(Qt::WA_DeleteOnClose);
		g_ProgressDialog->setWindowFlags(g_ProgressDialog->windowFlags() & ~Qt::WindowCloseButtonHint);
		g_ProgressDialog->setWindowFlags(g_ProgressDialog->windowFlags() & ~Qt::WindowContextHelpButtonHint);
		g_ProgressDialog->setModal(true);
		g_ProgressDialog->setCancelButtonText("Cancel");
	}
		
	g_ProgressDialog->setWindowTitle(QString::fromLocal8Bit("���"));

	DataParaManager::Instance()->SetFixedAngleMode(true);
	pAnimationController->OutputAnimation();
}

void MainWindow::MakeAnimation()
{
	Document* pDocument = Document::Instance();
	if(pDocument->GetSavePath().isNull())
	{
		AnimationSettingsDlg dlg;
		dlg.exec();
	}

	if(pDocument->GetSavePath().isNull())
		return;

	VideoMaker maker;
	maker.MakeVideo(pDocument->GetSavePath());
}

void MainWindow::AnimationSettings()
{
	OutputDlg dlg;
	dlg.exec();
}

void LoadModisRadioLayer(const QString& fileName)
{
	extern osgEarth::QtGui::DataManager* g_DataManager;
	QFileInfo fileInfo(fileName);

	QString string = fileName;
	QTextCodec *codec = QTextCodec::codecForName("GB18030");
	QByteArray encodedString = codec->fromUnicode(string);

	osgEarth::Drivers::GDALOptions layerOpt;
	layerOpt.url() = osgEarth::URI(encodedString.data());
	layerOpt.isRadioModis() = true;
	osg::ref_ptr<osgEarth::ImageLayer> ptrImageLayer = new osgEarth::ImageLayer(osgEarth::ImageLayerOptions(fileInfo.fileName().toUtf8().data(), layerOpt));

	if (ptrImageLayer == NULL)
		return;

	ptrImageLayer->SetGuid(SysUtility::GetGUIDString());
	g_DataManager->map()->addImageLayer(ptrImageLayer.get());
}

void MainWindow::addModisRadioLayer()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
		g_strLastDir,
		tr("Images (*.tif *.TIFF *.img)"));

	if (fileName.isEmpty())
		return;

	g_strLastDir = QFileInfo(fileName).absolutePath();

	QFileInfo fileInfo(fileName);

	QString string = fileName;
	QTextCodec *codec = QTextCodec::codecForName("GB18030");
	QByteArray encodedString = codec->fromUnicode(string);

	osgEarth::Drivers::GDALOptions layerOpt;
	layerOpt.url() = osgEarth::URI(encodedString.data());
	layerOpt.isRadioModis() = true;
	osg::ref_ptr<osgEarth::ImageLayer> ptrImageLayer = new osgEarth::ImageLayer(osgEarth::ImageLayerOptions(fileInfo.fileName().toUtf8().data(), layerOpt));

	if(ptrImageLayer == NULL)
		return;

	ptrImageLayer->SetGuid(SysUtility::GetGUIDString());
	_manager->map()->addImageLayer(ptrImageLayer.get());
}

void MainWindow::addModisTemperatureLayer()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
													g_strLastDir,
													tr("Images (*.tif *.TIFF *.img)"));

	if(fileName.isEmpty())
		return;

	g_strLastDir = QFileInfo(fileName).absolutePath();

	QFileInfo fileInfo(fileName);

	QString string = fileName;
	QTextCodec *codec = QTextCodec::codecForName("GB18030");
	QByteArray encodedString = codec->fromUnicode(string);

	osgEarth::Drivers::GDALOptions layerOpt;
	layerOpt.url() = osgEarth::URI(encodedString.data());
	layerOpt.isTemperatureModis() = true;
	osg::ref_ptr<osgEarth::ImageLayer> ptrImageLayer = new osgEarth::ImageLayer(osgEarth::ImageLayerOptions(fileInfo.fileName().toUtf8().data(), layerOpt));

	if(ptrImageLayer == NULL)
		return;

	ptrImageLayer->SetGuid(SysUtility::GetGUIDString());
	_manager->map()->addImageLayer(ptrImageLayer.get());
}


//osg::ref_ptr<osg::Array> g_ArrayAirColorArray;
void MainWindow::addAirData()
{
	int nOldWid, nOldHei;
	DataParaManager::Instance()->GetSize(nOldWid, nOldHei);

	double dResX = 360.0 / nOldWid;
	double dResY = 180.0 / nOldHei;

	AirDataDlg dlg;
	dlg.ui.lineEdit_Lon->setText(QString::number(dResX));
	dlg.ui.lineEdit_Lat->setText(QString::number(dResY));
	if (!dlg.exec())
		return;

	QString strFileName = dlg.ui.label_filePath->text();

	if (strFileName.isEmpty())
	{
		strFileName = QString::fromLocal8Bit("C:\\OSG_OSGEarth\\���ݸ�ʽ\\����.txt");
	}

	dResX = dlg.ui.lineEdit_Lon->text().toDouble();
	dResY = dlg.ui.lineEdit_Lat->text().toDouble();

	AnimationController::Instance()->LoadAirData(strFileName, dResX, dResY);
}

static char * vertexShader = {
	"varying vec4 color;\n"
	"void main(void ){\n"
	"color = gl_Vertex;\n"
	"gl_Position = gl_ModelViewProjectionMatrix*gl_Vertex;\n"
	"}\n"
};
static char * fragShader = {
	"varying vec4 color;\n"
	"void main(void){\n"
	"   gl_FragColor = clamp(color,0.0,1.0);\n"
	"}\n"
};

//void MainWindow::setTransparent(osgEarth::Annotation::FeatureNode* node, double dTrans)
//{
	//osgEarth::Annotation::Feature* pFeature = node->getFeature();
	//osg::Vec4Array* pArray = (osg::Vec4Array*)pFeature->getColorArray();

	//osg::Vec4Array* pArrayNew = new osg::Vec4Array;

	//for (int i = 0; i < pArray->size(); i ++)
	//{
	//	pArrayNew->push_back(osg::Vec4((*pArray)[i].r(), (*pArray)[i].g(), (*pArray)[i].b(), dTrans));
	//}

	//osgEarth::Annotation::Feature* pFeatureNew = new osgEarth::Annotation::Feature(*pFeature);
	//pFeatureNew->setColorArray(pArrayNew);

	//node->setFeature(pFeatureNew);
//}

//void MainWindow::UnloadAirData()
//{
//// 	DataParaManager::Instance()->ResetValue();
//// 	_manager->MapNode()->getTerrainEngine()->dirtyTerrain();
//	osg::Node* pNode = s_annoGroup->getChild(0);
//	osgEarth::Annotation::FeatureNode* pFeatureNode = dynamic_cast<osgEarth::Annotation::FeatureNode*>(pNode);
//
//	setTransparent(pFeatureNode, 0.5);
//}

void MainWindow::slotRefreshTargetData()
{
	osgEarth::QtGui::AnnotationVector out_annotations;
	_manager->getAnnotations(out_annotations);

	osgEarth::QtGui::AnnotationVector annotationsDelete;
	osgEarth::QtGui::AnnotationVector annotationsAdd;

	for (int i = 0; i < out_annotations.size(); i ++)
	{
		osgEarth::Annotation::LocalGeometryNode* pLocalGeoNode = dynamic_cast<osgEarth::Annotation::LocalGeometryNode*>(out_annotations[i].get());
		if (pLocalGeoNode == nullptr)
			continue;

		osgEarth::Annotation::AnnotationData* pData = pLocalGeoNode->getOrCreateAnnotationData();
		std::string strGuid = pData->getName();
		std::map<std::string, TargetDataPara>::iterator itr = TargetDataReader::s_mapPara.find(strGuid);
		if (itr != TargetDataReader::s_mapPara.end())
		{
			TargetDataPara& para = itr->second;
			TargetDataReader targetReader;

			osg::Node* pNode = targetReader.GetGeode(para);

			osgEarth::Annotation::Style circleStyle;
			circleStyle.getOrCreate<osgEarth::Annotation::PolygonSymbol>()->fill()->color() = osgEarth::Annotation::Color(osgEarth::Annotation::Color::Red, 0.5);

			osgEarth::Annotation::LocalGeometryNode* pLocalGeoNodeNew = new osgEarth::Annotation::LocalGeometryNode(_manager->MapNode(), pNode, circleStyle);
			pLocalGeoNodeNew->setAnnotationData(pData);
			pLocalGeoNodeNew->setPosition(pLocalGeoNode->getPosition());

			//pLocalGeoNode->setNode(pNode);
			annotationsDelete.push_back(pLocalGeoNode);
			annotationsAdd.push_back(pLocalGeoNodeNew);
		}
	}

	for (int i = 0; i < annotationsDelete.size(); i ++)
	{
		_manager->removeAnnotation(annotationsDelete[i].get(), s_annoGroup);
	}

	for (int i = 0; i < annotationsAdd.size(); i++)
	{
		_manager->addAnnotation(annotationsAdd[i].get(), s_annoGroup);
	}
}

void MainWindow::slotRefreshAirData()
{
	osgEarth::QtGui::AnnotationVector out_annotations;
	_manager->getAnnotations(out_annotations);

	osgEarth::Annotation::FeatureNode* pAirFeatureNode = NULL;
	for (int i = 0; i < out_annotations.size(); i++)
	{
		pAirFeatureNode = dynamic_cast<osgEarth::Annotation::FeatureNode*>(out_annotations[i].get());
		if (pAirFeatureNode != nullptr)
			break;
	}

	if (pAirFeatureNode == nullptr)
		return;

	osgEarth::Features::Feature* pFeatureNew = GetAirFeature(1.0);
	pAirFeatureNode->setFeature(pFeatureNew);
}

//void MainWindow::addTargetDataJet()
//{
//	QString strNewJetPath;
//
//	QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
//		"/home",
//		tr("Tecplot file (*.dat *.plt)"));
//
//	if (fileName.isEmpty())
//		return;
//
//	strNewJetPath = fileName;
//
//	osgEarth::QtGui::AnnotationVector out_annotations;
//	_manager->getAnnotations(out_annotations);
//
//	osgEarth::Annotation::LocalGeometryNode* pLastLocalGeoNode = NULL;
//
//	for (int i = 0; i < out_annotations.size(); i++)
//	{
//		osgEarth::Annotation::LocalGeometryNode* pLocalGeoNode = dynamic_cast<osgEarth::Annotation::LocalGeometryNode*>(out_annotations[i].get());
//		if (pLocalGeoNode == nullptr)
//			continue;
//
//		pLastLocalGeoNode = pLocalGeoNode;
//	}
//
//	if (!pLastLocalGeoNode)
//	{
//		return;
//	}
//
//	osgEarth::Annotation::AnnotationData* pData = pLastLocalGeoNode->getOrCreateAnnotationData();
//	std::string strGuid = pData->getName();
//	std::map<std::string, TargetDataPara>::iterator itr = TargetDataReader::s_mapPara.find(strGuid);
//	if (itr != TargetDataReader::s_mapPara.end())
//	{
//		itr->second.strJetPath = strNewJetPath;
//		TargetDataPara para = itr->second;
//		TargetDataReader targetReader;
//
//		osg::Node* pNode = targetReader.GetGeode(para);
//		pLastLocalGeoNode->setNode(pNode);
//	}
//}

void MainWindow::Target2plt()
{
	Target2PltDlg target2pltDlg;
	if (!target2pltDlg.exec())
		return;

	if (target2pltDlg.ui.lineEdit_Tri->text().isEmpty()
		|| target2pltDlg.ui.lineEdit_Tmp->text().isEmpty()
		|| target2pltDlg.ui.lineEdit_OutPlt->text().isEmpty())
	{
		return;
	}

	tempAverage(target2pltDlg.ui.lineEdit_Tmp->text(), 
		target2pltDlg.ui.lineEdit_Tri->text(), target2pltDlg.ui.lineEdit_OutPlt->text());
}

void MainWindow::slotTest()
{
	AnimationController* pAnimationController = AnimationController::Instance();
	pAnimationController->StopAnimation();

	extern osgViewer::Viewer* g_viewerMain;
	osgEarth::QtGui::AnnotationVector out_annotations;
	_manager->getAnnotations(out_annotations);

	osgEarth::Annotation::AnnotationNode* pCurrentAnnotationNode = NULL;
	osgEarth::Annotation::LocalGeometryNode* pLocalGeoNode = dynamic_cast<osgEarth::Annotation::LocalGeometryNode*>(out_annotations[0].get());
	if(!pLocalGeoNode)
		return;

	double dTargetX = pLocalGeoNode->getPosition().x();
	double dTargetY = pLocalGeoNode->getPosition().y();
	double dTargetZ = pLocalGeoNode->getPosition().z();

	const osgEarth::SpatialReference* pSrs = _manager->MapNode()->getMapSRS();
	osgEarth::GeoPoint geoTarget(pSrs, dTargetX, dTargetY, dTargetZ);

	osg::Vec3d targetPos;
	geoTarget.toWorld(targetPos);

	osg::Camera* camera = g_viewerMain->getCamera();
	osg::Vec3d cameraPosInverse1 = camera->getInverseViewMatrix().getTrans();

	osg::Matrixd rotation_matrix = osg::Matrixd::lookAt(cameraPosInverse1, targetPos, osg::Vec3d(1.0, 1.0, 1.0));
	osg::AnimationPath::ControlPoint cp(-rotation_matrix.getTrans(), rotation_matrix.getRotate().inverse());

	osg::Matrixd _matrix = osg::Matrixd::translate(cp.getPosition()) * osg::Matrixd::rotate(cp.getRotation());

// 	PathEarthManipulate* pPathEarthManipulate = new PathEarthManipulate(NULL);
// 	g_viewerMain->setCameraManipulator(pPathEarthManipulate);
	g_viewerMain->getCameraManipulator()->setByMatrix(_matrix);

	int x = 0;
}

void MainWindow::LoadPlt()
{
	PltTempDlg pltTempDlg;
	if (!pltTempDlg.exec())
		return;

	double dstart, dend, dfen, dfa;
	pltTempDlg.GetPara(dstart, dend, dfen, dfa);

	QString strPath = pltTempDlg.ui.label_Path->text();

// 	PltReader pltReader;
// 	pltReader.GetPltNode(strPath, dstart, dend, dfen, dfa);
}

void MainWindow::addTargetData()
{
// 	_manager->MapNode()->getTerrainEngine()->dirtyTerrain();
// 	return;

	TargetDataDlg dlgTarget;
	if (!dlgTarget.exec())
		return;

	TargetDataPara para;
	para.bTransToRadio = dlgTarget.ui.groupBox_ToRadio->isChecked();

	para.strIndexPath = dlgTarget.m_strTRI;
	para.strValuePath = dlgTarget.m_strTmp;
	para.strJetPath = dlgTarget.ui.label_LoadJet->text();
	para.strPltPath = dlgTarget.ui.label_Plt->text();

	QTableWidget* pTableWidget = dlgTarget.ui.tableWidget_JetPara;
	int nJetCount = pTableWidget->rowCount();

	for (int i = 0; i < nJetCount; i ++)
	{
		JetPosInfo info;
		QTableWidgetItem* pItem = pTableWidget->item(i, 0);
		info.dx = pItem->text().toDouble();

		pItem = pTableWidget->item(i, 1);
		info.dy = pItem->text().toDouble();

		pItem = pTableWidget->item(i, 2);
		info.dz = pItem->text().toDouble();

		pItem = pTableWidget->item(i, 3);
		info.dPitch = pItem->text().toDouble();

		pItem = pTableWidget->item(i, 4);
		info.dYaw = pItem->text().toDouble();

		pItem = pTableWidget->item(i, 5);
		info.dRoll = pItem->text().toDouble();

		para.listJetPosInfo.push_back(info);
	}

	para.dx = dlgTarget.GetX();
	para.dy = dlgTarget.GetY();
	para.dz = dlgTarget.GetZ();

	para.dAngleX = 0.0;
	para.dAngleY = 0.0;
	para.dAngleZ = 0.0;

	//para.pJet1 = NULL;
	//para.pJet2 = NULL;
	//para.pJet3 = NULL;
	//para.pJet4 = NULL;

	para.bCustomJet = dlgTarget.GetAuxPara(para.dstart, para.dend, para.dfen, para.dfa);
	AnimationController::Instance()->LoadTargetData(para);
}

void MainWindow::addModelLayer()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
		g_strLastDir,
		tr("Model (*.layer)"));

	if (fileName.isEmpty())
		return;

	g_strLastDir = QFileInfo(fileName).absolutePath();

	QFileInfo fileInfo(fileName);

	QString string = fileName;
	QTextCodec *codec = QTextCodec::codecForName("GB18030");
	QByteArray encodedString = codec->fromUnicode(string);

	ModelLayerReader modelReader;
	osgEarth::ModelLayer* pModelLayer = modelReader.CreateModelLayer(encodedString.data());
	//osgEarth::ModelLayer* pModelLayer = modelReader.CreateModelLayer("C:\\OSG_OSGEarth\\gwaldron-osgearth-2.7-0-g25ce0e1\\tests\\feature_models.earth");
	_manager->map()->addModelLayer(pModelLayer);
}

void MainWindow::addAnnotation()
{
	osgEarth::Annotation::PlaceNode* annotation = new osgEarth::Annotation::PlaceNode(
		_mapNode.get(),
		osgEarth::GeoPoint(_mapNode->getMapSRS(), -74.0, 40.714),
		osgDB::readImageFile("../data/placemark32.png"),
		"New York");

	osgEarth::Annotation::AnnotationData* annoData = new osgEarth::Annotation::AnnotationData();
	annoData->setName("New York");
	annoData->setViewpoint(osgEarth::Viewpoint("New Tork", -74, 40.714, 0, 0.0, -90.0, 1e5));
	annotation->setAnnotationData(annoData);

	annotation->installDecoration("selected", new osgEarth::Annotation::ScaleDecoration(2.0f));

	_manager->addAnnotation(annotation, _annoRoot.get());

	_addAnnotationAction->setDisabled(true);
}

void MainWindow::slotFixedAngleMode(bool checked)
{
	osgEarth::QtGui::AnnotationVector out_annotations;
	_manager->getAnnotations(out_annotations);

	osgEarth::Annotation::FeatureNode* pAirFeatureNode = NULL;
	for (int i = 0; i < out_annotations.size(); i++)
	{
		pAirFeatureNode = dynamic_cast<osgEarth::Annotation::FeatureNode*>(out_annotations[i].get());
		if (pAirFeatureNode != nullptr)
			break;
	}

	DataParaManager::Instance()->SetFixedAngleMode(checked);
	if (!pAirFeatureNode)
		return;
	
	_manager->doAction(this, new osgEarth::QtGui::ToggleNodeAction(pAirFeatureNode, !checked));
	
	extern osgViewer::Viewer* g_viewerMain;
	osgEarth::Util::EarthManipulator* mainMani = dynamic_cast<osgEarth::Util::EarthManipulator*>(g_viewerMain->getCameraManipulator());
	osgEarth::Viewpoint viewPoint = mainMani->getViewpoint();

	double dHeading = viewPoint.heading().value().getValue();
	double dpitch = viewPoint.pitch().value().getValue();

	viewPoint.focalPoint()->set(osgEarth::SpatialReference::get("wgs84"), AirDataReader::m_dy, AirDataReader::m_dx, AirDataReader::m_dz, osgEarth::ALTMODE_ABSOLUTE);
	viewPoint.setHeading(dHeading);
	viewPoint.setPitch(dpitch);
	viewPoint.setRange(0.0);

	mainMani->setViewpoint(viewPoint);

	double dLon = viewPoint.focalPoint().value().x();
	double dLat = viewPoint.focalPoint().value().y();
	double dH = viewPoint.focalPoint().value().z() + viewPoint.range().value().getValue();

	DataParaManager::Instance()->SetCameraPosInfo(dLon, dLat, dH);
//	g_bIsFixedAngldMode = checked;
	slotRefreshTargetData();
	_manager->MapNode()->getTerrainEngine()->dirtyTerrain();
}

int CPL_STDCALL OnProgress(double dfComplete, const char *pszMessage, void *pProgressArg)
{
	QProgressDialog* pdlg = (QProgressDialog*)pProgressArg;
	pdlg->setValue(dfComplete * 100.0);

	if(pdlg->wasCanceled())
	{
		return 0;
	}

	return 1;
}

void MainWindow::slotDataTranslateToRadio()
{
	TransDataToRadioDlg dlg;
	if (!dlg.exec())
		return;

	QProgressDialog process(this);
	process.setWindowTitle(QString::fromLocal8Bit("����"));
	process.setWindowFlags(process.windowFlags() & ~Qt::WindowCloseButtonHint);
	process.setWindowFlags(process.windowFlags() & ~Qt::WindowContextHelpButtonHint);

	//process.setRange(0, 100);
	process.setModal(true);
	process.setCancelButtonText("Cancel");

	QString strTempFile = dlg.ui.lineEdit_TempFile->text();
	QString strTransRatioFile = dlg.ui.lineEdit_TransRatio->text();
	QString strOutputFile = dlg.ui.lineEdit_Output->text();

	if (strTempFile.isEmpty() || strTransRatioFile.isEmpty() || strOutputFile.isEmpty())
	{
		QMessageBox box;
		box.setText(QString::fromLocal8Bit("��������Ч·��"));
		box.exec();
		return;
	}

	GDALAllRegister();

	GDALDataset* pTempDataset = (GDALDataset*)GDALOpen(strTempFile.toLocal8Bit().data(), GA_ReadOnly);
	GDALDataset* pTransRatioDataset = (GDALDataset*)GDALOpen(strTransRatioFile.toLocal8Bit().data(), GA_ReadOnly);

	int nTempWid = pTempDataset->GetRasterXSize();
	int nTempHei = pTempDataset->GetRasterYSize();

	int nTransWid = pTransRatioDataset->GetRasterXSize();
	int nTransHei = pTransRatioDataset->GetRasterYSize();

	if(nTempWid != nTransWid || nTempHei != nTransHei)
	{
		QMessageBox box;
		box.setText(QString::fromLocal8Bit("�¶��ļ��ͷ������ļ���߲�һ�£�Ŀǰ�㷨û��֧�֡�"));
		box.exec();
		return;
	}

	double dStartWave = dlg.ui.doubleSpinBox_StartWave->text().toDouble();
	double dEndWave = dlg.ui.doubleSpinBox_EndWave->text().toDouble();

	int nWid = nTempWid;
	int nHei = nTempHei;

	GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
	char** papszMetadata = poDriver->GetMetadata();
	GDALDataset* pOutputDataset = poDriver->Create(strOutputFile.toLocal8Bit().data(), nWid, nHei, 1, GDT_UInt16, papszMetadata);

	if(!pOutputDataset)
		return;

	double dProgerss = 0.0;

	int nSrcBlockSizeX = 256;
	int nSrcBlockSizeY = 256;
	//pTempDataset->GetRasterBand(1)->GetBlockSize(&nSrcBlockSizeX, &nSrcBlockSizeY);

	int nCols = (nWid % nSrcBlockSizeX) == 0 ? nWid / nSrcBlockSizeX : nWid / nSrcBlockSizeX + 1;
	int nRows = (nHei % nSrcBlockSizeY) == 0 ? nHei / nSrcBlockSizeY : nHei / nSrcBlockSizeY + 1;

	ushort* pTempBuffer = new ushort[nSrcBlockSizeX * nSrcBlockSizeY];
	ushort* pTransRatioBuffer = new ushort[nSrcBlockSizeX * nSrcBlockSizeY];
	ushort* pOutputBuffer = new ushort[nSrcBlockSizeX * nSrcBlockSizeY];

	process.setRange(0, nRows * nCols);

	for(int j = 0; j < nRows; j++)
	{
		for(int i = 0; i < nCols; i++)
		{
			int nBlockWid = nSrcBlockSizeX;
			int nBlockHei = nSrcBlockSizeY;

			int nX = i * nSrcBlockSizeX;
			int nY = j * nSrcBlockSizeY;

			dProgerss = (j * nCols + i) / (double)(nRows * nCols);
			//dlg.ui.label_Progress->setText(QString::fromLocal8Bit("���� ") + QString::number(dProgerss * 100.0) + "%");
			//process.setValue(dProgerss * 100.0);
			process.setValue(j * nCols + i);

			if (process.wasCanceled())
			{
				process.reset();
				GDALClose(pTempDataset);
				GDALClose(pTransRatioDataset);
				GDALClose(pOutputDataset);

				return;
			}

			if((nX + nBlockWid) > nWid)
			{
				nBlockWid = nWid - nX;
			}

			if((nY + nBlockHei) > nHei)
			{
				nBlockHei = nHei - nY;
			}

			int nBandCount = 1;
			int nPixelSpace = (GDALGetDataTypeSize(GDT_UInt16) / 8)*nBandCount;
			int nLineSpace = nPixelSpace * nBlockWid;
			int nBandSpace = (GDALGetDataTypeSize(GDT_UInt16) / 8);

			int nBandMap = 1;

			pTempDataset->RasterIO(GF_Read, nX, nY, nBlockWid, nBlockHei, pTempBuffer, nBlockWid, nBlockHei
								   , GDT_UInt16, nBandCount, &nBandMap, nPixelSpace, nLineSpace, nBandSpace);

			pTransRatioDataset->RasterIO(GF_Read, nX, nY, nBlockWid, nBlockHei, pTransRatioBuffer, nBlockWid, nBlockHei
										 , GDT_UInt16, nBandCount, &nBandMap, nPixelSpace, nLineSpace, nBandSpace);

			for(int y = 0; y < nBlockHei; y++)
			{
				for(int x = 0; x < nBlockWid; x++)
				{
					ushort temp = pTempBuffer[y * nBlockWid + x];
					ushort transRatio = pTransRatioBuffer[y * nBlockWid + x];

					double dTemp = temp * 0.02;
					double dTransRatio = transRatio * 0.002 + 0.49;

					double dValue = Temp2radiance(dTemp, dStartWave, dEndWave, 1.0, dTransRatio);
					pOutputBuffer[y * nBlockWid + x] = dValue * 1000.0;
				}
			}

			pOutputDataset->RasterIO(GF_Write, nX, nY, nBlockWid, nBlockHei, pOutputBuffer, nBlockWid, nBlockHei
									 , GDT_UInt16, nBandCount, &nBandMap, nPixelSpace, nLineSpace, nBandSpace);
		}
	}

	process.reset();

	const char* pProject = pTempDataset->GetProjectionRef();
	double dGeoTransform[6];
	pTempDataset->GetGeoTransform(dGeoTransform);

	pOutputDataset->SetProjection(pProject);
	pOutputDataset->SetGeoTransform(dGeoTransform);

	//����������

	do
	{
		double fSampleRate = 0.5;
		int nMinPyramidWdith = 512;
		int nMinPyramidHeigh = 512;

		int nRate = (int)(1 / fSampleRate + 0.5);
		int nXRate = pOutputDataset->GetRasterXSize() / static_cast<int>(nMinPyramidWdith);
		int nYRate = pOutputDataset->GetRasterYSize() / static_cast<int>(nMinPyramidHeigh);
		int nRatio = (nXRate < nYRate) ? nYRate : nXRate;
		if(nRatio < 1)
			break;

		//�����������
		int nLevel = int(1 + log((double)nRatio) / log((double)nRate));
		if(nLevel < 1)
			break;

		int* pBandList = new int[nLevel];
		for(int i = 0; i < nLevel; ++i)
		{
			pBandList[i] = int(pow(double(nRate), double(i + 1)));
		}
		CPLErr error = CE_Failure;

		QProgressDialog process1(this);
		process1.setWindowTitle(QString::fromLocal8Bit("����������"));
		process1.setWindowFlags(process1.windowFlags() & ~Qt::WindowCloseButtonHint);
		process1.setWindowFlags(process1.windowFlags() & ~Qt::WindowContextHelpButtonHint);

		process1.setRange(0, 100);
		process1.setModal(true);
		process1.setCancelButtonText("Cancel");

		//������û�����ý��ȵĻص�����

		GDALClose(pOutputDataset);
		pOutputDataset = (GDALDataset*)GDALOpen(strOutputFile.toLocal8Bit().data(), GA_ReadOnly);

		error = pOutputDataset->BuildOverviews("NEAREST", nLevel, pBandList, 0, NULL, OnProgress, &process1);
		const char* pszLastErrMsg = CPLGetLastErrorMsg();

		delete[] pBandList;
		pBandList = NULL;

		process1.reset();

		//���û�д����ɹ���Ҫɾ���������ļ�
		if(CE_Failure == error)
		{
			// ����������������´�ͼ��
			GDALClose(pOutputDataset);

			// ɾ���������ļ�
			bool bFlag = false;

			bFlag = QFile::remove(CPLResetExtension(strOutputFile.toUtf8().data(), "rrd"));
			bFlag = QFile::remove(CPLResetExtension(strOutputFile.toUtf8().data(), "aux"));

		}
	} while(0);

	GDALClose(pTempDataset);
	GDALClose(pTransRatioDataset);
	GDALClose(pOutputDataset);

	QMessageBox box;
	box.setText(QString::fromLocal8Bit("�������"));
	box.exec();

}

void MainWindow::slotSetRenderMode()
{
	RenderModeDlg dlg;
	if(!dlg.exec())
		return;

	if (dlg.ui.radioButton_Gray->isChecked())
	{
		DataParaManager::Instance()->SetRGBMode(false);
	}
	else
	{
		DataParaManager::Instance()->SetRGBMode(true);
	}

	DataParaManager::Instance()->SetRadioMax(dlg.ui.lineEdit_RadioMax->text().toDouble());
	DataParaManager::Instance()->SetRadioMin(dlg.ui.lineEdit_RadioMin->text().toDouble());

	DataParaManager::Instance()->SetTemperatureMax(dlg.ui.lineEdit_TemperatureMax->text().toDouble());
	DataParaManager::Instance()->SetTemperatureMin(dlg.ui.lineEdit_TemperatureMin->text().toDouble());

	slotRefreshTargetData();
	slotRefreshAirData();
	_manager->MapNode()->getTerrainEngine()->dirtyTerrain();
}

void MainWindow::terrainProfileToggled(bool checked)
{
	if (_terrainProfileDock)
		_terrainProfileDock->setVisible(checked);
}

void MainWindow::createGroupDocumentViews(Qtitan::RibbonGroup* group)
{
	QIcon fullScreenIcons;
	fullScreenIcons.addPixmap(QPixmap(m_strResPath + "_fullscreen.png"));
	fullScreenIcons.addPixmap(QPixmap(m_strResPath + "_smallfullscreen.png"));
	m_actionFullScreen = group->addAction(fullScreenIcons, tr("Full Screen Reading"), Qt::ToolButtonTextUnderIcon);

	m_actionFullScreen->setCheckable(true);
	m_actionFullScreen->setChecked(false);
	connect(m_actionFullScreen, SIGNAL(triggered(bool)), this, SLOT(fullScreen(bool)));
}

void MainWindow::fullScreen(bool checked)
{
	if (checked)
	{
		m_stateWindow = windowState();
		m_actionFullScreen->setChecked(true);
		ribbonBar()->setVisible(false);

		m_pDockWidgetAnno->setVisible(false);
		m_pDockWidgetImageLayers->setVisible(false);
		m_pDockWidgetLayers->setVisible(false);
		m_pDockWidgetViewPoints->setVisible(false);

		setWindowState(Qt::WindowFullScreen);
	}
	else
	{
		ribbonBar()->setVisible(true);

		m_pDockWidgetAnno->setVisible(true);
		m_pDockWidgetImageLayers->setVisible(true);
		m_pDockWidgetLayers->setVisible(true);
		m_pDockWidgetViewPoints->setVisible(true);

		setWindowState(m_stateWindow);
		m_actionFullScreen->setChecked(false);
	}
}

void MainWindow::createMenuFile()
{
	//QIcon iconLogo;
	//iconLogo.addPixmap(QPixmap(m_strResPath + "largezoom.png"));
	//iconLogo.addPixmap(QPixmap(":/shared/res/qtitanlogo32x32.png"));
	if (QAction* actionFile = ribbonBar()->addSystemButton(/*iconLogo,*/ QString::fromLocal8Bit("��Ŀ����")))
	{
		ribbonBar()->getSystemButton()->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

		m_actionOpenFile = new QAction(QIcon(m_strResPath + "_openfile.png"), QString::fromLocal8Bit("������Ŀ"), this);
		//m_actionOpenFile->setShortcut(QKeySequence::Open);
		m_actionOpenFile->setToolTip(tr("Open"));
		m_actionOpenFile->setStatusTip(tr("Open an existing document"));
		connect(m_actionOpenFile, SIGNAL(triggered()), this, SLOT(OpenProject()));

		m_actionFileNew = new QAction(QIcon(m_strResPath + "_newfile.png"), QString::fromLocal8Bit("�½���Ŀ"), this);
		//m_actionFileNew->setShortcut(QKeySequence::Open);
		//m_actionFileNew->setToolTip(tr("Open"));
		//m_actionFileNew->setStatusTip(tr("Open an existing document"));
		connect(m_actionFileNew, SIGNAL(triggered()), this, SLOT(NewProject()));

		m_actionSaveFile = new QAction(QIcon(m_strResPath + "_savefile.png"), QString::fromLocal8Bit("��Ŀ���Ϊ"), this);
		//m_actionSaveFile->setShortcut(QKeySequence::Save);
		//m_actionSaveFile->setToolTip(tr("Save"));
		//m_actionSaveFile->setStatusTip(tr("Save the active document"));
		//m_actionSaveFile->setEnabled(false);
		connect(m_actionSaveFile, SIGNAL(triggered()), this, SLOT(SaveAsProject()));

		actionFile->setToolTip(tr("Click here to see everything you can do with your document"));
		Qtitan::RibbonSystemPopupBar* popupBar = qobject_cast<Qtitan::RibbonSystemPopupBar*>(actionFile->menu());

		popupBar->addAction(m_actionFileNew);
		popupBar->addAction(m_actionOpenFile);
		popupBar->addAction(m_actionSaveFile);

		//QAction* actionSaveAsFile = popupBar->addAction(QIcon(m_strResPath + "_saveas.png"), QString::fromLocal8Bit("�������Ϊ"));
		//actionSaveAsFile->setPriority(QAction::LowPriority);
		//actionSaveAsFile->setToolTip(tr("Save As"));
		//actionSaveAsFile->setStatusTip(tr("Save the active document with a new name"));
		//connect(actionSaveAsFile, SIGNAL(triggered()), this, SLOT(fileSaveAs()));

		popupBar->addSeparator();
		QAction* actionClose = popupBar->addAction(QIcon(m_strResPath + "_close"), QString::fromLocal8Bit("�˳�"));
		//actionClose->setShortcut(tr("Ctrl+C"));
		actionClose->setStatusTip(tr("Exit"));
		actionClose->setEnabled(true);

		connect(actionClose, SIGNAL(triggered()), this, SLOT(close()));

	}
}

void MainWindow::createGroupAnnotation(Qtitan::RibbonPage* page)
{
	if (Qtitan::RibbonGroup* groupClipboard = page->addGroup(/*::qtnIcon(Image_Clipboard),*/ QString::fromLocal8Bit("��ע���")))
	{
		groupClipboard->setOptionButtonVisible(false);

		QAction* pActionPoint = groupClipboard->addAction(QIcon(m_strResPath + "_point.png"), QString::fromLocal8Bit("��"), Qt::ToolButtonTextUnderIcon);
		RibbonControl* control = groupClipboard->controlByAction(pActionPoint);
		control->sizeDefinition(RibbonControlSizeDefinition::GroupLarge)->setImageSize(RibbonControlSizeDefinition::ImageLarge);
		control->sizeDefinition(RibbonControlSizeDefinition::GroupMedium)->setImageSize(RibbonControlSizeDefinition::ImageLarge);
		control->sizeDefinition(RibbonControlSizeDefinition::GroupSmall)->setImageSize(RibbonControlSizeDefinition::ImageLarge);

		QAction* pActionLine = groupClipboard->addAction(QIcon(m_strResPath + "_line.png"), QString::fromLocal8Bit("��"), Qt::ToolButtonTextUnderIcon);
		control = groupClipboard->controlByAction(pActionLine);
		control->sizeDefinition(RibbonControlSizeDefinition::GroupLarge)->setImageSize(RibbonControlSizeDefinition::ImageLarge);
		control->sizeDefinition(RibbonControlSizeDefinition::GroupMedium)->setImageSize(RibbonControlSizeDefinition::ImageLarge);
		control->sizeDefinition(RibbonControlSizeDefinition::GroupSmall)->setImageSize(RibbonControlSizeDefinition::ImageLarge);

		QAction* pActionPolygon = groupClipboard->addAction(QIcon(m_strResPath + "_polygon.png"), QString::fromLocal8Bit("�����"), Qt::ToolButtonTextUnderIcon);
		control = groupClipboard->controlByAction(pActionPolygon);
		control->sizeDefinition(RibbonControlSizeDefinition::GroupLarge)->setImageSize(RibbonControlSizeDefinition::ImageLarge);
		control->sizeDefinition(RibbonControlSizeDefinition::GroupMedium)->setImageSize(RibbonControlSizeDefinition::ImageLarge);
		control->sizeDefinition(RibbonControlSizeDefinition::GroupSmall)->setImageSize(RibbonControlSizeDefinition::ImageLarge);

		QAction* pActionEllipse = groupClipboard->addAction(QIcon(m_strResPath + "_ellipse.png"), QString::fromLocal8Bit("��Բ"), Qt::ToolButtonTextUnderIcon);
		control = groupClipboard->controlByAction(pActionEllipse);
		control->sizeDefinition(RibbonControlSizeDefinition::GroupLarge)->setImageSize(RibbonControlSizeDefinition::ImageLarge);
		control->sizeDefinition(RibbonControlSizeDefinition::GroupMedium)->setImageSize(RibbonControlSizeDefinition::ImageLarge);
		control->sizeDefinition(RibbonControlSizeDefinition::GroupSmall)->setImageSize(RibbonControlSizeDefinition::ImageLarge);

		m_vecAnnoAction.push_back(pActionPoint);
		m_vecAnnoAction.push_back(pActionLine);
		m_vecAnnoAction.push_back(pActionPolygon);
		m_vecAnnoAction.push_back(pActionEllipse);
		_annotationToolbar = new osgEarth::QtGui::AnnotationToolbar(m_vecAnnoAction, _annoRoot, _mapNode, _manager);
		//addToolBar(_annotationToolbar);

		for (int i = 0; i < m_vecAnnoAction.size(); i ++)
		{
			QObject::connect(m_vecAnnoAction[i], SIGNAL(triggered()), this, SLOT(slotAnnoAdding()));
		}

		QObject::connect(_annotationToolbar, SIGNAL(sigAddFinished()), this, SLOT(slotAnnoAddFinished()));
	}
}

void MainWindow::createGroupSky(Qtitan::RibbonPage* page)
{
	if (Qtitan::RibbonGroup* groupSky = page->addGroup(/*::qtnIcon(Image_Clipboard),*/ QString::fromLocal8Bit("�ǿ�")))
	{
		QDateTimeEdit* pDataTimeEdit = new QDateTimeEdit;
		pDataTimeEdit->setDateTime(QDateTime::currentDateTime());
		pDataTimeEdit->setDisplayFormat("yyyy/MM/dd  hh : mm : ss");

		pDataTimeEdit->setCalendarPopup(true);

		QAction* pActionTime = groupSky->addWidget(QIcon(), QString::fromLocal8Bit("�趨ʱ��"), pDataTimeEdit);
		RibbonControl* control = groupSky->controlByAction(pActionTime);
		control->sizeDefinition(RibbonControlSizeDefinition::GroupLarge)->setImageSize(RibbonControlSizeDefinition::ImageLarge);
		control->sizeDefinition(RibbonControlSizeDefinition::GroupMedium)->setImageSize(RibbonControlSizeDefinition::ImageLarge);
		control->sizeDefinition(RibbonControlSizeDefinition::GroupSmall)->setImageSize(RibbonControlSizeDefinition::ImageLarge);
 
 		QObject::connect(pDataTimeEdit, SIGNAL(dateTimeChanged(const QDateTime &)), this, SLOT(slotDataTimeChanged(const QDateTime&)));
// 		//groupSky->addWidget(new QLabel("�趨ʱ��"));
	}
}

void MainWindow::slotSetCameraPos()
{
	m_pCameraPosDlg->show();
}

extern osgEarth::Util::SkyNode* s_sky;

void MainWindow::slotSliderHourChanged(int nValue)
{
	osgEarth::DateTime dataTime = s_sky->getDateTime();
	s_sky->setDateTime(osgEarth::DateTime(dataTime.year(), dataTime.month(), dataTime.day(), nValue));
}

void MainWindow::slotDataTimeChanged(const QDateTime& dataTime)
{
	double dHour = dataTime.time().hour() + dataTime.time().minute() / 60.0 + dataTime.time().second() / 3600.0;
	s_sky->setDateTime(osgEarth::DateTime(dataTime.date().year(), dataTime.date().month(), dataTime.date().day(), dHour));
}

void MainWindow::slotAnnoAdding()
{
	for (int i = 0; i < m_vecAnnoAction.size(); i ++)
	{
		m_vecAnnoAction[i]->setEnabled(false);
	}
}

void MainWindow::slotAnnoAddFinished()
{
	for (int i = 0; i < m_vecAnnoAction.size(); i++)
	{
		m_vecAnnoAction[i]->setEnabled(true);
	}
}

QDialog* g_pDetectToolDlg = NULL;
QTextEdit* g_pTextEdit = NULL;
void MainWindow::slotDetectTool()
{
	if (g_pDetectToolDlg == NULL)
	{
		g_pDetectToolDlg = new QDialog(this);
		QHBoxLayout* pHLayout = new QHBoxLayout;
		g_pTextEdit = new QTextEdit;
		pHLayout->addWidget(g_pTextEdit);
		g_pDetectToolDlg->setLayout(pHLayout);
		g_pDetectToolDlg->setWindowTitle(QString::fromLocal8Bit("̽�빤��"));
	}

	//if (bCheck)
	{
		g_pDetectToolDlg->show();
	}
}

void MainWindow::slotRefresh()
{
	_manager->MapNode()->getTerrainEngine()->dirtyTerrain();
}

void MainWindow::slotSetJetthreshold()
{
	JetThresholdDlg dlg(g_dJetValue);
	if (dlg.exec())
	{
		QString strValue = dlg.ui.lineEdit->text();
		g_dJetValue = strValue.toDouble();
	}
}

void MainWindow::createRibbon()
{
	m_pageHome = ribbonBar()->addPage(QString::fromLocal8Bit("�����"));
	if (m_pageHome)
	{
		createGroupAnimation(m_pageHome);
		createGroupRadioData(m_pageHome);
		createGroupTemperatureData(m_pageHome);
		createGroupTool(m_pageHome);
		createGroupAirData(m_pageHome);
		createGroupTarget(m_pageHome);
		createGroupRenderPara(m_pageHome);
		createGroupDataManager(m_pageHome);
		//createGroupFont(m_pageHome);
		createGroupAnnotation(m_pageHome);
		createGroupDataTranslate(m_pageHome);
		createGroupSky(m_pageHome);
	}

	m_pageLayout = ribbonBar()->addPage(QString::fromLocal8Bit("���ڲ���"));
	if (m_pageLayout)
	{
		createGroupDocumentViews(m_pageLayout->addGroup(QString::fromLocal8Bit("���ڲ���")));
	}
}

void MainWindow::slotDirectManipulator()
{
	DirectManipulatorDlg* dlg = new DirectManipulatorDlg(this);
	dlg->show();
}

void MainWindow::createGroupAnimation(Qtitan::RibbonPage* page)
{
	Qtitan::RibbonGroup* groupAnimation = page->addGroup(/*::qtnIcon(Image_Clipboard),*/ QString::fromLocal8Bit("����"));

	QAction* pActionDirectManipulator = groupAnimation->addAction(QIcon(m_strResPath + "_loadimg.png"), QString::fromLocal8Bit("λ������"), Qt::ToolButtonTextUnderIcon);
	pActionDirectManipulator->setToolTip(QString::fromLocal8Bit("λ������"));
	connect(pActionDirectManipulator, SIGNAL(triggered()), this, SLOT(slotDirectManipulator()));

	QAction* pActionloadCameraPathFile = groupAnimation->addAction(QIcon(m_strResPath + "_loadimg.png"), QString::fromLocal8Bit("�����ӵ�켣"), Qt::ToolButtonTextUnderIcon);
	pActionloadCameraPathFile->setToolTip(QString::fromLocal8Bit("�����ӵ�켣"));
	connect(pActionloadCameraPathFile, SIGNAL(triggered()), this, SLOT(loadCameraPath()));
	pActionloadCameraPathFile->setEnabled(false);

	QAction* pActionLoadTargetPathFile = groupAnimation->addAction(QIcon(m_strResPath + "_loadimg.png"), QString::fromLocal8Bit("����Ŀ��켣"), Qt::ToolButtonTextUnderIcon);
	pActionLoadTargetPathFile->setToolTip(QString::fromLocal8Bit("����Ŀ��켣"));
	connect(pActionLoadTargetPathFile, SIGNAL(triggered()), this, SLOT(loadTargetPath()));
	pActionLoadTargetPathFile->setEnabled(false);

	QAction* pActionPlay = groupAnimation->addAction(QIcon(m_strResPath + "_loadimg.png"), QString::fromLocal8Bit("����"), Qt::ToolButtonTextUnderIcon);
	pActionPlay->setToolTip(QString::fromLocal8Bit("����"));
	connect(pActionPlay, SIGNAL(triggered()), this, SLOT(startAnmiation()));
	pActionPlay->setEnabled(false);

	QAction* pActionStopPlay = groupAnimation->addAction(QIcon(m_strResPath + "_loadimg.png"), QString::fromLocal8Bit("ֹͣ"), Qt::ToolButtonTextUnderIcon);
	pActionStopPlay->setToolTip(QString::fromLocal8Bit("ֹͣ"));
	connect(pActionStopPlay, SIGNAL(triggered()), this, SLOT(StopAnimation()));
	pActionStopPlay->setEnabled(false);

	QAction* pActionOutput = groupAnimation->addAction(QIcon(m_strResPath + "_loadimg.png"), QString::fromLocal8Bit("����֡ͼƬ"), Qt::ToolButtonTextUnderIcon);
	pActionOutput->setToolTip(QString::fromLocal8Bit("����֡ͼƬ"));
	connect(pActionOutput, SIGNAL(triggered()), this, SLOT(OutputAnimation()));

	QAction* pActionSaveTxt = groupAnimation->addAction(QIcon(m_strResPath + "_loadimg.png"), QString::fromLocal8Bit("ͼƬ���Ϊtxt"), Qt::ToolButtonTextUnderIcon);
	pActionSaveTxt->setToolTip(QString::fromLocal8Bit("ͼƬ���Ϊtxt"));
	connect(pActionSaveTxt, SIGNAL(triggered()), this, SLOT(slotSavePicAsTxt()));

	QAction* pActionMakeVideo = groupAnimation->addAction(QIcon(m_strResPath + "_loadimg.png"), QString::fromLocal8Bit("������Ƶ"), Qt::ToolButtonTextUnderIcon);
	pActionMakeVideo->setToolTip(QString::fromLocal8Bit("������Ƶ"));
	connect(pActionMakeVideo, SIGNAL(triggered()), this, SLOT(MakeAnimation()));

	QAction* pActionSettings = groupAnimation->addAction(QIcon(m_strResPath + "_loadimg.png"), QString::fromLocal8Bit("��������"), Qt::ToolButtonTextUnderIcon);
	pActionSettings->setToolTip(QString::fromLocal8Bit("��������"));
	connect(pActionSettings, SIGNAL(triggered()), this, SLOT(AnimationSettings()));
}

void MainWindow::createGroupRadioData(Qtitan::RibbonPage* page)
{
	Qtitan::RibbonGroup* groupRadioData = page->addGroup(/*::qtnIcon(Image_Clipboard),*/ QString::fromLocal8Bit("��������"));

	m_pActionAddModisRadioLayer = groupRadioData->addAction(QIcon(m_strResPath + "_loadimg.png"), QString::fromLocal8Bit("����modis��������"), Qt::ToolButtonTextUnderIcon);
	m_pActionAddModisRadioLayer->setToolTip(QString::fromLocal8Bit("����modis��������"));
	connect(m_pActionAddModisRadioLayer, SIGNAL(triggered()), this, SLOT(addModisRadioLayer()));
	m_pActionAddModisRadioLayer->setDisabled(!_manager.valid());

	RibbonControl* control = groupRadioData->controlByAction(m_pActionAddModisRadioLayer);
	control->sizeDefinition(RibbonControlSizeDefinition::GroupLarge)->setImageSize(RibbonControlSizeDefinition::ImageLarge);
	control->sizeDefinition(RibbonControlSizeDefinition::GroupMedium)->setImageSize(RibbonControlSizeDefinition::ImageLarge);
	control->sizeDefinition(RibbonControlSizeDefinition::GroupSmall)->setImageSize(RibbonControlSizeDefinition::ImageLarge);
}

void MainWindow::createGroupTemperatureData(Qtitan::RibbonPage* page)
{
	Qtitan::RibbonGroup* groupTemperatureData = page->addGroup(/*::qtnIcon(Image_Clipboard),*/ QString::fromLocal8Bit("�¶�����"));

	QAction* pAddModisTemperatureLayer = groupTemperatureData->addAction(QIcon(m_strResPath + "_loadimg.png"), QString::fromLocal8Bit("����modis�¶�����"), Qt::ToolButtonTextUnderIcon);
	pAddModisTemperatureLayer->setToolTip(QString::fromLocal8Bit("����modis�¶�����"));
	connect(pAddModisTemperatureLayer, SIGNAL(triggered()), this, SLOT(addModisTemperatureLayer()));
	pAddModisTemperatureLayer->setDisabled(!_manager.valid());
}

void MainWindow::createGroupTool(Qtitan::RibbonPage* page)
{
	Qtitan::RibbonGroup* groupTool = page->addGroup(QString::fromLocal8Bit("����"));
	g_pActionDetectTool = groupTool->addAction(QIcon(m_strResPath + "_loadimg.png"), QString::fromLocal8Bit("̽��"), Qt::ToolButtonTextUnderIcon);
	//g_pActionDetectTool->setCheckable(true);

	connect(g_pActionDetectTool, SIGNAL(triggered()), this, SLOT(slotDetectTool()));
}

void MainWindow::createGroupTarget(Qtitan::RibbonPage* page)
{
	Qtitan::RibbonGroup* groupTarget = page->addGroup(QString::fromLocal8Bit("Ŀ��"));

	QAction* pAction = groupTarget->addAction(QIcon(m_strResPath + "_loadimg.png"), QString::fromLocal8Bit("β����ֵ"), Qt::ToolButtonTextUnderIcon);
	pAction->setToolTip(QString::fromLocal8Bit("β����ֵ"));
	connect(pAction, SIGNAL(triggered()), this, SLOT(slotSetJetthreshold()));
}

void MainWindow::createGroupAirData(Qtitan::RibbonPage* page)
{
	Qtitan::RibbonGroup* groupAirData = page->addGroup(/*::qtnIcon(Image_Clipboard),*/ QString::fromLocal8Bit("��������"));

	m_pActionAddAirData = groupAirData->addAction(QIcon(m_strResPath + "_loadimg.png"), QString::fromLocal8Bit("���ش�������"), Qt::ToolButtonTextUnderIcon);
	m_pActionAddAirData->setToolTip(QString::fromLocal8Bit("���ش�������"));
	connect(m_pActionAddAirData, SIGNAL(triggered()), this, SLOT(addAirData()));
	m_pActionAddAirData->setDisabled(!_manager.valid());

	//QAction* pActionTest = groupAirData->addAction(QIcon(m_strResPath + "_loadimg.png"), QString::fromLocal8Bit("����"), Qt::ToolButtonTextUnderIcon);
	//connect(pActionTest, SIGNAL(triggered()), this, SLOT(slotTest()));

	RibbonControl* control = groupAirData->controlByAction(m_pActionAddAirData);
	control->sizeDefinition(RibbonControlSizeDefinition::GroupLarge)->setImageSize(RibbonControlSizeDefinition::ImageLarge);
	control->sizeDefinition(RibbonControlSizeDefinition::GroupMedium)->setImageSize(RibbonControlSizeDefinition::ImageLarge);
	control->sizeDefinition(RibbonControlSizeDefinition::GroupSmall)->setImageSize(RibbonControlSizeDefinition::ImageLarge);

// 	QAction* pUnloadAirData = groupAirData->addAction(QIcon(m_strResPath + "_loadimg.png"), QString::fromLocal8Bit("ж�ش�������"), Qt::ToolButtonTextUnderIcon);
// 	pUnloadAirData->setToolTip(QString::fromLocal8Bit("ж�ش�������"));
// 	connect(pUnloadAirData, SIGNAL(triggered()), this, SLOT(UnloadAirData()));
// 	pUnloadAirData->setDisabled(!_manager.valid()
}

void MainWindow::createGroupRenderPara(Qtitan::RibbonPage* page)
{
	Qtitan::RibbonGroup* groupAirData = page->addGroup(QString::fromLocal8Bit("��Ⱦ����"));

	QAction* pActionRenderMode = groupAirData->addAction(QIcon(m_strResPath + "_loadimg.png"), QString::fromLocal8Bit("��ɫģʽ"), Qt::ToolButtonTextUnderIcon);
	connect(pActionRenderMode, SIGNAL(triggered()), this, SLOT(slotSetRenderMode()));

	QAction* pActionRefresh = groupAirData->addAction(QIcon(m_strResPath + "_loadimg.png"), QString::fromLocal8Bit("ˢ��"), Qt::ToolButtonTextUnderIcon);
	connect(pActionRefresh, SIGNAL(triggered()), this, SLOT(slotRefresh()));

	QAction* pActionFixedAngleMode = groupAirData->addAction(QIcon(m_strResPath + "_loadimg.png"), QString::fromLocal8Bit("����ģʽ"), Qt::ToolButtonTextUnderIcon);
	connect(pActionFixedAngleMode, SIGNAL(triggered(bool)), this, SLOT(slotFixedAngleMode(bool)));
	pActionFixedAngleMode->setCheckable(true);
}

void MainWindow::createGroupDataTranslate(Qtitan::RibbonPage* page)
{
	Qtitan::RibbonGroup* groupDataTranslate = page->addGroup(QString::fromLocal8Bit("����ת��"));

	QAction* pActionDataTranslateToRadio = groupDataTranslate->addAction(QIcon(m_strResPath + "_loadimg.png"), QString::fromLocal8Bit("תΪ����"), Qt::ToolButtonTextUnderIcon);
	connect(pActionDataTranslateToRadio, SIGNAL(triggered()), this, SLOT(slotDataTranslateToRadio()));
}

void MainWindow::createGroupDataManager(Qtitan::RibbonPage* page)
{
	if (Qtitan::RibbonGroup* groupClipboard = page->addGroup(/*::qtnIcon(Image_Clipboard),*/ QString::fromLocal8Bit("���ݹ���")))
	{
		{
			QAction* pActionCameraPos = groupClipboard->addAction(QIcon(m_strResPath + "_cameraPos.png"), QString::fromLocal8Bit("����������"), Qt::ToolButtonTextUnderIcon);
			pActionCameraPos->setToolTip(QString::fromLocal8Bit("����������"));

			RibbonControl* control1 = groupClipboard->controlByAction(pActionCameraPos);
			control1->sizeDefinition(RibbonControlSizeDefinition::GroupLarge)->setImageSize(RibbonControlSizeDefinition::ImageLarge);
			control1->sizeDefinition(RibbonControlSizeDefinition::GroupMedium)->setImageSize(RibbonControlSizeDefinition::ImageLarge);
			control1->sizeDefinition(RibbonControlSizeDefinition::GroupSmall)->setImageSize(RibbonControlSizeDefinition::ImageLarge);

			connect(pActionCameraPos, SIGNAL(triggered()), this, SLOT(slotSetCameraPos()));
		}

		groupClipboard->setOptionButtonVisible(false);
// 		QAction* act = groupClipboard->optionButtonAction();
// 		act->setText(tr("Clipboard"));
// 		act->setIcon(QIcon(m_strResPath + "clipboardToolTip.png"));
// 		act->setToolTip(tr("Show the Office clipboard Task Pane"));
// 		act->setStatusTip(tr("Show the Office clipboard Task Pane"));

		m_pActionAddTargetData = groupClipboard->addAction(QIcon(m_strResPath + "_loadimg.png"), QString::fromLocal8Bit("Ŀ������"), Qt::ToolButtonTextUnderIcon);
		m_pActionAddTargetData->setToolTip(QString::fromLocal8Bit("����Ŀ������"));
		connect(m_pActionAddTargetData, SIGNAL(triggered()), this, SLOT(addTargetData()));
		m_pActionAddTargetData->setDisabled(!_manager.valid());

		RibbonControl* control = groupClipboard->controlByAction(m_pActionAddTargetData);
		control->sizeDefinition(RibbonControlSizeDefinition::GroupLarge)->setImageSize(RibbonControlSizeDefinition::ImageLarge);
		control->sizeDefinition(RibbonControlSizeDefinition::GroupMedium)->setImageSize(RibbonControlSizeDefinition::ImageLarge);
		control->sizeDefinition(RibbonControlSizeDefinition::GroupSmall)->setImageSize(RibbonControlSizeDefinition::ImageLarge);

// 		QAction* pActionLoadTargetDataJet = groupClipboard->addAction(QIcon(m_strResPath + "_loadimg.png"), QString::fromLocal8Bit("����β��"), Qt::ToolButtonTextUnderIcon);
// 		pActionLoadTargetDataJet->setToolTip(QString::fromLocal8Bit("����β��"));
// 		connect(pActionLoadTargetDataJet, SIGNAL(triggered()), this, SLOT(addTargetDataJet()));
// 		pActionLoadTargetDataJet->setDisabled(!_manager.valid());

// 		//�����ı�plt
// 		QAction* pActionLoadPlt = groupClipboard->addAction(QIcon(m_strResPath + "_loadimg.png"), QString::fromLocal8Bit("����plt"), Qt::ToolButtonTextUnderIcon);
// 		pActionLoadPlt->setToolTip(QString::fromLocal8Bit("����plt"));
// 		connect(pActionLoadPlt, SIGNAL(triggered()), this, SLOT(LoadPlt()));
// 		pActionLoadPlt->setDisabled(!_manager.valid());

		//Ŀ��תplt
		QAction* pActionTarget2Plt = groupClipboard->addAction(QIcon(m_strResPath + "_loadimg.png"), QString::fromLocal8Bit("Ŀ��תplt"), Qt::ToolButtonTextUnderIcon);
		pActionTarget2Plt->setToolTip(QString::fromLocal8Bit("Ŀ��תplt"));
		connect(pActionTarget2Plt, SIGNAL(triggered()), this, SLOT(Target2plt()));
		pActionTarget2Plt->setDisabled(!_manager.valid());

		QAction* pActionAddModel = groupClipboard->addAction(QIcon(m_strResPath + "_addModel.png"), QString::fromLocal8Bit("ģ��"), Qt::ToolButtonTextUnderIcon);
		pActionAddModel->setToolTip(QString::fromLocal8Bit("����ģ��"));
		connect(pActionAddModel, SIGNAL(triggered()), this, SLOT(addModelLayer()));
		pActionAddModel->setDisabled(!_manager.valid());

		control = groupClipboard->controlByAction(pActionAddModel);
		control->sizeDefinition(RibbonControlSizeDefinition::GroupLarge)->setImageSize(RibbonControlSizeDefinition::ImageLarge);
		control->sizeDefinition(RibbonControlSizeDefinition::GroupMedium)->setImageSize(RibbonControlSizeDefinition::ImageLarge);
		control->sizeDefinition(RibbonControlSizeDefinition::GroupSmall)->setImageSize(RibbonControlSizeDefinition::ImageLarge);

		_addAnnotationAction = groupClipboard->addAction(QIcon(m_strResPath + "_loadanno.png"), QString::fromLocal8Bit("��ע"), Qt::ToolButtonTextUnderIcon);
		_addAnnotationAction->setToolTip(tr("Add an annotation"));
		connect(_addAnnotationAction, SIGNAL(triggered()), this, SLOT(addAnnotation()));
		_addAnnotationAction->setDisabled(!_manager.valid() || !_mapNode.valid() || !_annoRoot.valid());

		control = groupClipboard->controlByAction(_addAnnotationAction);
		control->sizeDefinition(RibbonControlSizeDefinition::GroupLarge)->setImageSize(RibbonControlSizeDefinition::ImageLarge);
		control->sizeDefinition(RibbonControlSizeDefinition::GroupMedium)->setImageSize(RibbonControlSizeDefinition::ImageLarge);
		control->sizeDefinition(RibbonControlSizeDefinition::GroupSmall)->setImageSize(RibbonControlSizeDefinition::ImageLarge);

		_terrainProfileAction = groupClipboard->addAction(QIcon(m_strResPath + "_profile.png"), QString::fromLocal8Bit("���η���"), Qt::ToolButtonTextUnderIcon);
		_terrainProfileAction->setToolTip(tr("Terrain Profile Tool"));
		_terrainProfileAction->setCheckable(true);
		connect(_terrainProfileAction, SIGNAL(toggled(bool)), this, SLOT(terrainProfileToggled(bool)));

		control = groupClipboard->controlByAction(_terrainProfileAction);
		control->sizeDefinition(RibbonControlSizeDefinition::GroupLarge)->setImageSize(RibbonControlSizeDefinition::ImageLarge);
		control->sizeDefinition(RibbonControlSizeDefinition::GroupMedium)->setImageSize(RibbonControlSizeDefinition::ImageLarge);
		control->sizeDefinition(RibbonControlSizeDefinition::GroupSmall)->setImageSize(RibbonControlSizeDefinition::ImageLarge);
	}
}
