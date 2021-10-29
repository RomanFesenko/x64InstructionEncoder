
#ifndef  _opcodes_
#define  _opcodes_

namespace encoding_x64{

const uint8_t not_opcode=0x67;//

enum  encoding_type_t:uint8_t
{
    dig0=0,dig1,dig2,dig3,dig4,dig5,dig6,dig7,
    added_opcode,no_encoding
};

////////////////////////////////////////////////////////
//           Jumps instructions
///////////////////////////////////////////////////////

enum jump_ins_t
{
    jmp,
    ja, jae, jb, jbe, jc, je, jg, jge, jl, jle, jo, jp, jpo,js, jz,
    jna,jnae,jnb,jnbe,jnc,jne,jng,jnge,jnl,jnle,jno,jnp,    jns,jnz,
};

struct jmp_op_data
{
    uint8_t rel8;
    uint8_t rel32;
};

constexpr jmp_op_data jmp_opcodes[jump_ins_t::jnz+1]=
{
    {0xEB,0xE9}, //jmp

    {0x77,0x87},
    {0x73,0x83},
    {0x72,0x82},
    {0x76,0x86},
    {0x72,0x82},
    {0x74,0x84},
    {0x7F,0x8F},
    {0x7D,0x8D},
    {0x7C,0x8C},
    {0x7E,0x8E},
    {0x70,0x80},
    {0x7A,0x8A},
    {0x7B,0x8B},
    {0x78,0x88},
    {0x74,0x84},
    //not
    {0x76,0x86},
    {0x72,0x82},
    {0x73,0x83},
    {0x77,0x87},
    {0x73,0x83},
    {0x75,0x85},
    {0x7E,0x8E},
    {0x7C,0x8C},
    {0x7D,0x8D},
    {0x7F,0x8F},
    {0x71,0x81},
    {0x7B,0x8B},

    {0x79,0x89},
    {0x75,0x85}
};

//////////////////////////////////////////////////////////
//      Binary instructions
//////////////////////////////////////////////////////////

enum bin_ins_t
{
    mov=0,add,sub,adc,sbb,cmp,_and,_or,_xor,test,
};

struct bin_op_data
{
    uint8_t   mr_8; uint8_t  mr_any;//8,16,32,64
    uint8_t   rm_8; uint8_t  rm_any;//8,16,32,64
    uint8_t   imm_8;uint8_t  imm_any; encoding_type_t dig;
};

constexpr bin_op_data bin_opcodes[bin_ins_t::test+1]=
{
    //mov
    {
        0x88,0x89,
        0x8A,0x8B,
        0xC6,0xC7,dig0,
    },
    //add
    {
        0x00,0x01,
        0x02,0x03,
        0x80,0x81,dig0,
    },
    //sub
    {
        0x28,0x29,
        0x2A,0x2B,
        0x80,0x81,dig5,
    },
    //adc
    {
        0x10,0x11,
        0x12,0x13,
        0x80,0x81,dig2,
    },
    //sbb
    {
        0x18,0x19,
        0x1A,0x1B,
        0x80,0x81,dig3,
    },
    //cmp
    {
        0x38,0x39,
        0x3A,0x3B,
        0x80,0x81,dig7,
    },
    //and
    {
        0x20,0x21,
        0x22,0x23,
        0x80,0x81,dig4,
    },
    //or
    {
        0x08,0x09,
        0x0A,0x0B,
        0x80,0x81,dig1,
    },
    //xor
    {
        0x30,0x31,
        0x32,0x33,
        0x80,0x81,dig6,
    },
    //test
    {
        0x84,0x84,
        0x84,0x84,
        0xF6,0xF7,dig0
    },
};

/////////////////////////////////////////////////////////
//      Unary  instructions
////////////////////////////////////////////////////////

enum un_ins_t
{
    inc=0,dec,neg,_not,shl,shr,mul,imul,div,idiv,push,pop //
};

struct un_op_data
{
    bool    need_imm;

    uint8_t rm_8;
    uint8_t rm_any;

    encoding_type_t encoding;
};

constexpr un_op_data un_opcodes[un_ins_t::pop+1]=
{
    {false,0xFE,0xFF,dig0},//inc
    {false,0xFE,0xFF,dig1},//dec
    {false,0xF6,0xF7,dig3},//neg
    {false,0xF6,0xF7,dig2},//not
    {true,0xC0,0xC1,dig4}, //shl
    {true,0xC0,0xC1,dig5}, //shr
    {false,0xF6,0xF7,dig4},//mul
    {false,0xF6,0xF7,dig5},//imul
    {false,0xF6,0xF7,dig6},//div
    {false,0xF6,0xF7,dig7},//idiv

    {false,not_opcode,0xFF,dig6}, //push mem/r
    {false,not_opcode,0x8F,dig0}, //pop
};

/////////////////////////////////////////////////////////
//              x87 instructions
//////////////////////////////////////////////////////////


enum x87_arith_ins_t
{
    fadd,fsub,fsubr,fmul,fdiv,fdivr
};

struct x87_arith_data
{
    uint8_t st0_sti;
    uint8_t sti_st0;
    uint8_t sti_st0_with_pop;

    uint8_t op_with_pop;

    encoding_type_t encoding;
};

constexpr x87_arith_data x87_arith_opcodes[x87_arith_ins_t::fdivr+1]=
{
    //fadd
    {
        0xC0,0xC0,0xC0,
        0xC1,
        dig0
    },
    //fsub
    {
        0xE0,0xE8,0xE8,
        0xE9,
        dig4
    },
    //fsubr
    {
        0xE8,0xE0,0xE0,
        0xE1,
        dig5
    },
    //fmul
    {
        0xC8,0xC8,0xC8,
        0xC9,
        dig1
    },
    //fdiv
    {
        0xF0,0xF8,0xF8,
        0xF9,
        dig6
    },
    //fdivr
    {
        0xF8,0xF0,0xF0,
        0xF1,
        dig7
    }
};

//Without operand

enum x87_void_ins_t
{
    fabs,fchs,fcos,
    fld1,fldl2t,fldl2e,fldpi,fldlg2,fldln2,fldz,
    fnop,
    fpatan,
    fprem,
    fptan,
    frndint,
    fsin,
    fsincos,
    fsqrt,
    ftst,
    fyl2x
    //,f2xm1,fscale - hardcoded
};

constexpr uint8_t x87_void_opcodes[x87_void_ins_t::fyl2x+1]=
{
    0xE1,
    0xE0,
    0xFF,
    0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,
    0xD0,
    0xF3,
    0xF8,
    0xF2,
    0xFC,
    0xFE,
    0xFB,
    0xFA,
    0xE4,
    0xF1
};

//One operand

enum x87_un_ins_t
{
    fld,fst,fstp,fild,fist,fistp
    //fxch - hardcoded
};

struct x87_un_data_t
{
    uint8_t         mem_opcodes[4];//2,4,8,10
    encoding_type_t encodings[4];  //2,4,8,10
    uint8_t         reg_opcodes[2];
    constexpr uint8_t         mem(uint8_t size)const
    {
        switch(size>>1)
        {
            case 1:return mem_opcodes[0];
            case 2:return mem_opcodes[1];
            case 4:return mem_opcodes[2];
            case 5:return mem_opcodes[3];
        }
    }
    constexpr encoding_type_t       enc(uint8_t size)const
    {
        switch(size>>1)
        {
            case 1:return encodings[0];
            case 2:return encodings[1];
            case 4:return encodings[2];
            case 5:return encodings[3];
        }
    }
};

constexpr x87_un_data_t x87_un_opcodes[x87_un_ins_t::fistp+1]=
{
    //fld
    {
        {not_opcode,0xD9,0xDD,0xDB},
        {no_encoding,dig0,dig0,dig5},
        {0xD9,0xC0}
    },
    //fst
    {
        {not_opcode,0xD9,0xDD,not_opcode},
        {no_encoding,dig2,dig2,no_encoding},
        {0xDD,0xD0}
    },
    //fstp
    {
        {not_opcode,0xD9,0xDD,0xDB},
        {no_encoding,dig3,dig3,dig7},
        {0xDD,0xD8}
    },
    //fild
    {
        {0xDF,0xDB,0xDF,not_opcode},
        {dig0,dig0,dig5,no_encoding},
        {not_opcode,not_opcode}
    },
    //fist
    {
        {0xDF,0xDB,not_opcode,not_opcode},
        {dig2,dig2,no_encoding,no_encoding},
        {not_opcode,not_opcode}
    },
    //fistp
    {
        {0xDF,0xDB,0xDF,not_opcode},
        {dig3,dig3,dig7,no_encoding},
        {not_opcode,not_opcode}
    }
};

/////////////////////////////////////////////////////////
//              SSE instructions
//////////////////////////////////////////////////////////

enum sse_bin_ins_t
{
    sse_mov,sse_add,sse_sub,sse_mul,sse_div,sse_sqrt
};

struct sse_bin_data
{
    const static uint8_t single_prefix=0xF3;
    const static uint8_t double_prefix=0xF2;
    uint8_t opcode;
};

constexpr sse_bin_data sse_bin_opcodes[sse_bin_ins_t::sse_sqrt+1]=
{
    0x10,//movss
    0x58,//addss
    0x5C,//subss
    0x59,//mulss
    0x5E,//divss
    0x51,//sqrtsd
};



}//encoding_x64

#endif












