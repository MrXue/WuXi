#include "viewerwidget.h"
#include <QGridLayout>
#include <QDebug>
#include <QFile>
#include <QProgressDialog>
#include <QCoreApplication>

#include <osgDB/ReadFile>

#include <osg/LineSegment>
#include <osg/MatrixTransform>
#include <osgUtil/IntersectVisitor>
#include <osgTerrain/Terrain>
#include <osgTerrain/Layer>
#include <osgTerrain/TerrainTile>
#include <osgTerrain/GeometryTechnique>

#include "measureanalysis.h"
#include "travelmanipulator.h"
#include "mytravelmanipulator.h"

#include <GDAL/ogrsf_frmts.h>
#include <GDAL/gdal_priv.h>


ViewerWidget::ViewerWidget(QWidget *parent, Qt::WindowFlags f, osgViewer::ViewerBase::ThreadingModel threadingModel): QWidget(parent, f)
{
    pixelRange=0.000009;
    pixelResolve=0.0;
    m_pickhandler=nullptr;
    is_myTravelManipulator=false;
    init();
}

ViewerWidget::~ViewerWidget()
{
}

QWidget *ViewerWidget::addViewWidget(osgQt::GraphicsWindowQt *gw)
{
    addView(_viewer);
    osg::Camera* camera = _viewer->getCamera();
    camera->setGraphicsContext( gw );

    const osg::GraphicsContext::Traits* traits = gw->getTraits();
    camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
//    camera->setClearColor( osg::Vec4(0,0,0, 1.0) );
    camera->setClearStencil(0);
    camera->setViewport( new osg::Viewport(0, 0, traits->width, traits->height) );
    camera->setProjectionMatrixAsPerspective(60.0f, static_cast<double>(traits->width)/static_cast<double>(traits->height), 0.001f, 5000.0f );

    return gw->getGLWidget();
}

osgQt::GraphicsWindowQt *ViewerWidget::createGraphicsWindow(int x, int y, int w, int h, const std::string &name, bool windowDecoration)
{
    osg::DisplaySettings* ds = osg::DisplaySettings::instance().get();
    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->windowName = name;
    traits->windowDecoration = windowDecoration;
    traits->x = x;
    traits->y = y;
    traits->width = w;
    traits->height = h;
    traits->doubleBuffer = true;
    traits->alpha = ds->getMinimumNumAlphaBits();
    traits->stencil = ds->getMinimumNumStencilBits();
    traits->sampleBuffers = ds->getMultiSamples();
    traits->samples = ds->getNumMultiSamples();
    traits->sharedContext = 0;

    return new osgQt::GraphicsWindowQt(traits.get());
}

void ViewerWidget::paintEvent(QPaintEvent *event)
{
    frame();
}

void ViewerWidget::setScene(osg::Group *root)
{
    _viewer->setSceneData(root);
//    osgGA::TrackballManipulator * tm = new osgGA::TrackballManipulator;
//    _viewer->setCameraManipulator( tm ,true);
//    tm->home(1);
}

void ViewerWidget::setScene(std::string &filename)
{
    unsigned int size=m_root->getNumChildren();
    m_root->removeChildren(0,size);
    osg::ref_ptr<osg::Node> node=osgDB::readNodeFile(filename);

    QString qstrfile=QString::fromStdString(filename);
    int oivesize=qstrfile.size();
    demMetafile=qstrfile.left(oivesize-4)+".DemInfo";
    imageInfofile=qstrfile.left(oivesize-4)+".ImageInfo";
    getImageProj4(imageInfofile);

    osg::ref_ptr<osg::CoordinateSystemNode> csn=new osg::CoordinateSystemNode;
    csn->setEllipsoidModel(new osg::EllipsoidModel);
    csn->setFormat("PROJ4");
    csn->setCoordinateSystem("+proj=merc +datum=WGS84");
    csn->addChild(node);
    csn->setName("CS");
    m_root->addChild(csn.get());

    osg::ref_ptr<osg::LightSource> ls=createLight();
    m_root->addChild(ls.get());

    //量测结果放到这个节点
    m_measure=new osg::Group();
    m_measure->setName("Measure");
    m_root->addChild(m_measure.get());

    osgGA::TrackballManipulator * tm = new osgGA::TrackballManipulator;
    _viewer->setCameraManipulator( tm ,true);

    tm->home(1);
}

void ViewerWidget::setDEMFile(std::string &filename)
{
    osg::ref_ptr<osg::Node> terrain=createDEMTerrain(filename);

    unsigned int size=m_root->getNumChildren();
    m_root->removeChildren(0,size);
    m_root->addChild(terrain.get());

    osgGA::TrackballManipulator * tm = new osgGA::TrackballManipulator;
    _viewer->setCameraManipulator( tm ,true);
}

void ViewerWidget::closeScene()
{
    unsigned int size=m_root->getNumChildren();
    m_root->removeChildren(0,size);
    m_pickhandler->setMeasureEnd();
}

osg::Camera *ViewerWidget::getCamera()
{
    if(!_viewer){
        return nullptr;
    }
    return _viewer->getCamera();
}

void ViewerWidget::setTwoPointMeaModel()
{
    if(m_pickhandler!=nullptr)
    {
        m_pickhandler->setTwoPointMeasure();
        setTravelManipulator();
    }
}

void ViewerWidget::setMultPointMeaModel()
{
    if(m_pickhandler!=nullptr)
    {
        m_pickhandler->setMultPointMeasure();
        setTravelManipulator();
    }

}

void ViewerWidget::setAreaMeaModel()
{
    if(m_pickhandler!=nullptr)
    {
        m_pickhandler->setAreaMeasure();
        setTravelManipulator();
    }
}

void ViewerWidget::setVisibilityMode()
{
    if(m_pickhandler!=nullptr)
    {
        m_pickhandler->setVisibility();
        setTravelManipulator();
    }
}

void ViewerWidget::setGetCoordMode()
{
    if(m_pickhandler!=nullptr)
    {
        m_pickhandler->setGetCoord();
        setTravelManipulator();
    }

}

void ViewerWidget::setEarthWorkVolumeMode()
{
    if(m_pickhandler!=nullptr)
    {
        m_pickhandler->setEarthworkVolume();
        setTravelManipulator();
    }

}

void ViewerWidget::setPointSlopMode()
{
    if(m_pickhandler!=nullptr)
    {
        m_pickhandler->setPointSlop();
        setTravelManipulator();
    }
}

void ViewerWidget::setLineSlopMode()
{
    if(m_pickhandler!=nullptr)
    {
        m_pickhandler->setLineSlop();
        setTravelManipulator();
    }
}

void ViewerWidget::clearMeasurement()
{
    if(m_measure==nullptr)
        return;
    unsigned int c_size=m_measure->getNumChildren();
    if(c_size == 0)
        return;
    m_measure->removeChildren(0,c_size);
    m_pickhandler->setMeasureEnd();

    osgGA::TrackballManipulator * tm = new osgGA::TrackballManipulator;
//    osg::Vec3 e,c,u;
//    this->getCamera()->getViewMatrixAsLookAt(e,c,u);
//    tm->setHomePosition(e,c,u);
//    tm->setWheelZoomFactor(10.0);
    if(is_myTravelManipulator)
    {
        TravelManipulator *myTM=dynamic_cast<TravelManipulator*>(_viewer->getCameraManipulator());
        osg::Matrixd m=myTM->getMatrix();
        tm->setByMatrix(m);
//        tm->setWheelZoomFactor(10.0);
        is_myTravelManipulator=false;
        _viewer->setCameraManipulator( tm ,false);
    }
    else
    {
        _viewer->setCameraManipulator( tm ,true);
    }


}

void ViewerWidget::init()
{
    if (_viewer == 0)
    {
        m_pickhandler=new PickHandler;
        connect(m_pickhandler,SIGNAL(SIGNAL_DrawLine(osg::Vec3d,osg::Vec3d)),this,SLOT(slot_drawLine(osg::Vec3d,osg::Vec3d)));
        connect(m_pickhandler,SIGNAL(SIGNAL_DrawTrangle(osg::Vec3d,osg::Vec3d,osg::Vec3d)),
                this,SLOT(slot_drawTrangle(osg::Vec3d,osg::Vec3d,osg::Vec3d)));
        connect(m_pickhandler,SIGNAL(SIGNAL_DisTwoPoint(osg::Vec3d,osg::Vec3d)),this,SLOT(slot_calDisTwoPoint(osg::Vec3d,osg::Vec3d)));
        connect(m_pickhandler,SIGNAL(SIGNAL_DisMultPoint(QVector<osg::Vec3d>)),this,SLOT(slot_calDisMultPoint(QVector<osg::Vec3d>)));
        connect(m_pickhandler,SIGNAL(SIGNAL_Area(QVector<osg::Vec3d>)),this,SLOT(slot_calArea(QVector<osg::Vec3d>)));
        connect(m_pickhandler,SIGNAL(SIGNAL_Visibility(osg::Vec3d,osg::Vec3d)),this,SLOT(slot_visibility(osg::Vec3d,osg::Vec3d)));
        connect(m_pickhandler,SIGNAL(SIGNAL_GetCoord(osg::Vec3d)),this,SLOT(slot_getCoord(osg::Vec3d)));
        connect(m_pickhandler,SIGNAL(SIGNAL_EarthWorkVolume(QVector<osg::Vec3d>)),this,SLOT(slot_calEarthWorkVolume(QVector<osg::Vec3d>)));
        connect(m_pickhandler,SIGNAL(SIGNAL_PointSlop(osg::Vec3d)),this,SLOT(slot_calPointSlop(osg::Vec3d)));
        connect(m_pickhandler,SIGNAL(SIGNAL_LineSlop(osg::Vec3d,osg::Vec3d)),this,SLOT(slot_calLineSlop(osg::Vec3d,osg::Vec3d)));

        _viewer = new osgViewer::View();
        _viewer->addEventHandler(m_pickhandler);
//        _viewer->setCameraManipulator(new osgGA::TrackballManipulator);
        setThreadingModel(osgViewer::ViewerBase::SingleThreaded);
        // disable the default setting of viewer.done() by pressing Escape.
        setKeyEventSetsDone(0);


        QWidget* popupWidget = addViewWidget( createGraphicsWindow(0,0,this->width()-2,this->height()-2));
        QGridLayout *grid=new QGridLayout;
//		QHBoxLayout* grid = new QHBoxLayout();
        grid->setMargin(1);
        grid->setSpacing(1);
        popupWidget->installEventFilter(this);
        grid->addWidget(popupWidget);
        setLayout( grid );

        connect( &_timer, SIGNAL(timeout()), this, SLOT(update()) );
        _timer.start( 10 );

        m_root=new osg::Group();
        this->setScene(m_root);

    }
}

osg::ref_ptr<osg::LightSource> ViewerWidget::createLight()
{
    osg::ref_ptr<osg::LightSource> ls = new osg::LightSource();
    osg::ref_ptr< osg::Light> lt  = new osg::Light;
    lt->setLightNum(0);
    lt->setPosition(osg::Vec4(0.0,0.0,1000.0,0));
    lt->setAmbient(osg::Vec4(0.5,0.5,0.5,1.0));
    lt->setDiffuse(osg::Vec4(1.0,1.0,1.0,1.0));
    lt->setDirection(osg::Vec3(0,0,-1));//方向
    ls->setLight(lt);
    return ls;
}

void ViewerWidget::showPosFlag(float p_x, float p_y, float p_z)
{
    osg::ref_ptr<osg::Geode>geode=new osg::Geode();
    osg::ref_ptr<osg::Geometry>geom=new osg::Geometry();
    geode->addDrawable(geom);

    osg::ref_ptr<osg::MatrixTransform>mt=new osg::MatrixTransform();
    mt->setMatrix(osg::Matrix::scale(8.0,8.0,8.0));
    mt->postMult(osg::Matrix::translate(p_x,p_y,p_z));
//    mt->setMatrix(osg::Matrix::translate(p_x,p_y,p_z));
    mt->addChild(geode.get());
    mt->setName("PosFlag");
    unsigned int c_count=m_measure->getNumChildren();
    bool has_posflag=false;
    for(unsigned int i=0;i<c_count;++i)
    {
        if(m_measure->getChild(i)->getName()=="PosFlag")
        {
            m_measure->replaceChild(m_measure->getChild(i),mt.get());
            has_posflag=true;
        }
    }
    if(!has_posflag)
    {
        m_measure->addChild(mt.get());
    }
    //光照模式关闭
    geode->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);

    osg::ref_ptr<osg::Vec3Array> v=new osg::Vec3Array();
    v->push_back(osg::Vec3(-pixelRange,0.0,-pixelRange));
    v->push_back(osg::Vec3(pixelRange,0.0,-pixelRange));
    v->push_back(osg::Vec3(pixelRange,0.0,pixelRange));
    v->push_back(osg::Vec3(-pixelRange,0.0,pixelRange));
    geom->setVertexArray(v.get());

    osg::ref_ptr<osg::Vec4Array> color_v=new osg::Vec4Array();
    color_v->push_back(osg::Vec4(1.0,1.0,1.0,0.0));
    geom->setColorArray(color_v.get());

    osg::ref_ptr<osg::Vec3Array>normal=new osg::Vec3Array();
    normal->push_back(osg::Y_AXIS);
    geom->setNormalArray(normal.get());
    geom->setNormalBinding(osg::Geometry::BIND_OVERALL);

    //指定几何体的纹理坐标
    osg::ref_ptr<osg::Vec2Array>tcoords=new osg::Vec2Array();
    tcoords->push_back(osg::Vec2(0.0f,0.0f));
    tcoords->push_back(osg::Vec2(1.0f,0.0f));
    tcoords->push_back(osg::Vec2(1.0f,1.0f));
    tcoords->push_back(osg::Vec2(0.0f,1.0f));
    geom->setTexCoordArray(0,tcoords.get());

    geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4));

    //贴纹理
    osg::ref_ptr<osg::Texture2D>texture=new osg::Texture2D();
    std::string flag_img="32x32/pos.png";
    osg::ref_ptr<osg::Image> img=osgDB::readImageFile(flag_img);
    texture->setImage(img);
    geom->getOrCreateStateSet()->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);

    //开启混合模式
    geom->getOrCreateStateSet()->setMode(GL_BLEND,osg::StateAttribute::ON);
    geom->getOrCreateStateSet()->setMode( GL_DEPTH_TEST, osg::StateAttribute::OFF );
}

void ViewerWidget::slot_drawLine(osg::Vec3d v1, osg::Vec3d v2)
{
//    qDebug()<<v1[0];
//    qDebug()<<v2[0];
    //osg::ref_ptr<osg::Geometry> geom = new osg::Geometry();
        osg::Geometry* geom = new osg::Geometry();
        osg::ref_ptr<osg::Vec3Array> v = new osg::Vec3Array;
        v->push_back(osg::Vec3(v1[0],v1[1],v1[2]+0.0));
        v->push_back(osg::Vec3(v2[0],v2[1],v2[2]+0.0));//
        geom->setVertexArray(v.get());

//        osg::ref_ptr<osg::Vec4Array> c = new osg::Vec4Array;
//        c->push_back(osg::Vec4(1.0f,1.0f,0.0f,1.0f));

        osg::Vec4Array* colors = new osg::Vec4Array;
        colors->push_back(osg::Vec4(1.0f,0.0f,0.0f,1.0f));
        geom->setColorArray(colors, osg::Array::BIND_OVERALL);
//        geom->setColorArray(c.get(),osg::Array::BIND_OVERALL);
        geom->setColorBinding(osg::Geometry::BIND_OVERALL);
        osg::Vec3Array* normals = new osg::Vec3Array;
        normals->push_back(osg::Vec3(0.0f,-1.0f,0.0f));
        geom->setNormalArray(normals, osg::Array::BIND_OVERALL);

        geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,2));

        osg::ref_ptr<osg::Geode> geode = new osg::Geode;
        //geode->addDrawable(geom.get());
        geode->addDrawable(geom);
        geode->setName("Line");
        m_measure->addChild(geode.get());
        //        return geode.get();
}

void ViewerWidget::slot_drawTrangle(osg::Vec3d v1, osg::Vec3d v2, osg::Vec3d v3)
{
    osg::Geometry* geom = new osg::Geometry();
    osg::ref_ptr<osg::Vec3Array> v = new osg::Vec3Array;
    v->push_back(osg::Vec3(v1[0],v1[1],v1[2]+0.0));
    v->push_back(osg::Vec3(v2[0],v2[1],v2[2]+0.0));//防止穿山而过
    v->push_back(osg::Vec3(v3[0],v3[1],v3[2]+0.0));
    geom->setVertexArray(v.get());

    osg::ref_ptr<osg::Vec4Array> c = new osg::Vec4Array;
    c->push_back(osg::Vec4(1.0f,1.0f,0.0f,1.0f));

    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back(osg::Vec4(1.0f,0.0f,0.0f,1.0f));
    geom->setColorArray(colors, osg::Array::BIND_OVERALL);

    osg::Vec3Array* normals = new osg::Vec3Array;
    normals->push_back(osg::Vec3(0.0f,-1.0f,0.0f));
    geom->setNormalArray(normals, osg::Array::BIND_OVERALL);

    geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES,0,3));
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    //geode->addDrawable(geom.get());
    geode->addDrawable(geom);
    geode->setName("Area");
    m_measure->addChild(geode.get());
}

void ViewerWidget::slot_calDisTwoPoint(osg::Vec3d v1, osg::Vec3d v2)
{
    //如果是经纬度坐标需要这个
//    osg::Vec3d world1=this->getWorldCoord(v1);
//    osg::Vec3d world2=this->getWorldCoord(v2);

    osg::Vec3d coordV1=getProjCoord(v1);
    osg::Vec3d coordV2=getProjCoord(v2);
    double dis=MeasureAnalysis::intance()->getDistanceTwoPoint(coordV1,coordV2);
    QString msg=QString::fromLocal8Bit("两点之间的距离为：\n%1").arg(dis,0,'f',3);
    QString title=QString::fromLocal8Bit("量测结果");
    emit SIGNAL_ShowMeasureResult(msg,title);
}

void ViewerWidget::slot_calDisMultPoint(QVector<osg::Vec3d> qv)
{
//    QVector<osg::Vec3d> world_qv;
//    unsigned int v_size=qv.size();
//    for(unsigned int i=0;i<v_size;++i)
//    {
//        osg::Vec3d world=this->getWorldCoord(qv[i]);
//        world_qv.push_back(world);
//    }
    for(int i=0;i<qv.size();++i)
        qv[i]=getProjCoord(qv[i]);
    double dis=MeasureAnalysis::intance()->getDistanceMultPoint(qv);
    QString msg=QString::fromLocal8Bit("多点之间的距离为：\n%1").arg(dis,0,'f',3);
    QString title=QString::fromLocal8Bit("量测结果");
    emit SIGNAL_ShowMeasureResult(msg,title);
}

void ViewerWidget::slot_calArea(QVector<osg::Vec3d> qv)
{
    double area=0;

//    QVector<osg::Vec3d> world_qv;
    unsigned int v_size=qv.size();
    if(v_size<3)
        return;

    for(int i=0;i<v_size;++i)
    {
        qv[i]=getProjCoord(qv[i]);
    }

//    for(unsigned int i=0;i<v_size;++i)
//    {
//        osg::Vec3d world=this->getWorldCoord(qv[i]);
//        world_qv.push_back(world);
//    }
    for(int i=0;i<v_size-2;++i)
    {
        area+=MeasureAnalysis::intance()->getTriangleArea(qv[0],qv[i+1],qv[i+2]);
    }
    QString msg=QString::fromLocal8Bit("选择区域的面积为：\n%1").arg(area,0,'f',3);
    QString title=QString::fromLocal8Bit("量测结果");
    emit SIGNAL_ShowMeasureResult(msg,title);
}

void ViewerWidget::slot_visibility(osg::Vec3d v1, osg::Vec3d v2)
{
//    qDebug()<<v1[0];
//    qDebug()<<v1[2];
    // 创建需要进行检测的两点之间的线段
    osg::ref_ptr<osg::LineSegment> line = new osg::LineSegment(v1,v2);
    // 创建一个IV
    osgUtil::IntersectVisitor iv;
    // 把线段添加到IV中
    iv.addLineSegment(line);

    unsigned int c_size=m_root->getNumChildren();
    osg::Node *c_node=nullptr;
    for(unsigned int i=0;i<c_size;++i)
    {
        if(m_root->getChild(i)->getName()=="CS")
        {
            c_node=dynamic_cast<osg::Group*>(m_root->getChild(i))->getChild(0);
        }
    }
    if(c_node==nullptr)
        return;
    c_node->accept(iv);
    // 最后可以对iv.hits()的返回值进行判断
    if (iv.hits())
    {
        // 两点间有障碍
        QString msg=QString::fromLocal8Bit("两点不通视！");
        QString title=QString::fromLocal8Bit("通视分析");
        emit SIGNAL_ShowMeasureResult(msg,title);
//        qDebug()<<"NO";
    }
    else
    {
       // 两点间无障碍
//        qDebug()<<"YES";
        QString msg=QString::fromLocal8Bit("两点通视！");
        QString title=QString::fromLocal8Bit("通视分析");
        emit SIGNAL_ShowMeasureResult(msg,title);
    }
}

void ViewerWidget::slot_getCoord(osg::Vec3d v)
{
    osg::Vec3d coordV1=getProjCoord(v);
    QString msg=QString::fromLocal8Bit("选择点坐标为：\n(%1，%2，%3)").arg(coordV1[0],0,'f',3).arg(coordV1[1],0,'f',3).arg(coordV1[2],0,'f',6);
    showPosFlag(v[0],v[1],v[2]);
    QString title=QString::fromLocal8Bit("坐标位置");
    emit SIGNAL_ShowMeasureResult(msg,title);
}

void ViewerWidget::slot_calEarthWorkVolume(QVector<osg::Vec3d> qv)
{   
    QProgressDialog proDlg;
    proDlg.setWindowTitle(QString::fromLocal8Bit("土方量计算"));

//    proDlg.setRange(0,);
    double volume_result_out=0.0;
    bool bbbb=demMetafile.isEmpty();
    bbbb=true;
    if(bbbb)
    {
        //获得包围多边形的矩形区域
        double xMin=999999999.0;
        double yMin=999999999.0;
        double xMax=-999999999.0;
        double yMax=-999999999.0;
        double zMin=999999999.0;
        double zMax=-999999999.0;
        for(int i=0;i<qv.size();++i)
        {
            if(xMin>qv[i].x())
                xMin=qv[i][0];
            if(yMin>qv[i].y())
                yMin=qv[i][1];
            if(xMax<qv[i].x())
                xMax=qv[i][0];
            if(yMax<qv[i].y())
                yMax=qv[i][1];
            if(zMin>qv[i].z())
                zMin=qv[i].z();
            if(zMax<qv[i].z())
                zMax=qv[i].z();
        }
        qv.push_back(qv[0]);
//        //经度
//        double longitude1=dGeotransform[0]+1*dGeotransform[1]+1*dGeotransform[2];
//        double longitude2=dGeotransform[0]+2*dGeotransform[1]+1*dGeotransform[2];
//        //纬度
//        double latitude1=dGeotransform[3]+1*dGeotransform[4]+1*dGeotransform[5];
//        double latitude1=dGeotransform[3]+1*dGeotransform[4]+2*dGeotransform[5];

        double x_iter=pixelRange;
        double y_iter=pixelRange;
        double range=((yMax-yMin)/y_iter);
//        qDebug()<<range;
        proDlg.setRange(0,int(range));
        proDlg.setCancelButtonText(QString::fromLocal8Bit("取消"));
        proDlg.show();
        int pro_i=0;
        for(double m=yMin;m<=yMax;m+=y_iter)
        {
            proDlg.setValue(pro_i);
            QCoreApplication::processEvents();
            ++pro_i;
            if(proDlg.wasCanceled())
                return;
            for(double n=xMin;n<=xMax;n+=x_iter)
            {
                int is_in=MeasureAnalysis::intance()->Is_Point_in_Polygon(n,m,qv);
                if(is_in==1)
                {
                    double high=getPointHigh(n,m);
//                    double longitude=dGeotransform[0]+n*dGeotransform[1]+m*dGeotransform[2];
//                    double latitude=dGeotransform[3]+n*dGeotransform[4]+m*dGeotransform[5];
//                    osg::Vec3d point=osg::Vec3d(longitude,latitude,high);
//                    osg::Vec3 world=getWorldCoord(point);
                    double area_volume=pixelResolve*pixelResolve*(high);
                    volume_result_out+=area_volume;
                }
                else if (is_in==0)
                {
                    double high=getPointHigh(n,m);
                    double area_volume=pixelResolve*pixelResolve*(high)*0.25;
                    volume_result_out+=area_volume;
                }
            }
        }
    }
    volume_result_out=abs(volume_result_out);
    QString msg=QString::fromLocal8Bit("土方量为：\n%1").arg(volume_result_out,0,'f',3);
    QString title=QString::fromLocal8Bit("土方量");
    emit SIGNAL_ShowMeasureResult(msg,title);
}

void ViewerWidget::slot_calPointSlop(osg::Vec3d v1)
{
    QVector<osg::Vec3d> qv;
    double high=getPointHigh(v1[0]-pixelRange,v1[1]-pixelRange);
    qv.push_back(osg::Vec3d(v1[0]-pixelRange,v1[1]-pixelRange,high));

    high=getPointHigh(v1[0],v1[1]-pixelRange);
    qv.push_back(osg::Vec3d(v1[0],v1[1]-pixelRange,high));

    high=getPointHigh(v1[0]+pixelRange,v1[1]-pixelRange);
    qv.push_back(osg::Vec3d(v1[0]+pixelRange,v1[1]-pixelRange,high));

    high=getPointHigh(v1[0]-pixelRange,v1[1]);
    qv.push_back(osg::Vec3d(v1[0]-pixelRange,v1[1],high));

    qv.push_back(v1);

    high=getPointHigh(v1[0]+pixelRange,v1[1]);
    qv.push_back(osg::Vec3d(v1[0]+pixelRange,v1[1],high));

    high=getPointHigh(v1[0]-pixelRange,v1[1]+pixelRange);
    qv.push_back(osg::Vec3d(v1[0]-pixelRange,v1[1]+pixelRange,high));

    high=getPointHigh(v1[0],v1[1]+pixelRange);
    qv.push_back(osg::Vec3d(v1[0],v1[1]+pixelRange,high));

    high=getPointHigh(v1[0]+pixelRange,v1[1]+pixelRange);
    qv.push_back(osg::Vec3d(v1[0]+pixelRange,v1[1]+pixelRange,high));
//    for(int i=0;i<qv.size();++i)
//    {
//        qv[i]=getProjCoord(qv[i]);
//    }

    double p_slop=MeasureAnalysis::intance()->getPointSlop(qv);

    QString msg=QString::fromLocal8Bit("当前点的坡度为：\n")+QString::number(p_slop);
    QString title=QString::fromLocal8Bit("坡度分析");
    emit SIGNAL_ShowMeasureResult(msg,title);
}

void ViewerWidget::slot_calLineSlop(osg::Vec3d v1, osg::Vec3d v2)
{
//    v1=getProjCoord(v1);
//    v2=getProjCoord(v2);
    double l_slop=MeasureAnalysis::intance()->getLineSlop(v1,v2);
    QString msg=QString::fromLocal8Bit("当前线的坡度为：\n")+QString::number(l_slop);
    QString title=QString::fromLocal8Bit("坡度分析");
    emit SIGNAL_ShowMeasureResult(msg,title);
}

bool ViewerWidget::eventFilter(QObject *, QEvent *e)
{
    switch (e->type()) {
    case QEvent::MouseButtonPress:
        emit on_mouse_event(e);
    case QEvent::MouseMove:
        emit on_mouse_event(e);
        break;
    case QEvent::KeyPress:
        emit on_key_event(e);
        break;
    default:
        break;
    }
    return false;
}

osg::ref_ptr<osg::Node> ViewerWidget::createDEMTerrain(std::string filename)
{
    osg::ref_ptr<osgTerrain::TerrainTile>terrainTile = new osgTerrain::TerrainTile;
    osg::ref_ptr<osgTerrain::Locator>locator = new osgTerrain::Locator;

    //设置高程无效数据
    osg::ref_ptr<osgTerrain::ValidDataOperator>validDataOperator = new osgTerrain::NoDataValue(0.0);
    osg::ref_ptr<osgTerrain::Layer>lastAppliedLayer;

    //设置坐标系统的类型(地球坐标，描述地球体)
    locator->setCoordinateSystemType(osgTerrain::Locator::GEOGRAPHIC);
    //设置坐标的范围值(x,y最小最大值)
    locator->setTransformAsExtents(0, 0, 2000, 2000);

    //读取高程图像
    osg::ref_ptr<osg::Image> image = osgDB::readImageFile(filename);
    if (!image.valid())
        return NULL;
    //定义图像层
    osg::ref_ptr<osgTerrain::ImageLayer> imageLayer = new osgTerrain::ImageLayer;
    imageLayer->setImage(image.get());
    imageLayer->setLocator(locator.get());
    imageLayer->setValidDataOperator(validDataOperator.get());
    //imageLayer->setFilter(filter);
    ////设置高程数据的缩放值
    //if (offset != 0.0f || scale != 1.0f){

    //}
    float offset = 0.0;
    float scale = 1.0;
    imageLayer->transform(offset, scale);
    terrainTile->setElevationLayer(imageLayer.get());
    lastAppliedLayer = imageLayer.get();

    //osg::ref_ptr<osgTerrain::ImageLayer>imageLayer = new osgTerrain::ImageLayer;
    //terrainTile->setColorLayer(1,)

    osg::ref_ptr<osgTerrain::GeometryTechnique> geometryTechnique = new osgTerrain::GeometryTechnique;
    terrainTile->setTerrainTechnique(geometryTechnique.get());
    return terrainTile;
}

osg::Vec3d ViewerWidget::getWorldCoord(osg::Vec3d pos)
{
    osg::Vec3d world;
    unsigned int c_size=m_root->getNumChildren();
    unsigned int flag=0;
    for(unsigned int i=0;i<c_size;++i)
    {
        if(m_root->getChild(i)->getName()=="CS")
            flag=i;
    }
    osg::CoordinateSystemNode *csn=dynamic_cast<osg::CoordinateSystemNode *>(m_root->getChild(flag));
    csn->getEllipsoidModel()->convertLatLongHeightToXYZ(osg::DegreesToRadians(pos.y()),
                                                        osg::DegreesToRadians(pos.x()),
                                                        pos.z(), world.x(), world.y(), world.z());
    return world;
}

osg::Vec3d ViewerWidget::getProjCoord(osg::Vec3d pos)
{
//    osg::Vec3d coord;
//    coord.set(0.0,0.0,0.0);
//    if(imgProj4==NULL)
//        return coord;
    OGRSpatialReference ogrSRF,*poLatLong;
    char *imgProj4=new char[128];
//    char imgProj4[128];
    QByteArray ba=strImgProj4.toLocal8Bit();
    imgProj4=ba.data();
    ogrSRF.importFromProj4(imgProj4);

    OGRCoordinateTransformation *poTransform;
    poLatLong = ogrSRF.CloneGeogCS();
    poLatLong->SetUTM(getBand6Zone(pos[0]), TRUE);
    poLatLong->SetWellKnownGeogCS("EPSG:4326");
    poTransform = OGRCreateCoordinateTransformation(&ogrSRF, poLatLong);
    if (poTransform->Transform(1, &pos[0], &pos[1]))
    {
        delete []imgProj4;
        return pos;
    }
    else
    {
        delete []imgProj4;
        pos[0]=0.0;
        pos[1]=0.0;
        pos[2]=0.0;
        return pos;
    }
}

int ViewerWidget::getBand6Zone(double longitude)
{
    double zone = longitude + 3;
    zone /= 6.0;
    zone += 30.0;
    int trueZone = int(round(zone));
    return trueZone;
}

double ViewerWidget::getPointHigh(double x, double y)
{
    osg::ref_ptr<osg::LineSegment> line=new osg::LineSegment;
    line->set(osg::Vec3(x,y,-9999999.0),osg::Vec3(x,y,99999999.0));
    osgUtil::IntersectVisitor iv;
    iv.addLineSegment(line.get());

    unsigned int c_size=m_root->getNumChildren();
    osg::Node *c_node=nullptr;
    for(unsigned int i=0;i<c_size;++i)
    {
        if(m_root->getChild(i)->getName()=="CS")
        {
            c_node=dynamic_cast<osg::Group*>(m_root->getChild(i))->getChild(0);
        }
    }
    if(c_node==nullptr)
        return 0.0;
    c_node->accept(iv);

    //得到交点列表
    osgUtil::IntersectVisitor::HitList hl=iv.getHitList(line);
    osgUtil::Hit height;
    if(hl.empty())
    {
        return 0.0;
    }else {
        height=hl.front();
        osg::Vec3d mnz=height.getWorldIntersectPoint();
        return mnz[2];
    }
}

void ViewerWidget::setTravelManipulator()
{
    if(is_myTravelManipulator)
        return;
    TravelManipulator *myTM=new TravelManipulator();

    osg::Vec3d e,c,u;

    this->getCamera()->getViewMatrixAsLookAt(e,c,u);
//    myTM->setByMatrix(this->getCamera()->getViewMatrix());
    myTM->setByInverseMatrix(this->getCamera()->getViewMatrix());
    double speed=pixelRange*5;
    myTM->setSpeed(speed);
//    myTM->SetViewer(_viewer);

//    osgGA::CameraManipulator *m_cameraManip=_viewer->getCameraManipulator();
//    m_cameraManip->getHomePosition(e,c,u);
//    m_cameraManip->getMatrix().getRotate().asVec3();


//    myTM->SetPosition(osg::Vec3(m_cameraManip->getMatrix().getTrans()));
//    myTM->SetRotate(osg::Vec3(m_cameraManip->getMatrix().getRotate().asVec3()));

//    myTM->setHomePosition(e,c,u);
//    MyTravelManipulator *myTraMan=new MyTravelManipulator();
//    myTraMan->setHomePosition(e,c,u);

    _viewer->setCameraManipulator(myTM,true);
    is_myTravelManipulator=true;
}

void ViewerWidget::getImageProj4(QString &imageInfo)
{
    QFile file(imageInfo);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QTextStream in(&file);
    QString line=in.readLine(128);
    double leftLon=0.0;
    strImgProj4=line;
    for(int i=0;i<3;++i)
    {
        line=in.readLine(128);
    }
    leftLon=line.toDouble();
    line=in.readLine(128);
    pixelRange=line.toDouble();
    line=in.readLine(128);
    line=in.readLine(128);
    double leftUpLat=line.toDouble();
    osg::Vec3d v1=osg::Vec3d(leftLon,leftUpLat,0.0);
    osg::Vec3d v2=osg::Vec3d(leftLon+pixelRange,leftUpLat,0.0);
    v1=getProjCoord(v1);
    v2=getProjCoord(v2);
    pixelResolve=abs(v2[0]-v1[0]);
//    QByteArray ba=line.toLocal8Bit();
//    imgProj4=ba.data();
    file.close();
}

























