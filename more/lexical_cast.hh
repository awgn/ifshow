/* $Id: lexical_cast.hh 45 2009-02-14 10:20:17Z nicola.bonelli $ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _LEXICAL_CAST_HH_
#define _LEXICAL_CAST_HH_ 

#include <stdexcept>
#include <typeinfo>
#include <sstream>
#include <cassert>

/////////////////////////////////////////////////////////////
// lexical_cast ala boost, inspired to that of Kevlin Henney

namespace more { 

    class bad_lexical_cast : public std::bad_cast
    {
        public:
            bad_lexical_cast()
            {}

            virtual const char *what() const throw()
            {
                return "bad lexical cast";
            }
            
            virtual ~bad_lexical_cast() throw()
            {}
    };

    template <typename Dst, typename Src>
    Dst lexical_cast(const Src &arg)
    {
        std::stringstream i;
        Dst ret;

        if( !(i << arg) || !(i>>ret) || !(i>> std::ws).eof()) {
#ifdef __EXCEPTIONS
            throw bad_lexical_cast();
#else
            assert(!"bad lexical cast");            
#endif
        }
        return ret;
    }  

} // namespace more

#endif /* _LEXICAL_CAST_HH_ */
