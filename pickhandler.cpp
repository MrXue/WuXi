#include "pickhandler.h"


PickHandler::PickHandler():m_x(0.0),m_y(0.0),
    b_twopoint(false),b_multpoint(false),b_area(false),b_visibility(false),
    b_getcoord(false),b_earthvolume(false),b_pointslop(false),b_lineslop(false)
{

}

PickHandler::~PickHandler()
{

}

bool PickHandler::handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
    osg::ref_ptr<osgViewer::View> view = dynamic_cast<osgViewer::View*>(&aa);
    if (!view)
        return false;

    switch (ea.getEventType())
    {
        //鼠标按下
    case(osgGA::GUIEventAdapter::PUSH) :
    {
        //更新鼠标位置
        m_x = ea.getX();
        m_y = ea.getY();
//        pick(view.get(), ea.getX(), ea.getY());
        break;

    }
    case(osgGA::GUIEventAdapter::RELEASE) :
    {
//        if (m_x == ea.getX() && m_y == ea.getY())
//        {
//            //执行对象选取

//        }
        if(ea.getButton()==1)
        {
            //LEFT_MOUSE_BUTTON
            pick(view.get(), ea.getX(), ea.getY());
        }
        else if (ea.getButton()==2)
        {
            //MIDDLE_MOUSE_BUTTON
        }
        else if (ea.getButton()==4)
        {
            //RIGHT_MOUSE_BUTTON
            pick(view.get(), ea.getX(), ea.getY());
//            b_twopoint=false;
//            b_visibility=false;
//            b_getcoord=false;
//            b_pointslop=false;
//            b_lineslop=false;
            if(b_multpoint)
            {
                emit SIGNAL_DisMultPoint(m_point);
                m_point.clear();
//                b_multpoint=false;
            }
            if(b_area)
            {
                emit SIGNAL_Area(m_point);
                m_point.clear();
//                b_area=false;
            }
            if(b_earthvolume)
            {
                emit SIGNAL_EarthWorkVolume(m_point);
                m_point.clear();
//                b_earthvolume=false;
            }
        }
        break;
    }
    default:
        break;
    }
    return false;
}

void PickHandler::pick(osg::ref_ptr<osgViewer::View> view, float x, float y)
{
    osgUtil::LineSegmentIntersector::Intersections intersections;
    if (view->computeIntersections(x, y, intersections))
    {
        osgUtil::LineSegmentIntersector::Intersection intersection = *intersections.begin();
        if(b_twopoint)
        {
            osg::Vec3d pos_world=intersection.getWorldIntersectPoint();
//            intersection.
            m_point.push_back(pos_world);
            if(m_point.size()==2)
            {
//                b_twopoint=false;
                emit SIGNAL_DrawLine(m_point[0],m_point[1]);
                emit SIGNAL_DisTwoPoint(m_point[0],m_point[1]);
                m_point.clear();
            }
        }
        else if (b_multpoint)
        {
            osg::Vec3d pos_world=intersection.getWorldIntersectPoint();
            m_point.push_back(pos_world);
            int point_size=m_point.size();
            if(point_size != 0 && point_size != 1)
            {
                emit SIGNAL_DrawLine(m_point[point_size-2],m_point[point_size-1]);
            }
        }
        else if (b_area)
        {
            osg::Vec3d pos_world=intersection.getWorldIntersectPoint();
            m_point.push_back(pos_world);
            int point_size=m_point.size();
            if(point_size==2)
            {
                emit SIGNAL_DrawLine(m_point[0],m_point[1]);
            }
            if(point_size>=3)
            {
                emit SIGNAL_DrawTrangle(m_point[0],m_point[point_size-2],m_point[point_size-1]);
            }
        }
        else if (b_earthvolume)
        {
            osg::Vec3d pos_world=intersection.getWorldIntersectPoint();
            m_point.push_back(pos_world);
            int point_size=m_point.size();
            if(point_size==2)
            {
                emit SIGNAL_DrawLine(m_point[0],m_point[1]);
            }
            if(point_size>=3)
            {
                emit SIGNAL_DrawTrangle(m_point[0],m_point[point_size-2],m_point[point_size-1]);
            }
        }
        else if (b_visibility)
        {
            osg::Vec3d pos_world=intersection.getWorldIntersectPoint();
            pos_world[2];
            m_point.push_back(pos_world);
            int point_size=m_point.size();
            if(point_size==2)
            {
                emit SIGNAL_DrawLine(m_point[0],m_point[1]);
                emit SIGNAL_Visibility(m_point[0],m_point[1]);
//                b_visibility=false;
                m_point.clear();
            }
        }
        else if (b_getcoord)
        {
            osg::Vec3d pos_world=intersection.getWorldIntersectPoint();
            emit SIGNAL_GetCoord(pos_world);
        }
        else if (b_pointslop)
        {
            osg::Vec3d pos_world=intersection.getWorldIntersectPoint();
            emit SIGNAL_PointSlop(pos_world);
        }
        else if (b_lineslop)
        {
            osg::Vec3d pos_world=intersection.getWorldIntersectPoint();
            m_point.push_back(pos_world);
            int point_size=m_point.size();
            if(point_size==2)
            {
                emit SIGNAL_DrawLine(m_point[0],m_point[1]);
                emit SIGNAL_LineSlop(m_point[0],m_point[1]);
//                b_lineslop=false;
                m_point.clear();
            }
        }
    }
}

void PickHandler::setTwoPointMeasure()
{
    b_area=false;
    b_earthvolume=false;
    b_getcoord=false;
    b_lineslop=false;
    b_multpoint=false;
    b_pointslop=false;
//    b_twopoint=false;
    b_visibility=false;

    if(b_twopoint==false)
        b_twopoint=true;
}

void PickHandler::setMultPointMeasure()
{
    b_area=false;
    b_earthvolume=false;
    b_getcoord=false;
    b_lineslop=false;
//    b_multpoint=false;
    b_pointslop=false;
    b_twopoint=false;
    b_visibility=false;

    if(b_multpoint==false)
        b_multpoint=true;
}

void PickHandler::setAreaMeasure()
{
//    b_area=false;
    b_earthvolume=false;
    b_getcoord=false;
    b_lineslop=false;
    b_multpoint=false;
    b_pointslop=false;
    b_twopoint=false;
    b_visibility=false;

    if(b_area==false)
        b_area=true;
}

void PickHandler::setVisibility()
{
    b_area=false;
    b_earthvolume=false;
    b_getcoord=false;
    b_lineslop=false;
    b_multpoint=false;
    b_pointslop=false;
    b_twopoint=false;
//    b_visibility=false;

    if(!b_visibility)
        b_visibility=true;
}

void PickHandler::setGetCoord()
{
    b_area=false;
    b_earthvolume=false;
//    b_getcoord=false;
    b_lineslop=false;
    b_multpoint=false;
    b_pointslop=false;
    b_twopoint=false;
    b_visibility=false;

    if(!b_getcoord)
        b_getcoord=true;
}

void PickHandler::setEarthworkVolume()
{
    b_area=false;
//    b_earthvolume=false;
    b_getcoord=false;
    b_lineslop=false;
    b_multpoint=false;
    b_pointslop=false;
    b_twopoint=false;
    b_visibility=false;

    if(!b_earthvolume)
        b_earthvolume=true;
}

void PickHandler::setPointSlop()
{
    b_area=false;
    b_earthvolume=false;
    b_getcoord=false;
    b_lineslop=false;
    b_multpoint=false;
//    b_pointslop=false;
    b_twopoint=false;
    b_visibility=false;

    if(!b_pointslop)
        b_pointslop=true;
}

void PickHandler::setLineSlop()
{
    b_area=false;
    b_earthvolume=false;
    b_getcoord=false;
//    b_lineslop=false;
    b_multpoint=false;
    b_pointslop=false;
    b_twopoint=false;
    b_visibility=false;

    if(!b_lineslop)
        b_lineslop=true;
}

void PickHandler::setMeasureEnd()
{
    b_area=false;
    b_earthvolume=false;
    b_getcoord=false;
    b_lineslop=false;
    b_multpoint=false;
    b_pointslop=false;
    b_twopoint=false;
    b_visibility=false;
    m_point.clear();
}
