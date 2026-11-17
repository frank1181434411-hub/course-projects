#include "normal.hpp"
using namespace std;
using namespace normal_lab;
u64 uint_arg(const string& s)
{
    u64 v=0;
    auto [end,error]=from_chars(s.data(),s.data()+s.size(),v);
    if(s.empty() || error!=errc() || end!=s.data()+s.size()) throw invalid_argument("invalid unsigned integer");
    return v;
}
int main(int argc,char** argv)
{
    try
    {
        map<string,string> args;
        vector<string> keys={"--mode","--method","--n","--m","--seed","--trials","--warmups","--output"};
        for(int i=1;i<argc;i+=2)
        {
            if(i+1==argc || find(keys.begin(),keys.end(),argv[i])==keys.end() || !args.emplace(argv[i],argv[i+1]).second)
                throw invalid_argument("invalid, duplicate or incomplete option");
        }
        for(auto& k:keys) if(!args.count(k)) throw invalid_argument("missing option: "+k);
        auto num=[&](const char* k){return uint_arg(args.at(k));};
        u64 n=num("--n"),m=num("--m"),seed=num("--seed"),trials=num("--trials"),warmups=num("--warmups");
        if(m>1000000 || !trials || trials>10000 || warmups>10000) throw invalid_argument("invalid m/trials/warmups");
        string mode=args.at("--mode"),method=args.at("--method");
        validate(method,n,int(m));
        if(mode!="distribution" && mode!="performance") throw invalid_argument("invalid mode");
        vector<double> out(n); //分配和页面初始化在计时外。
        mt19937_64 seeds(seed);
        ofstream file(args.at("--output"),mode=="distribution"?ios::binary:ios::out);
        if(!file) throw runtime_error("cannot open output");
        if(mode=="performance") file<<"method,m,n,repeat,seed,elapsed_ns,checksum\n";
        for(u64 rep=0;rep<(mode=="distribution"?1:warmups+trials);++rep)
        {
            u64 trial_seed=mode=="distribution"?seed:seeds();
            mt19937_64 rng(trial_seed);
            Uniform uniform{rng};
            Counts counts;
            long long elapsed=0;
            if(mode=="distribution") counts=generate<true>(method,out.data(),n,int(m),uniform);
            else
            {
                auto start=chrono::steady_clock::now();
                generate(method,out.data(),n,int(m),uniform);
                elapsed=chrono::duration_cast<chrono::nanoseconds>(chrono::steady_clock::now()-start).count();
            }
            long double checksum=0;
            for(double x:out)
            {
                if(!isfinite(x)) throw runtime_error("nonfinite output");
                if(method=="clt" && abs(x)>sqrt(3.0*m)+1e-10) throw runtime_error("CLT range");
                checksum+=x;
            }
            if(mode=="distribution")
            {
                file.write(reinterpret_cast<const char*>(out.data()),streamsize(n*sizeof(double)));
                cout<<"{\"candidates\":"<<counts.candidates<<",\"rejected\":"<<counts.rejected<<",\"n\":"<<n<<"}\n";
            }
            else if(rep>=warmups) file<<method<<','<<m<<','<<n<<','<<rep-warmups<<','<<trial_seed<<','<<elapsed<<','<<setprecision(17)<<double(checksum)<<'\n';
        }
        file.flush();
        if(!file) throw runtime_error("write failed");
    }
    catch(const exception& e) {cerr<<"ERROR: "<<e.what()<<'\n';return 1;}
}
