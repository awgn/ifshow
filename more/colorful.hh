/* $Id: colorful.hh 454 2010-02-12 22:06:55Z nicola.bonelli $ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef _COLORFUL_HH_
#define _COLORFUL_HH_ 

#include <typelist.hh>
#include <ostream>

namespace more { 

    namespace ecma
    {
        template <int n> struct integral_value { enum { attribute_value = n }; };
        typedef integral_value<0> reset;
        typedef integral_value<1> bold;
        typedef integral_value<2> half_bright;
        typedef integral_value<4> underline;
        typedef integral_value<5> blink;
        typedef integral_value<7> reverse;
        typedef integral_value<8> hidden;

        typedef integral_value<30> fg_black; 
        typedef integral_value<31> fg_red; 
        typedef integral_value<32> fg_green; 
        typedef integral_value<33> fg_yellow; 
        typedef integral_value<34> fg_blue; 
        typedef integral_value<35> fg_magenta; 
        typedef integral_value<36> fg_cyan; 
        typedef integral_value<37> fg_light_grey; 

        typedef integral_value<40> bg_black; 
        typedef integral_value<41> bg_red; 
        typedef integral_value<42> bg_green; 
        typedef integral_value<43> bg_yellow; 
        typedef integral_value<44> bg_blue; 
        typedef integral_value<45> bg_magenta; 
        typedef integral_value<46> bg_cyan; 
        typedef integral_value<47> bg_light_grey; 
    };

    template <typename T>
    struct colorful 
    {
        typedef T value_type;
    };


    template <typename T>
    struct ecma_parameter
    {
        typedef T value_type; 
    };

    
    template <typename CharT, typename Traits>
    inline std::basic_ostream<CharT, Traits> & 
    operator<<(std::basic_ostream<CharT, Traits> &out, ecma_parameter<more::TL::null>)
    {
        return out;
    }

    template <typename CharT, typename Traits, typename T>
    inline std::basic_ostream<CharT, Traits> & 
    operator<<(std::basic_ostream<CharT, Traits> &out, ecma_parameter<T>)
    {
        return out << ";" << T::head::attribute_value << ecma_parameter<typename T::tail>();
    }

    template <typename CharT, typename Traits, typename T>
    inline std::basic_ostream<CharT, Traits> & 
    operator<<(std::basic_ostream<CharT, Traits> &out, colorful<T>)
    {
        return out << "\E[" << ecma_parameter<T>() << "m";
    }

} // namespace more

 
#endif /* _COLORFUL_HH_ */
