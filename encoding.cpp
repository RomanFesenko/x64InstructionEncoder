
#include "encoding.h"

encoding_x64::function encoding_x64::CCode::Compile()const
{
    const int prefix_size=10;
    assert(!Empty());
    function ret;
    ret.m_alloc_size=Size()+prefix_size;
    ret.m_code_size=ret.m_alloc_size;//temporary
    void*void_fun_ptr=mmap(0,ret.m_alloc_size,PROT_READ|PROT_WRITE|PROT_EXEC,MAP_PRIVATE|MAP_ANON,-1,0);
    uint8_t*fun_ptr=reinterpret_cast<uint8_t*>(void_fun_ptr);
    uint32_t i=0;
    fun_ptr[i++]=0x48;
    fun_ptr[i++]=0xb8;
    uint8_t* ptr=ret.m_args;
    fun_ptr[i++]=get_byte<0>(ptr);
    fun_ptr[i++]=get_byte<1>(ptr);
    fun_ptr[i++]=get_byte<2>(ptr);
    fun_ptr[i++]=get_byte<3>(ptr);
    fun_ptr[i++]=get_byte<4>(ptr);
    fun_ptr[i++]=get_byte<5>(ptr);
    fun_ptr[i++]=get_byte<6>(ptr);
    fun_ptr[i++]=get_byte<7>(ptr);

    auto code_iter=m_code.begin();
    for(const auto&j:m_jumps)
    {
        for(;i!=j.jump_address+prefix_size;++i,++code_iter)
        {
            fun_ptr[i]=*code_iter;
        }
        auto iter=m_labels.find(j.to_label);
        assert(iter!=m_labels.end());
        int32_t delta=iter->second-j.jump_address;
        //near jump
        take_apart(fun_ptr+i-4,delta);
    }
    std::copy(code_iter,m_code.end(),fun_ptr+i);

    ret.m_code=std::shared_ptr<uint8_t>(fun_ptr,[size=ret.m_alloc_size](uint8_t*ptr)
    {
        munmap(ptr,size);
    });
    ret.m_fptr=reinterpret_cast<void(*)(void)>(fun_ptr);
    return ret;
}



