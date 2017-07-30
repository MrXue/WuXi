#include "compositeimage.h"
#include "ui_compositeimage.h"

#include <GDAL/ogrsf_frmts.h>

#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>

CompositeImage::CompositeImage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CompositeImage)
{
    ui->setupUi(this);
    ui->addPEdt->setReadOnly(true);
}

CompositeImage::~CompositeImage()
{
    delete ui;
}

void CompositeImage::on_srcBtn_clicked()
{
    QString srcfile=QFileDialog::getOpenFileName(this,QString::fromLocal8Bit("选择原始影像"),"","Image file (*.tif *.tiff);;All (*)");
    if(srcfile.isEmpty())
        return;
    ui->srcEdt->setText(srcfile);
}

void CompositeImage::on_addBtn_clicked()
{
    QStringList addfiles = QFileDialog::getOpenFileNames(
                            this,
                            QString::fromLocal8Bit("添加影像"),
                            "",
                            "Image files (*.tif *.tiff);;All (*)");
    if(addfiles.isEmpty())
        return;
    for(int i=0;i<addfiles.size();++i)
    {
        ui->addPEdt->appendPlainText(addfiles[i]);
    }
}


void CompositeImage::on_okBtn_clicked()
{
    QString srcfile=ui->srcEdt->text();
    if(srcfile.isEmpty())
    {
        QMessageBox::critical(this,QString::fromLocal8Bit("注意"),QString::fromLocal8Bit("请输入原图像！"));
        return;
    }
    QString add_str=ui->addPEdt->toPlainText();
    if(add_str.isEmpty())
    {
        QMessageBox::critical(this,QString::fromLocal8Bit("注意"),QString::fromLocal8Bit("请输入需要添加影像！"));
        return;
    }
    QStringList add_strList=add_str.split("\n");
    QString dstfile=ui->dstEdt->text();
    if(dstfile.isEmpty())
    {
        QMessageBox::critical(this,QString::fromLocal8Bit("注意"),QString::fromLocal8Bit("请设置输出影像！"));
        return;
    }
    composite(srcfile,add_strList,dstfile);
}

void CompositeImage::on_dstBtn_clicked()
{
    QString dstfile=QFileDialog::getSaveFileName(this,QString::fromLocal8Bit("设置输出影像"),"","Image file (*.tif *.tiff);;All (*)");
    if(dstfile.isEmpty())
        return;
    ui->dstEdt->setText(dstfile);
}

void CompositeImage::composite(QString &srcPath, QStringList &addListPath, QString &dstPath)
{
    GDALAllRegister();
    CPLSetConfigOption("GDAL_FILENAME_IS_UTF8","NO");

    const QByteArray srcba=srcPath.toLocal8Bit();
    const char *pszfilename=srcba.data();
    GDALDataset *srcDataset=(GDALDataset*)GDALOpen(pszfilename,GA_ReadOnly);
    int nXSize=srcDataset->GetRasterXSize();
    int nYSize=srcDataset->GetRasterYSize();
    int nBandsCount=srcDataset->GetRasterCount();
    GDALDataType srcDataType=srcDataset->GetRasterBand(1)->GetRasterDataType();
    double srcGeoTransform[6]={0};
    srcDataset->GetGeoTransform(srcGeoTransform);
    const char *srcproj=srcDataset->GetProjectionRef();

    const QByteArray dstBa=dstPath.toLocal8Bit();
    const char *dstfilename=dstBa.data();
    GDALDriver *pDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
    if (pDriver == NULL)
    {
        return;
    }

    GDALDataset *dstDataset = pDriver->CreateCopy(dstfilename,srcDataset,FALSE,NULL,NULL,NULL);
//    GDALDataset *dstDataset = pDriver->Create(dstfilename, nXSize, nYSize, nBandsCount, srcDataType, NULL);
    if(dstDataset==NULL)
    {
        GDALClose((GDALDatasetH) srcDataset);
        return;
    }
//    dstDataset->SetProjection(srcproj);
//    dstDataset->SetGeoTransform(srcGeoTransform);
    for(int i=0;i<addListPath.size();++i)
    {
        QString addPath=addListPath.at(i);
        addNewImage(dstDataset,srcGeoTransform,nXSize,nYSize,addPath);
    }
    QMessageBox::information(this,QString::fromLocal8Bit("合成影像"),QString::fromLocal8Bit("影像合成完毕！"));
    GDALClose((GDALDatasetH)srcDataset);
    GDALClose((GDALDatasetH)dstDataset);
}

void CompositeImage::resample(GDALDataset *srcDataset, GDALDataset *dstDataset,double *dGeoTrans, int nNewWidth, int nNewHeight,GDALResampleAlg eResample)
{
    if(srcDataset==NULL || dstDataset==NULL)
        return;

    GDALDataType dataType=srcDataset->GetRasterBand(1)->GetRasterDataType();
    int nBandsCount=srcDataset->GetRasterCount();
    char *pszSrcWKT = NULL;
    pszSrcWKT = const_cast<char *>(srcDataset->GetProjectionRef());

    //如果没有投影，人为设置一个
    if (strlen(pszSrcWKT) <= 0)
    {
        OGRSpatialReference oSRS;
        oSRS.SetUTM(get6Zone(134.0), true);   //北半球  东经120度
        oSRS.SetWellKnownGeogCS("WGS84");
        oSRS.exportToWkt(&pszSrcWKT);
    }
    void *hTransformArg;
    hTransformArg = GDALCreateGenImgProjTransformer((GDALDatasetH)srcDataset, pszSrcWKT, NULL, pszSrcWKT, FALSE, 0.0, 1);
    //(没有投影的影像到这里就走不通了)
    if (hTransformArg == NULL)
    {
//        GDALClose((GDALDatasetH)srcDataset);
        return;
    }

    if(GDALSuggestedWarpOutput((GDALDatasetH)srcDataset,GDALGenImgProjTransform,hTransformArg,dGeoTrans,&nNewWidth,&nNewHeight)!=CE_None)
    {
//        GDALClose((GDALDatasetH) srcDataset);
        return;
    }
    GDALDestroyGenImgProjTransformer(hTransformArg);

    //主要是下面的步骤
    GDALWarpOptions *psWo = GDALCreateWarpOptions();
    psWo->eWorkingDataType = dataType;
    psWo->eResampleAlg = eResample;
    psWo->hSrcDS=(GDALDatasetH)srcDataset;
    psWo->hDstDS=(GDALDatasetH)dstDataset;
    psWo->pfnTransformer=GDALGenImgProjTransform;
    psWo->pTransformerArg=GDALCreateGenImgProjTransformer((GDALDatasetH)srcDataset,pszSrcWKT,(GDALDatasetH)dstDataset,pszSrcWKT,FALSE,0.0,1);
    psWo->nBandCount=nBandsCount;
    psWo->panSrcBands=new int[nBandsCount];
    psWo->panDstBands=new int[nBandsCount];
//    psWo->dfWarpMemoryLimit=
    for(int i=0;i<nBandsCount;++i)
    {
        psWo->panSrcBands[i]=i+1;
        psWo->panDstBands[i]=i+1;
    }
    GDALWarpOperation oWo;
    if(oWo.Initialize(psWo)!=CE_None)
    {
//        GDALClose((GDALDatasetH) srcDataset);
//        GDALClose((GDALDatasetH) dstDataset);
        return;
    }

    oWo.ChunkAndWarpImage(0, 0, nNewWidth, nNewHeight);
    GDALFlushCache( dstDataset );
    GDALDestroyGenImgProjTransformer(psWo->pTransformerArg);
    GDALDestroyWarpOptions( psWo );
}
//这个srcDataset，不能是只读的，需要对其进行更新,而且此时两者的分辨率应该是一样的
void CompositeImage::addNewImage(GDALDataset *srcDataset, double *dGeoTrans, int nSrcWidth, int nSrcHeight, QString &addStr)
{
    if(srcDataset==NULL)
        return;
    if(dGeoTrans==NULL)
        return;
    GDALAllRegister();
    CPLSetConfigOption("GDAL_FILENAME_IS_UTF8","NO");

    //获得添加影像的信息
    const QByteArray addba=addStr.toLocal8Bit();
    const char *pszfilename=addba.data();
    GDALDataset *addDataset=(GDALDataset*)GDALOpen(pszfilename,GA_ReadOnly);
    int nAddWidth=addDataset->GetRasterXSize();
    int nAddHeight=addDataset->GetRasterYSize();
    int nAddBandsCount=addDataset->GetRasterCount();
    double addGeoTrans[6]={0};
    addDataset->GetGeoTransform(addGeoTrans);
    const char *addProj=addDataset->GetProjectionRef();

    const char *srcProj=srcDataset->GetProjectionRef();
    char *castSrcProj=const_cast<char*>(srcProj);
    char *castAddProj=const_cast<char*>(addProj);
    OGRSpatialReference srcOgrSRF,addOgrSRF;
    srcOgrSRF.importFromWkt(&castSrcProj);
    addOgrSRF.importFromWkt(&castAddProj);
    if(!srcOgrSRF.IsSame(&addOgrSRF))
    {
        //如果两个投影不一样,直接返回，如果有投影不一样的需求以后可以在这里进行投影转换
        return;
    }

    //分辨率也应该一样

    MyCoordinate srcCoorRightBottom;
    MyCoordinate srcCoorLeftUp;
    MyCoordinate addCoorRightBottom;
    MyCoordinate addCoorLeftUp;
    srcCoorRightBottom.x=dGeoTrans[0]+dGeoTrans[1]*nSrcWidth+dGeoTrans[2]*nSrcHeight;
    srcCoorRightBottom.y=dGeoTrans[3]+dGeoTrans[4]*nSrcWidth+dGeoTrans[5]*nSrcHeight;
    srcCoorLeftUp.x=dGeoTrans[0];
    srcCoorLeftUp.y=dGeoTrans[3];
    addCoorRightBottom.x=addGeoTrans[0]+addGeoTrans[1]*nAddWidth+addGeoTrans[2]*nAddHeight;
    addCoorRightBottom.y=addGeoTrans[3]+addGeoTrans[4]*nAddWidth+addGeoTrans[5]*nAddHeight;
    addCoorLeftUp.x=addGeoTrans[0];
    addCoorLeftUp.y=addGeoTrans[3];

    //如果添加区域正好全部都在里面
    if(isInRectangle(srcCoorLeftUp,srcCoorRightBottom,addCoorLeftUp) && isInRectangle(srcCoorLeftUp,srcCoorRightBottom,addCoorRightBottom))
    {
        int startCol=0;
        int startRow=0;
        int endCol=0;
        int endRow=0;
        projection2ImageRowCol(dGeoTrans,addCoorLeftUp,startCol,startRow);
        projection2ImageRowCol(dGeoTrans,addCoorRightBottom,endCol,endRow);
        //数据类型应该一样，这里应该有个判断，如果不一样就直接返回
        GDALDataType srcDataType=srcDataset->GetRasterBand(1)->GetRasterDataType();
        GDALDataType addDataType=addDataset->GetRasterBand(1)->GetRasterDataType();
//        float *pixelsData=new float[nAddWidth];
        void *pixelsData=NULL;
        switch (srcDataType) {
        case GDT_Byte:
            pixelsData=new unsigned char[nAddWidth];
            break;
        case GDT_UInt16:
            pixelsData=new unsigned short[nAddWidth];
            break;
        case GDT_Float32:
            pixelsData=new float[nAddWidth];
            break;
        case GDT_Int16:
            pixelsData=new short[nAddWidth];
            break;
        default:
            break;
        }
        for(int n=1;n<=nAddBandsCount;++n)
        {
            GDALRasterBand *srcBand=srcDataset->GetRasterBand(n);
            GDALRasterBand *addBand=addDataset->GetRasterBand(n);
            for(int j=0;j<nAddHeight;++j)
            {
                addBand->RasterIO(GF_Read,0,j,nAddWidth,1,pixelsData,nAddWidth,1,srcDataType,0,0);
                srcBand->RasterIO(GF_Write,startCol,startRow+j,nAddWidth,1,pixelsData,nAddWidth,1,srcDataType,0,0);
            }
        }
        delete []pixelsData;
    }
    GDALClose((GDALDatasetH)addDataset);
}

bool CompositeImage::isInRectangle(const MyCoordinate &leftup, const MyCoordinate &rightbottom, const MyCoordinate &coordinate)
{
    if(coordinate.x<=rightbottom.x && coordinate.x>=leftup.x && coordinate.y<=leftup.y && coordinate.y>=rightbottom.y)
        return true;
    return false;
}

bool CompositeImage::projection2ImageRowCol(double *adfGeoTransform, const MyCoordinate coord, int &iCol, int &iRow)
{
    double dTemp = adfGeoTransform[1]*adfGeoTransform[5] - adfGeoTransform[2]*adfGeoTransform[4];
    double dCol = 0.0, dRow = 0.0;
    dCol = (adfGeoTransform[5]*(coord.x - adfGeoTransform[0]) -
                adfGeoTransform[2]*(coord.y - adfGeoTransform[3])) / dTemp + 0.5;

    dRow = (adfGeoTransform[1]*(coord.y - adfGeoTransform[3]) -
                adfGeoTransform[4]*(coord.x - adfGeoTransform[0])) / dTemp + 0.5;
    iCol = int(dCol);
    iRow = int(dRow);
    return true;
}

double CompositeImage::getResolutionM(GDALDataset *pDSrc)
{
    if(pDSrc==NULL)
        return 0.0;
    double srcGeotransform[6] = { 0 };
    pDSrc->GetGeoTransform(srcGeotransform);
    char *pszSrcWKT = NULL;
    pszSrcWKT = const_cast<char *>(pDSrc->GetProjectionRef());

    double *x = new double[2];
    double *y = new double[2];
    double resolution=0.0;
    for (int i = 0; i < 2; ++i)
    {
        x[i] = srcGeotransform[0] + srcGeotransform[1] * i;
        y[i] = srcGeotransform[3];
    }
    OGRSpatialReference ogrSRF, *poLatLong;
    //ogrSRF.SetProjection(pszSrcWKT);
    ogrSRF.importFromWkt(&pszSrcWKT);

    if (ogrSRF.IsGeocentric())
    {
        qDebug()<<"IsGeocentric";
        delete[]x;
        delete[]y;
        return resolution;
    }
    if (ogrSRF.IsGeographic())
    {
        OGRCoordinateTransformation *poTransform;
        poLatLong = ogrSRF.CloneGeogCS();
        poLatLong->SetUTM(get6Zone(srcGeotransform[0]), TRUE);
        poLatLong->SetWellKnownGeogCS("EPSG:4326");

        poTransform = OGRCreateCoordinateTransformation(&ogrSRF, poLatLong);

        if (!poTransform->Transform(2, x, y))
        {
            //转换失败
            qDebug()<<"Transform failed!";
        }
        resolution=abs(x[1]-x[0]);
        delete[]x;
        delete[]y;
        return resolution;
    }
    if (ogrSRF.IsProjected())
    {
        delete[]x;
        delete[]y;
        return srcGeotransform[1];
    }
    delete []x;
    delete []y;
    return resolution;
}

int CompositeImage::get6Zone(const double &longitude)
{
    double zone = longitude + 3;
    zone /= 6.0;
    zone += 30.0;
    int trueZone = int(round(zone));
    return trueZone;
}












