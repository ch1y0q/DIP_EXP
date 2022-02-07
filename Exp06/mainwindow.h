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
    void ClickRetinex();
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

    float sumMatrix(float**, int, int, int,int);
    float varMatrix(float**, int, int, int,int);

    QImage *retinex(QImage &image);
    void histeq(QImage &image);

private:
    /* retinex parameters */
    float rL=0.5;
    float rH=5;
    float c=1;
    float d0=3;
};
#endif // MAINWINDOW_H
