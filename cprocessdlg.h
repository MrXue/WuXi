#ifndef CPROCESSDLG_H
#define CPROCESSDLG_H

#include <QProgressDialog>
#include "cprocessbase.h"



class CProcessDlg : public QProgressDialog,public CProcessBase
{
    Q_OBJECT
public:
    CProcessDlg(QWidget *parent = 0);
    ~CProcessDlg();

    /**
    * @brief 设置进度信息
    * @param pszMsg         进度信息
    */
    void SetMessage(const char* pszMsg);

    /**
    * @brief 设置进度值
    * @param dPosition      进度值
    */
    bool SetPosition(double dPosition);

    /**
    * @brief 进度条前进一步
    */
    bool StepIt();

    public slots:
        void updateProgress(int step);
};

#endif // CPROCESSDLG_H
