#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>

#include <string>

#include <osg/Camera>
#include <osg/Vec3>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->resize(800,600);
//    this->showMaximized();
    m_viewerWgt=new ViewerWidget();
    connect(m_viewerWgt,SIGNAL(on_mouse_event(QEvent*)),this,SLOT(slot_mouse(QEvent*)));
    connect(m_viewerWgt,SIGNAL(SIGNAL_ShowMeasureResult(QString,QString)),this,SLOT(slot_showCalResult(QString,QString)));
    setCentralWidget(m_viewerWgt);
    initWin();
    m_vpbTerrain=nullptr;
    m_compositeImage=nullptr;
    m_resample=nullptr;
//    ui->mainToolBar->hide();
//    ui->statusBar->hide();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpenFile_triggered()
{
    m_viewerWgt->clearMeasurement();
    QString path=QFileDialog::getOpenFileName(this,QString::fromLocal8Bit("打开场景文件"),"",tr("osg file(*.osg *.osgb *.ive)"));
    if(path.isEmpty())
        return;
    std::string filename=path.toStdString();
    m_viewerWgt->setScene(filename);


    ui->actionAreaMeasurement->setEnabled(true);
    ui->actionClearMeasure->setEnabled(true);
    ui->actionCloseFile->setEnabled(true);
    ui->actionEarthWorkVolume->setEnabled(true);
    ui->actionGetCoord->setEnabled(true);
    ui->actionLineSlop->setEnabled(true);
    ui->actionMultiPointMeasurement->setEnabled(true);
    ui->actionPointSlop->setEnabled(true);
    ui->actionTwoPointMeasurement->setEnabled(true);
    ui->actionVisibility->setEnabled(true);
}

void MainWindow::on_actionExit_triggered()
{
    this->close();
}

void MainWindow::on_actionTwoPointMeasurement_triggered()
{
    m_viewerWgt->setTwoPointMeaModel();
}

void MainWindow::on_actionMultiPointMeasurement_triggered()
{
    m_viewerWgt->setMultPointMeaModel();
}

void MainWindow::on_actionAreaMeasurement_triggered()
{
    m_viewerWgt->setAreaMeaModel();
}

void MainWindow::slot_mouse(QEvent*ev)
{
    osg::Camera* ca = m_viewerWgt->getCamera();
    if(!ca)return;

    osg::Vec3 e,c,u;
    ca->getViewMatrixAsLookAt(e,c,u);
//    double h = hx_terrain::intance()->get_height( e.x() , e.y());
    //qDebug() << h - e.z() << "\n";
    QString qs;

    {
        qs=QString::fromLocal8Bit("视点位置: x: %1 y: %2  高度: %3 m").arg(e.x()).arg(e.y()).arg(e.z(),0,'g',3);
    }
    ui->statusBar->showMessage(qs);
}

void MainWindow::slot_showCalResult(QString str, QString title)
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(title);
//    if(title>=-0.000001 && title<=0.000001)
//    {
//        showStr=str;
//        msgBox.setWindowTitle(QString::fromLocal8Bit("通视分析"));
//    }
//    else {
//        showStr=str+QString::number(title);
//        msgBox.setWindowTitle(QString::fromLocal8Bit("量测结果"));
//    }
    msgBox.setText(str);
    msgBox.exec();
}

void MainWindow::on_actionVPB_triggered()
{
    if(m_vpbTerrain==nullptr)
    {
        m_vpbTerrain=new VPBTerrain();
        m_vpbTerrain->show();
    }
    else {
        if(m_vpbTerrain->isHidden())
            m_vpbTerrain->show();
    }
}

void MainWindow::closeEvent(QCloseEvent *)
{
    if(m_vpbTerrain!=nullptr)
        m_vpbTerrain->close();
    if(m_compositeImage!=nullptr)
        m_compositeImage->close();
    if(m_resample!=nullptr)
        m_resample->close();
}

void MainWindow::on_actionClearMeasure_triggered()
{
    m_viewerWgt->clearMeasurement();
}

void MainWindow::on_actionVisibility_triggered()
{
    m_viewerWgt->setVisibilityMode();
}

void MainWindow::on_actionGetCoord_triggered()
{
    m_viewerWgt->setGetCoordMode();
}

void MainWindow::on_actionEarthWorkVolume_triggered()
{
    m_viewerWgt->setEarthWorkVolumeMode();
}

void MainWindow::on_actionPointSlop_triggered()
{
    m_viewerWgt->setPointSlopMode();
}

void MainWindow::on_actionLineSlop_triggered()
{
    m_viewerWgt->setLineSlopMode();
}

void MainWindow::on_actionCloseFile_triggered()
{
    m_viewerWgt->closeScene();
    m_viewerWgt->clearMeasurement();
    ui->actionAreaMeasurement->setEnabled(false);
    ui->actionClearMeasure->setEnabled(false);
    ui->actionCloseFile->setEnabled(false);
    ui->actionEarthWorkVolume->setEnabled(false);
    ui->actionGetCoord->setEnabled(false);
    ui->actionLineSlop->setEnabled(false);
    ui->actionMultiPointMeasurement->setEnabled(false);
    ui->actionPointSlop->setEnabled(false);
    ui->actionTwoPointMeasurement->setEnabled(false);
    ui->actionVisibility->setEnabled(false);
}

void MainWindow::initWin()
{

    ui->actionAreaMeasurement->setEnabled(false);
    ui->actionClearMeasure->setEnabled(false);
    ui->actionCloseFile->setEnabled(false);
    ui->actionEarthWorkVolume->setEnabled(false);
    ui->actionGetCoord->setEnabled(false);
    ui->actionLineSlop->setEnabled(false);
    ui->actionMultiPointMeasurement->setEnabled(false);
    ui->actionPointSlop->setEnabled(false);
    ui->actionTwoPointMeasurement->setEnabled(false);
    ui->actionVisibility->setEnabled(false);

//    ui->actionGetCoord->setVisible(false);
    ui->actionPointSlop->setVisible(false);
}

void MainWindow::on_actionInstructions_triggered()
{
    QMessageBox msgb;
    msgb.setWindowTitle(QString::fromLocal8Bit("使用说明"));
    QString instr=QString::fromLocal8Bit("用户可以用鼠标对地形进行浏览；当进入测量模式时，地形自动进入漫游模式。\n");
    instr+=QString::fromLocal8Bit("漫游模式用户只能通过键盘按键对地形进行浏览操作，使用鼠标进行测量。\n\n");
    instr+=QString::fromLocal8Bit("控制按键：\n");
    instr+=QString::fromLocal8Bit("移动：w/前进，a/左移，s/后退，d/右移，Home/上移，End/下移\n");
    instr+=QString::fromLocal8Bit("旋转：上下箭头/俯仰，左右箭头/偏航，k l/翻滚\n");
    instr+=QString::fromLocal8Bit("控制：+/增加平移速度，-/减少速度\n\n");
    instr+=QString::fromLocal8Bit("拾取坐标：鼠标左右键都可以进行拾取\n"
                                  "两点测距：鼠标左右键选取两点，弹出结果\n"
                                  "多点测距：左键开始选择点，右键结束选择并弹出结果\n"
                                  "面积量测：逆时针方向，左键选择区域，右键结束选择并弹出结果\n"
                                  "通视分析：鼠标左右键选取两点，弹出结果\n"
                                  "土方量：逆时针方向，左键选择区域，右键结束选择并弹出结果\n"
                                  "点坡度：鼠标左右键都可以进行选择点\n"
                                  "线坡度：鼠标左右键选取两点，弹出结果\n"
                                  "清除量测结果：将地形中的标识清楚并结束量测，进入鼠标操作模式\n");
    msgb.setText(instr);
    msgb.exec();
}

void MainWindow::on_actionQt_triggered()
{
    QApplication::aboutQt();
}

void MainWindow::on_actionCompositeImage_triggered()
{
    if(m_compositeImage==nullptr)
    {
        m_compositeImage=new CompositeImage();
        m_compositeImage->show();
    }
    else
    {
        if(m_compositeImage->isHidden())
            m_compositeImage->show();
    }
}

void MainWindow::on_actionResample_triggered()
{
    if(m_resample==nullptr)
    {
        m_resample=new ImageResample();
        m_resample->show();
    }
    else {
        if(m_resample->isHidden())
            m_resample->show();
    }
}

void MainWindow::on_actionInfo_triggered()
{
    //TODO:获得影像元数据
}

void MainWindow::on_actionDEM_triggered()
{
    QString path=QFileDialog::getOpenFileName(this,QString::fromLocal8Bit("打开场景文件"),"",tr("DEM file(*.tif *.tiff)"));
    if(path.isEmpty())
        return;
    std::string filename=path.toStdString();
    m_viewerWgt->setDEMFile(filename);

    ui->actionCloseFile->setEnabled(true);
}

void MainWindow::on_actionOpenImage_triggered()
{
    QProcess::startDetached("./ImageViewer/QImgMatchGUI.exe");

}
