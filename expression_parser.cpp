
#include <assert.h>
#include <iostream>
#include <string>
#include <map>
#include <math.h>

#include "expression_parser.h"
#include "string_utility.h"


//CBinaryOp

CBinaryOp::CBinaryOp(type_t t,node_ptr_t&&l,node_ptr_t&&r):
m_type(t),m_left(std::move(l)),m_right(std::move(r))
{}

//CEvaluator

void CEvaluator::Visit(const CBinaryOp&bin_op)
{
    bin_op.Left().AcceptVisitor(*this);
    bin_op.Right().AcceptVisitor(*this);
    assert(m_stack.size()>1);
    double right=m_stack.back(); m_stack.pop_back();
    double left= m_stack.back(); m_stack.pop_back();

    switch(bin_op.Type())
    {
        case CBinaryOp::add_id:
        m_stack.push_back(left+right);
        break;

        case CBinaryOp::sub_id:
        m_stack.push_back(left-right);
        break;

        case CBinaryOp::mul_id:
        m_stack.push_back(left*right);
        break;

        case CBinaryOp::div_id:
        m_stack.push_back(left/right);
        break;

        default:assert(false);
    }
}

void CEvaluator::Visit(const CVariable&var)
{
    m_stack.push_back(var.Value());
}

void CEvaluator::Visit(const CConstant&const_)
{
    m_stack.push_back(const_.Value());
}

void CEvaluator::Visit(const CNeg&neg)
{
    neg.Child().AcceptVisitor(*this);
    assert(!m_stack.empty());
    m_stack.back()=-m_stack.back();
}

void CEvaluator::Visit(const CFunction&func)
{
    for(int i=0;i<func.Arity();++i)
    {
        func.Arg(i).AcceptVisitor(*this);
    }
    assert(m_stack.size()>=func.Arity());
    double result=func.Eval(m_stack);
    m_stack.erase(m_stack.end()-func.Arity(),m_stack.end());
    m_stack.push_back(result);
}

double CEvaluator::Eval(const CASTNode&tree)
{
    m_stack.clear();
    tree.AcceptVisitor(*this);
    assert(m_stack.size()==1);
    return m_stack[0];
}
////////////////////////////////////////////////////
//                  CParser
///////////////////////////////////////////////////

// Auxilarity parse function

void CParser::m_SkipSpace()
{
    auto space=[](char ch){return ch==' '||ch=='\n'||ch=='\t';};
    while(!m_End()&&space(*m_current))  ++m_current;
}

char CParser::current()const
{
    assert(m_current!=m_end);
    return *m_current;
}

bool CParser::consume(char ch)
{
    assert(m_current!=m_end);
    if(ch==*m_current)
    {
        m_current++;
        return true;
    }
    return false;
}

bool CParser::m_End()const
{
    return m_current==m_end;
}

void CParser::m_SetError(CParseError::result_t status,const std::string&added)
{
    assert(!m_parse_error);
    m_args_ptr=nullptr;
    m_parse_error.m_status=status;
    m_parse_error.m_error_pos=m_current-m_begin;
    m_parse_error.m_added=added;
    throw parse_error_t();
}

void CParser::m_CheckNoEnd()
{
    if(m_End()) m_SetError(CParseError::unexpected_end_id);
}

void CParser::m_CheckAndConsume(char ch)
{
    if(!consume(ch))
    {
        if(ch==')') m_SetError(CParseError::parenthesis_error_id);
        else        m_SetError(CParseError::unexpected_char_id);
    }
}

// Utility parsing

double CParser::m_ParseNumber()
{
    std::size_t num_point=0;
    auto fst=m_current;
    for(;!m_End();++m_current)
    {
        if (is_digit(current())) {}
        else if(current()=='.')
        {
            num_point++;
            if(num_point>1)
            {
                m_SetError(CParseError::invalid_number_id);
                return 0;
            }
        }
        else break;
    }
    if(fst==m_current)
    {
        m_SetError(CParseError::invalid_number_id);
        return 0;
    }
    return std::stod(std::string(fst,m_current));
}

CParser::iter_range_t CParser::m_ParseIdentifier()
{
    assert(is_identifier_begin(current()));
    auto fst=m_current;
    while(!m_End()&&!is_identifier_end(current())) ++m_current;
    return {fst,m_current};
}


std::unique_ptr<CArityFunctor> CParser::m_GetFunctor(iter_range_t range)
{
    using af_ptr_t=std::unique_ptr<CArityFunctor>;
    struct fun_data_t
    {
        double(*fun_ptr)(double);
        CArityFunctor::type_t type;
    };
    using value_type=std::pair<std::string,fun_data_t>;
    const static std::map<std::string,fun_data_t,string_comparer_t> map=
    {
           value_type("abs",{abs,CArityFunctor::abs_id}),
           value_type("sqrt",{sqrt,CArityFunctor::sqrt_id}),
           value_type("exp",{exp,CArityFunctor::exp_id}),
           value_type("log",{log,CArityFunctor::log_id}),
           value_type("cos",{cos,CArityFunctor::cos_id}),
           value_type("sin",{sin,CArityFunctor::sin_id}),
           value_type("tan",{tan,CArityFunctor::tan_id}),
           value_type("asin",{asin,CArityFunctor::asin_id}),
           value_type("acos",{acos,CArityFunctor::acos_id}),
           value_type("atan",{atan,CArityFunctor::atan_id})
    };
    auto iter=map.find(range);
    if(iter==map.end())
    {
        m_SetError(CParseError::unknown_function_id,std::string(range.first,range.second));
        return nullptr;
    }
    auto new_=new CArity1(iter->second.fun_ptr,iter->second.type);
    return af_ptr_t(new_);
}

double* CParser::m_GetVariable(iter_range_t range)
{
    int i;
    for(i=0;i<m_args_names.size();++i)
    {
        const auto&str=m_args_names[i];
        if(std::equal(str.begin(),str.end(),range.first,range.second)) break;
    }
    if(i==m_args_names.size())
    {
        m_SetError(CParseError::unknown_variable_id,std::string(range.first,range.second));
        return nullptr;
    }
    return m_args_ptr+i;
}

// Main parse function

//E=T(('+'T)|('-'T))*

CParser::node_ptr_t CParser::m_ParseExpression()
{
    auto root=m_ParseTerm();
    while(true)
    {
        m_SkipSpace();
        if(m_End()) break;
        if(consume('+'))
        {
            root=std::make_unique<CBinaryOp>(CBinaryOp::add_id,std::move(root),m_ParseTerm());
        }
        else if(consume('-'))
        {
            root=std::make_unique<CBinaryOp>(CBinaryOp::sub_id,std::move(root),m_ParseTerm());
        }
        else break;
    }
    return root;
}

//T=F(('+'F)|('-'F))*

CParser::node_ptr_t CParser::m_ParseTerm()
{
    auto root=m_ParseFactor();
    if(!root) return nullptr;
    while(true)
    {
        m_SkipSpace();
        if(m_End()) break;
        if(consume('*'))
        {
            root=std::make_unique<CBinaryOp>(CBinaryOp::mul_id,std::move(root),m_ParseFactor());
        }
        else if(consume('/'))
        {
            root=std::make_unique<CBinaryOp>(CBinaryOp::div_id,std::move(root),m_ParseFactor());
        }
        else break;
    }
    return root;
}

/*F=Number  |
    Variable|
    Function(EList)|
    ('('E')')
*/

CParser::node_ptr_t CParser::m_ParseFactor()
{
    m_SkipSpace();
    m_CheckNoEnd();
    if(is_number_char(current()))
    {
        return std::make_unique<CConstant>(m_ParseNumber());
    }
    else if(is_identifier_begin(current()))
    {
        auto iden=m_ParseIdentifier();
        m_SkipSpace();
        if(!m_End()&&consume('(')) // function
        {
            auto ftr=m_GetFunctor(iden);
            auto list=m_ParseArgList(ftr->Arity());
            m_CheckAndConsume(')');
            list->m_functor=std::move(ftr);
            return list;
        }
        else // variable
        {
            return std::make_unique<CVariable>(*m_GetVariable(iden));
        }
    }
    else if(consume('('))
    {
        auto root=m_ParseExpression();
        m_CheckAndConsume(')');
        return root;
    }
    else if(consume('-'))
    {
        return std::make_unique<CNeg>(m_ParseExpression());
    }
    else if(consume('+'))
    {
        return m_ParseExpression();
    }
    else
    {
        m_SetError(CParseError::unexpected_char_id);
        return nullptr;
    }
}

//EList=E?(','E)*

std::unique_ptr<CFunction> CParser::m_ParseArgList(int arity)
{
    auto result=std::make_unique<CFunction>();
    m_SkipSpace();
    m_CheckNoEnd();

    if(current()==')')
    {
        if(arity!=0) m_SetError(CParseError::arity_error_id);
        return result;
    }
    else
    {
        result->m_args.push_back(m_ParseExpression());
        arity--;
    }
    while(true)
    {
        m_SkipSpace();
        m_CheckNoEnd();
        if(current()==')')
        {
            if(arity!=0) m_SetError(CParseError::arity_error_id);
            return result;
        }
        m_CheckAndConsume(',');
        result->m_args.push_back(m_ParseExpression());
        arity--;
    }
}


std::shared_ptr<CASTNode> CParser::BuildAST(const std::string&body,
                                            const std::vector<std::string>&args_name,
                                            double*args_ptr)
{
    m_args_ptr=args_ptr;
    m_begin=body.begin();
    m_end=body.end();
    m_current=m_begin;
    m_args_names=args_name;
    m_parse_error=CParseError{};//reset
    std::shared_ptr<CASTNode> root;
    try
    {
        root=m_ParseExpression();
        if(!m_End())
        {
            m_SetError(CParseError::unexpected_char_id);
        }
    }
    catch(parse_error_t&){}
    return root;
}


std::shared_ptr<CASTNode> CParser::BuildAST(const std::string&body,
                                            const std::vector<std::string>&args_name,
                                            double*args_ptr,
                                            CParseError&err)
{
    auto res=BuildAST(body,args_name,args_ptr);
    err=m_parse_error;
    return res;
}

CExpession CParser::Parse(const std::string&body,
                          const std::vector<std::string>&args_name)
{
    auto args=std::make_shared<std::vector<double>>(args_name.size());
    auto root=BuildAST(body,args_name,args->data());
    return CExpession(args,root);
}

CExpession CParser::Parse(const std::string&body,
                          const std::vector<std::string>&args_name,
                          CParseError&err)
{
    auto args=std::make_shared<std::vector<double>>(args_name.size());
    auto root=BuildAST(body,args_name,args->data(),err);
    return CExpession(args,root);
}

//CExpession

void CExpession::m_SetNull()
{
    m_args=nullptr;
    m_tree=nullptr;
}

//CParseError

std::string CParseError::What()const
{
    const static std::string what[]=
    {
        "success",
        "invalid_number",
        "unknown_variable",
        "unknown_function",
        "arity_error",
        "parenthesis_error",
        "unexpected_char",
        "unexpected_end"
    };
    return (m_added.empty())? what[m_status]:what[m_status]+":"+m_added;
}
