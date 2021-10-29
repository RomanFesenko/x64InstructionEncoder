
#ifndef  _timing_
#define  _timing_

#include<chrono>
#include<string>


template<class period_t>
const std::string& period_name()
{
    using str_t=/*static*/ const std::string;
    static str_t  nano="nanosec";
    static str_t  micro="microsec";
    static str_t  mili="milisec";
    static str_t  sec="sec";
    static str_t  minutes="minutes";
    static str_t  hours="minutes";
    if constexpr(std::is_same_v<period_t,std::chrono::nanoseconds>)
    {
        return nano;
    }
    else if constexpr(std::is_same_v<period_t,std::chrono::microseconds>)
    {
        return micro;
    }
    else if constexpr(std::is_same_v<period_t,std::chrono::milliseconds>)
    {
        return mili;
    }
    else if constexpr(std::is_same_v<period_t,std::chrono::seconds>)
    {
        return sec;
    }
    else if constexpr(std::is_same_v<period_t,std::chrono::minutes>)
    {
        return minutes;
    }
    else if constexpr(std::is_same_v<period_t,std::chrono::hours>)
    {
        return hours;
    }
    else{}
}

class CTimer
{
    using clock_t=std::chrono::high_resolution_clock;
    using time_t=clock_t::time_point;
    using duration_t=clock_t::duration;
    using period_t=clock_t::period;
    clock_t m_clock;
    time_t m_begin,m_end;
    bool m_stoped=false;
    public:
    CTimer():m_begin(m_clock.now()){}

    void Restart() {m_begin=m_clock.now();m_stoped=false;}
    void Stop(){m_end=m_clock.now();m_stoped=true;}
    bool IsStopped()const{return m_stoped;}
    template<class use_p_t=std::chrono::seconds>
    double Pass()const
    {
        using for_cast=std::chrono::duration<double,typename use_p_t::period>;
        duration_t d=(m_stoped)? m_end-m_begin:m_clock.now()-m_begin;
        return std::chrono::duration_cast<for_cast>(d).count();
    }
    template<class use_p_t=std::chrono::seconds>
    std::string PassAsString()const
    {
        return std::to_string(Pass<use_p_t>())+" "+period_name<use_p_t>();
    }
};

#endif

