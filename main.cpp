#include <iostream>
#include <algorithm>

#include "Test/test_encoding.h"
#include "Test/test_expression_parser.h"
#include "Test/bench_expression.h"

/*
//Minimal encoding sample

#include "encoding.h"

int main()
{
    using namespace encoding_x64;
    CCode c;
    c.finit();
    c.fld(qword_ptr(rax));
    c.fadd(qword_ptr(rax,8));
    c.fstp(qword_ptr(rax));
    c.ret();
    auto f=c.Compile();
    std::cout<<f.execute<double,double,double>(1.5,2.5)<<std::endl;
    return 0;
}
*/

/*
//Minimal jit compilance sample


#include <assert.h>
#include "expression_jit.h"

int main()
{
    Cx87Compiler comp;
    auto expr=comp.Compile("exp(sin(x)*sin(x)+cos(x)*cos(x)-1)",{"x"});
    assert(expr);
    std::cout<<expr(0.1234)<<std::endl;
    return 0;
}

*/



int main()
{
    test_encoding();
    test_expression();
    bench_expression();
    return 0;
}

