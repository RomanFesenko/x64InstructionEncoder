
#ifndef  _util_encoding
#define  _util_encoding

namespace encoding_x64{

constexpr std::size_t total_args_size(){return 0;}

template<class Fst,class...Args>
constexpr std::size_t total_args_size()
{
    std::size_t total=sizeof(Fst);
    if constexpr(sizeof...(Args)!=0)
    {
        total+=total_args_size<Args...>();;
    }
    return total;
}

template<class Fst,class...Args>
void take_apart(uint8_t*data,Fst&fst,Args&...rem)
{
    uint8_t*sourse=reinterpret_cast<uint8_t*>(&fst);
    for(unsigned i=0;i<sizeof(Fst);++i)
    {
        data[i]=sourse[i];
    }
    if constexpr(sizeof...(Args)>0)
    {
        take_apart<Args...>(data+sizeof(Fst),rem...);
    }
}

template<class T>
void assemble(const uint8_t*data,T&val)
{
    uint8_t*dest=reinterpret_cast<uint8_t*>(&val);
    for(std::size_t i=0;i<sizeof(T);++i)
    {
        dest[i]=data[i];;
    }
}

template<int I,class T>
constexpr uint8_t constexpr_get_byte(T value)
{
    static_assert(I<sizeof(T));
    //return *(reinterpret_cast<const uint8_t*>(&value)+I); not constexpr
    uint8_t res=0;
    res|=value>>(8*I);
    return res;
}

template<int I,class T>
uint8_t get_byte(T value)
{
    static_assert(I<sizeof(T));
    return *(reinterpret_cast<const uint8_t*>(&value)+I);
}

}//encoding_x64

#endif

