/* $Id: iomanip.hh 373 2010-01-07 09:07:08Z nicola.bonelli $ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef _IOMANIP_HH_
#define _IOMANIP_HH_ 

#include <iostream>
#include <limits>

#include <string-utils.hh>  // more

namespace more { 

    // Jusuttis The C++ Standard Library: User-defined manipulators (charter 13)
    //

    template <class charT, class Traits>
    inline
    std::basic_istream<charT,Traits> &
    ignore_line(std::basic_istream<charT,Traits> &__in)
    {
        __in.ignore(std::numeric_limits<std::streamsize>::max(), __in.widen('\n'));
        return __in;
    }

    struct _Spaces { int _M_n; };

    static inline _Spaces
    spaces(int __n)
    {
        _Spaces __x;
        __x._M_n = __n;
        return __x;
    }

    template <typename charT, typename Traits>
    inline std::basic_ostream<charT,Traits> &
    operator<<(std::basic_ostream<charT,Traits> &__out, _Spaces __s)
    {
        for(int i=0; i < __s._M_n; i++)
            __out.put(__out.widen(' '));
        return __out;
    }

    // token_string input manip
    //

    class token_string
    {
    public:
        token_string(const std::string &delim, bool esc = string_utils::escape_disabled)
        : _M_delim(delim),
          _M_value(),
          _M_esc(esc)
        {}

        ~token_string()
        {}

        friend
        std::istream & operator>>(std::istream &__in, token_string &rhs)
        {
            std::string & __str = rhs;
            more::getline(__in, __str, rhs._M_delim, rhs._M_esc);
            return __in;
        }

        operator const std::string &() const
        {
            return _M_value;
        }

        operator std::string &() 
        {
            return _M_value;
        }

        const std::string &
        str() const
        {
            return _M_value;
        }

    protected:
        const std::string _M_delim;
        std::string _M_value;
        bool _M_esc;
    };

    // token_string with escape support enabled
    //

    struct token_line : public token_string
    {
        token_line()
        : token_string("\n", string_utils::escape_enabled)
        {}

        friend
        std::istream & operator>>(std::istream &__in, token_line &rhs)
        {
            std::string & __str = rhs;
            more::getline(__in,__str, rhs._M_delim, rhs._M_esc);
            return __in;
        }
    };

} // namespace more

#endif /* _IOMANIP_HH_ */
