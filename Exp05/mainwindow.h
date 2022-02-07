#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>

#include <QtCharts>
QT_BEGIN_NAMESPACE
using namespace QtCharts;
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public slots:
    void ClickLoadImage();
    void ClickWindowSize();
    void ClickMean();
    void ClickMedian();
    void ClickReset();
    void ClickAdpMean();
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

    QImage* medianFilter(QImage&);
    QImage* meanFilter(QImage&);
    QImage adpMeanFilter(QImage&);
    void setKernel();

    float sumMatrix(float**, int, int, int,int);
    float varMatrix(float**, int, int, int,int);

    int **kernel;

    int sizeKernel = 5;
    int areaKernel= sizeKernel* sizeKernel;
    int sumKernel = 1;
};
#endif // MAINWINDOW_H
