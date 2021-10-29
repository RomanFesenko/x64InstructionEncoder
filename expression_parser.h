
#ifndef  _expression_parser_
#define  _expression_parser_

#include <utility>
#include <memory>
#include <string>
#include <vector>
#include <assert.h>

class CParser;

class CBinaryOp;
class CVariable;
class CConstant;
class CNeg;
class CFunction;

class CArityFunctor
{
    int m_arity;
    public:
    enum type_t
    {
        abs_id,sqrt_id,exp_id,log_id,cos_id,sin_id,tan_id,asin_id,acos_id,atan_id,other_id
    };
    const type_t type;
    explicit CArityFunctor(int arity,type_t t=other_id):m_arity(arity),type(t){}
    virtual double Eval(const std::vector<double>&stack)const=0;
    int Arity()const{return m_arity;}
    virtual~CArityFunctor(){}
};

class CArity0:public CArityFunctor
{
    double(*m_ftr)();
    public:
    CArity0(double(*ftr)()):CArityFunctor(0),m_ftr(ftr){}
    virtual double Eval(const std::vector<double>&stack)const override{return m_ftr();}
};

class CArity1:public CArityFunctor
{
    double(*m_ftr)(double);
    public:
    CArity1(double(*ftr)(double),type_t t=other_id):CArityFunctor(1,t),m_ftr(ftr){}
    virtual double Eval(const std::vector<double>&stack)const override{return m_ftr(stack.back());}
};

class CArity2:public CArityFunctor
{
    double(*m_ftr)(double,double);
    public:
    CArity2(double(*ftr)(double,double)):CArityFunctor(1),m_ftr(ftr){}
    virtual double Eval(const std::vector<double>&stack)const override
    {
        int i=stack.size()-1;
        return m_ftr(stack[i-1],stack[i]);
    }
};


class CVisitor
{
    public:
    virtual void Visit(const CBinaryOp&)=0;
    virtual void Visit(const CVariable&)=0;
    virtual void Visit(const CConstant&)=0;
    virtual void Visit(const CNeg&)=0;
    virtual void Visit(const CFunction&)=0;
    virtual~CVisitor(){}
};


class CASTNode
{
    mutable int m_index=0;
    using node_ptr_t=std::unique_ptr<CASTNode>;
    public:
    int Index()const{return m_index;}
    void SetIndex(int i)const{m_index=i;}
    virtual void AcceptVisitor(CVisitor&)const=0;
    virtual~CASTNode(){}
};

class CBinaryOp:public CASTNode
{
    public:
    enum type_t
    {
        add_id,sub_id,mul_id,div_id
    };
    private:
    using node_ptr_t=std::unique_ptr<CASTNode>;
    type_t m_type;
    std::unique_ptr<CASTNode> m_left;
    std::unique_ptr<CASTNode> m_right;
    public:
    explicit CBinaryOp(type_t t,node_ptr_t&&,node_ptr_t&&);
    virtual void AcceptVisitor(CVisitor&visitor)const override
    {
        visitor.Visit(*this);
    }
    type_t Type()const{return m_type;}
    const CASTNode& Left()const{return  *m_left;}
    const CASTNode& Right()const{return *m_right;}
    friend class CParser;
};

class CVariable:public CASTNode
{
    double& m_value;
    public:
    CVariable(double& val):m_value(val){}
    virtual void AcceptVisitor(CVisitor&visitor)const override
    {
        visitor.Visit(*this);
    }
    double Value()const{return m_value;}
    const double* ValuePtr()const{return &m_value;}
    friend class CParser;
};

class CConstant:public CASTNode
{
    double m_value;
    public:
    explicit CConstant(double val):m_value(val){}
    virtual void AcceptVisitor(CVisitor&visitor)const override
    {
        visitor.Visit(*this);
    }
    double Value()const{return m_value;}
    friend class CParser;
};

class CNeg:public CASTNode
{
    std::unique_ptr<CASTNode> m_child;
    public:
    CNeg(std::unique_ptr<CASTNode>&&ch):m_child(std::move(ch)){}
    virtual void AcceptVisitor(CVisitor&visitor)const override
    {
        visitor.Visit(*this);
    }
    const CASTNode& Child()const{return *m_child;}
    friend class CParser;
};

class CFunction:public CASTNode
{
    using node_ptr_t=std::unique_ptr<CASTNode>;
    std::vector<node_ptr_t> m_args;
    std::unique_ptr<CArityFunctor> m_functor;
    public:
    int Arity()const
    {
        assert(m_functor);
        return m_functor->Arity();
    }
    virtual void AcceptVisitor(CVisitor&visitor)const override
    {
        visitor.Visit(*this);
    }
    const CASTNode&Arg(int i)const{return *m_args[i];}
    double Eval(const std::vector<double>&stack)const{return m_functor->Eval(stack);}
    CArityFunctor::type_t Type()const{return m_functor->type;}
    friend class CParser;
};

template<class node_t>
constexpr void check_ast_node()
{
    static_assert(std::is_same_v<node_t,CBinaryOp>||
                  std::is_same_v<node_t,CVariable>||
                  std::is_same_v<node_t,CConstant>||
                  std::is_same_v<node_t,CFunction>||
                  std::is_same_v<node_t,CNeg>);
}

template<class in_visitor_t,class out_visitor_t>
class ast_traverser:public CVisitor
{
    in_visitor_t  m_in_vis;
    out_visitor_t m_out_vis;

    virtual void Visit(const CBinaryOp& bin_op)override
    {
        m_in_vis(bin_op);
        bin_op.Left().AcceptVisitor(*this);
        bin_op.Right().AcceptVisitor(*this);
        m_out_vis(bin_op);
    }
    virtual void Visit(const CVariable& var)override
    {
        m_in_vis(var);
        m_out_vis(var);
    }
    virtual void Visit(const CConstant&const_)override
    {
        m_in_vis(const_);
        m_out_vis(const_);
    }
    virtual void Visit(const CNeg&neg)override
    {
        m_in_vis(neg);
        neg.Child().AcceptVisitor(*this);
        m_out_vis(neg);
    }
    virtual void Visit(const CFunction&func)override
    {
        m_in_vis(func);
        for(int i=0;i<func.Arity();++i) func.Arg(i).AcceptVisitor(*this);
        m_out_vis(func);
    }
    public:
    ast_traverser(in_visitor_t in,out_visitor_t out):
    m_in_vis(in),m_out_vis(out)
    {}
    void traverse(const CASTNode&root)
    {
        root.AcceptVisitor(*this);
    }
};

class CEvaluator:protected CVisitor
{
    mutable std::vector<double> m_stack;
    virtual void Visit(const CBinaryOp&)override;
    virtual void Visit(const CVariable&)override;
    virtual void Visit(const CConstant&)override;
    virtual void Visit(const CNeg&)     override;
    virtual void Visit(const CFunction&)override;
    public:
    double Eval(const CASTNode&root);
};

class CParser;

class CExpession
{
    std::shared_ptr<std::vector<double>> m_args;
    mutable CEvaluator m_evaluator;
    std::shared_ptr<CASTNode> m_tree;
    void m_SetNull();
    CExpession(std::shared_ptr<std::vector<double>> args,
               std::shared_ptr<CASTNode> ast):m_args(args),m_tree(ast){}

    template<class Head,class...Args>
    void m_place_args(int i,Head hd,Args...args)const
    {
        (*m_args)[i]=hd;
        if constexpr(sizeof...(Args)>0)
        {
            m_place_args(i+1,args...);
        }
    }
    public:
    int Arity()const
    {
        assert(m_args);
        return m_args->size();
    }
    template<class...Args>
    double operator()(Args...args)const
    {
        assert(Arity()==sizeof...(Args));
        if constexpr(sizeof...(Args)!=0)
        {
            m_place_args(0,args...);
        }
        return m_evaluator.Eval(*m_tree);

    }
    operator bool()const{return m_tree!=nullptr;}
    friend class CParser;
};

class CParseError
{
    public:
    enum result_t
    {
        success_id,
        invalid_number_id,
        unknown_variable_id,
        unknown_function_id,
        arity_error_id,
        parenthesis_error_id,
        unexpected_char_id,
        unexpected_end_id
    };
    private:
    result_t m_status=success_id;
    std::string m_added;
    int m_error_pos=-1;
    public:
    result_t Status()const{return m_status;}
    std::string What()const;
    operator bool()const{return m_status!=success_id;}
    int Position()const{return m_error_pos;}
    friend class CParser;
};

class CParser
{
    using node_ptr_t=std::unique_ptr<CASTNode>;
    using iterator_t=std::string::const_iterator;
    using iter_range_t=std::pair<iterator_t,iterator_t>;
    struct parse_error_t{};

    iterator_t m_begin,m_end,m_current;
    CParseError m_parse_error;
    double* m_args_ptr;
    std::vector<std::string>  m_args_names;

    //Auxilarity functions

    void  m_SkipSpace();
    char  current()const;
    bool  consume(char);
    bool  m_End()const;
    void  m_SetError(CParseError::result_t,const std::string&added="");
    void  m_CheckNoEnd();
    void  m_CheckAndConsume(char);

    // Utility parsing
    std::unique_ptr<CArityFunctor> m_GetFunctor(iter_range_t);
    double*                        m_GetVariable(iter_range_t);
    iter_range_t                   m_ParseIdentifier();
    double                         m_ParseNumber();

    // Main parse functions
    node_ptr_t                 m_ParseExpression();
    node_ptr_t                 m_ParseTerm();
    node_ptr_t                 m_ParseFactor();
    node_ptr_t                 m_ParseUnit();
    std::unique_ptr<CFunction> m_ParseArgList(int arity);

    public:
    CExpession Parse(const std::string&body,
                     const std::vector<std::string>&args_name);

    CExpession Parse(const std::string&body,
                     const std::vector<std::string>&args_name,
                     CParseError&err);

    std::shared_ptr<CASTNode> BuildAST(const std::string&body,
                                       const std::vector<std::string>&args_name,
                                       double*args_ptr);

    std::shared_ptr<CASTNode> BuildAST(const std::string&body,
                                       const std::vector<std::string>&args_name,
                                       double*args_ptr,
                                       CParseError&err);
};



#endif

