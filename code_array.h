
#ifndef  _code_array_
#define  _code_array_

#include <initializer_list>

#include "util.h"


namespace encoding_x64{

class code_array
{
    uint8_t m_array[15];// max x64 instruction length
    uint8_t m_size;
    public:
    using iterator=uint8_t*;
    using const_iterator=const uint8_t*;
    using value_type=uint8_t;
    constexpr explicit code_array(uint8_t s=0):m_size(s){}
    constexpr code_array(std::initializer_list<uint8_t> list)
    {
        m_size=list.size();
        auto b=list.begin();
        for(uint8_t i=0;i<m_size;++i,++b) m_array[i]=*b;
    }
    constexpr code_array(const code_array&)=default;
    constexpr code_array(code_array&&)=default;
    constexpr uint8_t operator[](uint8_t i)const{return m_array[i];}
    constexpr uint8_t&operator[](uint8_t i){return m_array[i];}

    constexpr uint8_t back()const{return m_array[m_size-1];}
    constexpr uint8_t&back(){return m_array[m_size-1];}

    constexpr uint8_t size()const{return m_size;}
    constexpr void    resize(uint8_t res){ m_size=res;}

    constexpr bool operator==(const code_array&other)const
    {
        if(m_size!=other.m_size)  return false;
        for(int i=0;i<m_size;++i) if(m_array[i]!=other[i]) return false;
        return true;
    }
    constexpr bool operator!=(const code_array&other)const
    {
        return !((*this)==other);
    }
    constexpr iterator begin(){return m_array;}
    constexpr const_iterator begin()const{return m_array;}
    constexpr const_iterator cbegin()const{return m_array;}

    constexpr iterator end(){return m_array+m_size;}
    constexpr const_iterator end()const{return m_array+m_size;}
    constexpr const_iterator cend()const{return m_array+m_size;}

    constexpr bool empty()const{return m_size==0;}
    constexpr void clear(){m_size=0;}
    constexpr void push_byte(uint8_t val)
    {
        m_array[m_size++]=val;
    }
    template<class T>
    constexpr void push_data(T val)
    {
        static_assert(sizeof(T)==1||sizeof(T)==2||sizeof(T)==4||sizeof(T)==8);

        m_array[m_size++]=constexpr_get_byte<0>(val);
        if constexpr(sizeof(T)>=2)
        {
            m_array[m_size++]=constexpr_get_byte<1>(val);
        }
        if constexpr(sizeof(T)>=4)
        {
            m_array[m_size++]=constexpr_get_byte<2>(val);
            m_array[m_size++]=constexpr_get_byte<3>(val);
        }
        if constexpr(sizeof(T)==8)
        {
            m_array[m_size++]=constexpr_get_byte<4>(val);
            m_array[m_size++]=constexpr_get_byte<5>(val);
            m_array[m_size++]=constexpr_get_byte<6>(val);
            m_array[m_size++]=constexpr_get_byte<7>(val);
        }
    }
};

}//encoding_x64

#endif

