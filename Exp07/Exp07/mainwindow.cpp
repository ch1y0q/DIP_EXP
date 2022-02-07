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
#include <iostream>


QImage *MainWindow::double_region_growing(QImage &image)
{
    int width = image.width();
    int height = image.height();

    /* median filter */

    QImage *smoothImage = new QImage(width, height, QImage::Format_Grayscale8);
    for (int x = sizeKernel / 2; x < width - sizeKernel / 2; x++) {
        for (int y = sizeKernel / 2; y < image.height() - sizeKernel / 2; y++) {
            int *grayLevel = new int [ areaKernel];
            int count = 0;
            int new_gray = 0;
            for (int i = -sizeKernel / 2; i <= sizeKernel / 2; i++) {
                for (int j = -sizeKernel / 2; j <= sizeKernel / 2; j++) {
                    int color = qGray(image.pixel(x + i, y + j));
                    grayLevel[count] = color * kernel[sizeKernel / 2 + i][sizeKernel / 2 + j];
                    count ++;
                    if (count == areaKernel) {
                        std::sort(grayLevel, grayLevel + areaKernel);
                        new_gray = grayLevel[areaKernel / 2]; // get median
                    }
                }
            }
            new_gray = qBound(0, new_gray / sumKernel, 255); //normalize
            smoothImage->setPixel(x, y, qRgb( new_gray, new_gray, new_gray));
            delete [] grayLevel;
        }
    }
    //return smoothImage;

    /* double region grwoing */
    double seed;
    int graysum;          // sum of grayscale value of the region
    int pointsum;             // numer of points in the region
    int count;                // the number of points just added to the region

    QImage *seg1Image = new QImage(width, height, QImage::Format_Grayscale8);
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            seg1Image->setPixel(j, i, qRgb(0, 0, 0));
        }
    }
    seed = qGray(smoothImage->pixel(start_x1, start_y1));
    graysum = seed;
    pointsum = 1;
    count = 1;
    while (count > 0) {         // if at least 1 point was added to the region in the last iteration
        int s = 0;              // sum of grayscale of neighbors who is added to the region during this iteration
        count = 0;
        for (int i = 0; i < height / 2 - r1_r2_margin; ++i) {
            for (int j = 0; j < width; ++j) {
                if (qGray(seg1Image->pixel(start_x1, start_y1)) == 0) {
                    if ((i - 1) >= 0 && (i + 1) < height && (j - 1) >= 0 && (j + 1) < width ) { // point on border?
                        for (int u = -1; u <= 1; ++u) {                        // neighbors
                            for (int v = -1; v <= 1; ++v) {                    // u, v are offsets
                                if  (qGray(seg1Image->pixel(j + v, i + u)) == 0 // is black (not in the set)
                                     && abs(qGray(smoothImage->pixel(j + v, i + u)) - seed) <= threshold_r1 ) { // similar to the existing region
                                    seg1Image->setPixel(j + v, i + u, qRgb( 255, 255, 255)); // make it white point
                                    count++;
                                    s += qGray(smoothImage->pixel(j + v, i + u));
                                }
                            }
                        }
                    }
                }
            }
        }
        pointsum += count;
        graysum += s;
        seed = 1.0 * graysum / pointsum; // calculate new seed
    }

    // repeat for region 2
    QImage *seg2Image = new QImage(width, height, QImage::Format_Grayscale8);
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            seg2Image->setPixel(j, i, qRgb(0, 0, 0));
        }
    }

    seed = qGray(smoothImage->pixel(start_x2, start_y2));
    graysum = seed;
    pointsum = 1;
    count = 1;
    while (count > 0) {
        int s = 0;
        count = 0;
        for (int i = height / 2 - r1_r2_margin; i < height; ++i) { // bottom half
            for (int j = 0; j < width; ++j) {
                if (qGray(seg2Image->pixel(start_x2, start_y2)) == 0) {
                    if ((i - 1) >= 0 && (i + 1) < height && (j - 1) >= 0 && (j + 1) < width ) {
                        for (int u = -1; u <= 1; ++u) {
                            for (int v = -1; v <= 1; ++v) {
                                if  (qGray(seg2Image->pixel(j + v, i + u)) == 0 && abs(qGray(smoothImage->pixel(j + v, i + u)) - seed) <= threshold_r2 ) {
                                    seg2Image->setPixel(j + v, i + u, qRgb( 255, 255, 255));
                                    count++;
                                    s += qGray(smoothImage->pixel(j + v, i + u));
                                }
                            }
                        }
                    }
                }
            }
        }

        pointsum += count;
        graysum += s;
        seed = 1.0 * graysum / pointsum;
    }

    for (int x = sizeKernel / 2; x < width - sizeKernel / 2; x++) {
        for (int y = sizeKernel / 2; y < image.height() - sizeKernel / 2; y++) {
            int *grayLevel = new int [ areaKernel];
            int count = 0;
            int new_gray = 0;
            for (int i = -sizeKernel / 2; i <= sizeKernel / 2; i++) {
                for (int j = -sizeKernel / 2; j <= sizeKernel / 2; j++) {
                    int color = std::max(qGray(seg1Image->pixel(x + i, y + j)),
                                         qGray(seg2Image->pixel(x + i, y + j)));
                    grayLevel[count] = color * kernel[sizeKernel / 2 + i][sizeKernel / 2 + j];
                    count ++;
                    if (count == areaKernel) {
                        std::sort(grayLevel, grayLevel + areaKernel);
                        new_gray = grayLevel[areaKernel / 2]; // get median
                    }
                }
            }
            new_gray = qBound(0, new_gray / sumKernel, 255); //normalize
            smoothImage->setPixel(x, y, qRgb( new_gray, new_gray, new_gray));
            delete [] grayLevel;
        }
    }

    delete seg1Image;
    delete seg2Image;
    return smoothImage;
}


void MainWindow::ClickSegmentation()
{
    auto segmentation_img = *double_region_growing(image);

    /* show the new *image* */
    newImageLabel = new QLabel(this);
    newImageLabel->setPixmap(QPixmap::fromImage(segmentation_img));
    ui->newImgScrollArea->setWidget(newImageLabel);

    int count = countRice(segmentation_img);
    QString lblContent = "Rice count: ";
    ui->riceCountLabel->setText(lblContent.append(QString::number(count)));
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

#define RICE(y,x) (qGray(image.pixel(x,y))==0)
int MainWindow::dfs(int y, int x, QImage &image) {
    int width = image.width();
    int height = image.height();
    int area = 0;
    if (vis[y][x]) {
        return 0;
    }
    vis[y][x] = true;
    if (RICE(y, x)) {
        area++;
        for (int u = -1; u <= 1; ++u) {
            for (int v = -1; v <= 1; ++v) {
                if (!u && !v)continue;
                if ((y + u) >= 0 && (y + u) < height && (x + v) >= 0 && (x + v) < width && !vis[y + u][x + v]) {
                    area += dfs(y + u, x + v, image);
                }
            }
        }
    }
    return area;
}

int MainWindow::countRice(QImage &image) {
    int width = image.width();
    int height = image.height();
    vis = new bool* [height];
    for (int i = 0; i < height; ++i) {
        vis[i] = new bool[width];
        for (int j = 0; j < width; ++j) {
            vis[i][j] = RICE(i, j) ? false : true;
        }
    }
    std::vector<int> areas;
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            if (!vis[i][j]) {
                int area = dfs(i, j, image);
                if (area > MIN_AREA) {
                    areas.push_back(area);
                }
            }
        }
    }

    auto avg_area = accumulate( areas.begin(), areas.end(), 0.0) / areas.size();
    for (auto it = areas.begin(); it != areas.end(); ++it) {
        auto area = *it;
        int rices_in_area = std::ceil(area / avg_area / 1.5);
        if (rices_in_area > 1) {
            it = areas.erase(it);
            it--;
            for (int k = 0; k < rices_in_area; ++k) {
                areas.push_back(area / rices_in_area);
            }
        }
    }

    return areas.size();
}

void MainWindow::ClickLoadImage() {
    if (originalImageLabel != nullptr) delete originalImageLabel;
#ifndef MY_DEBUG
    QString fileName = QFileDialog::getOpenFileName(
                           this, tr("open image file"),
                           "./", tr("Image files(rice.bmp);;All files (*.*)"));
#else
    QString fileName = "C:/Users/qiyue/Desktop/rice.bmp";
#endif
    if (fileName.isEmpty())
    {
        QMessageBox mesg;
        mesg.warning(this, QStringLiteral("Warning"), QStringLiteral("Unable to open the image!"));
        return;
    }
    else {
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
    /* init kernel */
    assert(sizeKernel & 1); // odd
    kernel = new int*[sizeKernel];
    for (int i = 0; i < sizeKernel; ++i) {
        kernel[i] = new int[sizeKernel];
        for (int j = 0; j < sizeKernel; ++j) {
            kernel[i][j] = 1;
            sumKernel += kernel[i][j];
        }
    }
    sumKernel /= areaKernel;

    originalImageLabel = newImageLabel = nullptr;
    ui->setupUi(this);
    connect(ui->loadImageButton, SIGNAL(clicked()), this, SLOT(ClickLoadImage()));
    connect(ui->segmentationButton, SIGNAL(clicked()), this, SLOT(ClickSegmentation()));

#ifdef MY_DEBUG
    ClickLoadImage();
#endif
}

MainWindow::~MainWindow()
{
    delete ui;
    delete newImageLabel;
    delete originalImageLabel;

    for (int i = 0; i < sizeKernel; ++i) delete[] kernel[i];
    delete[] kernel;    // delete kernel for mean filter
}
