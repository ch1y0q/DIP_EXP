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
    void ClickHistogram();
    void ClickHE();
    void ClickGamma();
    void ClickReset();
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

    QChart* getHistogram(QImage&);
    void showHistogram();
};
#endif // MAINWINDOW_H
