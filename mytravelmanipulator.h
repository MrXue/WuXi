#ifndef MYTRAVELMANIPULATOR_H
#define MYTRAVELMANIPULATOR_H

#include <osgGA/CameraManipulator>
#include <osg/Node>

class MyTravelManipulator : public osgGA::CameraManipulator
{
public:
    MyTravelManipulator();
    ~MyTravelManipulator();

    // CameraManipulator interface
public:
    void setByMatrix(const osg::Matrixd &matrix);
    void setByInverseMatrix(const osg::Matrixd &matrix);
    osg::Matrixd getMatrix() const;
    osg::Matrixd getInverseMatrix() const;
    virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us);


//    //获取传入节点，用于使用CameraManipulator中的computeHomePosition
//     virtual const osg::Node* getNode() const { return _root; }
//     virtual osg::Node* getNode() { return _root;  }

private:
    osg::Vec3   m_eye;               //视点位置
    osg::Vec3   m_center;         //
    osg::Vec3   m_up;                //向上方向
//    osg::Node*  _root;


    // CameraManipulator interface
public:
    void setHomePosition(const osg::Vec3d &eye, const osg::Vec3d &center, const osg::Vec3d &up, bool autoComputeHomePosition=false);
};

#endif // MYTRAVELMANIPULATOR_H
