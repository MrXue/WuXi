#include "travelmanipulator.h"
#include <QDebug>

TravelManipulator::TravelManipulator(): m_fMoveSpeed(0.000009)
  , m_bLeftButtonDown(false)
  ,m_bRightButtonDown(false)
  ,m_bMiddleButtonDown(false)
  , m_fpushX(0)
  , m_fpushY(0)
  , m_fAngle(1.5)
  , m_bPeng(false)
{
    m_vPosition = osg::Vec3(500.0f, 500.0f, 300.0f);//osg::Vec3(0.0f, 0.0f, 0.0f);
    m_vRotation = osg::Vec3(osg::PI_2, 0.0f, 0.0f);//osg::PI_2
}

TravelManipulator::~TravelManipulator()
{

}

void TravelManipulator::quatToHPR(osg::Quat q, double &heading, double &pitch, double &roll)
{
    double test = q.y() * q.z() + q.x() * q.w();
    if (test > 0.4999)
    { // singularity at north pole
        heading = 2.0 * atan2(q.y(), q.w());
        pitch   = osg::PI_2;
        roll    = 0.0;
        return;
    }
    if (test < -0.4999)
    { // singularity at south pole
        heading = 2.0 * atan2(q.y(), q.w());
        pitch   = -osg::PI_2;
        roll    = 0.0;
        return;
    }
    double sqx = q.x() * q.x();
    double sqy = q.y() * q.y();
    double sqz = q.z() * q.z();
    heading = atan2(2.0 * q.z() * q.w() - 2.0 * q.y() * q.x(), 1.0 - 2.0 * sqz - 2.0 * sqx);
    pitch   = asin(2.0 * test);
    roll    = atan2(2.0 * q.y() * q.w() - 2.0 * q.z() * q.x(), 1.0 - 2.0 * sqy - 2.0 * sqx);
//    if(heading>-3.14160 && heading<-3.14158)
//        heading=0.0;
//    if(pitch>-3.14160 && pitch<-3.14158)
//        pitch=0.0;
//    if(roll>-3.14160 && roll<-3.14158)
//        roll=0.0;
//    if(heading>3.14158 && heading<3.14160)
//        heading=0.0;
//    if(pitch>3.14158 && pitch<3.14160)
//        pitch=0.0;
//    if(roll>3.14158 && roll<3.14160)
//        roll=0.0;

}
// 设置矩阵
void TravelManipulator::setByMatrix(const osg::Matrixd &matrix)
{
    osg::Quat rot=matrix.getRotate();

//    double anglex=0.0;
//    rot.getRotate(anglex,osg::Vec3d(1.0,0.0,0.0));
//    double angley=0.0;
//    rot.getRotate(angley,osg::Vec3d(0.0,1.0,0.0));
//    double anglez=0.0;
//    rot.getRotate(anglez,osg::Vec3d(0.0,0.0,1.0));
////    m_vRotation=rot.asVec3();
//    m_vRotation[0]=anglex;
//    m_vRotation[1]=angley;
//    m_vRotation[2]=anglez;

     m_vPosition=matrix.getTrans();

    double roll,pitch,yaw=0.0;
    quatToHPR(rot,yaw,pitch,roll);
    m_vRotation._v[0]=pitch;
    m_vRotation._v[1]=roll;
    m_vRotation._v[2]=yaw;
//    qDebug()<<pitch<<"    "<<roll<<"    "<<yaw;
        //     qDebug()<<m_vRotation[0];
        //     qDebug()<<m_vRotation[1];
        //     qDebug()<<m_vRotation[2];
        //     qDebug()<<m_vPosition[0];
        //     qDebug()<<m_vPosition[1];
        //     qDebug()<<m_vPosition[2];
}
//设置逆矩阵
void TravelManipulator::setByInverseMatrix(const osg::Matrixd &matrix)
{
//    osg::Matrixd inmatrix = osg::Matrix::inverse(matrix);
    setByMatrix(osg::Matrix::inverse(matrix));

}

osg::Matrixd TravelManipulator::getMatrix() const
{
    osg::Matrixd mat;

    mat.makeRotate(m_vRotation._v[0], osg::Vec3(1.0f, 0.0f, 0.0f),

        m_vRotation._v[1], osg::Vec3(0.0f, 1.0f, 0.0f),

        m_vRotation._v[2], osg::Vec3(0.0f, 0.0f, 1.0f));

    return mat * osg::Matrixd::translate(m_vPosition);
}

osg::Matrixd TravelManipulator::getInverseMatrix() const
{
    osg::Matrixd mat;

    mat.makeRotate(m_vRotation._v[0], osg::Vec3(1.0f, 0.0f, 0.0f),

        m_vRotation._v[1], osg::Vec3(0.0f, 1.0f, 0.0f),

        m_vRotation._v[2], osg::Vec3(0.0f, 0.0f, 1.0f));

    return osg::Matrixd::inverse(mat * osg::Matrixd::translate(m_vPosition));
}

bool TravelManipulator::handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &us)
{
    //得到鼠标的位置
    float mouseX = ea.getX();
    float mouseY = ea.getY();
    switch(ea.getEventType())
    {
    case(osgGA::GUIEventAdapter::KEYDOWN):
        {
            //空格键
            if (ea.getKey() == 0x20)
            {
                us.requestRedraw();
                us.requestContinuousUpdate(false);
                return true;
            }
            //上移动0xFF50:Home
            if (ea.getKey() == 0xFF50)
            {
                ChangePosition(osg::Vec3 (0, 0, m_fMoveSpeed)) ;
                return true;
            }
            //下移动0xFF57:End
            if (ea.getKey() == 0xFF57)
            {
                ChangePosition(osg::Vec3 (0, 0, -m_fMoveSpeed)) ;
                return true;
            }
            //增加速度,+0x2b
            if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Plus)
            {
                m_fMoveSpeed+=5.0f;
                return true;
            }
            //减少速度,Insert0x2D
            if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Minus)
            {
                m_fMoveSpeed -= 5.0f;
                if (m_fMoveSpeed < 1.0f)
                {
                    m_fMoveSpeed = 1.0f;
                }
                return true;
            }
            //前进0x77:F8,0xFF52:Up,0x57:W
            if (ea.getKey () == 0x57 || ea.getKey () == 0x77)//up
            {
                ChangePosition(osg::Vec3 (0, m_fMoveSpeed * sinf(osg::PI_2+m_vRotation._v[2]), 0)) ;
                ChangePosition(osg::Vec3 (m_fMoveSpeed * cosf(osg::PI_2+m_vRotation._v[2]), 0, 0)) ;
                return true;
            }
            //后退0x73:F4,0xFF54:Down,0x53:S
            if (ea.getKey () == 0x53 || ea.getKey () == 0x73 )//down
            {
                ChangePosition(osg::Vec3 (0, -m_fMoveSpeed * sinf(osg::PI_2+m_vRotation._v[2]), 0)) ;
                ChangePosition(osg::Vec3(-m_fMoveSpeed * cosf(osg::PI_2+m_vRotation._v[2]), 0, 0)) ;
                return true;
            }

            //向左，0x41:A,0x61:1
            if (ea.getKey () == 0x41||ea.getKey () == 0x61)
            {
                ChangePosition(osg::Vec3 (0, m_fMoveSpeed * cosf(osg::PI_2+m_vRotation._v[2]), 0)) ;
                ChangePosition(osg::Vec3 (-m_fMoveSpeed * sinf(osg::PI_2+m_vRotation._v[2]), 0, 0)) ;
                return true;
            }
            //向右0x64:4,0x44:D
            if (ea.getKey () == 0x44||ea.getKey () == 0x64)
            {
                ChangePosition(osg::Vec3 (0,-m_fMoveSpeed * cosf(osg::PI_2+m_vRotation._v[2]), 0)) ;
                ChangePosition(osg::Vec3 (m_fMoveSpeed * sinf(osg::PI_2+m_vRotation._v[2]), 0, 0)) ;
                return true;
            }
            //Up
            if(ea.getKey()==0xFF52)
            {
                //俯仰pitch
                m_vRotation._v[0] += osg::DegreesToRadians(m_fAngle);
            }
            //Down
            if(ea.getKey() == 0xFF54)
            {
                m_vRotation._v[0] -= osg::DegreesToRadians(m_fAngle);
            }
            //Right
            if (ea.getKey() == 0xFF53)
            {
                //偏航yaw
                m_vRotation._v[2] -= osg::DegreesToRadians(m_fAngle);
            }
            //Left
            if (ea.getKey()== 0xFF51)
            {
                m_vRotation._v[2] += osg::DegreesToRadians(m_fAngle);
            }
            if(ea.getKey()==osgGA::GUIEventAdapter::KEY_K)
            {
                //侧滚roll
                m_vRotation._v[1] += osg::DegreesToRadians(m_fAngle);
            }
            if(ea.getKey()==osgGA::GUIEventAdapter::KEY_L)
            {
                m_vRotation._v[1] -= osg::DegreesToRadians(m_fAngle);
            }
            //改变屏角0x66:6,0x46:F
            if (ea.getKey() == 0x46 || ea.getKey() == 0x66)
            {
                m_fAngle -= 0.2 ;
                return true ;
            }
            //0x47:G,0x67:7
            if (ea.getKey() == 0x47 || ea.getKey() == 0x67)//G
            {
                m_fAngle += 0.2 ;
                return true ;
            }
            return false;
        }
    default:
        return false;
    }
}

void TravelManipulator::ChangePosition(osg::Vec3 &delta)
{
    //碰撞检测
    if (m_bPeng)
    {
        //得到新的位置
        osg::Vec3 newPos1 = m_vPosition + delta;

        osgUtil::IntersectVisitor ivXY;
        //根据新的位置得到两条线段检测
        osg::ref_ptr<osg::LineSegment> lineXY = new osg::LineSegment(newPos1,
            m_vPosition);

        osg::ref_ptr<osg::LineSegment> lineZ = new osg::LineSegment(newPos1+osg::Vec3(0.0f,0.0f,10.0f),
            newPos1-osg::Vec3(0.0f,0.0f,-10.0f)) ;

        ivXY.addLineSegment(lineZ.get()) ;

        ivXY.addLineSegment(lineXY.get()) ;
        //结构交集检测
        m_pHostView->getSceneData()->accept(ivXY) ;
        //如果没有碰撞检测
        if(!ivXY.hits())
        {
            m_vPosition += delta;
        }
    }
    else
    {
        m_vPosition += delta;
    }
}

float TravelManipulator::getSpeed()
{
    return m_fMoveSpeed;
}

void TravelManipulator::setSpeed(double &sp)
{
    m_fMoveSpeed = sp ;
}

void TravelManipulator::SetPosition(osg::Vec3 &position)
{
    m_vPosition = position;
}

void TravelManipulator::SetRotate(osg::Vec3 &rotate)
{
    m_vRotation=rotate;
//    m_vRotation.x()=m_vPosition.x()+osg::PI_2;
//    m_vRotation._v[0]+=osg::PI_2;
}

void TravelManipulator::SetViewer(osg::ref_ptr<osgViewer::View> v)
{
    m_pHostView = v;
}

osg::Vec3 TravelManipulator::GetPosition()
{
    return m_vPosition;
}

osg::Vec3 TravelManipulator::GetRotate()
{
    return m_vRotation;
}

