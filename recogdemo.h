#ifndef RECOGDEMO_H
#define RECOGDEMO_H

#include <QMainWindow>
#include <QDir>
#include <QDirIterator>
#include <QStringList>
#include <QDebug>
#include <map>
#include <QThread>

#include <opencv2/freetype.hpp>


#include <core/api.hpp>

//#include <core/vehiclerecog.hpp>
//#include <core/plateprocess.hpp>


namespace Ui {
class RecogDemo;
}

using namespace cv::freetype;
using namespace core;

class RecogDemo : public QMainWindow
{
    Q_OBJECT

public:
    explicit RecogDemo(QWidget *parent = 0);
    ~RecogDemo();

    void create_api();

private slots:
    void on_actionOpen_triggered();

    void on_actionExit_triggered();

    void on_actionFirst_triggered();

    void on_actionPrevious_triggered();

    void on_actionNext_triggered();

    void on_actionLast_triggered();
    void on_actionRecog_triggered();

    void on_loaded();



protected:
    void resizeEvent(QResizeEvent *event) override;

private:

    Ui::RecogDemo *ui;

    QDir imageDir;
    QDir cfgDir;
    QStringList imageFiles;
    int currentIndex;
    QString currentImageName;
    QStringList flags;

    //core::VehicleRecog vehicleRecog;

    //core::PlateSegment * plateSegment;

    vector<string> plateCharLabels;

    std::map<string, QJsonObject> brandMap;

    Ptr<FreeType2> ch;

    void drawDetails( Mat & img, const RecogResult & result);
    QString fullBrand(const RecogResult & result);
    void showCurrentImage();
    void cleanRecogns();


    void recog();

    int newWidth, newHeight;
    float stdWidth = 1200. ;
    float stdHeight = 900. ;

    Context * _context;
    RecogAPI * api;
};

class LoadThread: public QThread {
    Q_OBJECT
public:
    LoadThread( RecogDemo * __recogDemo):recogDemo(__recogDemo) {}
    ~LoadThread(){}

protected:
    void run() {
        recogDemo->create_api();
        emit loaded();
    }

signals:
    void loaded();
private:
    RecogDemo * recogDemo;
};

#endif // RECOGDEMO_H
