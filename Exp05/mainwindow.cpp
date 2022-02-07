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

    ui->newImgScrollArea->setWidget(nullptr);
    delete newImageLabel;
}

int w_kernel[5][5] ={{0,1,3,1,0},
        {1,3,5,3,1},
        {3,5,7,5,3},
        {1,3,5,3,1},
        {0,1,3,1,0}};

/* generate all-one kernel matrix */
void MainWindow::setKernel()
{
    kernel = new int*[sizeKernel];
    for(int i=0;i<sizeKernel;++i){
        kernel[i]=new int[sizeKernel];
        for(int j=0;j<sizeKernel;++j){
            kernel[i][j] = 1;//w_kernel[i][j]; //;
            sumKernel+=kernel[i][j];
        }
    }
    sumKernel/=areaKernel;
}

void MainWindow::ClickWindowSize()
{
    int windowSize=ui->windowSizeInput->toPlainText().toInt();
    if(windowSize & 1){ // should be odd
        for(int i=0;i<sizeKernel;++i) delete[] kernel[i];
        delete[] kernel;    // delete old kernel

        sizeKernel = windowSize;
    }
    areaKernel = sizeKernel*sizeKernel;
    setKernel();
}

void MainWindow::ClickMean()
{
    modifiedImage = *meanFilter(image);

    /* show the new *image* */
    newImageLabel = new QLabel(this);
    newImageLabel->setPixmap(QPixmap::fromImage(modifiedImage));
    ui->newImgScrollArea->setWidget(newImageLabel);
}

void MainWindow::ClickAdpMean()
{
    modifiedImage = adpMeanFilter(image);

    /* show the new *image* */
    newImageLabel = new QLabel(this);
    newImageLabel->setPixmap(QPixmap::fromImage(modifiedImage));
    ui->newImgScrollArea->setWidget(newImageLabel);
}

void MainWindow::ClickMedian()
{
    modifiedImage = *medianFilter(image);

    /* show the new *image* */
    newImageLabel = new QLabel(this);
    newImageLabel->setPixmap(QPixmap::fromImage(modifiedImage));
    ui->newImgScrollArea->setWidget(newImageLabel);
}

/* mean filter */
QImage *MainWindow::meanFilter(QImage &image)
{
    QImage *newImage = new QImage(image.width(),image.height(),QImage::Format_Grayscale8);

    for(int x = sizeKernel/2;x<image.width() - sizeKernel/2;x++){
        for(int y= sizeKernel/2;y<image.height() - sizeKernel/2;y++){
            int *grayLevel = new int [ areaKernel];
            int count = 0;
            int new_gray = 0;
            for(int i = -sizeKernel/2;i<=sizeKernel/2;i++){
                for(int j = -sizeKernel/2;j<=sizeKernel/2;j++){
                    int color = qGray(image.pixel(x+i,y+j));
                    grayLevel[count] = color * kernel[sizeKernel/2+i][sizeKernel/2+j];
                    count ++;
                    if (count == areaKernel){
                        for(int k = 0;k<areaKernel;++k){
                            new_gray+=grayLevel[k];
                        }
                        new_gray/=areaKernel;   // calculate mean
                    }
                }
            }
            new_gray = qBound(0,new_gray/sumKernel,255);//normalize
            newImage->setPixel(x,y,qRgb( new_gray,new_gray,new_gray));
            delete [] grayLevel;
        }
    }

    return newImage;
}


/* median filter */
QImage *MainWindow::medianFilter(QImage &image)
{
    QImage *newImage = new QImage(image.width(),image.height(),QImage::Format_Grayscale8);

    for(int x = sizeKernel/2;x<image.width() - sizeKernel/2;x++){
        for(int y= sizeKernel/2;y<image.height() - sizeKernel/2;y++){
            int *grayLevel = new int [ areaKernel];
            int count = 0;
            int new_gray = 0;
            for(int i = -sizeKernel/2;i<=sizeKernel/2;i++){
                for(int j = -sizeKernel/2;j<=sizeKernel/2;j++){
                    int color = qGray(image.pixel(x+i,y+j));
                    grayLevel[count] = color * kernel[sizeKernel/2+i][sizeKernel/2+j];
                    count ++;
                    if (count == areaKernel){
                        std::sort(grayLevel, grayLevel+areaKernel);
                        new_gray = grayLevel[areaKernel/2];// get median
                    }
                }
            }
            new_gray = qBound(0,new_gray/sumKernel,255);//normalize
            newImage->setPixel(x,y,qRgb( new_gray,new_gray,new_gray));
            delete [] grayLevel;
        }
    }

    return newImage;
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

/* adaptive mean filter */
QImage MainWindow::adpMeanFilter(QImage& src)
{
    int window = sizeKernel;
    float nvar = 1000;

    assert(window&1);
    int width=src.width();
    int height=src.height();
    int half_window=window/2;
    float** mat=new float*[height+window];
    for(int i=0;i<height+window;i++){
        mat[i]=new float[width+window];
    }
    QImage ret(width,height,QImage::Format_Grayscale8);
    for(int i=0;i<height;i++){
        for(int j=0;j<width;j++){
            int color = qGray(src.pixel(j,i));
            mat[i+half_window][j+half_window]=color;
        }
    }

    //padding
    // Upper-left corner
    for(int i=0;i<half_window;i++)
        for(int j=0;j<half_window;j++)
        {
            mat[i][j]=mat[half_window][half_window];
        }
    // Upper-right corner
    for(int i=0;i<half_window;i++)
        for(int j=half_window+width;j<half_window+width+half_window;j++)
        {
            mat[i][j]=mat[half_window][half_window+width-1];
        }
    // Bottom-left corner
    for(int i=half_window+height;i<half_window+height+half_window;i++)
        for(int j=0;j<half_window;j++)
        {
            mat[i][j]=mat[half_window+height-1][half_window];
        }
    // Bottom-right corner
    for(int i=half_window+height;i<half_window+height+half_window;i++)
        for(int j=half_window+width;j<half_window+width+half_window;j++)
        {
            mat[i][j]=mat[half_window+height-1][half_window+width-1];
        }
    // Left
    for(int i=half_window;i<half_window+height;i++)
        for(int j=0;j<half_window;j++)
        {
            mat[i][j]=mat[i][half_window];
        }
    // Right
    for(int i=half_window;i<half_window+height;i++)
        for(int j=half_window+width;j<half_window+width+half_window;j++)
        {
            mat[i][j]=mat[i][half_window+width-1];
        }
    // Top
    for(int i=0;i<half_window;i++)
        for(int j=half_window;j<half_window+width;j++)
        {
            mat[i][j]=mat[half_window][j];
        }
    // Bottom
    for(int i=half_window+height;i<half_window+height+half_window;i++)
        for(int j=half_window;j<half_window+width;j++)
        {
            mat[i][j]=mat[half_window+height-1][j];
        }
    // end of padding

    // adaptive
    for(int i=0;i<height;i++)
        for(int j=0;j<width;j++)
        {
            float ans=0;
            float avg=sumMatrix(mat,j,j+half_window+half_window,i,i+half_window+half_window)/(window*window);
            float var=varMatrix(mat,j,j+half_window+half_window,i,i+half_window+half_window);
            if(nvar>var)//\sigma_\eta>\sigma_l , mostly noise
                ans=avg;
            else{
                ans=mat[i+half_window][j+half_window]-(nvar/var)*(mat[i+half_window][j+half_window]-avg);
            }
            ret.setPixel(j,i,qRgb(ans,ans,ans));
        }

    /* free memory */
    for(int i=0;i<height+window;i++)
    {
        delete [] mat[i];
    }
    delete[] mat;
    return ret;
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
    kernel = new int*[sizeKernel];
    for(int i=0;i<sizeKernel;++i){
        kernel[i]=new int[sizeKernel];
        for(int j=0;j<sizeKernel;++j){
            kernel[i][j] = 1;
        }
    }

    originalImageLabel = newImageLabel = nullptr;
    ui->setupUi(this);
    connect(ui->loadImageButton, SIGNAL(clicked()), this, SLOT(ClickLoadImage()));
    connect(ui->meanButton, SIGNAL(clicked()), this, SLOT(ClickMean()));
    connect(ui->medianButton, SIGNAL(clicked()), this, SLOT(ClickMedian()));
    connect(ui->windowSizeButton, SIGNAL(clicked()), this, SLOT(ClickWindowSize()));
    connect(ui->resetButton, SIGNAL(clicked()), this, SLOT(ClickReset()));
    connect(ui->adpMeanButton, SIGNAL(clicked()), this, SLOT(ClickAdpMean()));

}

MainWindow::~MainWindow()
{
    delete ui;
    delete newImageLabel;
    delete originalImageLabel;
    for(int i=0;i<sizeKernel;++i) delete[] kernel[i];
    delete[] kernel;
}
