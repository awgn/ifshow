/* $Id: string-utils.hh 357 2009-12-31 14:52:25Z nicola.bonelli $ */
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

    // trim
    //

    static inline std::string
    trim(const std::string &s, const char *str = " \t\n\r", int lr = 0)
    {
        std::string::size_type b = lr > 0 ? std::string::npos : s.find_first_not_of(str);
        std::string::size_type e = lr < 0 ? s.size() : s.find_last_not_of(str);
        b = (b == std::string::npos ? 0 : b);
        e = (e == std::string::npos ? 0 : e + 1);
        return s.substr(b,e-b);
    }

    static inline std::string 
    left_trim(const std::string &s, const char *str = " \t\n\r")
    {
        return trim(s,str,-1);
    }

    static inline std::string
    right_trim(const std::string &s, const char *str = " \t\n\r")
    {
        return trim(s,str,1);
    }
    
    // in-place trim
    //

    static inline std::string &
    left_trim_(std::string &s, const char *str = " \t\n\r") // in-place...
    {
        std::string::size_type b = s.find_first_not_of(str);
        s.erase(0,b);
        return s;
    }

    static inline std::string &
    right_trim_(std::string &s, const char *str = " \t\n\r") // in-place...
    {
        std::string::size_type e = s.find_last_not_of(str);
        s.erase(e+1);
        return s;
    }

    static inline std::string &
    trim_(std::string &s, const char *str = " \t\n\r") // in-place...
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
