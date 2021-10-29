
#ifndef  _expression_jit_
#define  _expression_jit_

#include <assert.h>

#include "expression_parser.h"
#include "encoding.h"

//class Cx87Compiler;

class CCompiledExpession
{
    encoding_x64::function m_function;
    std::shared_ptr<std::vector<double>> m_const;
    const int m_arity;
    CCompiledExpession( int arity):m_arity(arity){}
    public:
    int Arity()const{return m_arity;}
    template<class...Args>
    double operator()(Args...args)const
    {
        assert(sizeof...(Args)==m_arity);
        /*if(sizeof...(Args)!=m_arity)
        {
            std::cout<<"passed:"<<sizeof...(Args)<<'\n';
            std::cout<<"arity:"<<m_arity<<'\n';
            assert(false);
        }*/
        return  m_function.execute<double>(static_cast<double>(args)...);
    }
    operator bool()const{return m_function;}
    void Dump(std::vector<uint8_t>&vec){m_function.Dump(vec);}
    friend class Cx87Compiler;
    friend class CSSECompiler;
};

std::shared_ptr<std::vector<double>> InitTraverse(CASTNode&root,std::vector<int>&marks);

class Cx87Compiler:protected CVisitor
{
    encoding_x64::CCode c;
    std::vector<int> m_marks;
    uint8_t       m_stack_size;
    inline static double m_store[2]={};
    std::shared_ptr<std::vector<double>> m_const;
    virtual void Visit(const CBinaryOp&)override;
    virtual void Visit(const CVariable&)override;
    virtual void Visit(const CConstant&)override;
    virtual void Visit(const CNeg&)     override;
    virtual void Visit(const CFunction&)override;
    int  m_Mark(const CASTNode&node)const{return m_marks[node.Index()];}
    CCompiledExpession m_PostProcess(int arity);
    void m_GenerateTanCode();
    void m_GenerateLneCode();
    void m_GenerateAtanCode();
    void m_GenerateAsinCode();
    void m_GenerateAcosCode();
    void m_GenerateExpCode();
    public:
    CCompiledExpession Compile(const std::string&body,
                               const std::vector<std::string>&args_name);

};

class CSSECompiler:protected CVisitor
{
    encoding_x64::CCode c;
    std::vector<int> m_marks;
    uint8_t       m_stack_top;
    bool          m_87_init=false;
    inline const static double m_neg=-0.0;
    inline static double  m_store[10] alignas(16)={};// xmm6...xmm15 saved in Windows
    inline static double m_temp=0;
    int8_t m_for_save;
    std::shared_ptr<std::vector<double>> m_const;
    void m_MoveToFpu();
    void m_MoveFromFpu();
    int  m_Mark(const CASTNode&node)const{return m_marks[node.Index()];}
    virtual void Visit(const CBinaryOp&)override;
    virtual void Visit(const CVariable&)override;
    virtual void Visit(const CConstant&)override;
    virtual void Visit(const CNeg&)     override;
    virtual void Visit(const CFunction&)override;
    CCompiledExpession m_PostProcess(int arity);
    encoding_x64::sse_register_t m_reg(int i)const
    {
        return encoding_x64::sse_register_t(m_stack_top+i);
    }
    public:
    CCompiledExpession Compile(const std::string&body,
                               const std::vector<std::string>&args_name);

};

#endif

