#include "measureanalysis.h"

MeasureAnalysis::MeasureAnalysis()
{

}

MeasureAnalysis::~MeasureAnalysis()
{

}

MeasureAnalysis *MeasureAnalysis::intance()
{
    static MeasureAnalysis *ma=nullptr;
    if(!ma)
    {
        ma=new MeasureAnalysis;
    }
    return ma;
}

double MeasureAnalysis::getDistanceTwoPoint(osg::Vec3d &v1, osg::Vec3d &v2)
{
    double distance = 0;//返回值
    distance = (double)(sqrt((double)( (v2[0] - v1[0]) * (v2[0] - v1[0])) +
                                        (double)( (v2[1] - v1[1]) * (v2[1] - v1[1]) ) +
                                        (double)( (v2[2] - v1[2]) * (v2[2] - v1[2]) )));
    return distance;
}

double MeasureAnalysis::getDistanceMultPoint(QVector<osg::Vec3d> qv)
{
    double distance = 0;//返回值
    for(int i=0;i<qv.size()-1;++i)
    {
        distance+=getDistanceTwoPoint(qv[i],qv[i+1]);
    }
    return distance;
}

double MeasureAnalysis::getTriangleArea(osg::Vec3d &v1, osg::Vec3d &v2, osg::Vec3d &v3)
{
    double area=0.0;
    area=abs(0.5 * ( (v2.x() - v1.x())*(v3.y() - v1.y())  - ( v3.x() - v1.x() )*( v2.y() - v1.y() ) ));
//    area=0.5*(abs(v1[0]*v2[1]+v2[0]*v3[1]+v3[0]*v1[0]
//            -v1[0]*v3[1]-v2[0]*v1[1]-v3[0]*v2[1]));
    return area;
}

/*点在多边形内的判断,必须为封闭多边形，即首尾点相同
    返回值：-1不在多边形内，0在多边形顶点上，1在多边形内; */
int MeasureAnalysis::Is_Point_in_Polygon(double x, double y, QVector<osg::Vec3d> ptIn)
{
    int n = ptIn.size();//多边形点个数

    double *ployX = new double[n];//[n];
    double *ployY = new double[n];//[n];//分别存储x和y值的数组
    for (int i= 0; i< n; i++)
    {
        ployX[i] = ptIn[i].x();
        ployY[i] = ptIn[i].y();
    }

    //

    int i,count;
    double Xmin,Ymin,Xmax,Ymax,xx;
    double x0,x1,y0,y1,k;

    Xmin=999999999.0; Ymin=999999999.0;
    Xmax=-999999999.0;Ymax=-999999999.0;

    for(i=0;i<n;i++){
       if(x==ployX[i]&&y==ployY[i]) return 0;
       if(Xmin>ployX[i]) Xmin=ployX[i];
       if(Ymin>ployY[i]) Ymin=ployY[i];
       if(Xmax<ployX[i]) Xmax=ployX[i];
       if(Ymax<ployY[i]) Ymax=ployY[i];
    }
   if(x<Xmin||x>Xmax||y<Ymin||y>Ymax) return -1;

   count=0;
   for(i=0;i<n-1;i++){
       x0=ployX[i];x1=ployX[i+1];
       y0=ployY[i];y1=ployY[i+1];

       Xmax=qMax(x0,x1);
       Ymin=qMin(y1,y0);
       Ymax=qMax(y1,y0);

       if(y>Ymax||y<Ymin||x>Xmax) continue;
       if(y==Ymin) continue;

       if(x0==x1){
           if(x>x0)
               return -1;
           else
               count++;
       }
       else{
           if(y0==y1) return 1;
           k=(y1-y0)/(x1-x0);
           xx=x0+(y-y0)/k;
           if(x<xx) count++;
       }
   }

   delete[] ployX;
   delete[] ployY;

   if(count%2)
       return 1;
   else
       return -1;
}

double MeasureAnalysis::getPointSlop(QVector<osg::Vec3d> ptIn)
{

    if(ptIn.size()!=9)
        return 0.0;
    double x1,y1,x2,y2;
    double dx,dy,m_log;
    dx=0.000009;
    dy=0.000009;
    m_log=sqrt(dx*dx+dy*dy);
    double s[9],ss;
    s[0]=atan((ptIn[0][2]-ptIn[4][2])/m_log);
    s[1]=atan((ptIn[1][2]-ptIn[4][2])/dy);
    s[2]=atan((ptIn[2][2]-ptIn[4][2])/m_log);
    s[3]=atan((ptIn[3][2]-ptIn[4][2])/dx);
    s[4]=0.0;
    s[5]=atan((ptIn[5][2]-ptIn[4][2])/dx);
    s[6]=atan((ptIn[6][2]-ptIn[4][2])/m_log);
    s[7]=atan((ptIn[7][2]-ptIn[4][2])/dy);
    s[8]=atan((ptIn[8][2]-ptIn[4][2])/m_log);

    ss=-99999.0;

    for(int i=0;i<9;i++){
            if(s[i]<0.0) s[i]=-s[i];
            if(ss<s[i]) ss=s[i];
    }
    ss=ss*180.0/(osg::PI);
    return ss;
}

double MeasureAnalysis::getLineSlop(osg::Vec3d &v1, osg::Vec3d &v2)
{
    double m_log=sqrt((v2[0]-v1[0])*(v2[0]-v1[0])+(v2[1]-v1[1])*(v2[1]-v1[1]));
    double slop=atan(abs(v2[2]-v1[2])/m_log);
    slop=slop*180.0/(osg::PI);
    if(slop<0.0)
        slop=-slop;
    return slop;
}

