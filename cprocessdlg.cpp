#include "cprocessdlg.h"
#include <QCoreApplication>

CProcessDlg::CProcessDlg(QWidget *parent)
{
    m_dPosition = 0.0;
    m_iStepCount = 100;
    m_iCurStep = 0;

    setModal(true);
    setLabelText(QString::fromLocal8Bit("处理中..."));
    setAutoClose(false);
    setAutoReset(false);
    setCancelButtonText(QString::fromLocal8Bit("取消"));

    setWindowTitle(QString::fromLocal8Bit("进度"));

    //禁用关闭按钮
    setWindowFlags( Qt::FramelessWindowHint );//| Qt::WindowStaysOnTopHint Qt::WindowTitleHint |
}

CProcessDlg::~CProcessDlg()
{

}

void CProcessDlg::SetMessage(const char *pszMsg)
{
    if (pszMsg != NULL)
    {
        m_strMessage = pszMsg;
        setLabelText(QString::fromLocal8Bit(pszMsg));
    }
}

bool CProcessDlg::SetPosition(double dPosition)
{
    m_dPosition = dPosition;

    setValue( std::min( 100u, ( uint )( m_dPosition*100.0 ) ) );
//    QCoreApplication::instance()->processEvents();
    QCoreApplication::instance()->processEvents();

    if(this->wasCanceled())
        return false;

    return true;
}

bool CProcessDlg::StepIt()
{
    m_iCurStep ++;
    m_dPosition = m_iCurStep*1.0 / m_iStepCount;

    setValue( std::min( 100u, ( uint )( m_dPosition*100.0 ) ) );
    QCoreApplication::instance()->processEvents();

    if(this->wasCanceled())
        return false;

    return true;
}

void CProcessDlg::updateProgress(int step)
{
    this->setValue(step);
    QCoreApplication::instance()->processEvents();
}

