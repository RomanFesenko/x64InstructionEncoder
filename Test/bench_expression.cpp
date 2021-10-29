#include <math.h>
#include <iostream>

#include "../value_traits.h"
#include "../timing.h"

#include "../expression_parser.h"
#include "../expression_jit.h"

#include "bench_expression.h"


struct range_t
{
    double min;
    double max;
    double inc;
};

template<class f_t>
double max(const f_t&f,range_t rg1,range_t rg2,range_t rg3)
{
    double res=f(rg1.min,rg2.min,rg3.min);
    for(double i=rg1.min;i<rg1.max;i+=rg1.inc)
    {
        for(double j=rg2.min;j<rg2.max;j+=rg2.inc)
        {
            for(double k=rg3.min;k<rg3.max;k+=rg3.inc)
            {
                double temp=f(i,j,k);
                if(temp>res) res=temp;
            }
        }
    }
    return res;
}


void bench_expression()
{
    CTimer timer;
    auto native=[](double x,double y,double z)
    {
        return (x*y+x*z+y*z)/(x*x+y*y+z*z)+(x+y+z)/abs(x*y*z+1)-(x+y+3)*(z*x+5);
    };
    range_t r1={0.1,10,0.01};range_t r2={0.1,10,0.1};range_t r3={0.1,10,0.1};

    timer.Restart();
    double v1=max(native,r1,r2,r3);
    timer.Stop();
    std::cout<<"Native:"<<timer.Pass<>()<<'\n';

    std::string body="(x*y+x*z+y*z)/(x*x+y*y+z*z)+(x+y+z)/abs(x*y*z+1)-(x+y+3)*(z*x+5)";
    std::vector<std::string> args={"x","y","z"};


    CParser parser;
    auto pr_exp=parser.Parse(body,args);
    assert(pr_exp);

    timer.Restart();
    double v2=max(pr_exp,r1,r2,r3);
    timer.Stop();
    std::cout<<"Interpreter:"<<timer.Pass<>()<<'\n';

    Cx87Compiler comp;
    auto x87_exp_=comp.Compile(body,args);
    assert(x87_exp_);

    timer.Restart();
    double v3=max(x87_exp_,r1,r2,r3);
    timer.Stop();
    std::cout<<"x87 JIT:"<<timer.Pass<>()<<'\n';


    CSSECompiler ssecomp;
    auto sse_exp_=comp.Compile(body,args);
    assert(sse_exp_);

    timer.Restart();
    double v4=max(sse_exp_,r1,r2,r3);
    timer.Stop();
    std::cout<<"SSE JIT:"<<timer.Pass<>()<<'\n';

    double d2=v2-v1;
    double d3=v3-v1;
    double d4=v4-v1;
    std::cout<<"d2,d3,d4:"<<d2<<", "<<d3<<", "<<d4<<'\n';
}









