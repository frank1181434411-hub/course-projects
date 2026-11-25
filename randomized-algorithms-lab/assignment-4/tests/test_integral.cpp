#include "../src/integral.hpp"

using namespace std;
using namespace integral_lab;
int checks=0;
void check(bool ok)
{
    ++checks;
    if(!ok) throw runtime_error("check "+to_string(checks));
}
int main()
{
    try
    {
        for(u64 k:{1,4,16,64})
        {
            double a=-2,b=3;
            mt19937_64 rng(42);
            Uniform uniform{rng};
            auto constant=integrate(a,b,131,k,[](double){return 2.5;},uniform);
            check(abs(constant.estimate-12.5)<1e-13);
            check(constant.variance==0 && constant.standard_error==0);
            check(constant.evaluations==131);
        }
        //确定性输入核对映射、长度因子以及分层方差合并。
        vector<double> points;
        array<double,5> values={0.25,0.5,0.75,0.25,0.75};
        size_t i=0;
        auto injected=[&](){return values.at(i++);};
        auto linear=integrate(1,5,5,2,[&](double x){points.push_back(x);return x;},injected);
        check(points==vector<double>({1.5,2,2.5,3.5,4.5}));
        check(linear.estimate==12 && linear.evaluations==5 && i==5);
        check(abs(linear.variance-4.0/3)<1e-14);
        check(abs(linear.standard_error-sqrt(4.0/3))<1e-14);
        for(double b:{0.5,1.0,2.0,4.0})
        {
            mt19937_64 rng(123),copy(123),iid(123);
            Uniform u{rng},v{copy},w{iid};
            auto ans=integrate(0,b,1001,1,integrand,u);
            auto again=integrate(0,b,1001,1,integrand,v);
            Moments plain;
            for(int j=0;j<1001;++j) plain.add(integrand(b*w()));
            check(plain.count==ans.evaluations && rng==copy && rng==iid);
            check(ans.estimate==again.estimate && ans.variance==again.variance);
            check(ans.estimate==b*plain.mean && ans.variance==b*b*(plain.variance()/1001));
            check(isfinite(ans.estimate) && isfinite(ans.variance) && ans.variance>=0);
            check(isfinite(ans.standard_error) && ans.standard_error>=0);
        }
        const double inf=numeric_limits<double>::infinity(),nan=numeric_limits<double>::quiet_NaN();
        for(auto [a,b,n,k]:vector<tuple<double,double,u64,u64>>{
                {nan,1,4,1},{0,inf,4,1},{-inf,1,4,1},{1,1,4,1},{2,1,4,1},
                {0,1,0,1},{0,1,1,1},{0,1,10000001,1},{0,1,4,0},{0,1,5,3},
                {0,1,4,5},{-1e308,1e308,4,1},{0,1e200,4,1},{1,nextafter(1.0,2.0),8,4}})
        {
            bool caught=false;
            try {validate(a,b,n,k);} catch(const invalid_argument&) {caught=true;}
            check(caught);
        }
        cout<<"PASS "<<checks<<" C++ checks\n";
    }
    catch(const exception& e) {cerr<<e.what()<<'\n';return 1;}
}
