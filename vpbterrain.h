#ifndef VPBTERRAIN_H
#define VPBTERRAIN_H

#include <QWidget>
#include <QProcess>

namespace Ui {
class VPBTerrain;
}

class VPBTerrain : public QWidget
{
    Q_OBJECT

public:
    explicit VPBTerrain(QWidget *parent = 0);
    ~VPBTerrain();

private:
    void init();

private slots:
    void on_okBtn_clicked();

    void on_helpBtn_clicked();

    void on_tBtn_clicked();

    void on_dBtn_clicked();

    void on_oBtn_clicked();

    void on_closeBtn_clicked();

    void slot_outlog();

    void slot_error();

private:
    Ui::VPBTerrain *ui;
    QProcess *process;

    void saveDEMInfo(QString path,QString savefile);
    void saveImageInfo(QString imagefile,QString projfile);
};

#endif // VPBTERRAIN_H
