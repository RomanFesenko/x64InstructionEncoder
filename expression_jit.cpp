#include <iostream>

#include "expression_jit.h"

//x87 transcend function

void GenExp(encoding_x64::CCode&c)
{
    using namespace encoding_x64;
    c.fldl2e();
    c.fmulp();
    c.fld1();
    c.fscale();
    c.fxch(st1);
    c.fld1();
    c.fxch(st1);
    c.fprem();
    c.f2xm1();
    c.faddp();
    c.fmulp();
}

void GenTan(encoding_x64::CCode&c)
{
    using namespace encoding_x64;
    c.fptan();
    c.fstp(st0);
}

void GenAtan(encoding_x64::CCode&c)
{
    using namespace encoding_x64;
    c.fld1();
    c.fpatan();
}

void GenAcos(encoding_x64::CCode&c)
{
    using namespace encoding_x64;
    c.fld(st0);
    c.fld1();
    c.fadd(st1,st0);
    c.fsubr(st2,st0);
    c.fstp(st0);// pop 1
    c.fdivp();
    c.fsqrt();
    c.fld1();
    c.fpatan();//st0=atan((1-x)/(1+x))
    c.fld(st0);
    c.faddp();
}

void GenAsin(encoding_x64::CCode&c)
{
    using namespace encoding_x64;
    c.fld(st0);
    c.fmul(st0,st1);
    c.fld1();
    c.fsubrp();//st1=x,st0=1-x*x
    c.fsqrt();
    c.fpatan();
}

void GenLn(encoding_x64::CCode&c)
{
    using namespace encoding_x64;
    c.fld1();
    c.fxch(st1);
    c.fyl2x();
    c.fldl2e();
    c.fdivp();
}

// Traverse AST for mark it and count of constantss

std::shared_ptr<std::vector<double>> InitTraverse(CASTNode&root,std::vector<int>&marks)
{
    int total_const=0;
    marks.clear();
    auto out_visitor=[&](const auto&node)
    {
        using type_t=std::decay_t<decltype(node)>;
        check_ast_node<type_t>();
        node.SetIndex(marks.size());
        if constexpr(std::is_same_v<type_t,CBinaryOp>)
        {
            int m_l=marks[node.Left().Index()];
            int m_r=marks[node.Right().Index()];
            marks.push_back((m_l==m_r)? m_l+1:std::max(m_l,m_r));
        }
        else if constexpr(std::is_same_v<type_t,CNeg>)
        {
            marks.push_back(marks[node.Child().Index()]);
        }
        else if constexpr(std::is_same_v<type_t,CFunction>)
        {
            marks.push_back(marks[node.Arg(0).Index()]);
        }
        else // CVariable,CConstant
        {
            marks.push_back(1);
        }
        if constexpr(std::is_same_v<type_t,CConstant>)
        {
            total_const++;
        }
    };
    ast_traverser ast_tr([](auto&){},out_visitor);
    ast_tr.traverse(root);
    auto consts_=std::make_shared<std::vector<double>>();
    consts_->reserve(total_const);
    return consts_;
}

///////////////////////////////////////////////////////////
//   Cx87Compiler - generate instruction for x87 coprocessor
///////////////////////////////////////////////////////////

CCompiledExpession Cx87Compiler::Compile(const std::string&body,
                                         const std::vector<std::string>&args_name)
{
    CParser parser;
    double*args=reinterpret_cast<double*>(encoding_x64::function::m_args);
    auto root=parser.BuildAST(body,args_name,args);
    if(!root) return CCompiledExpession{0};
    m_const=InitTraverse(*root,m_marks);
    assert(m_Mark(*root)<=8);
    m_stack_size=0;
    c.Clear();
    c.finit();
    root->AcceptVisitor(*this);
    return m_PostProcess(args_name.size());
}

void Cx87Compiler::Visit(const CNeg&neg)
{
    neg.Child().AcceptVisitor(*this);
    c.fchs();
}

void Cx87Compiler::Visit(const CBinaryOp&bin_op)
{
    int m_l=m_Mark(bin_op.Left());
    int m_r=m_Mark(bin_op.Right());
    bool reverse=m_l<m_r;
    if(!reverse)
    {
        bin_op.Left().AcceptVisitor(*this);
        bin_op.Right().AcceptVisitor(*this);
    }
    else
    {
        bin_op.Right().AcceptVisitor(*this);
        bin_op.Left().AcceptVisitor(*this);
    }
    switch(bin_op.Type())
    {
        case CBinaryOp::add_id:
        c.faddp();
        break;

        case CBinaryOp::mul_id:
        c.fmulp();
        break;

        case CBinaryOp::sub_id:
        if(!reverse) c.fsubp();
        else         c.fsubrp();
        break;

        case CBinaryOp::div_id:
        if(!reverse) c.fdivp();
        else         c.fdivrp();
        break;

        default:assert(false);
    }
    m_stack_size--;
}


void Cx87Compiler::Visit(const CFunction&fun_op)
{
    assert(fun_op.Type()!=CArityFunctor::other_id);
    assert(fun_op.Arity()==1);
    fun_op.Arg(0).AcceptVisitor(*this);
    switch(fun_op.Type())
    {
        case CArityFunctor::abs_id:
        c.fabs();
        break;

        case CArityFunctor::sqrt_id:
        c.fsqrt();
        break;

        case CArityFunctor::exp_id:
        m_GenerateExpCode();
        break;

        case CArityFunctor::log_id:
        m_GenerateLneCode();
        break;

        case CArityFunctor::cos_id:
        c.fcos();
        break;

        case CArityFunctor::sin_id:
        c.fsin();
        break;

        case CArityFunctor::tan_id:
        m_GenerateTanCode();
        break;

        case CArityFunctor::asin_id:
        m_GenerateAsinCode();
        break;

        case CArityFunctor::acos_id:
        m_GenerateAcosCode();
        break;

        case CArityFunctor::atan_id:
        m_GenerateAtanCode();
        break;

        default: assert(false);
    }
}

void Cx87Compiler::Visit(const CVariable&var)
{
    using namespace encoding_x64;
    assert(m_stack_size<8);
    uint64_t addr=reinterpret_cast<uint64_t>(var.ValuePtr());
    c.mov(rcx,addr);
    c.fld(qword_ptr(rcx));
    m_stack_size++;
}

void Cx87Compiler::Visit(const CConstant&const_)
{
    using namespace encoding_x64;
    assert(m_stack_size<8);
    m_const->push_back(const_.Value());
    uint64_t addr=reinterpret_cast<uint64_t>(m_const->data()+m_const->size()-1);
    c.mov(rcx,addr);
    c.fld(qword_ptr(rcx));
    m_stack_size++;
}

CCompiledExpession Cx87Compiler::m_PostProcess(int arity)
{
    using namespace encoding_x64;
    assert(m_stack_size==1);
    c.fstp(qword_ptr(rax));
    c.ret();
    CCompiledExpession res(arity);
    res.m_function=c.Compile();
    res.m_const=std::move(m_const);
    return res;
}

// Generate code for some transcendent functions

void Cx87Compiler::m_GenerateTanCode()// tg(x)
{
    using namespace encoding_x64;
    if(m_stack_size<8) GenTan(c);
    else
    {
        uint64_t addr=reinterpret_cast<uint64_t>(m_store);
        c.fxch(st1);
        c.mov(rcx,addr);
        c.fstp(qword_ptr(rcx));
        GenTan(c);
        c.fld(qword_ptr(rcx));
        c.fxch(st1);
    }
}

void Cx87Compiler::m_GenerateLneCode()//lne(x)
{
    using namespace encoding_x64;
    if(m_stack_size<8) GenLn(c);
    else
    {
        uint64_t addr=reinterpret_cast<uint64_t>(m_store);
        c.fxch(st1);
        c.mov(rcx,addr);
        c.fstp(qword_ptr(rcx));
        GenLn(c);
        c.fld(qword_ptr(rcx));
        c.fxch(st1);
    }
}

void Cx87Compiler::m_GenerateAtanCode()//atan(x)
{
    using namespace encoding_x64;
    if(m_stack_size<8) GenAtan(c);
    else
    {
        uint64_t addr=reinterpret_cast<uint64_t>(m_store);
        c.fxch(st1);
        c.mov(rcx,addr);
        c.fstp(qword_ptr(rcx));
        GenAtan(c);
        c.fld(qword_ptr(rcx));
        c.fxch(st1);
    }
}

//asin(x)=atan(x/sqrt(1-x*x))

void Cx87Compiler::m_GenerateAsinCode()
{
    using namespace encoding_x64;
    if(m_stack_size<7) GenAsin(c);
    else
    {
        uint64_t addr=reinterpret_cast<uint64_t>(m_store);
        c.fxch(st2);
        c.mov(rcx,addr);
        c.fstp(qword_ptr(rcx));
        c.fstp(qword_ptr(rcx,8));
        GenAsin(c);
        c.fld(qword_ptr(rcx,8));
        c.fld(qword_ptr(rcx));
        c.fxch(st2);
    }
}

//acos(x)=2*atan(1-x/1+x)

void Cx87Compiler::m_GenerateAcosCode()
{
    using namespace encoding_x64;
    if(m_stack_size<7) GenAcos(c);
    else
    {
        uint64_t addr=reinterpret_cast<uint64_t>(m_store);
        c.fxch(st2);
        c.mov(rcx,addr);
        c.fstp(qword_ptr(rcx));
        c.fstp(qword_ptr(rcx,8));
        GenAcos(c);
        c.fld(qword_ptr(rcx,8));
        c.fld(qword_ptr(rcx));
        c.fxch(st2);
    }
}

void Cx87Compiler::m_GenerateExpCode()
{
    using namespace encoding_x64;
    if(m_stack_size<7) GenExp(c);
    else
    {
        uint64_t addr=reinterpret_cast<uint64_t>(m_store);
        c.fxch(st2);
        c.mov(rcx,addr);
        c.fstp(qword_ptr(rcx));
        c.fstp(qword_ptr(rcx,8));
        GenExp(c);
        c.fld(qword_ptr(rcx,8));
        c.fld(qword_ptr(rcx));
        c.fxch(st2);
    }
}

///////////////////////////////////////////////////////////
//CSSECompiler- generate SSE  instruction
///////////////////////////////////////////////////////////

CCompiledExpession CSSECompiler::Compile(const std::string&body,
                                         const std::vector<std::string>&args_name)
{
    using namespace encoding_x64;
    CParser parser;
    double*args=reinterpret_cast<double*>(encoding_x64::function::m_args);
    auto root=parser.BuildAST(body,args_name,args);
    if(!root) return CCompiledExpession{0};

    m_const=InitTraverse(*root,m_marks);
    assert(m_Mark(*root)<=16);

    if(m_for_save=m_Mark(*root)-6;m_for_save>0)//need save xmm6...xmm15 in Windows
    {
        uint64_t addr=reinterpret_cast<uint64_t>(m_store);
        c.mov(rcx,addr);
        for(int i=0;i<m_for_save;++i) c.movsd(ptr(rcx,i*8),sse_register_t(i+6));
    }
    m_stack_top=m_Mark(*root);
    c.Clear();
    // need use x87 instruction for transcend functions
    m_87_init=false;
    root->AcceptVisitor(*this);
    return m_PostProcess(args_name.size());
}

void CSSECompiler::Visit(const CNeg&neg)
{
    neg.Child().AcceptVisitor(*this);
    m_MoveToFpu();
    c.fchs();
    m_MoveFromFpu();
}

void CSSECompiler::Visit(const CBinaryOp&bin_op)
{
    using namespace encoding_x64;
    int m_l=m_Mark(bin_op.Left());
    int m_r=m_Mark(bin_op.Right());
    bool reverse=m_l<m_r;
    if(!reverse)
    {
        bin_op.Left().AcceptVisitor(*this);
        bin_op.Right().AcceptVisitor(*this);
    }
    else
    {
        bin_op.Right().AcceptVisitor(*this);
        bin_op.Left().AcceptVisitor(*this);
    }
    switch(bin_op.Type())
    {
        case CBinaryOp::add_id:
        c.addsd(m_reg(1),m_reg(0));
        break;

        case CBinaryOp::mul_id:
        c.mulsd(m_reg(1),m_reg(0));
        break;

        case CBinaryOp::sub_id:
        if(!reverse) c.subsd(m_reg(1),m_reg(0));
        else
        {
            c.subsd(m_reg(0),m_reg(1));
            c.movsd(m_reg(1),m_reg(0));
        }
        break;

        case CBinaryOp::div_id:
        if(!reverse) c.divsd(m_reg(1),m_reg(0));
        else
        {
            c.divsd(m_reg(0),m_reg(1));
            c.movsd(m_reg(1),m_reg(0));
        }
        break;

        default:assert(false);
    }
    m_stack_top++;
}

void CSSECompiler::Visit(const CFunction&fun_op)
{

    assert(fun_op.Type()!=CArityFunctor::other_id);
    assert(fun_op.Arity()==1);
    fun_op.Arg(0).AcceptVisitor(*this);
    switch(fun_op.Type())
    {
        case CArityFunctor::abs_id:
        m_MoveToFpu();
        c.fabs();
        m_MoveFromFpu();
        break;

        case CArityFunctor::sqrt_id:
        c.sqrtsd(m_reg(0),m_reg(0));
        break;

        case CArityFunctor::exp_id:
        m_MoveToFpu();
        GenExp(c);
        m_MoveFromFpu();
        break;

        case CArityFunctor::log_id:
        m_MoveToFpu();
        GenLn(c);
        m_MoveFromFpu();
        break;

        case CArityFunctor::cos_id:
        m_MoveToFpu();
        c.fcos();
        m_MoveFromFpu();
        break;

        case CArityFunctor::sin_id:
        m_MoveToFpu();
        c.fsin();
        m_MoveFromFpu();
        break;

        case CArityFunctor::tan_id:
        m_MoveToFpu();
        GenTan(c);
        m_MoveFromFpu();
        break;

        case CArityFunctor::asin_id:
        m_MoveToFpu();
        GenAsin(c);
        m_MoveFromFpu();
        break;

        case CArityFunctor::acos_id:
        m_MoveToFpu();
        GenAcos(c);
        m_MoveFromFpu();
        break;

        case CArityFunctor::atan_id:
        m_MoveToFpu();
        GenAtan(c);
        m_MoveFromFpu();
        break;

        default: assert(false);
    }
}

void CSSECompiler::Visit(const CVariable&var)
{
    using namespace encoding_x64;
    assert(m_stack_top>0);
    uint64_t addr=reinterpret_cast<uint64_t>(var.ValuePtr());
    c.mov(rcx,addr);
    m_stack_top--;
    c.movsd(m_reg(0),ptr(rcx));
}

void CSSECompiler::Visit(const CConstant&const_)
{
    using namespace encoding_x64;
    assert(m_stack_top>0);
    m_const->push_back(const_.Value());
    uint64_t addr=reinterpret_cast<uint64_t>(m_const->data()+m_const->size()-1);
    c.mov(rcx,addr);
    m_stack_top--;
    c.movsd(m_reg(0),ptr(rcx));
}

CCompiledExpession CSSECompiler::m_PostProcess(int arity)
{
    using namespace encoding_x64;
    c.movsd(ptr(rax),m_reg(0));
    if(m_for_save>0)// restore mmx registers
    {
        uint64_t addr=reinterpret_cast<uint64_t>(m_store);
        c.mov(rcx,addr);
        for(int i=0;i<m_for_save;++i) c.movsd(sse_register_t(i+6),ptr(rcx,i*8));
    }
    c.ret();
    CCompiledExpession res(arity);
    res.m_function=c.Compile();
    res.m_const=std::move(m_const);
    return res;
}

void CSSECompiler::m_MoveToFpu()
{
    using namespace encoding_x64;
    if(!m_87_init)
    {
        c.finit();
        m_87_init=true;
    }
    static const uint64_t addr=reinterpret_cast<uint64_t>(&m_temp);
    c.mov(rcx,addr);
    c.movsd(ptr(rcx),m_reg(0));
    c.fld(qword_ptr(rcx));
}

void CSSECompiler::m_MoveFromFpu()
{
    using namespace encoding_x64;
    c.fstp(qword_ptr(rcx));
    c.movsd(m_reg(0),ptr(rcx));
}



