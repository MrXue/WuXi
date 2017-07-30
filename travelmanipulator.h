#ifndef TRAVELMANIPULATOR_H
#define TRAVELMANIPULATOR_H
#include <osgViewer/Viewer>
#include <osgViewer/View>
#include <osg/LineSegment>
#include <osg/Point>
#include <osg/Geometry>
#include <osg/Node>
#include <osg/Geode>
#include <osg/Group>

#include <osgGA/CameraManipulator>

#include <osgUtil/IntersectVisitor>

class TravelManipulator : public osgGA::CameraManipulator
{
public:
    TravelManipulator();
    ~TravelManipulator();

private:
    osg::ref_ptr <osgViewer::View>	m_pHostView;

    //移动速度
    double m_fMoveSpeed;
    //
    osg::Vec3 m_vPosition;
    //
    osg::Vec3 m_vRotation; 

    //鼠标左键是否按下
    bool m_bLeftButtonDown;
    //鼠标右键是否按下
    bool m_bRightButtonDown;

    bool m_bMiddleButtonDown;
    //鼠标X,Y
    float m_fpushY;

    float m_fpushX;

    //碰撞检测是否开启
    bool m_bPeng;

    // 屏目角度
    float m_fAngle;

public:
    void quatToHPR(osg::Quat q,double &heading,double &pitch,double &roll);
    //设置矩阵
    virtual void setByMatrix(const osg::Matrixd& matrix);
    //设置逆矩阵
    virtual void setByInverseMatrix(const osg::Matrixd& matrix);
    //得到矩阵
    virtual osg::Matrixd getMatrix(void) const;
    //得到逆矩阵
    virtual osg::Matrixd getInverseMatrix(void)const ;

    //事件处理函数
    virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us);
    // 位置变换函数
    void ChangePosition(osg::Vec3& delta);
    //设置速度
    float getSpeed() ;
    void  setSpeed(double &sp) ;
    //设置起始位置
    void SetPosition(osg::Vec3 &position) ;
    void SetRotate(osg::Vec3 &rotate);
    void SetViewer(osg::ref_ptr<osgViewer::View> v);

    osg::Vec3 GetPosition();
    osg::Vec3 GetRotate(); 
};

#endif // TRAVELMANIPULATOR_H
