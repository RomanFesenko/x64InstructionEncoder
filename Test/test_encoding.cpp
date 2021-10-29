
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <assert.h>

#include "../value_traits.h"

#include "test_encoding.h"
#include "../encoding.h"


using namespace encoding_x64;


/*void out_dump(std::vector<uint8_t>&dump)
{
    for(uint8_t b:dump)
    {
        if(b<16)
        {
            std::cout<<'0';
        }
        std::cout<<std::hex<<(int)b<<',';
    }
}*/

static void test_encoding_two_operand()
{
    ///MOV
    // register->register
    static_assert(make_code(mov,eax,eax)==code_array{0x89,0xC0});
    static_assert(make_code(mov,eax,ecx)==code_array{0x89,0xC8});
    static_assert(make_code(mov,r8,rax)==code_array{0x49,0x89,0xC0});
    static_assert(make_code(mov,rbx,r14)==code_array{0x4C,0x89,0xF3});
    static_assert(make_code(mov,ax,dx)==code_array{0x66,0x89,0xD0});
    static_assert(make_code(mov,al,dl)==code_array{0x88,0xD0});
    // register->memory
    static_assert(make_code(mov,ptr(ebx),eax)==code_array{0x67,0x89,0x03});
    static_assert(make_code(mov,ptr(_2,rcx,rbx),eax)==code_array{0x89,0x04,0x4B});
    static_assert(make_code(mov,ptr(esp),r12)==code_array{0x67,0x4C,0x89,0x24,0x24});
    // memory->register
    static_assert(make_code(mov,eax,ptr(ebx))==code_array{0x67,0x8B,0x03});
    static_assert(make_code(mov,eax,ptr(_2,rcx,rbx))==code_array{0x8B,0x04,0x4B});
    static_assert(make_code(mov,r12,ptr(esp))==code_array{0x67,0x4c,0x8b,0x24,0x24});
    static_assert(make_code(mov,esp,ptr(_4,rcx,rsp,-420))==code_array{0x8b,0xa4,0x8c,0x5c,0xfe,0xff,0xff});
    // unsigneg immediate->register
    static_assert(make_code(mov,al,uint8_t(78))==code_array{0xC6,0xC0,0x4E});

    static_assert(make_code(mov,bx,uint16_t(256))==code_array{0x66,0xC7,0xC3,0x00,0x01});
    static_assert(make_code(mov,edx,uint32_t(256))==code_array{0xC7,0xC2,0x00,0x01,0x00,0x00});
    static_assert(make_code(mov,r14,int32_t(256))==code_array{0x49,0xc7,0xc6,0x00,0x01,0x00,0x00});

    ///ADD
    // register+->register
    static_assert(make_code(add,eax,eax)==code_array{0x01,0xc0});
    static_assert(make_code(add,eax,ecx)==code_array{0x01,0xC8});
    static_assert(make_code(add,r8,rax)==code_array{0x49,0x01,0xC0});
    static_assert(make_code(add,rbx,r14)==code_array{0x4C,0x01,0xF3});
    static_assert(make_code(add,ax,dx)==code_array{0x66,0x01,0xD0});
    static_assert(make_code(add,al,dl)==code_array{0x00,0xD0});

    // register+->memory
    static_assert(make_code(add,ptr(ebx),eax)==code_array{0x67,0x01,0x03});
    static_assert(make_code(add,ptr(_2,rcx,rbx),eax)==code_array{0x01,0x04,0x4B});
    static_assert(make_code(add,ptr(esp),r12)==code_array{0x67,0x4C,0x01,0x24,0x24});
    // memory+->register
    static_assert(make_code(add,eax,ptr(ebx))==code_array{0x67,0x03,0x03});
    static_assert(make_code(add,eax,ptr(_2,rcx,rbx))==code_array{0x03,0x04,0x4B});
    static_assert(make_code(add,r12,ptr(esp))==code_array{0x67,0x4c,0x03,0x24,0x24});
    static_assert(make_code(add,esp,ptr(_4,rcx,rsp,-420))==code_array{0x03,0xa4,0x8c,0x5c,0xfe,0xff,0xff});
    // unsigneg immediate+->register
    static_assert(make_code(add,al,uint8_t(8))==code_array{0x80,0xC0,0x08});
    static_assert(make_code(add,bx,uint16_t(256))==code_array{0x66, 0x81,0xC3,0x00,0x01});
    static_assert(make_code(add,edx,uint32_t(256))==code_array{0x81,0xC2,0x00,0x01,0x00,0x00});
    static_assert(make_code(add,r14,int32_t(1))==code_array{0x49,0x81,0xC6,0x01,0x00,0x00,0x00});


    ///SUB
    // register+->register
    static_assert(make_code(sub,eax,eax)==code_array{0x29,0xc0});
    static_assert(make_code(sub,eax,ecx)==code_array{0x29,0xC8});
    static_assert(make_code(sub,r8,rax)==code_array{0x49,0x29,0xC0});
    static_assert(make_code(sub,rbx,r14)==code_array{0x4C,0x29,0xF3});
    static_assert(make_code(sub,ax,dx)==code_array{0x66,0x29,0xD0});
    static_assert(make_code(sub,al,dl)==code_array{0x28,0xD0});

    // register+->memory
    static_assert(make_code(sub,ptr(ebx),eax)==code_array{0x67,0x29,0x03});
    static_assert(make_code(sub,ptr(_2,rcx,rbx),eax)==code_array{0x29,0x04,0x4B});
    static_assert(make_code(sub,ptr(esp),r12)==code_array{0x67,0x4C,0x29,0x24,0x24});
    // memory+->register
    static_assert(make_code(sub,eax,ptr(ebx))==code_array{0x67,0x2B,0x03});
    static_assert(make_code(sub,eax,ptr(_2,rcx,rbx))==code_array{0x2B,0x04,0x4B});
    static_assert(make_code(sub,r12,ptr(esp))==code_array{0x67,0x4c,0x2B,0x24,0x24});
    static_assert(make_code(sub,esp,ptr(_4,rcx,rsp,-420))==code_array{0x2B,0xa4,0x8c,0x5c,0xfe,0xff,0xff});
    ///SSE
    static_assert(make_code<4>(sse_mov,xmm15,ptr(r11))==code_array{0xF3,0x45,0x0F,0x10,0x3B});
    static_assert(make_code<8>(sse_mov,xmm15,xmm14)==code_array{0xF2,0x45,0x0F,0x10,0xFE});
    static_assert(make_code<8>(sse_mov,ptr(rdx),xmm6)==code_array{ 0xf2,0x0f,0x11,0x32});
    static_assert(make_code<8>(sse_mov,ptr(rdx,8),xmm6)==code_array{0xF2,0x0F,0x11,0x72,0x08});

    //auto code=make_code(movsd,ptr(rdx,8),xmm6);
    //for(auto b:code) std::cout<<std::hex<<(int)b<<",";
}

static void test_encoding_one_operand()
{
    ///INC
    // register
    static_assert(make_code(inc,ecx)==code_array{0xFF,0xC1});
    static_assert(make_code(inc,r8)==code_array{0x49,0xFF,0xC0});
    static_assert(make_code(inc,dx)==code_array{0x66,0xFF,0xC2});
    static_assert(make_code(inc,al)==code_array{0xFE,0xC0});
    // memory
    static_assert(make_code(inc,byte_ptr(ebx))==code_array{0x67,0xFE,0x03});
    static_assert(make_code(inc,dword_ptr(ebx))==code_array{0x67,0xFF,0x03 });
    static_assert(make_code(inc,byte_ptr(_2,rcx,rbx))==code_array{0xFE,0x04,0x4B});
    static_assert(make_code(inc,dword_ptr(esp))==code_array{0x67,0xFF,0x04,0x24});

    ///SHL
    static_assert(make_code(shl,rax,5)==code_array{0x48,0xC1,0xE0,0x05});
    static_assert(make_code(shl,rcx,2)==code_array{0x48,0xC1,0xE1,0x02});
    static_assert(make_code(shl,byte_ptr(_4,rax,rsp),5)==code_array{0xC0,0x24,0x84,0x05});

    ///IDIV
    // register
    static_assert(make_code(idiv,ecx)==code_array{0xF7,0xF9});
    static_assert(make_code(idiv,r8)==code_array{0x49,0xF7,0xF8});
    static_assert(make_code(idiv,dx)==code_array{0x66,0xF7,0xFA});
    static_assert(make_code(idiv,al)==code_array{0xF6,0xF8});
    // memory
    static_assert(make_code(idiv,byte_ptr(ebx))==code_array{0x67,0xF6,0x3B});
    static_assert(make_code(idiv,dword_ptr(ebx))==code_array{0x67,0xF7,0x3B});
    static_assert(make_code(idiv,byte_ptr(_2,rcx,rbx))==code_array{0xF6,0x3C,0x4B});
    static_assert(make_code(idiv,dword_ptr(esp))==code_array{0x67,0xF7,0x3C,0x24});

}

void test_sort_int() //insertion sort
{
    CCode c;
    c.push(rbx);//save ebx//push   rbx
    c.mov(rcx,ptr(rax));//rcx=begin
    c.movsx(rdx,dword_ptr(rax,8));//rdx=N
    c.lea(rdx,ptr(_4,rdx,rcx));// rdx=end

    c.cmp(rcx,rdx);
    c.je("exit");

    c.mov(r8,rcx);
    c.add(r8,4);//r8=begin+1

    c.label("extern_cycle");
    c.mov(r9,r8);
    c.mov(eax,ptr(r9));//eax=*r8
    c.cmp(ptr(r9,-4),eax);//*(r8-1)>*r8
    c.jng("intern_exit");

    c.label("intern_cycle");
    //swap *r9 and *(r9-1)
    c.mov(eax,ptr(r9));
    c.mov(ebx,ptr(r9,-4));
    c.mov(ptr(r9,-4),eax);
    c.mov(ptr(r9),ebx);
    c.sub(r9,4);// --r9

    c.cmp(r9,rcx);//curent!=begin
    c.je("intern_exit");
    c.mov(eax,ptr(r9));//eax=*r9
    c.cmp(ptr(r9,-4),eax);//*(r9-1)>*r9
    c.jg("intern_cycle");
    c.label("intern_exit");

    c.add(r8,4);
    c.cmp(r8,rdx);
    c.jne("extern_cycle");
    c.label("exit");
    c.pop(rbx);//restore rbx
    c.ret();
    auto jit=c.Compile();
    /*auto insertion_sort=[](int*begin,int num)
    {
        for(int* pivot=begin+1;pivot<begin+num;++pivot)
        {
            for(int* current=pivot;current!=begin&&*current<current[-1];--current)
            {
                std::iter_swap(current,current-1);
            }
        }
    };*/
    std::vector<int> vec={5,4,3,2,1,0,-1,-123};
    std::vector<int> sorted_vec={-123,0,1,2,3,4,5,6,7};
    jit.execute<void,int*,int>(vec.data(),vec.size());
    jit.execute<void,int*,int>(sorted_vec.data(),sorted_vec.size());
    assert(std::is_sorted(sorted_vec.begin(),sorted_vec.end()));
}


void test_sum_float()
{
    CCode c;
    c.finit();
    c.mov(rdx,ptr(rax));
    c.mov(rcx,ptr(rax,8));// num elements - 64 bit
    c.shl(rcx,2);
    c.add(rcx,rdx);
    c.fldz();
    c.cmp(rcx,rdx);
    c.je("exit");
    c.label("cycle");
    c.fadd(dword_ptr(rdx));
    c.add(rdx,4);
    c.cmp(rcx,rdx);
    c.jne("cycle");
    c.label("exit");
    c.fstp(dword_ptr(rax));
    c.ret();

    auto ftr=c.Compile();
    auto check=[](float*fst,int num)
    {
        float res=0;
        for(int i=0;i<num;++i,++fst ) res+=*fst;
        return res;
    };

    std::vector<float> vec={1,2,3,4,5.9876};
    float sum=ftr.execute<float,float*,uint64_t>(vec.data(),vec.size());
    bool  eq=value_traits<float>::almost_equal(check(vec.data(),vec.size()),sum);
    if(!eq)
    {
        std::cout<<"error:"<<sum<<'\n';;
    }
}

void test_encoding()
{
    test_encoding_two_operand();
    test_encoding_one_operand();

    test_sum_float();
    test_sort_int();
}


