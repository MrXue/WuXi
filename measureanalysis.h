#ifndef MEASUREANALYSIS_H
#define MEASUREANALYSIS_H

#include <QVector>

#include <osg/Vec3>
#include <osg/Vec3d>

class MeasureAnalysis
{
public:
    MeasureAnalysis();
    ~MeasureAnalysis();
    static MeasureAnalysis *intance();


    //两点距离
    double getDistanceTwoPoint(osg::Vec3d &v1,osg::Vec3d &v2);
    //多点距离
    double getDistanceMultPoint(QVector<osg::Vec3d> qv);
    //面积量测
    double getTriangleArea(osg::Vec3d &v1,osg::Vec3d &v2,osg::Vec3d &v3);

    //点x,y是否在ptIn所连接的多边形中
    int Is_Point_in_Polygon(double x, double y,QVector<osg::Vec3d> ptIn);

    //点坡度
    double getPointSlop(QVector<osg::Vec3d> ptIn);

    //线坡度，其中需要知道线段的两个端点
    double getLineSlop(osg::Vec3d &v1,osg::Vec3d &v2);
};

#endif // MEASUREANALYSIS_H
