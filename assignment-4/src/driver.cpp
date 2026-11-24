#include "integral.hpp"

using namespace std;
using namespace integral_lab;
u64 read_uint(const string& s)
{
    u64 value=0;
    auto [end,error]=from_chars(s.data(),s.data()+s.size(),value);
    if(error!=errc() || end!=s.data()+s.size()) throw invalid_argument("invalid unsigned integer");
    return value;
}
double read_double(const string& s)
{
    size_t pos=0;
    double value=stod(s,&pos);
    if(pos!=s.size()) throw invalid_argument("invalid endpoint");
    return value;
}
int main(int argc,char** argv)
{
    try
    {
        if(argc!=3) throw invalid_argument("usage: integral.exe jobs.csv core.csv");
        ifstream jobs(argv[1]);
        if(!jobs) throw runtime_error("cannot open jobs CSV");
        string line;
        if(!getline(jobs,line) || line!="a,b,N,K,trial_id,seed,warmup")
            throw invalid_argument("invalid job header");
        ofstream out(argv[2]);
        if(!out) throw runtime_error("cannot open output CSV");
        out<<"a,b,N,K,trial_id,seed,warmup,estimate,variance,standard_error,evaluations,elapsed_ns\n"
           <<setprecision(17);
        while(getline(jobs,line))
        {
            istringstream row(line);
            array<string,7> fields;
            for(auto& s:fields)
                if(!getline(row,s,',')) throw invalid_argument("missing job field");
            if(row.peek()!=EOF || line.back()==',') throw invalid_argument("extra job field");
            const double a=read_double(fields[0]),b=read_double(fields[1]);
            const u64 n=read_uint(fields[2]),k=read_uint(fields[3]),trial=read_uint(fields[4]);
            const u64 seed=read_uint(fields[5]),warmup=read_uint(fields[6]);
            validate(a,b,n,k);
            if(warmup>1) throw invalid_argument("warmup must be 0 or 1");
            mt19937_64 rng(seed);
            Uniform uniform{rng};
            const auto start=Clock::now();
            const auto result=integrate(a,b,n,k,integrand,uniform);
            const auto elapsed=chrono::duration_cast<chrono::nanoseconds>(Clock::now()-start).count();
            if(result.evaluations!=n || !isfinite(result.estimate) || !isfinite(result.variance)
                || result.variance<0 || !isfinite(result.standard_error))
                throw runtime_error("invalid integration result");
            out<<a<<','<<b<<','<<n<<','<<k<<','<<trial<<','<<seed<<','<<warmup<<','
               <<result.estimate<<','<<result.variance<<','<<result.standard_error<<','
               <<result.evaluations<<','<<elapsed<<'\n';
        }
        out.flush();
        if(!out || jobs.bad()) throw runtime_error("CSV I/O error");
    }
    catch(const exception& e) {cerr<<"ERROR: "<<e.what()<<'\n';return 1;}
}
