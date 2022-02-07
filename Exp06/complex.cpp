#include <complex.h>
#include <string>
#include <cmath>
complex complexAdd(complex a,complex b)
{
    complex ret;
    ret.re=a.re+b.re;
    ret.im=a.im+b.im;
    return ret;
}

complex operator+(const complex &a, const complex &b )
{
    complex ret;
    ret.re=a.re+b.re;
    ret.im=a.im+b.im;
    return ret;
}

complex operator-(const complex &a, const complex &b )
{
    complex ret;
    ret.re=a.re-b.re;
    ret.im=a.im-b.im;
    return ret;
}


complex complexMult(complex a, complex b)
{
    complex ret;
    ret.re=a.re*b.re-a.im*b.im;
    ret.im=a.im*b.re+a.re*b.im;
    return ret;
}

complex operator*(const complex &a, const complex &b )
{
    complex ret;
    ret.re=a.re*b.re-a.im*b.im;
    ret.im=a.im*b.re+a.re*b.im;
    return ret;
}

complex complexBuild(float re, float im)
{
    complex cx;
    cx.re=re;
    cx.im=im;
    return cx;
}

complex complexBuild(float re)
{
    complex cx;
    cx.re=re * 1.0f;
    cx.im=0.0f;
    return cx;
}

std::string toString(const complex &a)
{
    std::string str = "";
    str+=std::to_string(a.re);
    if(a.im >= 0) {
        str+=" + ";
        str+=std::to_string(a.im);
    }
    else{
        str+=" - ";
        str+=std::to_string(-a.im);
    }
    str+="i";
    return str;
}

float complexAbs(const complex &a)
{
    return std::sqrt(a.re*a.re+a.im*a.im);
}

complex complexExp(const complex &a)
{
    return complexBuild(exp(a.re)*cos(a.im),exp(a.re)*sin(a.im));
}
