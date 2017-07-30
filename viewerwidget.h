#ifndef VIEWERWIDGET_H
#define VIEWERWIDGET_H

#include <QWidget>
#include <QTimer>

#include <osgViewer/CompositeViewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgQt/GraphicsWindowQt>

#include <osgGA/TrackballManipulator>

#include "pickhandler.h"


class ViewerWidget : public QWidget,public osgViewer::CompositeViewer
{
    Q_OBJECT
public:
    ViewerWidget(QWidget* parent = 0, Qt::WindowFlags f = 0, osgViewer::ViewerBase::ThreadingModel threadingModel=osgViewer::CompositeViewer::SingleThreaded);
    ~ViewerWidget();

    QWidget* addViewWidget( osgQt::GraphicsWindowQt* gw );

    osgQt::GraphicsWindowQt* createGraphicsWindow( int x, int y, int w, int h, const std::string& name="", bool windowDecoration=false );

    virtual void paintEvent( QPaintEvent* event );

    void setScene(osg::Group* root);
    void setScene(std::string &filename);
    void setDEMFile(std::string &filename);
    void closeScene();
    osg::Camera *getCamera();

    void setTwoPointMeaModel();
    void setMultPointMeaModel();
    void setAreaMeaModel();
    void setVisibilityMode();
    void setGetCoordMode();
    void setEarthWorkVolumeMode();
    void setPointSlopMode();
    void setLineSlopMode();

    void clearMeasurement();
private:
    void init();
    osg::ref_ptr<osg::LightSource> createLight();
    void showPosFlag(float p_x, float p_y, float p_z);

    PickHandler *m_pickhandler;
    QString demMetafile;
    QString imageInfofile;
    QString strImgProj4;
    double pixelRange;
    double pixelResolve;

protected:
    QTimer _timer;
    osg::ref_ptr<osgViewer::View> _viewer;
    osg::ref_ptr<osg::Group> m_root;
    osg::ref_ptr<osg::Group> m_measure;

signals:
    void on_mouse_event(QEvent*);
    void on_key_event(QEvent*);

    void SIGNAL_ShowMeasureResult(QString,QString);

public slots:
    void slot_drawLine(osg::Vec3d v1, osg::Vec3d v2);
    void slot_drawTrangle(osg::Vec3d v1, osg::Vec3d v2,osg::Vec3d v3);

    void slot_calDisTwoPoint(osg::Vec3d v1, osg::Vec3d v2);
    void slot_calDisMultPoint(QVector<osg::Vec3d> qv);
    void slot_calArea(QVector<osg::Vec3d> qv);
    void slot_visibility(osg::Vec3d v1, osg::Vec3d v2);
    void slot_getCoord(osg::Vec3d v);
    void slot_calEarthWorkVolume(QVector<osg::Vec3d> qv);
    void slot_calPointSlop(osg::Vec3d v1);
    void slot_calLineSlop(osg::Vec3d v1, osg::Vec3d v2);

    // QObject interface
public:
    bool eventFilter(QObject *, QEvent *e);

private:
    osg::ref_ptr<osg::Node> createDEMTerrain(std::string filename);
    osg::Vec3d getWorldCoord(osg::Vec3d pos);
    osg::Vec3d getProjCoord(osg::Vec3d pos);
    int getBand6Zone(double longitude);
    double getPointHigh(double x,double y);
    void setTravelManipulator();
    void getImageProj4(QString &imageInfo);

    bool is_myTravelManipulator;
};

#endif // VIEWERWIDGET_H
