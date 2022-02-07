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
    void ClickNN();
    void ClickBilinear();
    void ClickGet669();
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void loadFile(QString);
private:
    Ui::MainWindow *ui;
    QImage image;
    QLabel *originalImageLabel;
    QLabel *newImageLabel;

    QRgb nn(qreal xx, qreal yy);
    QRgb bilinear(qreal xx, qreal yy);
};
#endif // MAINWINDOW_H
