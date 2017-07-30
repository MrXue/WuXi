#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "viewerwidget.h"
#include "vpbterrain.h"
#include "compositeimage.h"
#include "imageresample.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionOpenFile_triggered();

    void on_actionExit_triggered();

    void on_actionTwoPointMeasurement_triggered();

    void on_actionMultiPointMeasurement_triggered();

    void on_actionAreaMeasurement_triggered();

    void slot_mouse(QEvent*ev);

    void slot_showCalResult(QString str,QString title);

    void on_actionVPB_triggered();

    void on_actionClearMeasure_triggered();

    void on_actionVisibility_triggered();

    void on_actionGetCoord_triggered();

    void on_actionEarthWorkVolume_triggered();

    void on_actionPointSlop_triggered();

    void on_actionLineSlop_triggered();

    void on_actionCloseFile_triggered();

    void on_actionInstructions_triggered();

    void on_actionQt_triggered();

    void on_actionCompositeImage_triggered();

    void on_actionResample_triggered();

    void on_actionInfo_triggered();

    void on_actionDEM_triggered();

    void on_actionOpenImage_triggered();

private:
    Ui::MainWindow *ui;
    ViewerWidget *m_viewerWgt;
    VPBTerrain *m_vpbTerrain;
    CompositeImage *m_compositeImage;
    ImageResample *m_resample;

    void initWin();

    // QWidget interface
protected:
    void closeEvent(QCloseEvent *);
};

#endif // MAINWINDOW_H
