
#ifndef  _encoding_
#define  _encoding_

#include <algorithm>
#include <memory>
#include <vector>
#include <string>
#include <map>
#include <assert.h>
//#include <iomanip>

#include <sys/mman.h>

#include "util.h"
#include "backend_encoding.h"


// Binary operations define

#define DEF_BINARY_OPERATION( MNEMONICS)\
template<int32_t bit_size>\
void MNEMONICS(gpr_register_t<bit_size> reg1,gpr_register_t<bit_size> reg2)\
{\
    m_Add(make_code(bin_ins_t::MNEMONICS,reg1,reg2));\
}\
template<int32_t bit_size,class A>\
void MNEMONICS(gpr_register_t<bit_size> reg,A addr)\
{\
    m_Add(make_code(bin_ins_t::MNEMONICS,reg,addr));\
}\
template<int32_t bit_size,class A>\
void MNEMONICS(A addr,gpr_register_t<bit_size> reg)\
{\
    m_Add(make_code(bin_ins_t::MNEMONICS,addr,reg));\
}\
template<int32_t bit_size,class value_t>\
std::enable_if_t<(sizeof(value_t)<8)>\
MNEMONICS(gpr_register_t<bit_size> reg,value_t imm)\
{\
    m_Add(make_code(bin_ins_t::MNEMONICS,reg,imm));\
}\
template<class A,class value_t>\
void MNEMONICS(A addr,value_t imm)\
{\
    m_Add(make_code(bin_ins_t::MNEMONICS,addr,imm));\
}

// Jumps operations define

#define DEF_JUMP_OPERATION( MNEMONICS)\
void MNEMONICS(const std::string&str)\
{\
    m_Add(make_code(jump_ins_t::MNEMONICS,int32_t(0)));\
    m_jumps.push_back({str,Size()});\
}

// Unary operations define (not allowed immediate argument)

#define DEF_UNARY_OPERATION( MNEMONICS)\
template<uint32_t bit_size>\
void MNEMONICS(gpr_register_t<bit_size> reg)\
{\
    m_Add(make_code(un_ins_t::MNEMONICS,reg));\
}\
template<uint32_t bit_addr_size_,int bit_dest_size_>\
void MNEMONICS(indirect_address_t<bit_addr_size_,bit_dest_size_> addr)\
{\
    m_Add(make_code(un_ins_t::MNEMONICS,addr));\
}

// Unary operations define (allowed immediate argument) ///shr,shl,sar,sal
#define DEF_UNARY_OPERATION_ALLOW_IMMEDIATE( MNEMONICS)\
template<uint32_t bit_size>\
void MNEMONICS(gpr_register_t<bit_size> reg,uint8_t imm)\
{\
    m_Add(make_code(un_ins_t::MNEMONICS,reg,imm));\
}

//// x87 arithmetic define
#define DEF_x87_ARITH( MNEMONICS)\
void f##MNEMONICS(x87_register_t reg1,x87_register_t reg2)\
{\
    m_Add(make_code(x87_arith_ins_t::f##MNEMONICS,reg1,reg2));\
}\
void f##MNEMONICS##p(x87_register_t reg)\
{\
    m_Add(make_code(x87_arith_ins_t::f##MNEMONICS,reg));\
}\
void f##MNEMONICS##p()\
{\
    m_Add(make_code(x87_arith_ins_t::f##MNEMONICS));\
}\
template<class A>\
void f##MNEMONICS(A addr)\
{\
    m_Add(make_code(x87_arith_ins_t::f##MNEMONICS,addr,true));\
}\
template<class A>\
void fi##MNEMONICS(A addr)\
{\
    m_Add(make_code(x87_arith_ins_t::f##MNEMONICS,addr,false));\
}

//// x87 void define
#define DEF_x87_VOID( MNEMONICS)\
void MNEMONICS()\
{\
    m_Add(make_code(x87_void_ins_t::MNEMONICS));\
}

//// x87 unary instruction define
#define DEF_x87_UNARY( MNEMONICS)\
template<class A>\
void MNEMONICS(A addr)\
{\
    m_Add(make_code(x87_un_ins_t::MNEMONICS,addr));\
}\
void MNEMONICS(x87_register_t reg)\
{\
    m_Add(make_code(x87_un_ins_t::MNEMONICS,reg));\
}

// Binary SSE instruction
#define DEF_SSE_BINARY( MNEMONICS)\
template<class A>\
void MNEMONICS##ss(sse_register_t reg,A addr)\
{\
    m_Add(make_code<4>(sse_bin_ins_t::sse_##MNEMONICS,reg,addr));\
}\
template<class A>\
void MNEMONICS##sd(sse_register_t reg,A addr)\
{\
    m_Add(make_code<8>(sse_bin_ins_t::sse_##MNEMONICS,reg,addr));\
}\
void MNEMONICS##ss(sse_register_t reg1,sse_register_t reg2)\
{\
    m_Add(make_code<4>(sse_bin_ins_t::sse_##MNEMONICS,reg1,reg2));\
}\
void MNEMONICS##sd(sse_register_t reg1,sse_register_t reg2)\
{\
    m_Add(make_code<8>(sse_bin_ins_t::sse_##MNEMONICS,reg1,reg2));\
}


#define DEF_SSE_BINARY_MEM_REG( MNEMONICS)\
template<class A>\
void MNEMONICS##ss(A addr,sse_register_t reg)\
{\
    m_Add(make_code<4>(sse_bin_ins_t::sse_##MNEMONICS,addr,reg));\
}\
template<class A>\
void MNEMONICS##sd(A addr,sse_register_t reg)\
{\
    m_Add(make_code<8>(sse_bin_ins_t::sse_##MNEMONICS,addr,reg));\
}

namespace encoding_x64{


class CCode;

class function
{
    static const int max_input_size=128;
    void(*m_fptr)(void) =nullptr;
    std::shared_ptr<uint8_t> m_code;
    int32_t m_alloc_size=0;
    int32_t m_code_size=0;
    public:
    inline static uint8_t m_args[max_input_size] alignas(16)={};
    function(){};
    template<class R,class...Args>
    R execute(Args...args)const
    {
        assert(m_fptr);
        take_apart<Args...>(m_args,args...);
        m_fptr();
        if constexpr(!std::is_same_v<R,void>)
        {
            R result;
            assemble(m_args,result);
            return result;
        }

    }
    void Dump(std::vector<uint8_t>&code)const
    {
        code.clear();
        uint8_t*bts=reinterpret_cast<uint8_t*>(m_fptr);
        for(int i=0;i<m_alloc_size;++i)
        {
            code.push_back(bts[i]);
        }
    }
    operator bool()const{return m_fptr!=nullptr;}
    friend class CCode;
};

class CCode
{
    std::vector<uint8_t> m_code;
    std::map<std::string,uint32_t> m_labels;
    struct jump_t
    {
        std::string to_label;
        uint32_t    jump_address;
    };
    std::vector<jump_t> m_jumps;
    void m_Add(const code_array&ca)
    {
        for(uint8_t b:ca) m_code.push_back(b);
    }
    public:
    void Clear()
    {
        m_code.clear();
        m_labels.clear();
        m_jumps.clear();
    }
    void label(const std::string&str)
    {
        auto ins_res=m_labels.insert({str,Size()});
        assert(ins_res.second);
    }
    // Binary
    DEF_BINARY_OPERATION(mov)
    void mov(gpr_register_t<64> reg,uint64_t imm)
    {
        m_Add(aux_make_code(0xB8,reg,imm,added_opcode));
    }
    template<uint32_t bit_addr_size_,int bit_dest_size_>
    void mov( indirect_address_t<bit_addr_size_,bit_dest_size_> addr,uint64_t imm)
    {
        m_Add(aux_make_code(0xB8,addr,imm,added_opcode));
    }
    DEF_BINARY_OPERATION(add)
    DEF_BINARY_OPERATION(sub)
    DEF_BINARY_OPERATION(adc)
    DEF_BINARY_OPERATION(sbb)
    DEF_BINARY_OPERATION(cmp)
    DEF_BINARY_OPERATION(_and)
    DEF_BINARY_OPERATION(_or)
    DEF_BINARY_OPERATION(_xor)
    DEF_BINARY_OPERATION(test)
    template<uint32_t bit_size,uint32_t bit_addr_size_,int bit_dest_size_>
    void lea(gpr_register_t<bit_size> reg,indirect_address_t<bit_addr_size_,bit_dest_size_> addr)
    {
        m_Add(aux_make_code(0x8D,reg,addr));
    }
    // mov with sign extended movsx,movsxd
    template<uint32_t bit_size_dest,uint32_t bit_size_source>
    constexpr void movsx(gpr_register_t<bit_size_dest> dest,
                         gpr_register_t<bit_size_source> source)
    {
        static_assert(bit_size_dest>bit_size_source);
        if constexpr(bit_size_dest==16) m_code.push_back(override_operand_size);
        else
        {
            if(uint8_t rex=calculate_rex_prefix(dest,source);rex) m_code.push_back(rex);
        }

        if constexpr(bit_size_source==8)
        {
            m_code.push_back(0x0F);
            m_code.push_back(0xBE);
        }
        else if constexpr(bit_size_source==16)
        {
            m_code.push_back(0x0F);
            m_code.push_back(0xBF);
        }
        else {m_code.push_back(0x63);}
        m_code.push_back(get_modrm(dest,source));
    }

    template<uint32_t bit_reg_size,uint32_t bit_addr_size_,int bit_source_size_>
    constexpr void movsx(gpr_register_t<bit_reg_size> reg,indirect_address_t<bit_addr_size_,bit_source_size_> addr)
    {
        using A=indirect_address_t<bit_addr_size_,bit_source_size_>;
        static_assert(!A::dest_size_auto);
        static_assert(bit_reg_size>bit_source_size_);

        //override_address_size
        code_array ca;
        if constexpr(bit_addr_size_!=64) ca.push_byte(override_address_size);
        //override operand size
        if constexpr(bit_reg_size==16)  ca.push_byte(override_operand_size);

        //REX prefix
        if(uint8_t rex=calculate_rex_prefix(reg,addr);rex) ca.push_byte(rex);

        //opcodes
        if constexpr(bit_source_size_==8)
        {
            ca.push_byte(0x0F);
            ca.push_byte(0xBE);
        }
        else if constexpr(bit_source_size_==16)
        {
            ca.push_byte(0x0F);
            ca.push_byte(0xBF);
        }
        else {ca.push_byte(0x63);}

        //ModRM
        ca.push_byte(get_modrm(reg,addr));
        //SIB
        if(addr.is_sib) ca.push_byte(addr.sib());
        ////Displacement
        if(addr.disp!=0)
        {
            if(addr.disp_size()==1) ca.push_byte(addr.disp);
            else                    ca.push_data(addr.disp);
        }
        m_Add(ca);
    }
    // mov with zero extended
    /*template<uint32_t bit_reg_size,uint32_t bit_addr_size_,int bit_source_size_>
    constexpr void movzx(gpr_register_t<bit_reg_size> reg,indirect_address_t<bit_addr_size_,bit_source_size_> addr)
    {
        using A=indirect_address_t<bit_addr_size_,bit_source_size_>;
        static_assert(!A::dest_size_auto);
        static_assert(bit_reg_size>bit_source_size_);
        static_assert(bit_source_size_<=16);

        //override_address_size
        code_array ca;
        if constexpr(bit_addr_size_!=64) ca.push_byte(override_address_size);
        //override operand size
        if constexpr(bit_reg_size==16)  ca.push_byte(override_operand_size);

        //REX prefix
        if(uint8_t rex=calculate_rex_prefix(reg,addr);rex) ca.push_byte(rex);

        //opcodes
        if constexpr(bit_source_size_==8)
        {
            ca.push_byte(0x0F);
            ca.push_byte(0xB6);
        }
        else//bit_source_size_==16
        {
            ca.push_byte(0x0F);
            ca.push_byte(0xB7);
        }

        //ModRM
        ca.push_byte(get_modrm(reg,addr));
        //SIB
        if(addr.is_sib) ca.push_byte(addr.sib());
        ////Displacement
        if(addr.disp!=0)
        {
            if(addr.disp_size()==1) ca.push_byte(addr.disp);
            else                    ca.push_data(addr.disp);
        }
        m_Add(ca);
    }*/
    //Jumps
    DEF_JUMP_OPERATION(jmp)
    DEF_JUMP_OPERATION(ja)
    DEF_JUMP_OPERATION(jae)
    DEF_JUMP_OPERATION(jb)
    DEF_JUMP_OPERATION(jbe)
    DEF_JUMP_OPERATION(jc)
    DEF_JUMP_OPERATION(je)
    DEF_JUMP_OPERATION(jg)
    DEF_JUMP_OPERATION(jge)
    DEF_JUMP_OPERATION(jl)
    DEF_JUMP_OPERATION(jle)
    DEF_JUMP_OPERATION(jo)
    DEF_JUMP_OPERATION(jp)
    DEF_JUMP_OPERATION(jpo)
    DEF_JUMP_OPERATION(js)
    DEF_JUMP_OPERATION(jz)

    DEF_JUMP_OPERATION(jna)
    DEF_JUMP_OPERATION(jnae)
    DEF_JUMP_OPERATION(jnb)
    DEF_JUMP_OPERATION(jnbe)
    DEF_JUMP_OPERATION(jnc)
    DEF_JUMP_OPERATION(jne)
    DEF_JUMP_OPERATION(jng)
    DEF_JUMP_OPERATION(jnge)
    DEF_JUMP_OPERATION(jnl)
    DEF_JUMP_OPERATION(jnle)
    DEF_JUMP_OPERATION(jno)
    DEF_JUMP_OPERATION(jnp)
    DEF_JUMP_OPERATION(jns)
    DEF_JUMP_OPERATION(jnz)

    //Unary
    DEF_UNARY_OPERATION(inc)
    DEF_UNARY_OPERATION(dec)
    DEF_UNARY_OPERATION(neg)
    DEF_UNARY_OPERATION(_not)
    DEF_UNARY_OPERATION(shl)
    DEF_UNARY_OPERATION(shr)
    DEF_UNARY_OPERATION(mul)
    DEF_UNARY_OPERATION(imul)
    DEF_UNARY_OPERATION(div)
    DEF_UNARY_OPERATION(idiv)
    DEF_UNARY_OPERATION(push)
    DEF_UNARY_OPERATION(pop)
    DEF_UNARY_OPERATION_ALLOW_IMMEDIATE(shl)
    DEF_UNARY_OPERATION_ALLOW_IMMEDIATE(shr)
    template<class val_t>
    void push(val_t imm)
    {
        static_assert(is_integer<val_t>());
        static_assert(sizeof(val_t)<8);

        code_array temp;
        if constexpr(sizeof(val_t)==1) temp.push_byte(0x6A);
        else if constexpr(sizeof(val_t)==2)
        {
            temp.push_byte(override_operand_size);
            temp.push_byte(0x68);
        }
        else
        {
            temp.push_byte(0x68);
        }
        temp.push_data(imm);
        m_Add(temp);
    }
    // x87 Arithmetic
    DEF_x87_ARITH(add)
    DEF_x87_ARITH(sub)
    DEF_x87_ARITH(subr)
    DEF_x87_ARITH(mul)
    DEF_x87_ARITH(div)
    DEF_x87_ARITH(divr)
    // x87 void
    DEF_x87_VOID(fabs)
    DEF_x87_VOID(fchs)
    DEF_x87_VOID(fcos)
    DEF_x87_VOID(fld1)
    DEF_x87_VOID(fldl2t)
    DEF_x87_VOID(fldl2e)
    DEF_x87_VOID(fldpi)
    DEF_x87_VOID(fldlg2)
    DEF_x87_VOID(fldln2)
    DEF_x87_VOID(fldz)
    DEF_x87_VOID(fnop)
    DEF_x87_VOID(fpatan)
    DEF_x87_VOID(fprem)
    DEF_x87_VOID(fptan)
    DEF_x87_VOID(frndint)
    DEF_x87_VOID(fsin)
    DEF_x87_VOID(fsincos)
    DEF_x87_VOID(fsqrt)
    DEF_x87_VOID(ftst)
    DEF_x87_VOID(fyl2x)

    //Unary x87 instructions
    DEF_x87_UNARY(fld)
    DEF_x87_UNARY(fst)
    DEF_x87_UNARY(fstp)
    DEF_x87_UNARY(fild)
    DEF_x87_UNARY(fist)
    DEF_x87_UNARY(fistp)

    //SSE
    DEF_SSE_BINARY(mov)
    DEF_SSE_BINARY(add)
    DEF_SSE_BINARY(sub)
    DEF_SSE_BINARY(mul)
    DEF_SSE_BINARY(div)
    DEF_SSE_BINARY(sqrt)
    DEF_SSE_BINARY_MEM_REG(mov)

    //special
    void ret(){m_code.push_back(0xC3);}
    void fwait(){m_code.push_back(0x9B);}
    void fninit()
    {
        m_code.push_back(0xDB);
        m_code.push_back(0xE3);
    }
    void finit()
    {
        fwait();
        fninit();
    }
    void f2xm1()
    {
        m_code.push_back(0xD9);
        m_code.push_back(0xF0);
    }
    void fscale()
    {
        m_code.push_back(0xD9);
        m_code.push_back(0xFD);
    }
    void fxch(x87_register_t reg)
    {
        m_code.push_back(0xD9);
        m_code.push_back(0xC8+reg.code);
    }
    uint32_t Size()const{return m_code.size();}
    bool Empty()const{return m_code.empty();}
    function Compile()const;
};

}//encoding_x64
#endif












