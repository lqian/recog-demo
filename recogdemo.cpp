#include "recogdemo.h"
#include "ui_recogdemo.h"


#include <QFileDialog>
#include <QImageReader>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>


using namespace core;
using namespace std;


RecogDemo::RecogDemo(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::RecogDemo)
{
    ui->setupUi(this);
    ui->centralWidget->setLayout(ui->horizontalLayout);


    cfgDir = QDir("/var/lib/aison/etc");
    if (cfgDir.exists()) {
//         LoadThread * loadThread = new LoadThread(this);
//         connect(loadThread, SIGNAL(finished()), this, SLOT(on_loaded()));
//         loadThread->start();
        create_api();
    }
    else {
        QMessageBox::warning(this, QApplication::applicationDisplayName(),
                             tr("invalid directory"));
    }

    flags.append("未知");
    flags.append("否");
    flags.append("是");
}


void RecogDemo::on_loaded() {
    qDebug() << "loaded ";
}

void RecogDemo::create_api() {
    this->_context = Context::init(10000);
    this->api = _context->createRecogAPI();
    ch = cv::freetype::createFreeType2();
    ch->loadFontData(cfgDir.filePath("font_ch.ttf").toStdString(), 0);
}

RecogDemo::~RecogDemo()
{
    delete api;
    delete _context;
    delete ui;
}

void RecogDemo::on_actionOpen_triggered()
{
    QString ret = QFileDialog::getExistingDirectory(this,tr("Open An Image Direcotry"), "/home" );
    if (ret.size() == 0) return;
    imageFiles.clear();
    //    imageDir = QDir(ret).filePath("JPEGImages");
    imageDir = QDir(ret);
    imageDir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    QDirIterator dirIter(imageDir);
    while (dirIter.hasNext()) {
        QString name = dirIter.next();
        int idx = name.lastIndexOf("/");
        if (idx != -1 ) {
            imageFiles.append(name.right(name.size() - idx - 1));
        }
    }
    imageFiles.sort();
    qDebug() << tr("scan image files: %1").arg(imageFiles.size()) ;
    currentIndex = 0;
    showCurrentImage();
}

void RecogDemo::recog()
{
    this->currentImageName = imageFiles.at(currentIndex);
    qDebug() << this->currentImageName ;
    QString fullName = this->imageDir.filePath(currentImageName);

    vector<RecogResult> recogResults;
    api->recogSingle(fullName.toStdString(), core::ContentType::FILE, recogResults);
    if (recogResults.size() == 0 ) {
        QMessageBox::information(this, QApplication::applicationDisplayName(), "未识别到车辆或者车体太小");

    }
    else  {
        RecogResult result = recogResults[0];
        ui->leColor->setText(QString::fromStdString(result.colorName));
        ui->leType->setText(QString::fromStdString(result.typeName));
        ui->leFullBrand->setText(QString::fromStdString(result.recogBrand.brandFullName));

        Mat img = imread(fullName.toStdString());
        if (result.DMD > 0) {
            ui->leMDM->setText("Yes");
        }
        else {
            ui->leMDM->setText("No");
        }

        if (result.plateFlag == FOUND_PLATE ) {
            ui->lePlateNo->setText(QString::fromStdString(result.plateNo));
            ui->lePlateColor->setText(QString::fromStdString(result.plateColorName));
            //        Mat plate(img, result.plateBox);
            //        int plateColor = core::predictPlateColor(plate);
            //        if (plateColor == core::YELLOW) {
            //            ui->lePlateColor->setText("黄");
            //        }
            //        else {
            //            ui->lePlateColor->setText("蓝");
            //        }
        }
        else {
            ui->lePlateNo->setText("UN-KNOWN");
            ui->lePlateColor->setText("UN-KNOWN");
        }

        QString format = "主:%1 \t 副:%2";
        ui->leDriver->setText(QString(format)
                              .arg( flags[result.mainDriverFlag + 1] )
                              .arg( flags[result.secondDriverFlag + 1] ));
        ui->leBelt->setText(QString(format)
                              .arg( flags[result.mainDriverBeltFlag + 1] )
                              .arg( flags[result.secondDriverBeltFlag + 1] ));

        ui->lePhone->setText(QString(format)
                              .arg( flags[result.mainDriverPhoneFlag + 1] )
                              .arg( flags[result.secondDriverPhoneFlag + 1] ));

        ui->leSunVisor->setText(QString(format)
                              .arg( flags[result.mainSunVisorFlag + 1] )
                              .arg( flags[result.secondSunVisorFlag + 1] ));


        this->drawDetails(img, result);

        Rect roi = result.vehicleBox;
        roi.x -= roi.width / 10;
        roi.y -= roi.height / 10;
        roi.width += roi.width / 5;
        roi.height += roi.height / 5;

        if (roi.x < 0 ) roi.x = 0;
        if (roi.y < 0 ) roi.y = 0;
        if (roi.width + roi.x > img.cols) roi.width = img.cols - roi.x;
        if (roi.height + roi.y > img.rows)  roi.height = img.rows - roi.y;

        Mat mat(img, roi);
        //cv::resize(img, mat, cv::Size(newWidth, newHeight));

        // Mat mat;
        //cv::resize(img, mat, cv::Size(newWidth, newHeight));
        Mat rgb;
        cv::cvtColor(mat, rgb, CV_BGR2RGB);
        QImage tmpImg(rgb.data, rgb.cols, rgb.rows,  rgb.cols * rgb.channels(),  QImage::Format_RGB888 );


        if (tmpImg.width() < stdWidth && tmpImg.height() < stdHeight) {
            ui->image->resize(tmpImg.width(), tmpImg.height());
        }
        else {
            float sw = tmpImg.width() / stdWidth;
            float sh = tmpImg.height() / stdHeight;
            newWidth = stdWidth;
            newHeight = stdHeight;
            if (sw < sh) {
                newWidth = sw / sh * stdWidth;
            }
            else {
                newHeight = sh / sw * stdHeight;
            }

            //       qDebug() <<  tr("new width %1 new height %2").arg(newWidth).arg(newHeight);

            tmpImg = tmpImg.scaled(newWidth, newHeight);
            ui->image->resize(newWidth, newHeight);
        }

        ui->image->setPixmap(QPixmap::fromImage(tmpImg));
    }

    double total = api->totalCost();
    double gpu = api->gpuCost();
    ui->statusBar->showMessage(tr("%1 recognized cost, total: %2, gpu: %3, cpu: %4 (ms)")
                               .arg(fullName).arg(total).arg(gpu).arg(total - gpu));
}

void RecogDemo::showCurrentImage() {
    this->cleanRecogns();

    this->currentImageName = imageFiles.at(currentIndex);
    QString fullName = this->imageDir.filePath(currentImageName);
    ui->statusBar->showMessage(fullName);
    QImageReader reader(fullName);
    reader.setAutoTransform(true);
    QImage image = reader.read();
    if (image.isNull()) {
        QMessageBox::warning(this, QApplication::applicationDisplayName(),
                             tr("cannot read image: %1").arg(fullName));
    }
    else {
        float sw = image.width() / stdWidth;
        float sh = image.height() / stdHeight;
        newWidth = stdWidth;
        newHeight = stdHeight;
        if (sw < sh) {
            newWidth = sw / sh * stdWidth;
        }
        else {
            newHeight = sh / sw * stdHeight;
        }

        //       qDebug() <<  tr("new width %1 new height %2").arg(newWidth).arg(newHeight);

        image = image.scaled(newWidth, newHeight);
        ui->image->resize(newWidth, newHeight);
        ui->image->setPixmap(QPixmap::fromImage(image));

        if (!ui->actionAuto_Recog->isChecked()) return ;

        recog();
    }
}

void RecogDemo::drawDetails(Mat & mat, const RecogResult & result) {
    int x = result.vehicleBox.x;
    int y = result.vehicleBox.y;

    rectangle(mat, result.vehicleBox, Scalar(20, 20, 255));

    for (size_t i=0; i< result.faceDetails.size(); i++) {
        InnerDetail fd = result.faceDetails[i];
        int baseline;
        Size size = ch->getTextSize(String(fd.name), 10, FILLED, &baseline);
        Rect r = fd.box;
        r.x += x;
        r.y += y;
        cv::rectangle(mat, r, Scalar::all(220));
        Point textOrg(r.x + size.width / 2, r.y + size.height);
        ch->putText(mat, String(fd.name), textOrg, 10, Scalar::all(240), cv::FILLED, cv::LINE_AA, true);
    }
}

QString RecogDemo::fullBrand(const RecogResult & result) {
    QString brand("");
    std::map<string, QJsonObject>::const_iterator it = this->brandMap.find(result.recogBrand.label);
    qDebug() << tr("vehicle brand: %1").arg(QString::fromStdString(result.recogBrand.label));
    //    qDebug() << tr("find label: %1").arg(QString::fromStdString(result.recogBrand.label)) ;
    if (it != brandMap.end()) {
        brand = it->second["YearModelName"].toString();
    }
    return brand;
}

void RecogDemo::cleanRecogns() {
    ui->leFullBrand->setText("");
    ui->leColor->setText("");
    ui->leType->setText("");
    ui->lePlateNo->setText("");
    ui->lePlateColor->setText("");
    ui->leMDM->setText("");
    ui->leDriver->setText("");
    ui->leBelt->setText("");
    ui->lePhone->setText("");
    ui->leSunVisor->setText("");
}

void RecogDemo::on_actionExit_triggered()
{
    exit(0);
}

void RecogDemo::on_actionFirst_triggered()
{
    currentIndex = 0;
    showCurrentImage();
}

void RecogDemo::on_actionPrevious_triggered()
{
    if (currentIndex >0) {
        currentIndex--;
        showCurrentImage();
    }
}

void RecogDemo::on_actionNext_triggered()
{
    if (currentIndex < imageFiles.size() - 1) {
        currentIndex++;
        showCurrentImage();
    }
}

void RecogDemo::on_actionLast_triggered()
{
    currentIndex = imageFiles.size() - 1;
    showCurrentImage();
}

void RecogDemo::resizeEvent(QResizeEvent * event) {
    int oldWidth = ui->centralWidget->width();
    int oldHeight = ui->centralWidget->height();

    if (width() > oldWidth  && height() > oldHeight) {

        int newWidth = qMax(width()+ 20, oldWidth);
        int newHeight = qMax(height()+ 20, oldHeight);

        ui->centralWidget->resize(QSize(newWidth, newHeight));
        ui->centralWidget->update();
        update();
    }


    QWidget::resizeEvent(event);
}

void RecogDemo::on_actionRecog_triggered()
{
    recog();
}
