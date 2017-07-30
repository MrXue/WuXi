#ifndef COMPOSITEIMAGE_H
#define COMPOSITEIMAGE_H

#include <QWidget>

#include <GDAL/gdal_priv.h>
#include <GDAL/gdalwarper.h>

namespace Ui {
class CompositeImage;
}

struct MyCoordinate
{
    double x;
    double y;
};

class CompositeImage : public QWidget
{
    Q_OBJECT

public:
    explicit CompositeImage(QWidget *parent = 0);
    ~CompositeImage();

private slots:
    void on_srcBtn_clicked();

    void on_addBtn_clicked();

    void on_okBtn_clicked();

    void on_dstBtn_clicked();

private:
    Ui::CompositeImage *ui;

    //影像合成，默认影像分辨率一样，投影一样
    void composite(QString &srcPath,QStringList &addListPath,QString &dstPath);

    //重采样
    void resample(GDALDataset *srcDataset, GDALDataset *dstDataset, double *dGeoTrans, int nNewWidth, int nNewHeight, GDALResampleAlg eResample);

    //添加影像
    void addNewImage(GDALDataset *srcDataset,double *dGeoTrans,int nSrcWidth,int nSrcHeight,QString &addStr);

    //坐标（x,y）是否在矩形区域范围，其中rectangleScope为矩形范围，大小为4，需要判断的坐标为coordinate，大小为2
    bool isInRectangle(const MyCoordinate &leftup,const MyCoordinate &rightbottom,const MyCoordinate &coordinate);

    //根据地理范围获得在图像上的行列号
    bool projection2ImageRowCol(double *adfGeoTransform, const MyCoordinate coord, int &iCol, int &iRow);

    //获得数据集的分辨率，以米表示。默认X，Y方向分辨率一样
    double getResolutionM(GDALDataset *pDSrc);

    //根据经度获得在哪个6度带
    int get6Zone(const double &longitude);
};

#endif // COMPOSITEIMAGE_H
