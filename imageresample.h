#ifndef IMAGERESAMPLE_H
#define IMAGERESAMPLE_H

#include <QWidget>
#include <GDAL/gdal_priv.h>
#include <GDAL/gdalwarper.h>
#include <GDAL/ogrsf_frmts.h>
#include "cprocessdlg.h"

#ifndef STD_API
#define STD_API __stdcall
#endif

int  STD_API  ALGTermProgress(double dfComplete, const char *pszMessage, void * pProgressArg );

namespace Ui {
class ImageResample;
}

class ImageResample : public QWidget
{
    Q_OBJECT

public:
    explicit ImageResample(QWidget *parent = 0);
    ~ImageResample();

private slots:
    void on_srcBtn_clicked();

    void on_dstBtn_clicked();

    void on_okBtn_clicked();

private:
    Ui::ImageResample *ui;

    int ResampleGDAL(const char* pszSrcFile, const char* pszOutFile, float fResX , float fResY , GDALResampleAlg eResample,CProcessBase *pProgress);

};

#endif // IMAGERESAMPLE_H
