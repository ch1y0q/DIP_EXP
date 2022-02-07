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
#include <QChart>
#include <QBarSet>
/* QCharts required, which is not part of the default configuraion */

void MainWindow::ClickReset()
{
    originalImageLabel = new QLabel(this);
    originalImageLabel->setPixmap(QPixmap::fromImage(image));
    ui->originalImgScrollArea->setWidget(originalImageLabel);
    ClickHistogram();
}

/* histogram for the original image */
void MainWindow::ClickHistogram()
{
    ui->gammaInput->setText("1");
    auto chart = getHistogram(image);

    ui->graphicsView->setChart(chart);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    ui->graphicsView->setVisible(true);
}

void MainWindow::ClickHE(){
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

    /* show the new *image* */
    newImageLabel = new QLabel(this);
    newImageLabel->setPixmap(QPixmap::fromImage(modifiedImage));
    ui->originalImgScrollArea->setWidget(newImageLabel);

    auto chart = getHistogram(modifiedImage);

    ui->graphicsView->setChart(chart);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    ui->graphicsView->setVisible(true);
}

void MainWindow::ClickGamma(){
    qreal gamma=ui->gammaInput->toPlainText().toFloat();
    assert(gamma>=0);
    int width = image.width();
    int height = image.height();
    for(int i = 0; i <width; i++){
        for(int j = 0; j < height; j++){
            int index = qGray(image.pixel(i, j));
            index = pow((float)index/255,gamma)*255;
            if(index>255)index=255;//saturation
            modifiedImage.setPixel(i,j,qRgb(index,index,index));
        }
    }

    /* show the new *image* */
    newImageLabel = new QLabel(this);
    newImageLabel->setPixmap(QPixmap::fromImage(modifiedImage));
    ui->originalImgScrollArea->setWidget(newImageLabel);

    auto chart = getHistogram(modifiedImage);

    ui->graphicsView->setChart(chart);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    ui->graphicsView->setVisible(true);
}

QChart* MainWindow::getHistogram(QImage& grayimage){
    int data[256]={0};//256-level grayscale
    int yRange = 0;
    int width = grayimage.width();
    int height = grayimage.height();
    for(int i = 0; i <width; i++){
        for(int j = 0; j < height; j++){
            int index = qGray(grayimage.pixel(i, j));
            assert(index<256);
            ++data[index];
        }
    }

    //set color, width, value for each bar
    QBarSet *set = new QBarSet("Gray Level");
    for (int i=0;i<256;i++) {
       set->append(data[i]);
       if(yRange<data[i]) yRange = data[i];
       set->setColor(QColor::Rgb);
    }
    QBarSeries *series = new QBarSeries();
    series->append(set);
    series->setBarWidth(1.2);

    // set x
    QStringList categories;
    for(int i=0; i<256; i++)
        categories.push_back("");
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);

    // set y
    QValueAxis *axisY = new QValueAxis;
    axisY->setLabelFormat("%d");
    axisY->setRange(0,yRange);

    // construct QChart
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->createDefaultAxes();
    chart->setAxisX(axisX);
    chart->setAxisY(axisY);
    chart->setTitle("Histogram");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setVisible(false);

    return chart;
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
    connect(ui->histogramButton, SIGNAL(clicked()), this, SLOT(ClickHistogram()));
    connect(ui->HEButton, SIGNAL(clicked()), this, SLOT(ClickHE()));
    connect(ui->gammaButton, SIGNAL(clicked()), this, SLOT(ClickGamma()));
    connect(ui->resetButton, SIGNAL(clicked()), this, SLOT(ClickReset()));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete newImageLabel;
    delete originalImageLabel;
}
