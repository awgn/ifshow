/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _MORE_STRING_UTILS_HPP_
#define _MORE_STRING_UTILS_HPP_ 

#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <iterator>
#include <limits>

namespace more { 

    namespace string_utils 
    {
        struct white_space 
        {
            static const char *
            value(char)
            {
                return " \f\n\r\t\v";
            }

            static const wchar_t *
            value(wchar_t)
            {
                return  L" \f\n\r\t\v";
            }
        };

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
        
        const typename std::basic_string<CharT, Traits, Alloc>::size_type n = __str.max_size();
        CharT c = __in.rdbuf()->sgetc();
        unsigned int extracted = 0;

        bool esc = false;
        __str.erase();        

        if (do_escape) {   
            while ( extracted < n && !Traits::eq(c,Traits::eof()) &&
                    (__delim.find(c) == std::basic_string<CharT,Traits,Alloc>::npos || esc) ) 
            {
                if ( Traits::eq(c, '\\') && !esc) {
                    esc = true;
                    c = __in.rdbuf()->snextc();
                    continue;
                }

                if (esc) {
                    esc = false;
                    if (__delim.find(c) == std::basic_string<CharT,Traits,Alloc>::npos && !Traits::eq(c,'\\') ) {
                        __str += '\\'; 
                        ++extracted;
                    }
                }

                __str += c;
                ++extracted;
                c = __in.rdbuf()->snextc();
            }
        } else {
            while ( extracted < n && !Traits::eq(c,Traits::eof()) &&
                    __delim.find(c) == std::basic_string<CharT,Traits,Alloc>::npos ) 
            {
                __str += c;
                ++extracted;
                c = __in.rdbuf()->snextc();
            }
        }

        while ( !Traits::eq(c, Traits::eof()) &&
                __delim.find(c) != std::basic_string<CharT,Traits,Alloc>::npos )
        {
            ++extracted;
            c = __in.rdbuf()->snextc();    
        }            

        if ( Traits::eq(c,Traits::eof()) )
            __err |= std::ios_base::eofbit;

        if (!extracted)
            __err |= std::ios_base::failbit;

        if (__err)
            __in.setstate(__err);

        return __in;
    }

    // split a string into a container 
    //

    template<typename Iter, typename CharT, typename Traits, typename Alloc>
    inline void 
    __split(const std::basic_string<CharT,Traits,Alloc> &str, Iter out, const std::basic_string<CharT,Traits,Alloc> &sep, char)
    {
        std::stringstream ss(str);
        std::basic_string<CharT,Traits,Alloc> token;
        while(more::getline(ss, token, sep)) 
            *out ++ = token;
    };
    template<typename Iter, typename CharT, typename Traits, typename Alloc>
    inline void 
    __split(const std::basic_string<CharT,Traits,Alloc> &str, Iter out, const std::basic_string<CharT,Traits,Alloc> &sep, wchar_t)
    {
        std::wstringstream ss(str);
        std::basic_string<CharT,Traits,Alloc> token;
        while(more::getline(ss, token, sep)) 
            *out ++ = token;
    };

    template<typename Iter, typename CharT, typename Traits, typename Alloc>
    inline void 
    split(const std::basic_string<CharT,Traits,Alloc> &str, Iter out, const std::basic_string<CharT,Traits,Alloc> &sep)
    {
        __split(str,out,sep,CharT());
    };

    // join a container of strings into a string
    //

    template<typename Iter, typename CharT, typename Traits, typename Alloc>
    inline std::basic_string<CharT,Traits,Alloc>
    join(Iter it, Iter end, const typename std::basic_string<CharT,Traits,Alloc> & sep)
    {
        if (it == end)
            return typename std::basic_string<CharT,Traits,Alloc>();
        typename std::basic_string<CharT,Traits,Alloc> ret;
        do {
            ret.append(*it);
        } while( ++it != end && (ret.append(sep), true) );
        return ret;
    }

    // replace occurrence of __old with __new string
    //

    template<typename CharT, typename Traits, typename Alloc>
    inline const std::basic_string<CharT,Traits,Alloc> & 
    repl(std::basic_string<CharT,Traits,Alloc> &orig, const std::basic_string<CharT,Traits,Alloc> &__old,
         const std::basic_string<CharT,Traits,Alloc> &__new, unsigned int m = std::numeric_limits<unsigned int>::max())
    {
        unsigned int n = 0;
        typename std::basic_string<CharT,Traits,Alloc>::size_type __osize = __old.size(), __nsize = __new.size(), __start = 0;

        for(typename std::basic_string<CharT,Traits,Alloc>::size_type p; n < m 
            && (p=orig.find(__old, __start)) != std::basic_string<CharT,Traits,Alloc>::npos; ++n)
        {
            orig.replace(p, __osize, __new);
            __start = p + __nsize;
        }

        return orig;
    }

    template<typename CharT, typename Traits, typename Alloc>
    inline std::basic_string<CharT,Traits,Alloc>
    repl_copy(std::basic_string<CharT,Traits,Alloc> str, const std::basic_string<CharT,Traits,Alloc> &__old,
              const std::basic_string<CharT,Traits,Alloc> &__new, unsigned int n = std::numeric_limits<unsigned int>::max())
    {
        repl(str, __old, __new, n);
        return str;
    }

    // trim
    //

    template<typename CharT, typename Traits, typename Alloc>
    inline std::basic_string<CharT,Traits,Alloc>
    trim_copy(const std::basic_string<CharT,Traits,Alloc> &s, const CharT *str = string_utils::white_space::value(CharT()), int lr = 0)
    {
        typename std::basic_string<CharT,Traits,Alloc>::size_type b = lr > 0 ? 
        std::basic_string<CharT,Traits,Alloc>::npos : s.find_first_not_of(str);
        typename std::basic_string<CharT,Traits,Alloc>::size_type e = lr < 0 ? 
        s.size() : s.find_last_not_of(str);
        b = (b == std::basic_string<CharT,Traits,Alloc>::npos ? 0 : b);
        e = (e == std::basic_string<CharT,Traits,Alloc>::npos ? 0 : e + 1);
        return s.substr(b,e-b);
    }

    template<typename CharT, typename Traits, typename Alloc>
    inline std::basic_string<CharT,Traits,Alloc> 
    left_trim_copy(const std::basic_string<CharT,Traits,Alloc> &s, const CharT *str = string_utils::white_space::value(CharT()))
    {
        return trim_copy(s,str,-1);
    }

    template<typename CharT, typename Traits, typename Alloc>
    inline std::basic_string<CharT,Traits,Alloc>
    right_trim_copy(const std::basic_string<CharT,Traits,Alloc> &s, const CharT *str = string_utils::white_space::value(CharT()))
    {
        return trim_copy(s,str,1);
    }

    // in-place trim
    //

    template<typename CharT, typename Traits, typename Alloc>
    inline const std::basic_string<CharT,Traits,Alloc> & 
    left_trim(typename std::basic_string<CharT,Traits,Alloc> &s, const CharT *str = string_utils::white_space::value(CharT())) // in-place...
    {
        typename std::basic_string<CharT,Traits,Alloc>::size_type b = s.find_first_not_of(str);
        s.erase(0,b);
        return s;
    }

    template<typename CharT, typename Traits, typename Alloc>
    inline const std::basic_string<CharT,Traits,Alloc> & 
    right_trim(typename std::basic_string<CharT,Traits,Alloc> &s, const CharT *str = string_utils::white_space::value(CharT())) // in-place...
    {
        typename std::basic_string<CharT,Traits,Alloc>::size_type e = s.find_last_not_of(str);
        s.erase(e+1);
        return s;
    }

    template<typename CharT, typename Traits, typename Alloc>
    inline const std::basic_string<CharT,Traits,Alloc> & 
    trim(typename std::basic_string<CharT,Traits,Alloc> &s, const CharT *str = string_utils::white_space::value(CharT())) // in-place...
    {
        right_trim(s,str);
        left_trim(s,str);
        return s;
    }

    // upcase
    //

    template<typename CharT, typename Traits, typename Alloc>
    inline std::basic_string<CharT,Traits,Alloc>
    upcase_copy(const std::basic_string<CharT,Traits,Alloc> &s)
    {
        typename std::basic_string<CharT,Traits,Alloc> ret;
        std::transform(s.begin(), s.end(), std::back_inserter(ret), ::toupper); 
        return ret;
    }

    template<typename CharT, typename Traits, typename Alloc>
    inline const std::basic_string<CharT,Traits,Alloc> & 
    upcase(std::basic_string<CharT,Traits,Alloc> &s) // in-place...
    {
        std::transform(s.begin(), s.end(), s.begin(), ::toupper); 
        return s;
    }

    // downcase
    //

    template<typename CharT, typename Traits, typename Alloc>
    inline std::basic_string<CharT,Traits,Alloc>
    downcase_copy(const std::basic_string<CharT,Traits,Alloc> &s)
    {
        typename std::basic_string<CharT,Traits,Alloc> ret;
        std::transform(s.begin(), s.end(), std::back_inserter(ret), ::tolower); 
        return ret;
    }

    template<typename CharT, typename Traits, typename Alloc>
    inline const std::basic_string<CharT,Traits,Alloc> &
    downcase(std::basic_string<CharT,Traits,Alloc> &s) // in-place...
    {
        std::transform(s.begin(), s.end(), s.begin(), ::tolower); 
        return s;
    }

    // swapcase
    //

    template<typename CharT, typename Traits, typename Alloc>
    struct swapcase_op : public std::unary_function< typename std::basic_string<CharT,Traits,Alloc>::value_type, 
                       typename std::basic_string<CharT,Traits,Alloc>::value_type>
    {
        typename std::basic_string<CharT,Traits,Alloc>::value_type
        operator()(typename std::basic_string<CharT,Traits,Alloc>::value_type c) const
        { 
            return ::isupper(c) ? ::tolower(c) : ::toupper(c); 
        }
    };

    template<typename CharT, typename Traits, typename Alloc>
    inline std::basic_string<CharT,Traits,Alloc>
    swapcase_copy(const std::basic_string<CharT,Traits,Alloc> &s)
    {
        std::basic_string<CharT,Traits,Alloc> ret;
        std::transform(s.begin(), s.end(), std::back_inserter(ret), swapcase_op<CharT,Traits,Alloc>()); 
        return ret;
    }

    template<typename CharT, typename Traits, typename Alloc>
    inline const std::basic_string<CharT,Traits,Alloc> & 
    swapcase(std::basic_string<CharT,Traits,Alloc> &s) // in-place...
    {
        std::transform(s.begin(), s.end(), s.begin(), swapcase_op<CharT,Traits,Alloc>()); 
        return s;
    }

    // casecompare
    //

    template<typename CharT, typename Traits, typename Alloc>
    struct case_equal_op : public std::binary_function< 
    typename std::basic_string<CharT,Traits,Alloc>::value_type, 
             typename std::basic_string<CharT,Traits,Alloc>::value_type, bool>
    {
        bool operator()(typename std::basic_string<CharT,Traits,Alloc>::value_type a, 
                        typename std::basic_string<CharT,Traits,Alloc>::value_type b) const
        {
            return ::toupper(a) == ::toupper(b);
        }
    };

    template<typename CharT, typename Traits, typename Alloc>
    inline bool
    casecmp(const std::basic_string<CharT,Traits,Alloc> &a, const std::basic_string<CharT,Traits,Alloc> &b)
    {
        return std::equal(a.begin(), a.end(), b.begin(), case_equal_op<CharT,Traits,Alloc>());
    }

    // capitalize
    //

    template<typename CharT, typename Traits, typename Alloc>
    struct capitalize_op : public std::unary_function<typename std::basic_string<CharT,Traits,Alloc>::value_type, 
                         typename std::basic_string<CharT,Traits,Alloc>::value_type>
    {
        capitalize_op()
        : m_state(0)
        {}

        typename std::basic_string<CharT,Traits,Alloc>::value_type
        operator()(typename std::basic_string<CharT,Traits,Alloc>::value_type c) const
        {
            if ( ::isspace(c) ) {
                m_state = 0;
                return c;
            }
            return m_state++ ? ::tolower(c) : ::toupper(c);
        }

        mutable int m_state;
    };

    template<typename CharT, typename Traits, typename Alloc>
    inline typename std::basic_string<CharT,Traits,Alloc>
    capitalize_copy(const std::basic_string<CharT,Traits,Alloc> &s)
    {
        std::basic_string<CharT,Traits,Alloc> ret;
        std::transform(s.begin(), s.end(), std::back_inserter(ret), capitalize_op<CharT,Traits,Alloc>() ); 
        return ret;
    }

    template<typename CharT, typename Traits, typename Alloc>
    inline const typename std::basic_string<CharT,Traits,Alloc> & 
    capitalize(std::basic_string<CharT,Traits,Alloc> &s) // in-place...
    {
        std::transform(s.begin(), s.end(), s.begin(), capitalize_op<CharT,Traits,Alloc>() ); 
        return s;
    }

    // reverse
    // 

    template<typename CharT, typename Traits, typename Alloc>
    inline std::basic_string<CharT,Traits,Alloc>
    reverse_copy(const std::basic_string<CharT,Traits,Alloc> &s)
    {
        std::basic_string<CharT,Traits,Alloc> ret;
        std::reverse_copy(s.begin(), s.end(), std::back_inserter(ret));
        return ret;      
    }

    template<typename CharT, typename Traits, typename Alloc>
    inline const std::basic_string<CharT, Traits, Alloc> & 
    reverse(std::basic_string<CharT,Traits,Alloc> &s) // in-place...
    {
        std::reverse(s.begin(), s.end());
        return s;
    }

} // namespace more

#endif /* _STRING_UTILS_HPP_ */
