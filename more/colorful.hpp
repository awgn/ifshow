/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef _COLORFUL_HPP_
#define _COLORFUL_HPP_ 

#include <ostream>
#include <type_traits>

namespace more { 

    namespace ecma
    {
        typedef std::integral_constant<int,0> reset;
        typedef std::integral_constant<int,1> bold;
        typedef std::integral_constant<int,2> half_bright;
        typedef std::integral_constant<int,4> underline;
        typedef std::integral_constant<int,5> blink;
        typedef std::integral_constant<int,7> reverse;
        typedef std::integral_constant<int,8> hidden;
    
        namespace fg 
        {
            typedef std::integral_constant<int,30> black; 
            typedef std::integral_constant<int,31> red; 
            typedef std::integral_constant<int,32> green; 
            typedef std::integral_constant<int,33> yellow; 
            typedef std::integral_constant<int,34> blue; 
            typedef std::integral_constant<int,35> magenta; 
            typedef std::integral_constant<int,36> cyan; 
            typedef std::integral_constant<int,37> light_grey; 
        }

        namespace bg
        {
            typedef std::integral_constant<int,40> black; 
            typedef std::integral_constant<int,41> red; 
            typedef std::integral_constant<int,42> green; 
            typedef std::integral_constant<int,43> yellow; 
            typedef std::integral_constant<int,44> blue; 
            typedef std::integral_constant<int,45> magenta; 
            typedef std::integral_constant<int,46> cyan; 
            typedef std::integral_constant<int,47> light_grey; 
        }
    };

    template <typename ...Ti> struct colorful {};
    template <typename ...Ti> struct ecma_param {};

    template <typename CharT, typename Traits>
    inline std::basic_ostream<CharT, Traits> & 
    operator<<(std::basic_ostream<CharT, Traits> &out, ecma_param<>)
    {
        return out;
    }

    template <typename CharT, typename Traits, typename T0, typename ...Ti>
    inline std::basic_ostream<CharT, Traits> & 
    operator<<(std::basic_ostream<CharT, Traits> &out, ecma_param<T0, Ti...>)
    {
        return out << ";" << T0::value << ecma_param<Ti...>();
    }

    template <typename CharT, typename Traits, typename ...Ti>
    inline std::basic_ostream<CharT, Traits> & 
    operator<<(std::basic_ostream<CharT, Traits> &out, colorful<Ti...>)
    {
        return out << "\E[" << ecma_param<Ti...>() << "m";
    }


} // namespace more

 
#endif /* _MORE_COLORFUL_HPP_ */
