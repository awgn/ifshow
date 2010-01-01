/* $Id: iomanip.hh 358 2010-01-01 12:54:55Z nicola.bonelli $ */
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
            __out.put(' ');
        return __out;
    }

} // namespace more

#endif /* _IOMANIP_HH_ */
