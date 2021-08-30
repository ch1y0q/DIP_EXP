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
    void ClickShowSpectrum();

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void loadFile(QString);

private:
    Ui::MainWindow *ui;

    QImage image;
    QLabel *imageLabel;
    QLabel *spectrumLabel;
};
#endif // MAINWINDOW_H
