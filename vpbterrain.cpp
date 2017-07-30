#include "vpbterrain.h"
#include "ui_vpbterrain.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDebug>

#include <gdal_priv.h>
#include <GDAL/ogrsf_frmts.h>


VPBTerrain::VPBTerrain(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VPBTerrain)
{
    ui->setupUi(this);
    process = new QProcess();
    connect(process,SIGNAL(readyReadStandardOutput()),this,SLOT(slot_outlog()));
    connect(process,SIGNAL(readyReadStandardError()),this,SLOT(slot_error()));
    init();
}

VPBTerrain::~VPBTerrain()
{
    delete ui;
    delete process;
}

void VPBTerrain::init()
{
    ui->csEdt->setText("\"+proj=latlong +datum=WGS84\"");
}

void VPBTerrain::on_okBtn_clicked()
{
    QString program ="osgdem.exe";//vpbmaster
    QStringList arguments;
    QString tfile=ui->tEdt->text();
    if(tfile.isEmpty())
    {
        QMessageBox msg;
        msg.setWindowTitle(QString::fromLocal8Bit("提示"));
        msg.setText(QString::fromLocal8Bit("纹理文件不能为空！"));
        msg.exec();
        return;
    }
    QString dfile=ui->dEdt->text();
    if(dfile.isEmpty())
    {
        QMessageBox msg;
        msg.setWindowTitle(QString::fromLocal8Bit("提示"));
        msg.setText(QString::fromLocal8Bit("地形文件不能为空！"));
        msg.exec();
        return;
    }
    QString level=ui->lEdt->text();
    if(level.isEmpty())
        level="6";
    QString vRabio=ui->vEdt->text();
    if(vRabio.isEmpty())
        vRabio="1";
    QString cs=ui->csEdt->text();
    if(cs.isEmpty())
        cs="\"+proj=latlong +datum=WGS84\"";
    QString oive=ui->oEdt->text();
    if(oive.isEmpty())
    {
        QMessageBox msg;
        msg.setWindowTitle(QString::fromLocal8Bit("提示"));
        msg.setText(QString::fromLocal8Bit("输出文件不能为空！"));
        msg.exec();
        return;
    }
    int lastIndex=oive.lastIndexOf("/");
    QString aosga=oive.left(lastIndex)+"/"+"pegout.osga";
    arguments<<"-t"<<tfile<<"-d"<<dfile<<"-l"<<level
            <<"-o"<<oive<<"-a"<<aosga;
    //<<"--cs"<<cs<<"-v"<<vRabio
//    arguments<<"--xx"<<"10"<<"--yy"<<"10"<<"-t"<<"E:\\Data\\Mars\\roi2_color.tif"
//            <<"--xx"<<"10"<<"--yy"<<"10"<<"-d"<<"E:\\Data\\Mars\\roi2_dem.tif"
//           <<"-l"<<"6"<<"-v"<<"100"<<"--cs"<<"\"+proj=latlong"<<"+datum=WGS84\""<<"-o"<<"E:/code/Qt5.4.1/TestVPB/puget.ive"
//          <<"-a"<<"E:/code/Qt5.4.1/TestVPB/pegout.osga";
//    QStringList helpArguments;
//    helpArguments<<"--help";

//    process->start(program,arguments);
    if(process->startDetached(program,arguments))
    {
           QMessageBox msg;
           msg.setWindowTitle(QString::fromLocal8Bit("提示"));
           msg.setText(QString::fromLocal8Bit("程序调用成功！"));
           msg.exec();
    }
    if(process->error()==QProcess::FailedToStart)
    {
//        std::cout<<"QProcess::FailedToStart"<<std::endl;
        QMessageBox msg;
        msg.setWindowTitle(QString::fromLocal8Bit("提示"));
        msg.setText(QString::fromLocal8Bit("程序未能正常运行，请检查程序是否存在！"));
        msg.exec();
        return;
    }
//    if(process->waitForFinished())
//    {
//        QMessageBox msg;
//        msg.setWindowTitle(QString::fromLocal8Bit("提示"));
//        msg.setText(QString::fromLocal8Bit("创建地形完成！"));
//        msg.exec();
//    }
    int oivesize=oive.size();
    QString outDEMInfo=oive.left(oivesize-4)+".DemInfo";
    QString outImageInffile=oive.left(oivesize-4)+".ImageInfo";
    saveDEMInfo(dfile,outDEMInfo);
    saveImageInfo(tfile,outImageInffile);
//    process->startDetached(program,arguments);
}

void VPBTerrain::on_helpBtn_clicked()
{
//    process->start("notepad.exe help.txt");
    QMessageBox msgb;
    msgb.setWindowTitle(QString::fromLocal8Bit("使用说明:"));
    QString instr=QString::fromLocal8Bit("构建地形：\n"
                                         "LOD等级：指定地形生成的精细程度；\n"
                                         "               值越大越精细，构建速度越慢，默认为6\n"
                                         "垂直比率：DEM缩放比例，默认为1\n"
                                         "设置坐标系统：默认即可\n"
                                         "纹理文件：纹理影像\n"
                                         "DEM文件：DEM数据\n"
                                         "数据页输出文件：输出文件");
    msgb.setText(instr);
    QAbstractButton *helpBtn=msgb.addButton(QString::fromLocal8Bit("详细帮助"),QMessageBox::ActionRole);
    msgb.addButton(QMessageBox::Ok);
    msgb.exec();
    if(msgb.clickedButton()==helpBtn)
    {
        process->start("notepad.exe help.txt");
    }
}

void VPBTerrain::on_tBtn_clicked()
{
    QString tfile=QFileDialog::getOpenFileName(this,QString::fromLocal8Bit("选择影像文件"),"",tr("Image file (*.tif *.tiff);;All (*)"));
    if(tfile.isEmpty())
        return;
    ui->tEdt->setText(tfile);
}

void VPBTerrain::on_dBtn_clicked()
{
    QString dfile=QFileDialog::getOpenFileName(this,QString::fromLocal8Bit("选择地形文件"),"",tr("DEM file (*.tif *.tiff);;All (*)"));
    if(dfile.isEmpty())
        return;
    ui->dEdt->setText(dfile);
}

void VPBTerrain::on_oBtn_clicked()
{
    QString ofile=QFileDialog::getSaveFileName(this,QString::fromLocal8Bit("设置保存文件"),"",tr("OSG file (*.ive);;OSG file (*.osgb)"));
    if(ofile.isEmpty())
        return;
    ui->oEdt->setText(ofile);
}

void VPBTerrain::on_closeBtn_clicked()
{
    this->close();
}

void VPBTerrain::slot_outlog()
{
    QByteArray ba=process->readAllStandardOutput();
    QFile file("out.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    QTextStream out(&file);
    out<<ba;
    file.close();
}

void VPBTerrain::slot_error()
{
    qDebug()<<process->errorString();
}

void VPBTerrain::saveDEMInfo(QString path, QString savefile)
{
    GDALAllRegister();
    CPLSetConfigOption("GDAL_FILENAME_IS_UTF8","NO");
    const QByteArray ba=path.toLocal8Bit();
    const char *pszfilename=ba.data();
    GDALDataset *poDataset=(GDALDataset*)GDALOpen(pszfilename,GA_ReadOnly);
    int nXSize=poDataset->GetRasterXSize();
    int nYSize=poDataset->GetRasterYSize();
    double aGeotransform[6]={0};
    poDataset->GetGeoTransform(aGeotransform);
    QFile file(savefile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    QTextStream out(&file);
    out<<"Columns: "<<nXSize<<"\n";
    out<<"Rows: "<<nYSize<<"\n";
    for(int i=0;i<6;++i)
    {
        out<<aGeotransform[i]<<"\n";
    }
    file.close();
    GDALClose((GDALDatasetH)poDataset);
}

void VPBTerrain::saveImageInfo(QString imagefile, QString projfile)
{
    const QByteArray ba=imagefile.toLocal8Bit();
    const char *pszfilename=ba.data();
    GDALDataset *poDataset=(GDALDataset*)GDALOpen(pszfilename,GA_ReadOnly);
    int nXSize=poDataset->GetRasterXSize();
    int nYSize=poDataset->GetRasterYSize();
    double aGeotransform[6]={0};
    poDataset->GetGeoTransform(aGeotransform);
    char *pszSrcWKT = NULL;
    pszSrcWKT = const_cast<char *>(poDataset->GetProjectionRef());

    OGRSpatialReference ogrSRF;
    ogrSRF.importFromWkt(&pszSrcWKT);

    char *projchar = new char[128];
    ogrSRF.exportToProj4(&projchar);

    QFile file(projfile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    QTextStream out(&file);
    out<<projchar<<"\n";

    out<<"Columns: "<<nXSize<<"\n";
    out<<"Rows: "<<nYSize<<"\n";
    for(int i=0;i<6;++i)
    {
        out<<aGeotransform[i]<<"\n";
    }
    file.close();
    GDALClose((GDALDatasetH)poDataset);
}











