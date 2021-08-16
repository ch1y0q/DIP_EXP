/* FILE ENCODING is UTF8, no need QString::fromLocal8Bit() */
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

QImage image;

void MainWindow::ClickButton(){
    auto row = ui->plainTextEdit_row->toPlainText().toUInt();
    auto column = ui->plainTextEdit_column->toPlainText().toUInt();
    auto width = image.width();
    auto height = image.height();
    // row, column starting from 0
    if(row >= width || column >= height || row < 0 || column < 0){
        QMessageBox mesg;
        mesg.warning(this, QStringLiteral("Warning"), QStringLiteral("Width or height exceeds the bound."));
    }
    else {
        //This is WRONG: uchar g = image.bits()[(row)*width+(column)];
        uchar g = image.pixel(column, row);
        ui->label_grey_val->setText(QString::number(g));
    }
}


void MainWindow::loadFile(const QString filename) {
    // 读取图像
    QImageReader reader;
    reader.setFileName(filename);

    if (reader.canRead())
    {
        // 读取图片
        // 以单通道灰度格式读取
        image = reader.read().convertToFormat(QImage::Format_Grayscale8);

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

        ui->label_info->setText(info);
        //QMessageBox mesg;
        //mesg.information(this, "Open image", info);

    }
    else
    {
        // 获取错误信息
        QImageReader::ImageReaderError error = reader.error();
        QString strError = reader.errorString();
        qDebug() << "Last Error : " << strError;
    }

}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

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

    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(ClickButton()));

    ui->label->resize(image.width(),image.height());
    ui->label->setPixmap(QPixmap::fromImage(image));
}

MainWindow::~MainWindow()
{
    delete ui;
}

