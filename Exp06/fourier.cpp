#include "complex.h"
#include "fourier.h"
#include "math.h"
#include <iostream>
#include <QDebug>
void DFT(const complex x[],complex X[],int N)
{
    int k,n;
    complex Wnk;
    for (k=0;k<N;k++)
    {
        X[k].re=0;
        X[k].im=0;
        for (n=0;n<N;n++){
            Wnk.re=(float)cos(2*PI*k*n/N);
            Wnk.im=(float)-sin(2*PI*k*n/N);
            X[k]=complexAdd(X[k],complexMult(x[n],Wnk));
        }
    }
}

void IDFT(const complex X[],complex x[],int N)
{
    int k,n;
    float im=0;
    complex ejw;
    for (k=0;k<N;k++) {
        x[k].re=0;
        x[k].im=0;
        for (n=0;n<N;n++) {
            ejw.re=(float)cos(2*PI*k*n/N);
            ejw.im=(float)sin(2*PI*k*n/N);
            x[k]=complexAdd(x[k],complexMult(X[n],ejw));
        }
        x[k].re/=N;
        x[k].im/=N;
    }
}

/**
 * @brief DFT2 brute-force二维DFT
 * @param x x[row][column]
 * @param X
 * @param row
 * @param column
 */
void DFT2(complex **x,complex **X,int row, int column)
{
    if (column*row<=0||x==nullptr||X==nullptr)
    {
        return;
    }
    /* F(u, v)= \sum_{0}^{M-1} \sum_{0}^{N-1} f(x, y) e^{-j*2Pi(ux/M+vy/N)} */
    int i,j,u,v;
    for (u=0;u<row;u++)
    {
        for(v=0;v<column;v++){
            complex F=complexBuild(0);
            complex ejw=complexBuild(0);
            for(i=0;i<row;i++){
                for(j=0;j<column;j++){
                    ejw.re=(float)cos(-2*PI*(1.0f*u*i/row+1.0f*v*j/column));
                    ejw.im=(float)sin(-2*PI*(1.0f*u*i/row+1.0f*v*j/column));
                    F=F+ x[i][j] *ejw;
                }
            }
            X[u][v]=F;
        }
    }
}

inline int lowbit(int x){
    return x & (-x);
}

int bit_length(int n) {
    for (int i = 0; i < 32; i++) {
        if ((1 << i) == n) return i;
    }
    return 0;
}

int bit_inverse(int i, int bit_len) {
    int inversed_i = 0;
    while (bit_len--) {
        inversed_i = (inversed_i << 1) + (i & 1);
        i >>= 1;
    }
    return inversed_i;
}

void transform(complex* X, complex* weights, int bit_len) {
    int n = 1 << bit_len;
    int dist = 1;
    for (int m = 1; m <= bit_len; m++) {
        for (int i = 0; i < n; i += 2 * dist) {
            for (int j = 0; j < dist; j++) {
                int k = i + j;
                int r = j << (bit_len - m);
                complex t = X[k + dist] * weights[r];
                X[k + dist] = X[k] - t;
                X[k] =X[k]+ t;
            }
        }
        dist <<= 1;
    }
}

void FFT2(complex** src, complex** dst, int row, int col, bool inverse) {
    if(row!=lowbit(row) || col!=lowbit(col)){
        qDebug() << "FFT2: row or col must be a power of 2.\n";
        return;
    }
    int row_bit_len = bit_length(row);
    int col_bit_len = bit_length(col);
    float w = inverse ? 1.0f : -1.0f;

    int col_weights_len = col / 2;
    complex* col_weights = new complex[col_weights_len];
    for (int i = 0; i < col_weights_len; i++)
        col_weights[i] = complexBuild(cos(2*PI* (w * i)/ col), sin(2*PI* (w * i)/ col));

    for (int x = 0; x < row; x++) {
        for (int y = 0; y < col; y++) {
            dst[x][y] = src[x][bit_inverse(y, col_bit_len)];
        }
        transform(dst[x], col_weights, col_bit_len);
    }

    int row_weights_len = row / 2;
    complex* row_weights = new complex[row_weights_len];
    for (int i = 0; i < row_weights_len; i++)
        row_weights[i] =complexBuild(cos(2*PI* (w * i)/ row), sin(2*PI* (w * i)/ row));
    complex* temps = new complex[row];
    for (int y = 0; y < col; y++) {
        for (int x = 0; x < row; x++)
            temps[x] = dst[bit_inverse(x, row_bit_len)][y];
        transform(temps, row_weights, row_bit_len);
        for (int x = 0; x < row; x++)   // copy to dst
            dst[x][y] = temps[x];
    }

    if (inverse) {
        int row_col = row * col;
        for (int i = 0; i < row; i++) {
            for (int j = 0; j < col; j++) {
                dst[i][j].re/=row_col;
                dst[i][j].im/=row_col;
            }
        }
    }

    /* free up memory */
    delete[] col_weights;
    delete[] row_weights;
    delete[] temps;
}

void IFFT2(complex** src, complex** dst, int u, int v){
    FFT2(src, dst,u,v,true);
}

/**
 * @brief normalize 归一化到[0,255]区间
 * @param mat row*col的矩阵
 * @param row 矩阵行数
 * @param col 矩阵列数
 */
void normalize(float** mat,int row,int col)
{
    float min=std::numeric_limits<float>::infinity();
    for(int i=0;i<row;i++)
        for(int j=0;j<col;j++)
        {
            if (mat[i][j]<min)
                min=mat[i][j];
        }
/*
    for(int i=0;i<row;i++)
       for(int j=0;j<col;j++)
       {
           mat[i][j]-=min; // i.e. +abs(min)
       }
*/
    float maxm=-1;
    for(int i=0;i<row;i++){
        for(int j=0;j<col;j++)
        {
            if (mat[i][j]>maxm)
                maxm=mat[i][j];
        }
    }

    for(int i=0;i<row;i++){
        for(int j=0;j<col;j++){
            mat[i][j]=255*(mat[i][j]-min)/(maxm-min);
        }
    }
}

/**
 * @brief shift 平移矩阵使得低频中心移动到频谱图的正中间
 * @param mat row*col的矩阵
 * @param row 矩阵行数
 * @param col 矩阵列数
 */
void shift(float **mat,int row,int col)
{
    float **old=new float*[row];
    for(int i=0;i<row;i++)
    {
        old[i]=new float[col];
    }
    for (int i = 0; i < row; i++)
    {
        for (int j = 0; j < col; j++)
        {
            old[i][j]=mat[i][j];
        }
    }
    for (int i = 0; i < row; i++)
    {
        for (int j = 0; j < col; j++)
        {
            if (i<(row/2)&&j<(col/2))   // upper left => bottom right
            {
               mat[i][j] =old[row/2+i][col/2+j];
            }
            else if (i<(row/2)&& j>=(col/2))    // upper right => bottom left
            {
                mat[i][j] =old[row/2+i][j-col/2];
            }
            else if (i >= (row / 2) && j < (col / 2))   // bottom left => upper right
            {
                mat[i][j] =old[i-row/2][col/2+j];
            }
            else if (i >= (row / 2) && j >= (col / 2))  // bottom right => upper left
            {
                mat[i][j] =old[i-row/2][j-col/2];
            }
        }
    }

    /* free up memory */
    for(int i=0;i<row;i++)
    {
        delete [] old[i];
    }
    delete[] old;
}
