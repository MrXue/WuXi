#include "imageresample.h"
#include "ui_imageresample.h"
#include <QFileDialog>
#include <QMessageBox>

ImageResample::ImageResample(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImageResample)
{
    ui->setupUi(this);
}

ImageResample::~ImageResample()
{
    delete ui;
}

void ImageResample::on_srcBtn_clicked()
{
    QString srcfile=QFileDialog::getOpenFileName(this,QString::fromLocal8Bit("选择原始影像"),"","Image file (*.tif *.tiff);;All (*)");
    if(srcfile.isEmpty())
        return;
    ui->srcEdt->setText(srcfile);
}

void ImageResample::on_dstBtn_clicked()
{
    QString dstfile=QFileDialog::getSaveFileName(this,QString::fromLocal8Bit("设置输出影像"),"","Image file (*.tif *.tiff);;All (*)");
    if(dstfile.isEmpty())
        return;
    ui->dstEdt->setText(dstfile);
}

/***
* 遥感影像重采样   (要求影像必须有投影，否则走不通)
* @param pszSrcFile        输入文件的路径
* @param pszOutFile        写入的结果图像的路径
* @param eResample         采样模式，有五种，具体参见GDALResampleAlg定义，默认为双线性内插
* @param fResX             X转换采样比，默认大小为1.0，大于1图像变大，小于1表示图像缩小。数值上等于采样后图像的宽度和采样前图像宽度的比
* @param fResY             Y转换采样比，默认大小为1.0，大于1图像变大，小于1表示图像缩小。数值上等于采样后图像的高度和采样前图像高度的比
* @param pProgress      进度条指针
* @retrieve     0   成功
* @retrieve     -1  打开源文件失败
* @retrieve     -2  创建新文件失败
* @retrieve     -3  处理过程中出错
*/
int ImageResample::ResampleGDAL(const char *pszSrcFile, const char *pszOutFile, float fResX, float fResY, GDALResampleAlg eResample, CProcessBase *pProgress)
{
    if(pProgress != NULL)
    {
        pProgress->ReSetProcess();
//        pProgress->setWindowTitle(QString::fromLocal8Bit("重采样"));
        pProgress->SetMessage("正在执行影像重采样...");
    }

    GDALAllRegister();
    CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
    GDALDataset *pDSrc = (GDALDataset *)GDALOpen(pszSrcFile, GA_ReadOnly);
    if (pDSrc == NULL)
    {
        if(pProgress != NULL)
           pProgress->SetMessage("输入文件不能打开，请检查文件是否存在！");
        GDALClose( (GDALDatasetH) pDSrc );
        return -1;
    }

    GDALDriver *pDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
    if (pDriver == NULL)
    {
        if(pProgress != NULL)
            pProgress->SetMessage("不能创建制定类型的文件，请检查该文件类型GDAL是否支持创建！");
        GDALClose((GDALDatasetH)pDSrc);
        return -2;
    }
    int width = pDSrc->GetRasterXSize();
    int height = pDSrc->GetRasterYSize();
    int nBandCount = pDSrc->GetRasterCount();
    GDALDataType dataType = pDSrc->GetRasterBand(1)->GetRasterDataType();

    char *pszSrcWKT = NULL;
    pszSrcWKT = const_cast<char *>(pDSrc->GetProjectionRef());

    //如果没有投影，人为设置一个
    if (strlen(pszSrcWKT) <= 0)
    {
        OGRSpatialReference oSRS;
        oSRS.SetUTM(50, true);   //北半球  东经120度
        oSRS.SetWellKnownGeogCS("WGS84");
        oSRS.exportToWkt(&pszSrcWKT);
    }



    void *hTransformArg;
    hTransformArg = GDALCreateGenImgProjTransformer((GDALDatasetH)pDSrc, pszSrcWKT, NULL, pszSrcWKT, FALSE, 0.0, 1);

    //(没有投影的影像到这里就走不通了)
    if (hTransformArg == NULL)
    {
        if(pProgress != NULL)
            pProgress->SetMessage("转换参数错误！");
        GDALClose((GDALDatasetH)pDSrc);
        return -3;
    }

    double dGeoTrans[6] = { 0 };
    int nNewWidth = 0, nNewHeight = 0;
    if (GDALSuggestedWarpOutput((GDALDatasetH)pDSrc, GDALGenImgProjTransform, hTransformArg, dGeoTrans, &nNewWidth, &nNewHeight) != CE_None)
    {
        GDALClose((GDALDatasetH)pDSrc);
        return -3;
    }

    GDALDestroyGenImgProjTransformer(hTransformArg);

    //adfGeoTransform[0] /* top left x */
    //adfGeoTransform[1] /* w-e pixel resolution */
    //adfGeoTransform[2] /* rotation, 0 if image is "north up" */
    //adfGeoTransform[3] /* top left y */
    //adfGeoTransform[4] /* rotation, 0 if image is "north up" */
    //adfGeoTransform[5] /* n-s pixel resolution */

    dGeoTrans[1] = dGeoTrans[1] / fResX;
    dGeoTrans[5] = dGeoTrans[5] / fResY;
    nNewWidth = static_cast<int>(nNewWidth*fResX + 0.5);
    nNewHeight = static_cast<int>(nNewHeight*fResY + 0.5);

    //创建结果数据集
    GDALDataset *pDDst = pDriver->Create(pszOutFile, nNewWidth, nNewHeight, nBandCount, dataType, NULL);
    if (pDDst == NULL)
    {
        if(pProgress != NULL)
            pProgress->SetMessage("创建输出文件失败！");
        GDALClose((GDALDatasetH)pDSrc);
        return -2;
    }

    pDDst->SetProjection(pszSrcWKT);
    pDDst->SetGeoTransform(dGeoTrans);

    if(pProgress != NULL)
    {
        pProgress->SetMessage("正在执行重采样...");
//        pProgress->SetStepCount(nBandCount*nNewHeight);
    }

    GDALWarpOptions *psWo = GDALCreateWarpOptions();

    //psWo->papszWarpOptions = CSLDuplicate(NULL);
    psWo->eWorkingDataType = dataType;
    psWo->eResampleAlg = eResample;

    psWo->hSrcDS = (GDALDatasetH)pDSrc;
    psWo->hDstDS = (GDALDatasetH)pDDst;

    psWo->pfnTransformer = GDALGenImgProjTransform;
    psWo->pTransformerArg = GDALCreateGenImgProjTransformer((GDALDatasetH)pDSrc, pszSrcWKT, (GDALDatasetH)pDDst, pszSrcWKT, FALSE, 0.0, 1);

    GDALProgressFunc pfnProgress=ALGTermProgress;

    psWo->pfnProgress = pfnProgress;
    psWo->pProgressArg = pProgress;

    psWo->nBandCount = nBandCount;
    psWo->panSrcBands = (int *)CPLMalloc(nBandCount*sizeof(int));
    psWo->panDstBands = (int *)CPLMalloc(nBandCount*sizeof(int));
    /*psWo->dfWarpMemoryLimit = 512.0 * 1024.0;*/

    for (int i = 0; i < nBandCount; i++)
    {
        psWo->panSrcBands[i] = i + 1;
        psWo->panDstBands[i] = i + 1;
    }

    GDALWarpOperation oWo;
    if (oWo.Initialize(psWo) != CE_None)
    {
        if(pProgress != NULL)
            pProgress->SetMessage("转换参数错误！");
        GDALClose((GDALDatasetH)pDSrc);
        GDALClose((GDALDatasetH)pDDst);
        return -3;
    }

    oWo.ChunkAndWarpImage(0, 0, nNewWidth, nNewHeight);
    GDALFlushCache(pDDst);

    GDALDestroyGenImgProjTransformer(psWo->pTransformerArg);
    GDALDestroyWarpOptions(psWo);
    GDALClose((GDALDatasetH)pDSrc);
    GDALClose((GDALDatasetH)pDDst);

    if(pProgress != NULL)
        pProgress->SetMessage("重采样完成！");
    return 0;
}


void ImageResample::on_okBtn_clicked()
{
    QString srcfile=ui->srcEdt->text();
    if(srcfile.isEmpty())
    {
        QMessageBox::critical(this,QString::fromLocal8Bit("注意"),QString::fromLocal8Bit("请输入原图像！"));
        return;
    }
    const QByteArray srcba=srcfile.toLocal8Bit();
    const char *pszfilename=srcba.data();

    QString dstfile=ui->dstEdt->text();
    if(dstfile.isEmpty())
    {
        QMessageBox::critical(this,QString::fromLocal8Bit("注意"),QString::fromLocal8Bit("请设置输出图像！"));
        return;
    }
    const QByteArray dstba=dstfile.toLocal8Bit();
    const char *dstfilename=dstba.data();

    QString ratio=ui->resampleRatioEdt->text();
    if(ratio.isEmpty())
    {
        QMessageBox::critical(this,QString::fromLocal8Bit("注意"),QString::fromLocal8Bit("请输入采样比率！"));
        return;
    }
    int index=ui->resampleComBo->currentIndex();
    GDALResampleAlg gResamplealg=GRA_Bilinear;
    switch (index) {
    case 0:
        gResamplealg=GRA_NearestNeighbour;
        break;
    case 1:
        gResamplealg=GRA_Bilinear;
        break;
    case 2:
        gResamplealg=GRA_Cubic;
        break;
    default:
        break;
    }
    double dRatio=ratio.toDouble();
    CProcessDlg *pProgress=new CProcessDlg();
    pProgress->show();
    int res = ResampleGDAL(pszfilename,dstfilename,dRatio,dRatio,gResamplealg,pProgress);
    delete pProgress;
}


int STD_API ALGTermProgress(double dfComplete, const char *pszMessage, void *pProgressArg)
{
    if(pProgressArg != NULL)
    {
        CProcessBase * pProcess = (CProcessBase*) pProgressArg;
        pProcess->m_bIsContinue = pProcess->SetPosition(dfComplete);

        if(pProcess->m_bIsContinue)
            return TRUE;
        else
            return FALSE;
    }
    else
        return TRUE;
}
