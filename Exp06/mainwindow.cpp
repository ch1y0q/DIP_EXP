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
#include "complex.h"
#include "fourier.h"
#include <iostream>

void MainWindow::histeq(QImage &image)
{
        qreal PDF[256]={0};//256-level grayscale
        qreal CDF[256]={0};
        int newPixCount[256] = { 0 };
        int yRange = 0;
        int width = image.width();
        int height = image.height();
        int size = width*height;
        for(int i = 0; i <width; i++){
            for(int j = 0; j < height; j++){
                int index = qGray(image.pixel(i, j));
                assert(index<256);
                ++PDF[index];
            }
        }

        //CLHE
        const int EPSILON = 500;
        const int THROTTLE = 0.15*size+EPSILON;
        bool flag = true;
        int recoup=0;
        while(flag){
            flag=false;
            for(int i = 0; i <256; i++){
                if(PDF[i]>THROTTLE){
                    recoup+=PDF[i]-THROTTLE;
                    PDF[i]=THROTTLE;
                    flag=true;
                }
            }
            if(flag){
                for(int i = 0; i <256; i++){PDF[i]+=recoup/256;}
                recoup=0;
            }
        }


        for(int i = 0; i <256; i++){
            PDF[i]/=size;
        }


        for(int i = 0; i <256; i++){
            CDF[i]=(i?CDF[i-1]+PDF[i]:PDF[i]);
            newPixCount[i] = CDF[i]*255.0+0.5;
        }

        for(int i = 0; i <width; i++){
            for(int j = 0; j < height; j++){
                int newGray=newPixCount[qGray(image.pixel(i,j))];
                modifiedImage.setPixel(i,j,qRgb(newGray,newGray,newGray));
            }
        }
}

QImage *MainWindow::retinex(QImage &image)
{
    QImage *newImage = new QImage(image.width(),image.height(),QImage::Format_Grayscale8);
    int width = image.width();
    int height = image.height();


    int height_padding = 1, width_padding = 1;
    while(height_padding<height)height_padding*=2;
    while(width_padding<width)width_padding*=2;

    complex **f_x_y=new complex*[height_padding];
    complex **F_u_v=new complex*[height_padding];   // F(u,v): input in frequency domain
    for(int i =0;i<height_padding;++i){
        f_x_y[i] = new complex[width_padding];
        F_u_v[i] = new complex[width_padding];
        for(int j =0;j<width_padding;++j){
            f_x_y[i][j]=F_u_v[i][j]=complexBuild(0);
        }
    }
    // Step 1: log
    for(int i =0;i<height;++i){
        for(int j=0;j<width;++j){
            float color = qGray(image.pixel(j, i));
            color = log(color);
            f_x_y[i][j]=complexBuild(color);
        }
    }

    qDebug()<<"Step 1 done.\n";

    // Step 2: DFT
    FFT2(f_x_y,F_u_v,height_padding,width_padding);

    qDebug()<<"Step 2 done.\n";

    // Step 3: H
    int cy=height_padding/2;
    int cx=width_padding/2;
    for(int i =0;i<height_padding;++i){
        for(int j=0;j<width_padding;++j){
            double d = pow((i-cy+1),2)+pow((j-cx+1),2);
            F_u_v[i][j] =  F_u_v[i][j]*complexBuild((rH-rL)*(exp(c*(-d/d0/d0)))+rL);
        }
    }
    qDebug()<<"Step 3 done.\n";

    // Step 4: IDFT
    IFFT2(F_u_v, f_x_y, height_padding, width_padding);

    qDebug()<<"Step 4 done.\n";
    // Step 5: exp
    float **normalized=new float*[height_padding];   // F(u,v): input in frequency domain
    for(int i =0;i<height_padding;++i)
        normalized[i] = new float[width_padding];

    for(int i =0;i<height_padding;++i){
        for(int j=0;j<width_padding;++j){
            normalized[i][j]=exp(complexAbs(f_x_y[i][j]));
            assert(normalized[i][j] >= 0);
        }
    }

   normalize(normalized, height_padding, width_padding);

    for(int i =0;i<height;++i){
        for(int j=0;j<width;++j){
            int color = normalized[i][j];
            assert(color >= 0);
            newImage->setPixel(j,i,qRgb( color,color,color));
        }
    }
    qDebug()<<"Step 5 done.\n";

    // free memory

    for(int i =0;i<height_padding;++i){
        delete[] f_x_y[i];
        delete[] F_u_v[i];
    }
    delete[] F_u_v;
    delete[] f_x_y;

    return newImage;
}


void MainWindow::ClickRetinex()
{
    auto retinex_img = *retinex(image);

    histeq(retinex_img);
    /* show the new *image* */
    newImageLabel = new QLabel(this);
    newImageLabel->setPixmap(QPixmap::fromImage(modifiedImage));
    ui->newImgScrollArea->setWidget(newImageLabel);
}

float MainWindow::sumMatrix(float** mat, int j1, int j2,int i1, int i2){
    float ans = 0.0;
    for(int i=i1;i<=i2;++i){
        for(int j=j1;j<=j2;++j){
            ans+=mat[i][j];
        }
    }
    return ans;
}

float MainWindow::varMatrix(float** mat, int j1, int j2,int i1, int i2){
    int count = (j2-j1+1)*(i2-i1+1);
    float avg = sumMatrix(mat,i1,i2,j1,j2) / count;
    float ans = 0.0;
    for(int i=i1;i<=i2;++i){
        for(int j=j1;j<=j2;++j){
            ans+=(mat[i][j]-avg)*(mat[i][j]-avg);
        }
    }
    ans/=(count-1);
    return ans;
}


void MainWindow::loadFile(const QString filename) {
    // 读取图像
    QImageReader reader;
    reader.setFileName(filename);

    if (reader.canRead())
    {
        // 读取图片
        // 读取失败将会提示“libpng error: Read Error”
        image = reader.read().convertToFormat(QImage::Format_Grayscale8);
        modifiedImage = image;  //deep-copy
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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    originalImageLabel = newImageLabel = nullptr;
    ui->setupUi(this);
    connect(ui->loadImageButton, SIGNAL(clicked()), this, SLOT(ClickLoadImage()));
    connect(ui->retinexButton, SIGNAL(clicked()), this, SLOT(ClickRetinex()));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete newImageLabel;
    delete originalImageLabel;

}
