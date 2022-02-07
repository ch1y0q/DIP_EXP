#ifndef COMPLEX_H
#define COMPLEX_H
#include <string>
typedef struct {
    float re;
    float im;
}complex,*pcomplex;

complex complexAdd(complex a,complex b);

complex operator+(const complex &a, const complex &b );

complex operator-(const complex &a, const complex &b );

complex complexMult(complex a,complex b);

complex operator*(const complex &a, const complex &b );

complex complexBuild(float re, float im);

complex complexBuild(float re);

std::string toString(const complex &a);

float complexAbs(const complex &a);

complex complexExp(const complex &a);
#endif // COMPLEX_H
