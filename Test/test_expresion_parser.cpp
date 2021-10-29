#include <math.h>
#include <iostream>
#include "test_expression_parser.h"
#include "../expression_parser.h"
#include "../expression_jit.h"

#include "../value_traits.h"

/*void dump_out(const std::vector<uint8_t>&vec)
{
    for(auto b:vec)
    {
        if(b<16)std::cout<<'0';
        std::cout<<std::hex<<(int)b<<' ';
    }
}*/

using real_t=double;

struct range_t
{
    real_t min;
    real_t max;
    real_t inc;
};

template<class F1,class F2>
real_t accumulate_error(const F1&f1,const F2&f2,range_t rg)
{
    real_t res=0;
    for(real_t i=rg.min;i<rg.max;i+=rg.inc)
    {
        res=std::max(res,abs(f1(i)-f2(i)));
    }
    return res;
}

template<class F1,class F2>
real_t accumulate_error(const F1&f1,const F2&f2,
                        range_t rg1,range_t rg2)// for any more ranges?..
{
    real_t res=0;
    for(real_t i=rg1.min;i<rg1.max;i+=rg1.inc)
    {
        for(real_t j=rg2.min;j<rg2.max;j+=rg2.inc)
        {
            res=std::max(res,abs(f1(i,j)-f2(i,j)));
        }
    }
    return res;
}

template<class F1,class F2>
real_t accumulate_error(const F1&f1,const F2&f2,
                        range_t rg1,range_t rg2,range_t rg3)// for any more ranges?..
{
    real_t res=0;
    for(real_t i=rg1.min;i<rg1.max;i+=rg1.inc)
    {
        for(real_t j=rg2.min;j<rg2.max;j+=rg2.inc)
        {
            for(real_t k=rg3.min;k<rg3.max;k+=rg3.inc)
            {
                res=std::max(res,abs(f1(i,j,k)-f2(i,j,k)));
            }
        }
    }
    return res;
}

static std::string GenerateDeepTree(const std::string&frag,int n)
{
    std::string res=frag;
    for(int i=0;i<n;++i)
    {
        res='('+res+')';
        auto temp=res+'+';
        temp.append(res);
        res=std::move(temp);
    }
    return res;
}

static std::string GenerateDeepTree(const std::string&frag,int n,const std::string&fun)
{
    std::string res=fun;
    res.push_back('(');
    res.append(GenerateDeepTree(frag,n));
    res.push_back(')');
    return res;
}

template<class functor_t,class...ranges_t>
void aux_test_arithmetic(int number,functor_t f,const std::string&body,
                         const std::vector<std::string>&args,const ranges_t&...rgs)
{
    {
        CParser parser;
        auto exp_=parser.Parse(body,args);
        assert(exp_);
        real_t error=accumulate_error(exp_,f,rgs...);
        //bool check=value_traits<real_t>::almost_zero(error);
        std::cout<<"Interpreter error "<<number<<": "<<error<<'\n';

    }
    {
        Cx87Compiler comp;
        auto exp_=comp.Compile(body,args);
        assert(exp_);
        real_t error=accumulate_error(exp_,f,rgs...);
        //bool check=value_traits<real_t>::almost_zero(error);
        std::cout<<"x87 error "<<number<<": "<<error<<'\n';;
    }
    {
        CSSECompiler comp;
        auto exp_=comp.Compile(body,args);
        assert(exp_);
        real_t error=accumulate_error(exp_,f,rgs...);
        //bool check=value_traits<real_t>::almost_zero(error);
        std::cout<<"SSE error "<<number<<": "<<error<<'\n';
    }
    std::cout<<"=========================================\n";
}

void test_expression()
{
    aux_test_arithmetic(1,[](real_t x){return (1+2)*x;},"(1+2)*x",{"x"},
                        range_t{0,1,0.1});

    aux_test_arithmetic(2,[](real_t x,real_t y){return (y+2)*x;},"(y+2)*x",{"x","y"},
                        range_t{0,1,0.1},range_t{0,1,0.1});


    {
        std::string body="1 / (0.3 + s) * cos (4.0 * time-4.0 * s)";
        std::vector<std::string> vars={"s","t","time"};
        auto check_fun=[](real_t s,real_t t,real_t time)
        {
            return  1/ (0.3 + s) * cos (4.00 * time-4.0 * s);
        };
        aux_test_arithmetic(3,check_fun,body,vars,
                            range_t{0,5,0.5},range_t{0,3,0.1},range_t{0,1,0.1});
    }
    {
        std::vector<std::string> vars={"x","y"};
        std::string body="(sqrt(1.567*abs(y))+2*y)*x+cos(+ x-y*sin(x))/(1.8000+x*x)";
        auto check_fun=[](real_t x,real_t y)
        {
            return (sqrt(1.567*abs(y))+2*y)*x+cos(+ x-y*sin(x))/(1.8000+x*x);
        };
        aux_test_arithmetic(4,check_fun,body,vars,
                            range_t{0,1,0.1},range_t{0,1,0.1});
    }

    // Test transcend functions
    aux_test_arithmetic(5,[](real_t x){return atan(x);},"atan(x)",{"x"},
                        range_t{-10,10,0.02});

    aux_test_arithmetic(6,[](real_t x){return acos(x);},"acos(x)",{"x"},
                        range_t{-0.999,0.999,0.02});

    aux_test_arithmetic(7,[](real_t x){return asin(x);},"asin(x)",{"x"},
                        range_t{-0.999,0.999,0.02});

    aux_test_arithmetic(8,[](real_t x){return log(x);},"log(x)",{"x"},
                        range_t{0.001,10,0.2});

    aux_test_arithmetic(9,[](real_t x){return exp(x);},"exp(x)",{"x"},
                        range_t{-10,10,0.1});

    //test stack overflow
    aux_test_arithmetic(10,[](real_t x){return (1<<7)*x;},GenerateDeepTree("x",7),{"x"},//max depth
                        range_t{0.00,10,0.2});

    aux_test_arithmetic(11,[](real_t x){return atan(128*x);},GenerateDeepTree("x",7,"atan"),{"x"},//max depth
                        range_t{-10,10,0.2});

    aux_test_arithmetic(12,[](real_t x){return acos(64*x/200.);},GenerateDeepTree("x/200",6,"acos"),{"x"},//max depth
                        range_t{-0.999,0.999,0.02});

    aux_test_arithmetic(13,[](real_t x){return asin(64*x/200.);},GenerateDeepTree("x/200",6,"asin"),{"x"},//max depth
                        range_t{-0.999,0.999,0.02});

    aux_test_arithmetic(14,[](real_t x){return log(128*x);},GenerateDeepTree("x",7,"log"),{"x"},//max depth
                        range_t{0.001,10,0.2});

    aux_test_arithmetic(15,[](real_t x){return 1.0;},"exp(sin(x)*sin(x)+cos(x)*cos(x)-1)",{"x"},
                        range_t{-10,10,0.1});
    //test error
    CParser parser;
    {

        CParseError error;
        auto exp=parser.Parse("x+z",{"x","y"},error);
        assert(error&&error.Status()==CParseError::unknown_variable_id);
    }
    {
        CParseError error;
        auto exp=parser.Parse("x+sis(y)",{"x","y"},error);
        assert(error&&error.Status()==CParseError::unknown_function_id);

    }
    {
        CParseError error;
        auto exp=parser.Parse("x+y)",{"x","y"},error);
        assert(error);
    }
    {
        CParseError error;
        auto exp=parser.Parse("()*x+y",{"x","y"},error);
        assert(error);
    }
    {
        CParseError error;
        auto exp=parser.Parse("(x+)y",{"x","y"},error);
        assert(error);
    }
    std::cout<<"Arithmetic tested\n";
}









