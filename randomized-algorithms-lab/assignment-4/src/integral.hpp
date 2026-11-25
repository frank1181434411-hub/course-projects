#pragma once
#include<bits/stdc++.h>

namespace integral_lab
{
using namespace std;
using u64=uint64_t;
using Clock=chrono::steady_clock;
struct Uniform
{
    mt19937_64& rng;
    //沿用作业三的52位中点网格，均匀数严格落在(0,1)。
    double operator()() {return (double(rng()>>12)+0.5)*0x1p-52;}
};
struct Moments
{
    u64 count=0;
    double mean=0,m2=0;
    void add(double x)
    {
        ++count;
        const double delta=x-mean;
        mean+=delta/double(count);
        m2+=delta*(x-mean);
    }
    double variance() const {return m2/double(count-1);}
};
struct Result
{
    double estimate=0,variance=0,standard_error=0;
    u64 evaluations=0;
};
inline double integrand(double x) {return exp(-x*x);}
inline void validate(double a,double b,u64 n,u64 k)
{
    if(!isfinite(a) || !isfinite(b) || !(a<b) || !isfinite(b-a))
        throw invalid_argument("require finite a<b and finite interval width");
    if(n<2 || n>10000000) throw invalid_argument("N must be in 2..10000000");
    if(k==0 || k>n/2) throw invalid_argument("K must allow at least two samples per stratum");
    const double h=(b-a)/double(k);
    if(h<=0 || !isfinite(h*h) || (k>1 && (!(a+h>a) || !(b-h<b))))
        throw invalid_argument("stratum width is outside supported floating-point range");
}

//调用前检查参数。K=1时同一循环即为普通均值法。
template<class Function,class Sampler>
Result integrate(double a,double b,u64 n,u64 k,Function f,Sampler& uniform)
{
    Result out;
    const double h=(b-a)/double(k);
    const u64 per=n/k,extra=n%k;
    for(u64 j=0;j<k;++j)
    {
        Moments layer;
        const double left=a+double(j)*h;
        const u64 count=per+(j<extra);
        for(u64 i=0;i<count;++i) layer.add(f(left+h*uniform()));
        out.estimate+=h*layer.mean;
        out.variance+=h*h*(layer.variance()/double(count));
        out.evaluations+=layer.count;
    }
    out.standard_error=sqrt(out.variance);
    return out;
}
}
