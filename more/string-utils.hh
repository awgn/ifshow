/* $Id: string-utils.hh 372 2010-01-06 17:50:57Z nicola.bonelli $ */
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

#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <cctype>

#include <algorithm>
#include <iterator>
#include <limits>

namespace more { 

    namespace string_utils 
    {
        static const char white_space[] = " \f\n\r\t\v";

        const bool escape_disabled = false;
        const bool escape_enabled  = true;
    };

    // extended getline with optional escape support and multiple-char separator...
    //

    template<typename CharT, typename Traits, typename Alloc>
    inline std::basic_istream<CharT, Traits>&
    getline(std::basic_istream<CharT, Traits>& __in,
        std::basic_string<CharT, Traits, Alloc>& __str, 
        const std::basic_string<CharT, Traits, Alloc>& __delim, bool do_escape = string_utils::escape_disabled)
    {
        std::ios_base::iostate __err = std::ios_base::goodbit;

        const std::string::size_type n = __str.max_size();
        char c = __in.rdbuf()->sgetc();
        unsigned int extracted = 0;

        bool esc = false;
        __str.erase();        

        if (do_escape) {   
            while ( extracted < n && c != EOF &&
                    (__delim.find(c) == std::string::npos || esc) ) 
            {
                if ( c == '\\' && !esc) {
                    esc = true;
                    c = __in.rdbuf()->snextc();
                    continue;
                }

                if (esc) {
                    esc = false;
                    if (__delim.find(c) == std::string::npos && c !='\\') {
                        __str += '\\'; 
                        ++extracted;
                    }
                }

                __str += c;
                ++extracted;
                c = __in.rdbuf()->snextc();
            }
        } else {
            while ( extracted < n && c != EOF &&
                    __delim.find(c) == std::string::npos ) 
            {
                __str += c;
                ++extracted;
                c = __in.rdbuf()->snextc();
            }
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



    // split a string into a container 
    //

    template <typename Iter>
    void 
    split(const std::string &str, Iter out, const std::string &sep)
    {
        std::stringstream ss(str);
        std::string token;
        while(more::getline(ss, token, sep)) 
            *out ++ = token;
    };

    // join a container of strings into a string
    //

    template <typename Iter>
    inline 
    std::string
    join(Iter it, Iter end, const std::string & sep = std::string())
    {
        if (it == end)
            return std::string();
        std::string ret;
        do {
            ret.append(*it);
        } while( ++it != end && (ret.append(sep), true) );
        return ret;
    }
        
    // replace occurrence of __old with __new string
    //

    static inline const std::string & 
    repl(std::string &orig, const std::string &__old,
          const std::string &__new, unsigned int m = std::numeric_limits<unsigned int>::max())
    {
        unsigned int n = 0;
        std::string::size_type __osize = __old.size(), __nsize = __new.size(), __start = 0;

        for(std::string::size_type p; n < m && (p=orig.find(__old, __start)) != std::string::npos; ++n)
        {
            orig.replace(p, __osize, __new);
            __start = p + __nsize;
        }

        return orig;
    }

    static inline std::string
    repl_copy(std::string str, const std::string &__old,
         const std::string &__new, unsigned int n = std::numeric_limits<unsigned int>::max())
    {
        repl(str, __old, __new, n);
        return str;
    }

    // trim
    //

    static inline std::string
    trim_copy(const std::string &s, const char *str = string_utils::white_space, int lr = 0)
    {
        std::string::size_type b = lr > 0 ? std::string::npos : s.find_first_not_of(str);
        std::string::size_type e = lr < 0 ? s.size() : s.find_last_not_of(str);
        b = (b == std::string::npos ? 0 : b);
        e = (e == std::string::npos ? 0 : e + 1);
        return s.substr(b,e-b);
    }

    static inline std::string 
    left_trim_copy(const std::string &s, const char *str = string_utils::white_space)
    {
        return trim_copy(s,str,-1);
    }

    static inline std::string
    right_trim_copy(const std::string &s, const char *str = string_utils::white_space)
    {
        return trim_copy(s,str,1);
    }
    
    // in-place trim
    //

    static inline const std::string & 
    left_trim(std::string &s, const char *str = string_utils::white_space) // in-place...
    {
        std::string::size_type b = s.find_first_not_of(str);
        s.erase(0,b);
        return s;
    }

    static inline const std::string & 
    right_trim(std::string &s, const char *str = string_utils::white_space) // in-place...
    {
        std::string::size_type e = s.find_last_not_of(str);
        s.erase(e+1);
        return s;
    }

    static inline const std::string & 
    trim(std::string &s, const char *str = string_utils::white_space) // in-place...
    {
        right_trim(s,str);
        left_trim(s,str);
        return s;
    }

    // upcase
    //

    static inline std::string
    upcase_copy(const std::string &s)
    {
        std::string ret;
        std::transform(s.begin(), s.end(), std::back_inserter(ret), ::toupper); 
        return ret;
    }

    static inline const std::string & 
    upcase(std::string &s) // in-place...
    {
        std::transform(s.begin(), s.end(), s.begin(), ::toupper); 
        return s;
    }

    // downcase
    //

    static inline std::string
    downcase_copy(const std::string &s)
    {
        std::string ret;
        std::transform(s.begin(), s.end(), std::back_inserter(ret), ::tolower); 
        return ret;
    }

    static inline const std::string &
    downcase(std::string &s) // in-place...
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
    swapcase_copy(const std::string &s)
    {
        std::string ret;
        std::transform(s.begin(), s.end(), std::back_inserter(ret), swapcase_op()); 
        return ret;
    }
    
    static inline const std::string & 
    swapcase(std::string &s) // in-place...
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
    capitalize_copy(const std::string &s)
    {
        std::string ret;
        std::transform(s.begin(), s.end(), std::back_inserter(ret), capitalize_op() ); 
        return ret;
    }
 
    static inline const std::string & 
    capitalize(std::string &s) // in-place...
    {
        std::transform(s.begin(), s.end(), s.begin(), capitalize_op() ); 
        return s;
    }

    // reverse
    // 

    static inline std::string
    reverse_copy(const std::string &s)
    {
        std::string ret;
        std::reverse_copy(s.begin(), s.end(), std::back_inserter(ret));
        return ret;      
    }

    static inline const std::string & 
    reverse(std::string &s) // in-place...
    {
        std::reverse(s.begin(), s.end());
        return s;
    }


} // namespace more

#endif /* _STRING_UTILS_HH_ */
