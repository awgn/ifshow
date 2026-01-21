/* $Id$ */
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
#include <string>
#include <type_traits>

/////////////////////////////////////////////////////////////
// lexical_cast ala boost, inspired to that of Kevlin Henney

namespace more { 

    class bad_lexical_cast : public std::bad_cast
    {
        public:
            bad_lexical_cast()
            {}

            virtual ~bad_lexical_cast() throw()
            {}
            
            virtual const char *what() const throw()
            {
                return "bad lexical cast";
            }
    };

    /////////////////////////////////////////// policy cast

    namespace detail {

        template <typename Target, typename Source>
        struct generic_lexical_cast_policy
        {
            static 
            Target 
            apply(const Source &arg)
            {
                Target ret;
                static __thread std::stringstream * ss;
                if (!ss) {
                    ss = new std::stringstream;
                }

                ss->clear();
                if(!( *ss << arg &&  *ss >> ret && (*ss >> std::ws).eof() )) 
                {
                    ss->str(std::string());
                    throw bad_lexical_cast();
                }
                return ret;
            }
        };

        template <typename Target, typename Source>
        struct convertible_lexical_cast_policy
        {
            static 
            Target apply(const Source &arg)
            {
                return arg;
            };
        };
        
        template <typename T>
        struct null_lexical_cast_policy
        {
            static
            const T &apply(const T &arg)
            {
                return arg;
            }
        };
        
        /////////////////////////////////////////// lexical_traits

        template <typename Target, typename Source, typename Enable = void> 
        struct lexical_traits;

        template <typename Target, typename Source> 
        struct lexical_traits<Target, Source, typename std::enable_if<!std::is_convertible<Source,Target>::value>::type> 
        {
            typedef generic_lexical_cast_policy<Target,Source> policy; 
        };

        template <typename Target, typename Source> 
        struct lexical_traits<Target, Source, typename std::enable_if<std::is_convertible<Source,Target>::value && 
                                                                     !std::is_same<Source,Target>::value>::type> 
        {
            typedef convertible_lexical_cast_policy<Target,Source> policy; 
        };
 
        template <typename T> 
        struct lexical_traits<T,T,void>
        {
            typedef null_lexical_cast_policy<T> policy; 
        };
    }

    template <typename Target, typename Source> 
    typename
    std::remove_reference<Target>::type lexical_cast(const Source &arg)
    {
        return detail::lexical_traits<typename std::remove_reference<Target>::type,Source>::policy::apply(arg);
    }  

} // namespace more

#endif /* _LEXICAL_CAST_HH_ */
