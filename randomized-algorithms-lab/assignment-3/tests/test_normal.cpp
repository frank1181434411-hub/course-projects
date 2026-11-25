#include "../src/normal.hpp"
using namespace std;
using namespace normal_lab;
int checks=0;
void check(bool ok) {++checks;if(!ok) throw runtime_error("check "+to_string(checks));}
struct Sequence
{
    vector<double> values;
    size_t pos=0;
    double* output=nullptr;
    double operator()()
    {
        //前两个数产生拒绝；下一候选开始时输出仍是哨兵。
        if(output && pos==2) check(output[0]==123 && output[1]==456);
        return values.at(pos++);
    }
};
int main()
{
    try
    {
        auto [x,y]=box_pair(exp(-2.0),0.125);
        check(abs(x-sqrt(2.0))<1e-14);check(abs(y-sqrt(2.0))<1e-14);
        for(size_t n:{1,2,3})
        {
            Sequence pair_input{{exp(-2.0),0.125,exp(-0.5),0.0}};
            vector<double> values(n+1,999);
            box_muller(values.data(),n,pair_input);
            check(abs(values[0]-sqrt(2.0))<1e-14);
            if(n>=2) check(abs(values[1]-sqrt(2.0))<1e-14);
            if(n==3) check(abs(values[2]-1)<1e-14);
            check(values[n]==999);check(pair_input.pos==2*((n+1)/2));
        }
        double out[2]={123,456};
        Sequence seq{{exp(-4.0),0.9,exp(-1.0),0.5,0.25},0,out};
        Counts counts;
        rejection<true>(out,1,seq,counts);
        check(abs(out[0]+1)<1e-14);check(out[1]==456);check(seq.pos==5);
        check(counts.candidates==2 && counts.rejected==1 && counts.candidates==1+counts.rejected);
        Sequence positive{{exp(-1.0),0.5,0.75}};
        rejection<true>(out,1,positive,counts);check(abs(out[0]-1)<1e-14);
        for(string method:{"clt","box_muller","rejection"})
        for(int m:{3,12,48}) for(size_t n:{1,2,3,10,101,1000})
        {
            vector<double> a(n+2,999),b(n+2,999);
            mt19937_64 ra(12345),rb(12345);Uniform ua{ra},ub{rb};
            auto c=generate<true>(method,a.data()+1,n,m,ua);
            generate<true>(method,b.data()+1,n,m,ub);
            check(a==b);check(a.front()==999 && a.back()==999);
            for(size_t i=1;i<=n;++i)
            {
                check(isfinite(a[i]) && a[i]!=999);
                if(method=="clt") check(abs(a[i])<=sqrt(3.0*m)+1e-12);
            }
            if(method=="rejection") check(c.candidates==n+c.rejected);
        }
        for(auto [n,m]:vector<pair<size_t,int>>{{0,12},{10000001,12},{1,0},{1,-1},{1,1000001}})
        {
            bool caught=false;try {validate("clt",n,m);} catch(const invalid_argument&) {caught=true;}check(caught);
        }
        bool caught=false;try {validate("bad",1,12);} catch(const invalid_argument&) {caught=true;}check(caught);
        mt19937_64 rng(1);Uniform u{rng};for(int i=0;i<10000;++i) {double v=u();check(v>0 && v<1);}
        check((double(u64(0))+0.5)*0x1p-52>0);
        check((double((u64(1)<<52)-1)+0.5)*0x1p-52<1);
        cout<<"PASS "<<checks<<" checks\n";
    }
    catch(const exception& e) {cerr<<e.what()<<'\n';return 1;}
}
