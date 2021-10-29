
#ifndef  _backend_encoding_
#define  _backend_encoding_

#include <stdexcept>
#include <type_traits>
#include <string>

#include "code_array.h"
#include "opcodes.h"

namespace encoding_x64{

constexpr void constexpr_assert(bool val,const char* what="")
{
    if(!val) throw std::logic_error(what);
}

const uint8_t first_jumps32_opcode=0x0F;//


// Prefixes
const uint8_t override_address_size=0x67;// 64 bit->32 bit
const uint8_t override_operand_size=0x66;// 32 bit->16 bit


// REX prefixes
const uint8_t rex_w=0x48;
const uint8_t rex_r=0x44;
const uint8_t rex_x=0x42;
const uint8_t rex_b=0x41;

// ModRM
const uint8_t modrm_mod_rr=0xC0;
const uint8_t modrm_mod_rm=0;
const uint8_t modrm_mod_rm_disp8=0x40;
const uint8_t modrm_mod_rm_disp32=0x80;

template<uint32_t bit_size_,class der_t>
struct base_register_t
{
    static const uint32_t bit_size=bit_size_;
    static const uint8_t byte_size=bit_size_>>3;
    const uint8_t code;
    constexpr uint8_t three_bit_code()const// sest in modrm byte
    {
        return (code<8)? code:code-8;
    }
    template<class other_t>
    constexpr std::enable_if_t<!std::is_same_v<other_t,der_t>,bool>
    operator==(const other_t&other)const{return false;}

    template<class other_t>
    constexpr std::enable_if_t<!std::is_same_v<other_t,der_t>,bool>
    operator!=(const other_t&other)const{return true;}

    constexpr bool operator==(const der_t&other)const
    {
        return code==other.code;
    }
    constexpr bool operator!=(const der_t&other)const
    {
        return !((*this)==other);
    }
};

template<uint32_t bit_size_>
struct gpr_register_t:public base_register_t<bit_size_,gpr_register_t<bit_size_>>
{
    constexpr gpr_register_t(uint8_t i):base_register_t<bit_size_,gpr_register_t<bit_size_>>(i){}
};

using gpr8=gpr_register_t<8>;
using gpr16=gpr_register_t<16>;
using gpr32=gpr_register_t<32>;
using gpr64=gpr_register_t<64>;


// 8- bit

constexpr gpr8 al=gpr8(0);
constexpr gpr8 cl=gpr8(1);
constexpr gpr8 dl=gpr8(2);
constexpr gpr8 bl=gpr8(3);

// 16- bit

constexpr gpr16 ax=gpr16(0);
constexpr gpr16 cx=gpr16(1);
constexpr gpr16 dx=gpr16(2);
constexpr gpr16 bx=gpr16(3);
constexpr gpr16 sp=gpr16(4);
constexpr gpr16 bp=gpr16(5);
constexpr gpr16 si=gpr16(6);
constexpr gpr16 di=gpr16(7);

// 32-bit

constexpr gpr32 eax=gpr32(0);
constexpr gpr32 ecx=gpr32(1);
constexpr gpr32 edx=gpr32(2);
constexpr gpr32 ebx=gpr32(3);
constexpr gpr32 esp=gpr32(4);
constexpr gpr32 ebp=gpr32(5);
constexpr gpr32 esi=gpr32(6);
constexpr gpr32 edi=gpr32(7);


// 64-bit

constexpr gpr64 rax=gpr64(0);
constexpr gpr64 rcx=gpr64(1);
constexpr gpr64 rdx=gpr64(2);
constexpr gpr64 rbx=gpr64(3);
constexpr gpr64 rsp=gpr64(4);
constexpr gpr64 rbp=gpr64(5);
constexpr gpr64 rsi=gpr64(6);
constexpr gpr64 rdi=gpr64(7);

constexpr gpr64 r8=gpr64(8);
constexpr gpr64 r9=gpr64(9);
constexpr gpr64 r10=gpr64(10);
constexpr gpr64 r11=gpr64(11);
constexpr gpr64 r12=gpr64(12);
constexpr gpr64 r13=gpr64(13);
constexpr gpr64 r14=gpr64(14);
constexpr gpr64 r15=gpr64(15);

//x87 registers

struct x87_register_t:public base_register_t<80,x87_register_t>
{
    constexpr x87_register_t(uint8_t i):base_register_t<80,x87_register_t>(i){}
};


constexpr x87_register_t st0=x87_register_t(0);
constexpr x87_register_t st1=x87_register_t(1);
constexpr x87_register_t st2=x87_register_t(2);
constexpr x87_register_t st3=x87_register_t(3);
constexpr x87_register_t st4=x87_register_t(4);
constexpr x87_register_t st5=x87_register_t(5);
constexpr x87_register_t st6=x87_register_t(6);
constexpr x87_register_t st7=x87_register_t(7);


//SSE registers
struct sse_register_t:public base_register_t<64,sse_register_t>
{
    constexpr sse_register_t(uint8_t i):base_register_t<64,sse_register_t>(i){}
};


constexpr sse_register_t xmm0=sse_register_t(0);
constexpr sse_register_t xmm1=sse_register_t(1);
constexpr sse_register_t xmm2=sse_register_t(2);
constexpr sse_register_t xmm3=sse_register_t(3);
constexpr sse_register_t xmm5=sse_register_t(5);
constexpr sse_register_t xmm6=sse_register_t(6);
constexpr sse_register_t xmm7=sse_register_t(7);
constexpr sse_register_t xmm8=sse_register_t(8);
constexpr sse_register_t xmm9=sse_register_t(9);
constexpr sse_register_t xmm10=sse_register_t(10);
constexpr sse_register_t xmm11=sse_register_t(11);
constexpr sse_register_t xmm12=sse_register_t(12);
constexpr sse_register_t xmm13=sse_register_t(13);
constexpr sse_register_t xmm14=sse_register_t(14);
constexpr sse_register_t xmm15=sse_register_t(15);



// scaling

struct scaling_t
{
    const uint8_t value;
};

constexpr scaling_t _1=scaling_t(1);
constexpr scaling_t _2=scaling_t(2);
constexpr scaling_t _4=scaling_t(4);
constexpr scaling_t _8=scaling_t(8);

// Indirect adress

template<uint32_t bit_addr_size_,int bit_dest_size_>
struct indirect_address_t
{
    const static uint32_t bit_addr_size=bit_addr_size_;
    const static uint32_t byte_addr_size=bit_addr_size_>>3;
    const static uint32_t bit_dest_size=bit_dest_size_;
    const static uint32_t byte_dest_size=bit_dest_size_>>3;
    const static bool     dest_size_auto=bit_dest_size_==-1;

    const bool       is_sib;
    const uint8_t    scaling;//is_sib==true
    const gpr_register_t<bit_addr_size_> index;  //is_sib==true
    const gpr_register_t<bit_addr_size_> base;
    const int32_t    disp;

    constexpr uint8_t disp_size()const
    {
        if(disp==0)                return 0;
        if((disp<128)&&(disp>-129))return 1;
        return 4;
    }
    constexpr uint8_t modrm_mod()const
    {
        if(disp==0)                return 0;
        if((disp<128)&&(disp>-129))return 0x40;
        return 0x80;
    }
    constexpr uint8_t rex_xb()const
    {
        if(bit_addr_size!=64) return 0;
        if(!is_sib)
        {
            return (base.code<8)? 0:0x40|rex_b;
        }
        else
        {
            uint8_t res=0;
            if(index.code>=8) res|=rex_x;
            if(base.code>=8)  res|=rex_b;
            if(res) res|=0x40;
            return res;
        }
    }
    constexpr uint8_t sib()const
    {
        uint8_t sib=0;
        if(!is_sib) return 0;

        if(scaling==2)
        {
            sib|=1<<6;
        }
        else if(scaling==4)
        {
            sib|=2<<6;
        }
        else if(scaling==8) sib|=3<<6;
        else{}

        sib|=(scaling!=0)? index.three_bit_code()<<3:0x20;
        sib|=base.three_bit_code();
        return sib;
    }
};

template<int bit_dest_size_,uint32_t bit_addr_size_>
constexpr auto aux_get_ptr(gpr_register_t<bit_addr_size_> reg,int32_t disp=0)
{
    static_assert(bit_addr_size_==32||bit_addr_size_==64,
    "Address size must be 32 or 64 bits in 64-bits mode");
    using res_t=indirect_address_t<bit_addr_size_,bit_dest_size_>;
    if(reg!=esp&&reg!=rsp) return res_t{false,1,reg,reg,disp};
    else                   return res_t{true,0,reg,reg,disp};
}

template<uint32_t bit_addr_size_>
constexpr auto ptr(gpr_register_t<bit_addr_size_> reg,int32_t disp=0)
{return aux_get_ptr<-1>(reg,disp);}

template<uint32_t bit_addr_size_>
constexpr auto byte_ptr(gpr_register_t<bit_addr_size_> reg,int32_t disp=0)
{return aux_get_ptr<8>(reg,disp);}

template<uint32_t bit_addr_size_>
constexpr auto word_ptr(gpr_register_t<bit_addr_size_> reg,int32_t disp=0)
{return aux_get_ptr<16>(reg,disp);}

template<uint32_t bit_addr_size_>
constexpr auto dword_ptr(gpr_register_t<bit_addr_size_> reg,int32_t disp=0)
{return aux_get_ptr<32>(reg,disp);}

template<uint32_t bit_addr_size_>
constexpr auto qword_ptr(gpr_register_t<bit_addr_size_> reg,int32_t disp=0)
{return aux_get_ptr<64>(reg,disp);}

template<uint32_t bit_addr_size_>
constexpr auto tbyte_ptr(gpr_register_t<bit_addr_size_> reg,int32_t disp=0)
{return aux_get_ptr<80>(reg,disp);}


template<int bit_dest_size_,uint32_t bit_addr_size_>
constexpr auto aux_get_ptr(scaling_t sc,
                                         gpr_register_t<bit_addr_size_> index,
                                         gpr_register_t<bit_addr_size_> base,
                                         int32_t disp=0)
{
    constexpr_assert((index!=esp)&&(index!=rsp),
    "esp and rsp register can not be index in indirect address mode");

    return indirect_address_t<bit_addr_size_,bit_dest_size_>{true,sc.value,index,base,disp};
}


template<uint32_t bit_addr_size_>
constexpr auto ptr(scaling_t sc,gpr_register_t<bit_addr_size_> index,gpr_register_t<bit_addr_size_> base,int32_t disp=0)
{return aux_get_ptr<-1>(sc,index,base,disp);}


template<uint32_t bit_addr_size_>
constexpr auto byte_ptr(scaling_t sc,gpr_register_t<bit_addr_size_> index,gpr_register_t<bit_addr_size_> base,int32_t disp=0)
{return aux_get_ptr<8>(sc,index,base,disp);}


template<uint32_t bit_addr_size_>
constexpr auto word_ptr(scaling_t sc,gpr_register_t<bit_addr_size_> index,gpr_register_t<bit_addr_size_> base,int32_t disp=0)
{return aux_get_ptr<16>(sc,index,base,disp);}


template<uint32_t bit_addr_size_>
constexpr auto dword_ptr(scaling_t sc,gpr_register_t<bit_addr_size_> index,gpr_register_t<bit_addr_size_> base,int32_t disp=0)
{return aux_get_ptr<32>(sc,index,base,disp);}


template<uint32_t bit_addr_size_>
constexpr auto qword_ptr(scaling_t sc,gpr_register_t<bit_addr_size_> index,gpr_register_t<bit_addr_size_> base,int32_t disp=0)
{return aux_get_ptr<64>(sc,index,base,disp);}

template<uint32_t bit_addr_size_>
constexpr auto tbyte_ptr(scaling_t sc,gpr_register_t<bit_addr_size_> index,gpr_register_t<bit_addr_size_> base,int32_t disp=0)
{return aux_get_ptr<80>(sc,index,base,disp);}


// REX prefix evaluating

template<class R,uint32_t bit_addr_size_,int bit_dest_size_>
constexpr uint8_t calculate_rex_prefix(R reg,indirect_address_t<bit_addr_size_,bit_dest_size_> addr)
{
    uint8_t rex=addr.rex_xb();
    if(reg.bit_size!=64&&rex==0) return 0;
    if constexpr(R::bit_size==64)
    {
        if constexpr(!std::is_same_v<R,sse_register_t>)
        {
            rex|=rex_w;
        }
        if(reg.code>=8)  rex|=rex_r;
    }
    return rex;
}

template<class R1,class R2>
constexpr uint8_t calculate_rex_prefix(R1 r1,R2 r2)
{
    uint8_t res=0;
    if constexpr(R1::bit_size==64)// set REX prefix
    {
        if constexpr(!std::is_same_v<R1,sse_register_t>)
        {
            res=rex_w;
        }
        if(r1.code>=8)  res|=rex_r;
        if(r2.code>=8)  res|=rex_b;
        if(res!=0)      res|=0x40;
    }
    return res;
}

template<class R1,class R2>
constexpr uint8_t get_modrm(R1 r1,R2 r2)
{
    uint8_t res=modrm_mod_rr;
    res|=r1.three_bit_code()<<3;
    res|=r2.three_bit_code();
    return res;
}

template<class R,uint32_t bit_addr_size_,int bit_dest_size_>
constexpr uint8_t get_modrm(R reg,indirect_address_t<bit_addr_size_,bit_dest_size_> addr)
{
    uint8_t res=addr.modrm_mod();
    res|=reg.three_bit_code()<<3;
    if(!addr.is_sib)
    {
        res|=addr.base.three_bit_code();
    }
    else//SIB
    {
        res|=4;//ModRM.rm=100b
    }
    return res;
}

/////////////////////////////////////////////////////////////
//                      Encoding
////////////////////////////////////////////////////////////

// Binary mnemonics

template<class R>
constexpr code_array aux_make_code(uint8_t opcode,R reg1,R reg2)
{
    code_array res;
    //REX
    if(uint8_t rex=calculate_rex_prefix(reg1,reg2);rex) res.push_byte(rex);
    //override operand size
    if constexpr(R::bit_size==16) res.push_byte(override_operand_size);

    res.push_byte(opcode);
    //ModRM
    res.push_byte(get_modrm(reg1,reg2));
    return res;
}

template<class R,uint32_t bit_addr_size_,int bit_dest_size_>
constexpr code_array aux_make_code(uint8_t opcode,R reg,indirect_address_t<bit_addr_size_,bit_dest_size_> addr)
{
    using A=indirect_address_t<bit_addr_size_,bit_dest_size_>;
    static_assert(A::dest_size_auto||A::byte_dest_size==R::byte_size,
    "Invalid destination register size");

    code_array res;
    //override_address_size
    if constexpr(A::bit_addr_size!=64) res.push_byte(override_address_size);
    //override operand size
    if constexpr(R::bit_size==16)       res.push_byte(override_operand_size);

    //REX prefix
    if(uint8_t rex=calculate_rex_prefix(reg,addr);rex) res.push_byte(rex);
    //opcode
    res.push_byte(opcode);
    //ModRM
    res.push_byte(get_modrm(reg,addr));
    //SIB
    if(addr.is_sib) res.push_byte(addr.sib());
    //Displacement
    if(addr.disp!=0)
    {
        if(addr.disp_size()==1) res.push_byte(addr.disp);
        else                    res.push_data(addr.disp);
    }
    return res;
}

// Unary mnemonics
template<uint32_t bit_size_,class der_t>
constexpr code_array aux_make_code(uint8_t opcode,base_register_t<bit_size_,der_t>&reg,encoding_type_t enc)
{
    using R=der_t;
    code_array res;
    // override operand size
    if constexpr(R::bit_size==16) res.push_byte(override_operand_size);
    // REX
    else if constexpr(R::bit_size==64)
    {
        res.push_byte( ((reg.code<8)? rex_w:rex_w|rex_b) );
    }
    else{}

    if(enc==added_opcode)
    {
        res.push_byte(opcode+reg.three_bit_code());
    }
    else// ModRM
    {
        res.push_byte(opcode);
        res.push_byte(reg.three_bit_code()|modrm_mod_rr);
        res.back()|=enc<<3;
    }
    return res;
}

template<class R,class imm_t,bool no_check_size=false>
constexpr code_array aux_make_code(uint8_t opcode,R reg,imm_t imm,encoding_type_t enc)
{
    static_assert(no_check_size||sizeof(imm_t)==R::byte_size||(sizeof(imm_t)==4&&R::byte_size==8));
    code_array res=aux_make_code(opcode,reg,enc);
    res.push_data(imm);
    return res;
}

template<uint32_t bit_addr_size_,int bit_dest_size_>
constexpr code_array aux_make_code(uint8_t opcode,indirect_address_t<bit_addr_size_,bit_dest_size_> addr,encoding_type_t enc)
{
    using A=indirect_address_t<bit_addr_size_,bit_dest_size_>;
    code_array res;
    //override_address_size
    if constexpr(A::bit_addr_size!=64) res.push_byte(override_address_size);

    //REX prefix
    if(uint8_t rex=addr.rex_xb();rex) res.push_byte(rex);

    //opcode
    res.push_byte(opcode);

    //ModRM
    res.push_byte(addr.modrm_mod());
    res.back()|=enc<<3;
    if(!addr.is_sib) {res.back()|= addr.base.three_bit_code();}
    else//SIB
    {
        res.back()|=4;//ModRM.rm=100b
        res.push_byte(addr.sib());
    }
    //Displacement
    if(addr.disp!=0)
    {
        if(addr.disp_size()==1) res.push_byte(addr.disp);
        else                    res.push_data(addr.disp);
    }

    return res;
}

template<uint32_t bit_addr_size_,int bit_dest_size_,class imm_t,bool no_check_size=false>
constexpr code_array aux_make_code(uint8_t opcode,indirect_address_t<bit_addr_size_,bit_dest_size_> addr,imm_t imm,encoding_type_t enc)
{
    using A=indirect_address_t<bit_addr_size_,bit_dest_size_>;
    static_assert(no_check_size||A::dest_size_auto||sizeof(imm_t)==A::byte_addr_size||(sizeof(imm_t)==4&&A::byte_addr_size==8));

    code_array res=aux_make_code(opcode,addr,enc);
    res.push_data(imm);
    return res;
}

//////////////////////////////////////////////////////////
//      Front-end for different type make code functions
//////////////////////////////////////////////////////////

//Binary instruction

template<uint32_t bit_size>
constexpr code_array make_code(bin_ins_t ins,gpr_register_t<bit_size> reg1,gpr_register_t<bit_size> reg2)
{
    if constexpr(bit_size==8)
    {
        return aux_make_code(bin_opcodes[ins].mr_8,reg2,reg1);
    }
    else
    {
        return aux_make_code(bin_opcodes[ins].mr_any,reg2,reg1);
    }
}

template<uint32_t bit_size,uint32_t bit_addr_size_,int bit_dest_size_>
inline constexpr code_array make_code(bin_ins_t ins,gpr_register_t<bit_size> reg,
                                      indirect_address_t<bit_addr_size_,bit_dest_size_> addr)
{
    if constexpr(bit_size==8)
    {
        return aux_make_code(bin_opcodes[ins].rm_8,reg,addr);
    }
    else
    {
        return aux_make_code(bin_opcodes[ins].rm_any,reg,addr);
    }
}

template<uint32_t bit_size,uint32_t bit_addr_size_,int bit_dest_size_>
constexpr code_array make_code(bin_ins_t ins,indirect_address_t<bit_addr_size_,bit_dest_size_> addr,gpr_register_t<bit_size> reg)
{
    if constexpr(bit_size==8)
    {
        return aux_make_code(bin_opcodes[ins].mr_8,reg,addr);
    }
    else
    {
        return aux_make_code(bin_opcodes[ins].mr_any,reg,addr);
    }
}

template<class value_t>
constexpr bool is_integer()
{
    return std::is_same_v<value_t,uint8_t> ||std::is_same_v<value_t,int8_t> ||
           std::is_same_v<value_t,uint16_t>||std::is_same_v<value_t,int16_t>||
           std::is_same_v<value_t,uint32_t>||std::is_same_v<value_t,int32_t>||
           std::is_same_v<value_t,uint64_t>||std::is_same_v<value_t,int64_t>;
}

template<uint8_t bytes,class value_t>
constexpr auto cast_save_sign(value_t val)
{
    static_assert(is_integer<value_t>());
    static_assert(bytes>=sizeof(value_t));
    static_assert(bytes==1||bytes==2||bytes==4||bytes==8);
    if constexpr(bytes==1)
    {
        return val;
    }
    else if constexpr(bytes==2)
    {
        if constexpr(std::is_signed_v<value_t>)
        {
            return static_cast<int16_t>(val);
        }
        else
        {
            return static_cast<uint16_t>(val);
        }
    }
    else if constexpr(bytes==4)
    {
        if constexpr(std::is_signed_v<value_t>)
        {
            return static_cast<int32_t>(val);
        }
        else
        {
            return static_cast<uint32_t>(val);
        }
    }
    else
    {
        if constexpr(std::is_signed_v<value_t>)
        {
            return static_cast<int64_t>(val);
        }
        else
        {
            return static_cast<uint64_t>(val);
        }
    }
}

// value_t!=64 immediate ( for mov instruction)

template<uint32_t bit_size,class value_t>
constexpr code_array make_code(bin_ins_t ins,gpr_register_t<bit_size> reg,value_t imm)
{
    static_assert(is_integer<value_t>());
    static_assert(bit_size>=sizeof(value_t)*8);
    static_assert(sizeof(value_t)<=4);

    auto cast_imm=cast_save_sign<std::min(4u,bit_size>>3)>(imm);
    if constexpr(bit_size==8)
    {
        return aux_make_code(bin_opcodes[ins].imm_8,reg,cast_imm,bin_opcodes[ins].dig);
    }
    else
    {
        return aux_make_code(bin_opcodes[ins].imm_any,reg,cast_imm,bin_opcodes[ins].dig);
    }
}

template<uint32_t bit_addr_size_,int bit_dest_size_,class value_t>
constexpr code_array make_code(bin_ins_t ins,indirect_address_t<bit_addr_size_,bit_dest_size_> addr,value_t imm)
{
    using A=indirect_address_t<bit_addr_size_,bit_dest_size_>;
    static_assert(is_integer<value_t>());
    static_assert(!A::dest_size_auto);
    static_assert(A::byte_dest_size>=sizeof(value_t)*8);
    static_assert(sizeof(value_t)<=4);

    auto cast_imm=cast_save_sign<std::min(4u,A::bit_dest_size>>3)>(imm);
    if constexpr(sizeof(value_t)==1)
    {
        return aux_make_code(bin_opcodes[ins].imm_8,addr,cast_imm,bin_opcodes[ins].dig);
    }
    else
    {
        return aux_make_code(bin_opcodes[ins].imm_any,addr,cast_imm,bin_opcodes[ins].dig);
    }
}

//Unary instruction

template<uint32_t bit_size>
constexpr code_array make_code(un_ins_t ins,gpr_register_t<bit_size> reg)
{
    uint8_t opc=(bit_size==8)? un_opcodes[ins].rm_8: un_opcodes[ins].rm_any;
    constexpr_assert(opc!=not_opcode,"Some instruction does not support the operand 8-bit register");
    if constexpr(bit_size==32) constexpr_assert(!(ins==pop||ins==push),
    "push reg32 and pop reg32 not support in 64 bits mode");
    return aux_make_code(opc,reg, un_opcodes[ins].encoding);
}

template<uint32_t bit_size>
constexpr code_array make_code(un_ins_t ins,gpr_register_t<bit_size> reg,uint8_t imm)//shl,shr,sal,sar
{
    constexpr_assert(un_opcodes[ins].need_imm,"Some instruction does not support immediate operand");
    uint8_t opc=(bit_size==8)? un_opcodes[ins].rm_8:un_opcodes[ins].rm_any;
    constexpr_assert(opc!=not_opcode,"Some instruction does not support the operand 8-bit register");
    return aux_make_code<gpr_register_t<bit_size>,uint8_t,true>(opc,reg,imm,un_opcodes[ins].encoding);
}

template<class A>
constexpr code_array make_code(un_ins_t ins,A addr)
{
    static_assert(!A::dest_size_auto,"Undefined destination size");
    uint8_t opc=(A::byte_dest_size==1)? un_opcodes[ins].rm_8:un_opcodes[ins].rm_any;
    if constexpr(A::bit_dest_size==32) constexpr_assert(!(ins==pop||ins==push),
    "push mem32 and pop mem32 not support in 64 bits mode");
    constexpr_assert(opc!=not_opcode,"Some instruction does not support the operand 8-bit destination in memory");
    return aux_make_code(opc,addr,un_opcodes[ins].encoding);
}

template<uint32_t bit_addr_size_,int bit_dest_size_>
constexpr code_array make_code(un_ins_t ins,indirect_address_t<bit_addr_size_,bit_dest_size_> addr,uint8_t imm)//shl,shr,sal,sar
{
    using A=indirect_address_t<bit_addr_size_,bit_dest_size_>;
    constexpr_assert(un_opcodes[ins].need_imm,"Some instruction does not support immediate operand");
    static_assert(!A::dest_size_auto,"Undefined destination size");
    uint8_t opc=(A::byte_dest_size==1)?  un_opcodes[ins].rm_8:un_opcodes[ins].rm_any;
    constexpr_assert(opc!=not_opcode,"Some instruction does not support the operand 8-bit destination in memory");
    return aux_make_code<bit_addr_size_,bit_dest_size_,uint8_t,true>(opc,addr,imm,un_opcodes[ins].encoding);
}

//Jump instruction

constexpr code_array make_code(jump_ins_t ins,int8_t rel)
{
    code_array res;
    res.push_byte(jmp_opcodes[ins].rel8);
    res.push_byte(rel);
    return res;
}

constexpr code_array make_code(jump_ins_t ins,int32_t rel)
{
    code_array res;
    if(ins!=jmp) res.push_byte(first_jumps32_opcode);
    res.push_byte(jmp_opcodes[ins].rel32);
    res.push_data(rel);
    return res;
}


///////////////////////////////////////////////////////////////////////
//                  x87 Encoding
///////////////////////////////////////////////////////////////////////

//non-operand instructions

constexpr code_array make_code(x87_void_ins_t ins)
{
    return code_array{0xD9,x87_void_opcodes[ins]};
}

//binary instructions

//reg1 op= reg2
constexpr code_array make_code(x87_arith_ins_t ins,x87_register_t reg1,x87_register_t reg2)
{
    constexpr_assert(reg1==st0||reg2==st0,"One of the operands must be st0");
    code_array res;
    if(reg1==st0)
    {
        res.push_byte(0xD8);
        res.push_byte(x87_arith_opcodes[ins].st0_sti+reg2.code);
    }
    else
    {
        res.push_byte(0xDC);
        res.push_byte(x87_arith_opcodes[ins].sti_st0+reg1.code);
    }
    return res;
}

//reg op=st0,pop st0
constexpr code_array make_code(x87_arith_ins_t ins,x87_register_t reg)
{
    code_array res;
    res.push_byte(0xDE);
    res.push_byte(x87_arith_opcodes[ins].sti_st0_with_pop+reg.code);
    return res;
}

//st1 op=st0,pop st0
constexpr code_array make_code(x87_arith_ins_t ins)
{
    code_array res;
    res.push_byte(0xDE);
    res.push_byte(x87_arith_opcodes[ins].op_with_pop);
    return res;
}

template<class A>
constexpr code_array make_code(x87_arith_ins_t ins,A addr,bool is_float_ptr)
{
    static_assert(!A::dest_size_auto,"Undefined destination size");
    static_assert(A::byte_dest_size==4||A::byte_dest_size==8);
    uint8_t opcode;
    if(is_float_ptr)
    {
        opcode=(addr.byte_dest_size==4)? 0xD8:0xDC;
    }
    else
    {
        opcode=(addr.byte_dest_size==4)? 0xDA:0xDE;
    }
    return aux_make_code(opcode,addr,x87_arith_opcodes[ins].encoding);
}

//unary instructions

template<class A>
constexpr code_array make_code(x87_un_ins_t ins,A addr)
{
    static_assert(!A::dest_size_auto,"Undefined destination size");
    static_assert(A::byte_dest_size>1);
    uint8_t opcode=x87_un_opcodes[ins].mem(addr.byte_dest_size);
    constexpr_assert(opcode!=not_opcode,"No opcode for some unary x87 instruction");
    return aux_make_code(opcode,addr,x87_un_opcodes[ins].enc(addr.byte_dest_size));
}

constexpr code_array make_code(x87_un_ins_t ins,x87_register_t reg)
{
    constexpr_assert(x87_un_opcodes[ins].reg_opcodes[0]!=not_opcode,"No opcode for some unary x87 instruction");
    return code_array{x87_un_opcodes[ins].reg_opcodes[0],
                      uint8_t(x87_un_opcodes[ins].reg_opcodes[1]+reg.code)};
}

///////////////////////////////////////////////////////////////////////
//                  SSE Encoding
///////////////////////////////////////////////////////////////////////

template<uint8_t byte_float>
constexpr code_array make_code(sse_bin_ins_t ins,sse_register_t reg1,sse_register_t reg2)
{
    code_array res;
    if constexpr(byte_float==4)
    {
        res.push_byte(sse_bin_data::single_prefix);
    }
    else
    {
        res.push_byte(sse_bin_data::double_prefix);
    }
    if(uint8_t rex=calculate_rex_prefix(reg1,reg2);rex) res.push_byte(rex);
    res.push_byte(0x0F);
    res.push_byte(sse_bin_opcodes[ins].opcode);
    res.push_byte(get_modrm(reg1,reg2));
    return res;
}

template<uint8_t byte_float,class A>
constexpr code_array make_code(sse_bin_ins_t ins,sse_register_t reg,A addr)
{
    static_assert(byte_float==A::byte_dest_size||A::dest_size_auto);

    code_array res;
    if constexpr(A::bit_addr_size!=64) res.push_byte(override_address_size);
    if constexpr(byte_float==4)
    {
        res.push_byte(sse_bin_data::single_prefix);
    }
    else
    {
        res.push_byte(sse_bin_data::double_prefix);
    }
    if(uint8_t rex=calculate_rex_prefix(reg,addr);rex) res.push_byte(rex);

    res.push_byte(0x0F);
    res.push_byte(sse_bin_opcodes[ins].opcode);
    res.push_byte(get_modrm(reg,addr));
    if(addr.is_sib) res.push_byte(addr.sib());
    //Displacement
    if(addr.disp!=0)
    {
        if(addr.disp_size()==1) res.push_byte(addr.disp);
        else                    res.push_data(addr.disp);
    }
    return res;
}

template<uint8_t byte_float,class A>
constexpr code_array make_code(sse_bin_ins_t ins,A addr,sse_register_t reg)
{
    static_assert(byte_float==A::byte_dest_size||A::dest_size_auto);
    constexpr_assert(ins==sse_mov,"ins==movss||ins==movsd for SSE register to memory");

    code_array res;
    if constexpr(A::bit_addr_size!=64) res.push_byte(override_address_size);
    if constexpr(byte_float==4)
    {
        res.push_byte(sse_bin_data::single_prefix);
    }
    else
    {
        res.push_byte(sse_bin_data::double_prefix);
    }
    if(uint8_t rex=calculate_rex_prefix(reg,addr);rex) res.push_byte(rex);

    res.push_byte(0x0F);
    res.push_byte(0x11);
    res.push_byte(get_modrm(reg,addr));
    if(addr.is_sib) res.push_byte(addr.sib());
    //Displacement
    if(addr.disp!=0)
    {
        if(addr.disp_size()==1) res.push_byte(addr.disp);
        else                    res.push_data(addr.disp);
    }
    return res;
}


}//encoding_x64

#endif












