#pragma once
#include<bits/stdc++.h>

namespace normal_lab
{
using namespace std;
using u64=uint64_t;
struct Uniform
{
    mt19937_64& rng;
    //52位中点网格，严格处于(0,1)，避免log(0)和舍入到1。
    double operator()() {return (double(rng()>>12)+0.5)*0x1p-52;}
};
struct Counts {u64 candidates=0,rejected=0;};
inline void validate(const string& method,size_t n,int m)
{
    if(n==0 || n>10000000) throw invalid_argument("n must be in 1..10000000");
    if(method!="clt" && method!="box_muller" && method!="rejection")
        throw invalid_argument("unknown method");
    if(m<=0 || m>1000000) throw invalid_argument("m must be in 1..1000000");
}
inline pair<double,double> box_pair(double u1,double u2)
{
    double r=sqrt(-2*log(u1)),theta=2*acos(-1.0)*u2;
    return {r*cos(theta),r*sin(theta)};
}
template<class Sampler>
void clt(double* out,size_t n,int m,Sampler& uniform)
{
    const double scale=sqrt(12.0/m);
    for(size_t i=0;i<n;++i)
    {
        double sum=0;
        for(int j=0;j<m;++j) sum+=uniform();
        out[i]=scale*(sum-m/2.0);
    }
}
template<class Sampler>
void box_muller(double* out,size_t n,Sampler& uniform)
{
    for(size_t i=0;i<n;i+=2)
    {
        double u1=uniform(),u2=uniform();
        auto [x,y]=box_pair(u1,u2);
        out[i]=x;
        if(i+1<n) out[i+1]=y;
    }
}
template<bool Diagnostic,class Sampler>
void rejection(double* out,size_t n,Sampler& uniform,Counts& counts)
{
    size_t accepted=0;
    while(accepted<n)
    {
        double u1=uniform(),u2=uniform(),x=-log(u1);
        if constexpr(Diagnostic) ++counts.candidates;
        //使用对数判据；拒绝的候选不写入输出，也不抽取符号。
        if(-2*log(u2)<(x-1)*(x-1))
        {
            if constexpr(Diagnostic) ++counts.rejected;
            continue;
        }
        out[accepted++]=uniform()<0.5?-x:x;
    }
}
template<bool Diagnostic=false,class Sampler>
Counts generate(const string& method,double* out,size_t n,int m,Sampler& uniform)
{
    Counts counts;
    if(method=="clt") clt(out,n,m,uniform);
    else if(method=="box_muller") box_muller(out,n,uniform);
    else rejection<Diagnostic>(out,n,uniform,counts);
    return counts;
}
}
