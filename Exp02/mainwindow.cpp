#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "complex.h"
#include "math.h"
#include "fourier.h"
#include <QMainWindow>
#include <QFile>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QStandardItemModel>
#include <QImageReader>
#include <QDebug>
#include <QLabel>
#include <iostream>

void MainWindow::loadFile(const QString filename) {
    // 读取图像
    QImageReader reader;
    reader.setFileName(filename);

    if (reader.canRead())
    {
        // 读取图片
        // 以单通道灰度格式读取
        // 如果不是8位，将会提示“libpng error: Read Error”
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
        QImageReader::ImageReaderError error = reader.error();
        QString strError = reader.errorString();
        qDebug() << "Last Error : " << strError;
    }
}

void MainWindow::ClickLoadImage(){
    if(imageLabel!=nullptr) delete imageLabel;
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

    imageLabel = new QLabel(this);
    imageLabel->setPixmap(QPixmap::fromImage(image));
    ui->imgScrollArea->setWidget(imageLabel);
}

void MainWindow::ClickShowSpectrum(){
    if(spectrumLabel!=nullptr) delete spectrumLabel;
    int height = image.height();
    int width = image.width();
    float **normalizedPixels = new float*[height];
    for(int i=0;i<height;++i){
        normalizedPixels[i] = new float[width];
        for(int j=0;j<width;++j){
            uchar g =image.pixel(i,j);
            normalizedPixels[i][j]=1.0f*g/255;    // normalize
        }
    }

    complex **x2=new complex*[height];
    complex **X2=new complex*[height];
    for(int i=0;i<height;++i){
        x2[i]=new complex[width];
        X2[i]=new complex[width];
        for(int j=0;j<width;++j){
            x2[i][j]=complexBuild(normalizedPixels[i][j],0);   // build complex
        }
    }

    /*
    for(int _=0;_<height;++_){
        for(int __=0;__<width;++__){
            if(__)std::cout<<", ";
            std::cout<<x2[_][__].re;
        }
        std::cout<<std::endl;
    }
    */

    FFT2(x2,X2,height,width);

    /*
    for(int _=0;_<height;++_){
        for(int __=0;__<width;++__){
            if(__)std::cout<<", ";
            std::cout<<toString(X2[_][__]);
        }
        std::cout<<std::endl;
    }
    */

    std::cout<<"Grayscale at central point: "<<complexAbs(X2[0][0]) << std::endl;

    /* normalize, shift, plot */
    float **ans=new float*[height];
    for (int i=0;i<height ;i++ ) {
        ans[i]=new float[width];
    }

    for(int i=0;i<height;++i){
        for(int j=0;j<width;++j){
            //if(j)std::cout<<", ";
            //std::cout<<(255 * complexAbs(X2[i][j]));
            ans[i][j]=log(complexAbs(X2[i][j]));
        }
        //std::cout<<std::endl;
    }
    normalize(ans,height,width);
    shift(ans,height,width);
    QImage spectrum=QImage(width, height, QImage::Format_Grayscale8);
    for (int i=0;i<height ;++i ) {
        for (int j=0;j<width ; ++j) {
            spectrum.setPixel(QPoint(i,j),qRgb(ans[i][j],ans[i][j],ans[i][j]));
        }
    }

    spectrumLabel = new QLabel(this);
    spectrumLabel->setPixmap(QPixmap::fromImage(spectrum));
    ui->spectrumScrollArea->setWidget(spectrumLabel);

    /* free up memory */
    for (int i=0;i<height ;i++ ) {
        delete[] ans[i];
        delete[] x2[i];
        delete[] X2[i];
        delete[] normalizedPixels[i];
    }
    delete[] ans;
    delete[] x2;
    delete[] X2;
    delete[] normalizedPixels;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    imageLabel=spectrumLabel=nullptr;
    ui->setupUi(this);

    connect(ui->loadImageButton, SIGNAL(clicked()), this, SLOT(ClickLoadImage()));
    connect(ui->spectrumButton, SIGNAL(clicked()), this, SLOT(ClickShowSpectrum()));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete imageLabel;
    delete spectrumLabel;
}


