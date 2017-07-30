#ifndef PICKHANDLER_H
#define PICKHANDLER_H

#include <QObject>
#include <QVector>

#include <osgGA/GUIEventHandler>

#include <osgViewer/Viewer>

class PickHandler:public QObject ,public osgGA::GUIEventHandler
{
    Q_OBJECT
public:
    PickHandler();
    ~PickHandler();

    bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);
    //对象选取时间处理器
    void pick(osg::ref_ptr<osgViewer::View> view, float x, float y);

    void setTwoPointMeasure();
    void setMultPointMeasure();
    void setAreaMeasure();
    void setVisibility();
    void setGetCoord();
    void setEarthworkVolume();
    void setPointSlop();
    void setLineSlop();

    void setMeasureEnd();

private:
    float m_x;
    float m_y;

    //两点测距的判断条件
    bool b_twopoint;

    //多点测距
    bool b_multpoint;

    //面积量算
    bool b_area;

    //通视分析
    bool b_visibility;

    //获得选择位置点的坐标
    bool b_getcoord;

    //土方量测量，多边形可以有多个点
    bool b_earthvolume;

    //点坡度
    bool b_pointslop;

    //线坡度
    bool b_lineslop;

    //存储点
    QVector<osg::Vec3d> m_point;
signals:
    void SIGNAL_DrawLine(osg::Vec3d,osg::Vec3d);
    void SIGNAL_DrawTrangle(osg::Vec3d,osg::Vec3d,osg::Vec3d);

    void SIGNAL_DisTwoPoint(osg::Vec3d,osg::Vec3d);
    void SIGNAL_DisMultPoint(QVector<osg::Vec3d>);
    void SIGNAL_Area(QVector<osg::Vec3d>);
    void SIGNAL_Visibility(osg::Vec3d,osg::Vec3d);
    void SIGNAL_GetCoord(osg::Vec3d);
    void SIGNAL_EarthWorkVolume(QVector<osg::Vec3d>);
    void SIGNAL_PointSlop(osg::Vec3d);
    void SIGNAL_LineSlop(osg::Vec3d,osg::Vec3d);
};

#endif // PICKHANDLER_H
