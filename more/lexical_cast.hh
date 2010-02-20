/* $Id: lexical_cast.hh 461 2010-02-19 09:46:07Z nicola.bonelli $ */
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

    template <typename Target, typename Source>
    struct lexical_cast_policy
    {
        static std::stringstream &
        _S_ss()
        {
            static __thread std::stringstream * ret;
            if (!ret) 
                ret = new std::stringstream;
            return *ret;
        }

        static 
        Target apply(const Source &arg)
        {
            Target ret;
            _S_ss().clear();

            if(!( _S_ss() << arg &&  _S_ss() >> ret && (_S_ss() >> std::ws).eof() )) 
            {
                _S_ss().str(std::string());
                throw bad_lexical_cast();
            }
            return ret;
        }
    };

    // template <typename Target, typename Source>
    // std::stringstream lexical_cast_policy<Target,Source>::_S_ss;

    // null cast optimization...
    //
    template <typename T>
    struct lexical_cast_policy<T,T>
    {
        static 
        const T & apply(const T &arg)
        {
            return arg;
        }
    };

    template <typename Target, typename Source> 
    Target lexical_cast(const Source &arg)
    {
        return lexical_cast_policy<Target,Source>::apply(arg);
    }  

} // namespace more

#endif /* _LEXICAL_CAST_HH_ */
