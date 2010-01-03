/* $Id: string-utils.hh 363 2010-01-03 10:41:35Z nicola.bonelli $ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _STRING_UTILS_HH_
#define _STRING_UTILS_HH_ 

#include <string>
#include <cstring>
#include <cctype>
#include <algorithm>
#include <iterator>

namespace more { 

    namespace string_utils 
    {
        static const char white_space[] = " \f\n\r\t\v";
    };

    // extended getline with multiple-char separator
    //   
    template<typename CharT, typename Traits, typename Alloc>
    inline std::basic_istream<CharT, Traits>&
    getline(std::basic_istream<CharT, Traits>& __in,
        std::basic_string<CharT, Traits, Alloc>& __str, const std::basic_string<CharT, Traits, Alloc>& __delim)
    {
        std::ios_base::iostate __err = std::ios_base::goodbit;

        const std::string::size_type n = __str.max_size();
        char c = __in.rdbuf()->sgetc();
        unsigned int extracted = 0;

        __str.erase();
        while ( extracted < n && c != EOF &&
                __delim.find(c) == std::string::npos ) 
        {
            __str += c;
            ++extracted;
            c = __in.rdbuf()->snextc();
        }

        while ( c != EOF &&
                __delim.find(c) != std::string::npos )
        {
            ++extracted;
            c = __in.rdbuf()->snextc();    
        }            

        if ( c == EOF )
            __err |= std::ios_base::eofbit;

        if (!extracted)
            __err |= std::ios_base::failbit;

        if (__err)
            __in.setstate(__err);

        return __in;
    }

    // trim
    //

    static inline std::string
    trim(const std::string &s, const char *str = string_utils::white_space, int lr = 0)
    {
        std::string::size_type b = lr > 0 ? std::string::npos : s.find_first_not_of(str);
        std::string::size_type e = lr < 0 ? s.size() : s.find_last_not_of(str);
        b = (b == std::string::npos ? 0 : b);
        e = (e == std::string::npos ? 0 : e + 1);
        return s.substr(b,e-b);
    }

    static inline std::string 
    left_trim(const std::string &s, const char *str = string_utils::white_space)
    {
        return trim(s,str,-1);
    }

    static inline std::string
    right_trim(const std::string &s, const char *str = string_utils::white_space)
    {
        return trim(s,str,1);
    }
    
    // in-place trim
    //

    static inline std::string &
    left_trim_(std::string &s, const char *str = string_utils::white_space) // in-place...
    {
        std::string::size_type b = s.find_first_not_of(str);
        s.erase(0,b);
        return s;
    }

    static inline std::string &
    right_trim_(std::string &s, const char *str = string_utils::white_space) // in-place...
    {
        std::string::size_type e = s.find_last_not_of(str);
        s.erase(e+1);
        return s;
    }

    static inline std::string &
    trim_(std::string &s, const char *str = string_utils::white_space) // in-place...
    {
        right_trim_(s,str);
        left_trim_(s,str);
        return s;
    }

    // upcase
    //

    static inline std::string
    upcase(const std::string &s)
    {
        std::string ret;
        std::transform(s.begin(), s.end(), std::back_inserter(ret), ::toupper); 
        return ret;
    }

    static inline std::string &
    upcase_(std::string &s) // in-place...
    {
        std::transform(s.begin(), s.end(), s.begin(), ::toupper); 
        return s;
    }

    // downcase
    //

    static inline std::string
    downcase(const std::string &s)
    {
        std::string ret;
        std::transform(s.begin(), s.end(), std::back_inserter(ret), ::tolower); 
        return ret;
    }

    static inline std::string &
    downcase_(std::string &s) // in-place...
    {
        std::transform(s.begin(), s.end(), s.begin(), ::tolower); 
        return s;
    }

    // swapcase
    //

    struct swapcase_op : public std::unary_function<std::string::value_type, std::string::value_type>
    {
        std::string::value_type
        operator()(std::string::value_type c) const
        { return ::isupper(c) ? ::tolower(c) : ::toupper(c); }
    };

    static inline std::string
    swapcase(const std::string &s)
    {
        std::string ret;
        std::transform(s.begin(), s.end(), std::back_inserter(ret), swapcase_op() ); 
        return ret;
    }
    
    static inline std::string &
    swapcase_(std::string &s) // in-place...
    {
        std::transform(s.begin(), s.end(), s.begin(), swapcase_op() ); 
        return s;
    }

    // casecompare
    //

    static inline bool
    casecmp(const std::string &a, const std::string &b)
    {
        return ::strcasecmp(a.c_str(), b.c_str()) == 0;
    }

    // capitalize
    //
    
    struct capitalize_op : public std::unary_function<std::string::value_type, std::string::value_type>
    {
        capitalize_op()
        : _M_state(0)
        {}

        std::string::value_type
        operator()(std::string::value_type c) const
        {
            if ( ::isspace(c) ) {
                _M_state = 0;
                return c;
            }

            return _M_state++ ? ::tolower(c) : ::toupper(c);
        }
        
        mutable int _M_state;
    };

    static inline std::string
    capitalize(const std::string &s)
    {
        std::string ret;
        std::transform(s.begin(), s.end(), std::back_inserter(ret), capitalize_op() ); 
        return ret;
    }
 
    static inline std::string &
    capitalize_(std::string &s) // in-place...
    {
        std::transform(s.begin(), s.end(), s.begin(), capitalize_op() ); 
        return s;
    }

    // reverse
    // 

    static inline std::string
    reverse(const std::string &s)
    {
        std::string ret;
        std::reverse_copy(s.begin(), s.end(), std::back_inserter(ret));
        return ret;      
    }

    static inline std::string &
    reverse_(std::string &s) // in-place...
    {
        std::reverse(s.begin(), s.end());
        return s;      
    }


} // namespace more

#endif /* _STRING_UTILS_HH_ */
