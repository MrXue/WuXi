#include "mytravelmanipulator.h"
#include <osg/Quat>

MyTravelManipulator::MyTravelManipulator()
{
//    computeHomePosition();
    m_eye=_homeEye;//osg::Vec3(0.0,-1.0,0.0);
    m_center=_homeCenter;//osg::Vec3(0.0,0.0,0.0);
    m_up=_homeUp;//osg::Vec3(0.0,0.0,1.0);
}

MyTravelManipulator::~MyTravelManipulator()
{

}

void MyTravelManipulator::setByMatrix(const osg::Matrixd &matrix)
{
    osg::Vec3 postion=matrix.getTrans();

}

void MyTravelManipulator::setByInverseMatrix(const osg::Matrixd &matrix)
{

}

osg::Matrixd MyTravelManipulator::getMatrix() const
{
    osg::Matrix m;
    return m;
}

osg::Matrixd MyTravelManipulator::getInverseMatrix() const
{
    osg::Matrix m;
    return m;
}

bool MyTravelManipulator::handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &us)
{
    return false;
}

void MyTravelManipulator::setHomePosition(const osg::Vec3d &eye, const osg::Vec3d &center, const osg::Vec3d &up, bool autoComputeHomePosition)
{
    _homeEye=eye;
    _homeCenter=center;
    _homeUp=up;
}


