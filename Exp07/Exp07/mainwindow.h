#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public slots:
    void ClickLoadImage();
    void ClickSegmentation();
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void loadFile(QString);
private:
    Ui::MainWindow *ui;
    QImage image;
    QImage modifiedImage;
    QLabel *originalImageLabel;
    QLabel *newImageLabel;

    QImage *double_region_growing(QImage &image);
    void histeq(QImage &image);

    int countRice(QImage &image);
    int dfs(int y, int x, QImage &image);
private:
    /* median filter parameters */
    int **kernel;

    int sizeKernel = 5; // should be odd
    int areaKernel= sizeKernel* sizeKernel;
    int sumKernel = 1;

    /* region growing parameters */
    int start_x1= 242;
    int start_y1=240;
    double threshold_r1 = 60;
    int start_x2=250;
    int start_y2=470;
    double threshold_r2 = 60;
    int r1_r2_margin = 0;

    /* count */
    bool **vis;
    const int MIN_AREA = 10;
};

//#define MY_DEBUG


#endif // MAINWINDOW_H
