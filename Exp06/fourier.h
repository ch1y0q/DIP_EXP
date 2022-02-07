#ifndef FOURIER_H
#define FOURIER_H

#include "complex.h"

/* constants */
#define PI 3.141592654

/* Fourier transforms */
void DFT(const complex x[],complex X[],int N);
void IDFT(const complex x[],complex X[],int N);

void DFT2(complex **x,complex **X,int row, int column);
void FFT2(complex **src, complex **dst, int row, int col, bool inverse=false);
void IFFT2(complex **src, complex **dst, int row, int col);

/* utilities */
void shift(float **mat,int row,int col);
void normalize(float **mat,int row,int col);

void transform(complex *X, complex *weights, int bit_len);
int bit_length(int n);
int bit_inverse(int i, int bit_len);
inline int lowbit(int x);

#endif // FOURIER_H
