#ifndef  _string_utility_
#define  _string_utility_

#include<string>
#include<algorithm>
#include<utility>


//Utility dunctions

// Heterogeneous comparison
struct string_comparer_t
{
    using is_transparent=int;
    using str_citerator=std::string::const_iterator;
    using ipair=std::pair<str_citerator,str_citerator>;
    bool operator()(const std::string&str1,const std::string&str2)const
    {
        return str1<str2;
    }
    bool operator()(const ipair&pr,const std::string&str)const
    {
        return std::lexicographical_compare(pr.first,pr.second,
                                            str.begin(),str.end());
    }
    bool operator()(const std::string&str,const ipair&pr)const
    {
        return std::lexicographical_compare(str.begin(),str.end(),
                                            pr.first,pr.second);
    }
};

inline bool is_digit(char sym)
{
    return (sym>='0')&&(sym<='9');
}

inline bool is_identifier_begin(char sym)
{
    return ((sym>='a')&&(sym<='z'))||((sym>='A')&&(sym<='Z'))||sym=='_';
}

inline bool is_number_char(char sym)
{
    return is_digit(sym)||sym=='.';
}

inline bool is_number_begin(char sym)
{
    return is_number_char(sym)||sym=='-'||sym=='+';
}

inline bool is_letter(char sym)
{
    return ((sym>='a')&&(sym<='z'))||((sym>='A')&&(sym<='Z'));
}

inline bool is_identifier_end(char sym)
{
    return !(is_identifier_begin(sym)||is_digit(sym));
}



#endif
