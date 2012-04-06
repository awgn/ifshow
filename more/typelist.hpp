/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef _TYPELIST_HPP_
#define _TYPELIST_HPP_ 

// a c++0x typelist implementation...
//

#include <type_traits>   

namespace more { namespace type { 

    // typelist forward declaration...
    //
    template <typename ...Tp> struct typelist {};

    // length<>::value            
    //
    template <typename Tl> struct length;
    template <typename ...Ti>
    struct length<typelist<Ti...>>
    {
        enum { value = sizeof...(Ti) };
    };

    // at<>::type
    //
    template <typename T, std::size_t N> struct at;
    template <typename T, typename ...Ti, std::size_t N>
    struct at<typelist<T, Ti...>, N>
    {
        typedef typename at<typelist<Ti...>, N-1>::type type;
    };
    template <typename T, typename ...Ti>
    struct at<typelist<T, Ti...>, 0>
    {
        typedef T type;
    };
    template <typename T>
    struct at<typelist<T>, 0>
    {
        typedef T type;
    };
    
    // append<>::type
    //
    template <typename Tl, typename Tp> struct append;
    template <typename Tp, typename ...Ti>
    struct append<typelist<Ti...>, Tp>
    {
        typedef typelist<Ti...,Tp> type;
    };

    // insert<>::type
    //
    template <typename Tl, typename Tp> struct insert;
    template <typename Tp, typename ...Ti>
    struct insert<typelist<Ti...>, Tp>
    {
        typedef typelist<Tp, Ti...> type;
    };
 
    // index_of<>::value
    //
    template <typename Tl, typename T> struct indexof;

    template <typename T, typename T0, typename ...Ti>
    struct indexof<typelist<T0, Ti...>, T>
    {
        enum { value = indexof<typelist<Ti...>, T>::value == -1 ? -1 : 1 + indexof< typelist<Ti...>, T>::value };
    };
    template <typename T, typename ...Ti>
    struct indexof<typelist<T, Ti...>, T>
    {
        enum { value = 0 };
    };
    template <typename T0, typename T>
    struct indexof<typelist<T0>, T>
    {
        enum { value = -1 };
    };
    template <typename T>
    struct indexof<typelist<T>, T>
    {
        enum { value = 0 };
    };
    template <typename T>
    struct indexof<typelist<>, T>
    {
        enum { value = -1 };
    };

    // is_same<>::value (also std::is_same works)
    //
    template <typename T1, typename T2> struct is_same;

    template <typename ...T1, typename ...T2>
    struct is_same<typelist<T1...>, typelist<T2...>>
    {
        enum { value = false }; 
    };

    template <typename ...Ti>
    struct is_same<typelist<Ti...>, typelist<Ti...>>
    {
        enum { value = true }; 
    };

    // for_each (unary metafunction)
    //
    template <typename Tl, template <typename> class F > struct for_each;
    template <typename ...Ti, template <typename> class F>
    struct for_each<typelist<Ti...>, F >
    {
        typedef typelist< typename F<Ti>::type ... > type;
    };

}; // namespace type
}; // namespace more

#endif /* _TYPELIST_HPP_ */
