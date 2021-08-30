#include "mainwindow.h"
#include "complex.h"
#include "fourier.h"
#include <QApplication>
#include <iostream>
#include <QTime>
#include <QDebug>
#include <random>

void Q4(){
    /* code for Q4 */
    const int N=4096;
    QTime timedebug;
    std::srand(std::time(nullptr));
    complex rndcplx[N];
    for(int _=0;_<N;++_){
        rndcplx[_]=complexBuild((float)rand(),(float)rand());
    }
    complex **xx = new complex*[1];
    xx[0]=rndcplx;
    complex **FFT_XX = new complex*[1];
    FFT_XX[0]=new complex[N];

    timedebug.restart();
    FFT2(xx,FFT_XX,1,N);
    qDebug()<<"FFT用时"<<timedebug.elapsed()<<"ms.\n";

    complex DFT_XX[N]={complexBuild(0)};
    timedebug.restart();
    DFT(rndcplx,DFT_XX,N);
    qDebug()<<"DFT用时"<<timedebug.elapsed()<<"ms.\n";

    std::cout<<N<<" random complexes FFT result: "<<std::endl;
    for(int __=0;__<N;++__){
        if(__)std::cout<<", ";
        std::cout<<toString(FFT_XX[0][__]);
    }

    std::cout<<std::endl;

    std::cout<<N<<" random complexes DFT result: "<<std::endl;
    for(int __=0;__<N;++__){
        if(__)std::cout<<", ";
        std::cout<<toString(DFT_XX[__]);
    }
    std::cout.flush();
    delete[] xx;
    delete[] FFT_XX;
}

void Q1()
{
    /* code for Q1 */
    complex x[] = {complexBuild(1),complexBuild(1),complexBuild(1),complexBuild(1),
                   complexBuild(-1),complexBuild(-1),complexBuild(-1),complexBuild(-1),
                   complexBuild(1),complexBuild(1),complexBuild(1),complexBuild(1),
                   complexBuild(-1),complexBuild(-1),complexBuild(-1),complexBuild(-1),
                   complexBuild(1),complexBuild(1),complexBuild(1),complexBuild(1),
                   complexBuild(-1),complexBuild(-1),complexBuild(-1),complexBuild(-1),
                   complexBuild(1),complexBuild(1),complexBuild(1),complexBuild(1),
                   complexBuild(-1),complexBuild(-1),complexBuild(-1),complexBuild(-1)};
    complex X[32];
    DFT(x, X, 32);
    for(int i = 0;i<32;++i){
        if(i)std::cout<<", ";
        std::cout<< toString(X[i]);
    }
    std::cout<<std::endl;
}

int main(int argc, char *argv[])
{

    Q1();
    //Q4();

    /* test dft2
    complex **x2=new complex*[2];
    x2[0]=new complex[2];
   x2[1]=new complex[2];
   x2[0][0]=x2[1][0]=complexBuild(1);
      x2[0][1]=x2[1][1]=complexBuild(2);

     complex **X2=new complex*[2];
         X2[0]=new complex[2];
        X2[1]=new complex[2];
    DFT2(x2,X2,2,2);

    for(int _=0;_<2;++_){
        for(int __=0;__<2;++__){
            if(__)std::cout<<", ";
            std::cout<<toString(X2[_][__]);
        }
        std::cout<<std::endl;
    }
    delete[] x2[0];
    delete[] x2[1];
    delete[] X2[0];
    delete[] X2[1];
    delete[] x2;
    delete[] X2;
    */

    /* test FFT2
    complex **FFTx2=new complex*[2];
    FFTx2[0]=new complex[2];
   FFTx2[1]=new complex[2];
   FFTx2[0][0]=FFTx2[1][0]=complexBuild(1);
      FFTx2[0][1]=FFTx2[1][1]=complexBuild(2);

     complex **FFTX2=new complex*[2];
         FFTX2[0]=new complex[2];
        FFTX2[1]=new complex[2];
    FFT2(FFTx2, FFTX2, 2,2);
    for(int _=0;_<2;++_){
        for(int __=0;__<2;++__){
            if(__)std::cout<<", ";
            std::cout<<toString(FFTX2[_][__]);
        }
        std::cout<<std::endl;
    }

    delete[] FFTx2[0];
    delete[] FFTx2[1];
    delete[] FFTX2[0];
    delete[] FFTX2[1];
    delete[] FFTX2;
    delete[] FFTx2;
    */
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
