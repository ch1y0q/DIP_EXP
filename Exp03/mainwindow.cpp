#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMainWindow>
#include <QFile>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QStandardItemModel>
#include <QImageReader>
#include <QDebug>
#include <QLabel>


#define UPRIGHT(x,y) (image.bits()[(int)(ceil(x)+ image.bytesPerLine()*floor(y))])
#define UPLEFT(x,y) (image.bits()[(int)(floor(x)+ image.bytesPerLine()*floor(y))])
#define BOTTOMRIGHT(x,y) (image.bits()[(int)(ceil(x)+ image.bytesPerLine()*ceil(y))])
#define BOTTOMLEFT(x,y) (image.bits()[(int)(floor(x)+ image.bytesPerLine()*ceil(y))])
void MainWindow::loadFile(const QString filename) {
    // 读取图像
    QImageReader reader;
    reader.setFileName(filename);

    if (reader.canRead())
    {
        // 读取图片
        // 读取失败将会提示“libpng error: Read Error”
        image = reader.read().convertToFormat(QImage::Format_Grayscale8);

        // 获取描述信息
        QStringList keys = reader.textKeys();
        QString strValue("");
        foreach (QString strKey, keys)
        {
            strValue = reader.text(strKey).toLocal8Bit();
            qDebug() << QString("key : %1  value : %2").arg(strKey).arg(strValue);
        }

        QString info = QString("image size: %1 \n width: %2 \n height: %3")
                .arg(image.sizeInBytes())
                .arg(image.width())
                .arg(image.height());

    }
    else
    {
        // 获取错误信息
        QString strError = reader.errorString();
        qDebug() << "Last Error : " << strError;
    }
}

void MainWindow::ClickLoadImage(){
    if(originalImageLabel!=nullptr) delete originalImageLabel;
    QString fileName = QFileDialog::getOpenFileName(
        this, tr("open image file"),
        "./", tr("Image files(*.bmp *.jpg *.pbm *.pgm *.png *.ppm *.xbm *.xpm);;All files (*.*)"));

    if (fileName.isEmpty())
    {
        QMessageBox mesg;
        mesg.warning(this, QStringLiteral("Warning"), QStringLiteral("Unable to open the image!"));
        return;
    }
    else{
        loadFile(fileName);
    }

    originalImageLabel = new QLabel(this);
    originalImageLabel->setPixmap(QPixmap::fromImage(image));
    ui->originalImgScrollArea->setWidget(originalImageLabel);
}

QRgb MainWindow::nn(qreal xx, qreal yy){
    int x = qRound(xx);
    int y = qRound(yy);
    if(x>image.width())x=image.width()-1;
    if(y>image.height())y=image.height()-1;
    if(x<0)x=0;
    if(y<0)y=0;
    return image.pixel(x,y);
}

QRgb MainWindow::bilinear(qreal xx, qreal yy){
    qreal x = floor(xx);
    qreal y = floor(yy);
    int gray = UPLEFT(xx, yy)
            +1.0*(UPRIGHT(xx,yy)-UPLEFT(xx,yy))*(xx-x)
            +1.0*(BOTTOMLEFT(xx,yy)-UPLEFT(xx,yy))*(yy-y)
            +1.0*(BOTTOMRIGHT(xx,yy)+UPLEFT(xx,yy)-BOTTOMLEFT(xx,yy)-UPRIGHT(xx,yy))*(xx-x)*(yy-y);

    assert(gray>=0);
    assert(gray<(1<<image.depth()));
    return QColor(gray,gray,gray).rgb();
}

void MainWindow::ClickNN(){
    if(originalImageLabel==nullptr){
        qDebug() << "No image loaded...\n";
        return;
    }
    int width = image.width();
    int height = image.height();
    QImage nn_image = QPixmap(width, height).toImage();

    for(int i=0; i<width; ++i){
        for(int j=0; j<height; ++j){
            nn_image.setPixel(i,j,nn((qreal)(2*image.width()+i)/5,(qreal)(2*image.height()+j)/5));
        }
    }

    newImageLabel = new QLabel(this);
    newImageLabel->setPixmap(QPixmap::fromImage(nn_image));
    ui->newImgScrollArea->setWidget(newImageLabel);
}

void MainWindow::ClickBilinear(){
    if(originalImageLabel==nullptr){
        qDebug() << "No image loaded...\n";
        return;
    }
    int width = image.width();
    int height = image.height();
    QImage bilinear_image = QPixmap(width, height).toImage();

    for(int i=0; i<width; ++i){
        for(int j=0; j<height; ++j){
            bilinear_image.setPixel(i,j,bilinear((qreal)(2*image.width()+i)/5,(qreal)(2*image.height()+j)/5));
        }
    }

    newImageLabel = new QLabel(this);
    newImageLabel->setPixmap(QPixmap::fromImage(bilinear_image));
    ui->newImgScrollArea->setWidget(newImageLabel);
}

void MainWindow::ClickGet669(){
    if(originalImageLabel!=nullptr) delete originalImageLabel;
    QString fileName = QFileDialog::getOpenFileName(
        this, tr("open image file"),
        "./", tr("669 Image file(669.bmp);;All files (*.*)"));

    if (fileName.isEmpty())
    {
        QMessageBox mesg;
        mesg.warning(this, QStringLiteral("Warning"), QStringLiteral("Unable to open the image!"));
        return;
    }
    else{
        loadFile(fileName);
    }

    originalImageLabel = new QLabel(this);
    originalImageLabel->setPixmap(QPixmap::fromImage(image));
    ui->originalImgScrollArea->setWidget(originalImageLabel);

    /* copy original image */
    QImage _669image = image;
    /* translate */
    for(int j=0; j<100; ++j){
        for(int i=100; i<200; ++i){
            _669image.setPixel(i,j,image.pixel(i-100,j));
        }
    }
    /* rotate */
    for(int j=0; j<100; ++j){
        for(int i=200; i<300; ++i){
            _669image.setPixel(i,j,image.pixel(300-i,99-j));
        }
    }

    newImageLabel = new QLabel(this);
    newImageLabel->setPixmap(QPixmap::fromImage(_669image));
    ui->newImgScrollArea->setWidget(newImageLabel);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    originalImageLabel = newImageLabel = nullptr;
    ui->setupUi(this);
    connect(ui->loadImageButton, SIGNAL(clicked()), this, SLOT(ClickLoadImage()));
    connect(ui->NNButton, SIGNAL(clicked()), this, SLOT(ClickNN()));
    connect(ui->BilinearButton, SIGNAL(clicked()), this, SLOT(ClickBilinear()));
    connect(ui->get669Button, SIGNAL(clicked()), this, SLOT(ClickGet669()));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete newImageLabel;
    delete originalImageLabel;
}
