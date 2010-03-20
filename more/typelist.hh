/* $Id: typelist.hh 477 2010-03-20 12:09:41Z nicola.bonelli $ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef TYPELIST_HH
#define TYPELIST_HH

#ifndef __GXX_EXPERIMENTAL_CXX0X__
#include <tr1/type_traits>
namespace std { using namespace std::tr1; }
#else
#include <type_traits>
#endif

/* the so-called __VA_NARG__ (PP_NARG) macro from the thread at 
   http://groups.google.com/group/comp.std.c/browse_frm/thread/77ee8c8f92e4a3fb 
 */

#ifndef PP_NARG
#define PP_NARG(...) \
         PP_NARG_(__VA_ARGS__,PP_RSEQ_N())
#define PP_NARG_(...) \
         PP_ARG_N(__VA_ARGS__)
#define PP_ARG_N( \
          _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
         _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
         _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
         _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
         _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
         _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
         _61,_62,_63,N,...) N
#define PP_RSEQ_N() \
         63,62,61,60,                   \
         59,58,57,56,55,54,53,52,51,50, \
         49,48,47,46,45,44,43,42,41,40, \
         39,38,37,36,35,34,33,32,31,30, \
         29,28,27,26,25,24,23,22,21,20, \
         19,18,17,16,15,14,13,12,11,10, \
         9,8,7,6,5,4,3,2,1,0 
#endif /* PP_NARG */

#ifndef PASTE
#define PASTE(a,b)  a ## b
#define XPASTE(a,b) PASTE(a,b)
#endif /* PASTE */

// typelist macro helper ala loki...
//

#ifndef TYPELIST
#define TYPELIST_1(a)                  more::TL::typelist<a,more::TL::null>
#define TYPELIST_2(a,...)              more::TL::typelist<a,TYPELIST_1(__VA_ARGS__) >
#define TYPELIST_3(a,...)              more::TL::typelist<a,TYPELIST_2(__VA_ARGS__) >
#define TYPELIST_4(a,...)              more::TL::typelist<a,TYPELIST_3(__VA_ARGS__) >
#define TYPELIST_5(a,...)              more::TL::typelist<a,TYPELIST_4(__VA_ARGS__) >
#define TYPELIST_6(a,...)              more::TL::typelist<a,TYPELIST_5(__VA_ARGS__) >
#define TYPELIST_7(a,...)              more::TL::typelist<a,TYPELIST_6(__VA_ARGS__) >
#define TYPELIST_8(a,...)              more::TL::typelist<a,TYPELIST_7(__VA_ARGS__) >
#define TYPELIST_9(a,...)              more::TL::typelist<a,TYPELIST_8(__VA_ARGS__) >
#define TYPELIST_10(a,...)             more::TL::typelist<a,TYPELIST_9(__VA_ARGS__) >
#define TYPELIST_11(a,...)             more::TL::typelist<a,TYPELIST_10(__VA_ARGS__) >
#define TYPELIST_12(a,...)             more::TL::typelist<a,TYPELIST_11(__VA_ARGS__) >
#define TYPELIST_13(a,...)             more::TL::typelist<a,TYPELIST_12(__VA_ARGS__) >
#define TYPELIST_14(a,...)             more::TL::typelist<a,TYPELIST_13(__VA_ARGS__) >
#define TYPELIST_15(a,...)             more::TL::typelist<a,TYPELIST_14(__VA_ARGS__) >
#define TYPELIST_16(a,...)             more::TL::typelist<a,TYPELIST_15(__VA_ARGS__) >
#define TYPELIST_17(a,...)             more::TL::typelist<a,TYPELIST_16(__VA_ARGS__) >
#define TYPELIST_18(a,...)             more::TL::typelist<a,TYPELIST_17(__VA_ARGS__) >
#define TYPELIST_19(a,...)             more::TL::typelist<a,TYPELIST_18(__VA_ARGS__) >
#define TYPELIST_20(a,...)             more::TL::typelist<a,TYPELIST_19(__VA_ARGS__) >
#define TYPELIST(...)                  XPASTE(TYPELIST_ ,PP_NARG(__VA_ARGS__)) ( __VA_ARGS__) 
#endif /* TYPELIST */

namespace more { namespace TL {

    class null {};
    struct empty {};

    // the typelist element
    template <typename T, typename U>
    struct typelist {
        typedef T head;
        typedef U tail;
    };

    // length<TLIST>::value
    //
    template <class TLIST> struct length;
    template <>
    struct length<null>
    {
        enum { value = 0 };
    };
    template <typename T,typename U>
    struct length< typelist<T,U> >
    {
        enum { value = 1 + length<U>::value };
    };

    // at<TLIST>::type
    //
    template <class TLIST, int N>
    struct at
    {
        typedef typename at<typename TLIST::tail, N-1>::type type;
    };
    template <class TLIST>
    struct at<TLIST,0>
    {
        typedef typename TLIST::head type;
    };

    // append<TLIST,T>::type
    //
    template <class TLIST, typename T> struct append;
    template <>
    struct append<null, null>
    {
        typedef null type;
    };
    template <typename U>
    struct append<null, U>
    {
        typedef TYPELIST(U) type; 
    };
    template <typename H, typename T>
    struct append<null, typelist<H,T> >
    {
        typedef typelist <H,T> type;
    };
    template <typename H, typename T, typename U>
    struct append<typelist<H,T> , U>
    {
        typedef typelist<H, typename append<T, U>::type > type;
    };

    // insert<TLIST,T>::type
    //
    template <class L, typename E> 
    struct insert
    {
        typedef typename append< typelist<E,null>, L>::type type;
    };
    template <typename L, typename H, typename T>
    struct insert<L, typelist<H,T> >
    {
        typedef typename append< typelist<H,T>, L>::type type;
    };

    // index_of<TLIST,T>::value
    //
    template <typename L, typename E> struct index_of;
    template <typename E>
    struct index_of<null, E> 
    {
        enum { value = -1 };
    };
    template <typename T, typename E>
    struct index_of< typelist<E, T>, E > 
    {
        enum { value = 0 };
    };
    template <typename H, typename T, typename E>
    struct index_of< typelist <H, T> , E >
    {
        enum { value = index_of<T, E>::value == -1 ? -1 : 1 + index_of<T,E>::value };
    };

    // has_type<TLIST,T>::type or fail to compile
    //
    template <typename TLIST, typename T>
    struct has_type 
    {
        template <typename V, int n>
        struct valid_type
        {
            typedef V type;
        };

        template <typename V>
        struct valid_type<V,-1>
        {};

        typedef typename valid_type<T, more::TL::index_of<TLIST,T>::value >::type type;
    };

    // TL::is_same<TLIST1, TLIST2>::value
    //
    template <typename T1, typename T2> struct is_same;
    template <>
    struct is_same<null,null>
    {
        enum { value = true };
    };
    template <typename H1, typename T1, typename Q>
    struct is_same< typelist<H1,T1>, Q>
    {
        enum { value = false };
    };
    template <typename H1, typename T1, typename Q>
    struct is_same< Q, typelist<H1,T1> >
    { 
        enum { value = false };
    };
    template <typename H1, typename T1, typename H2, typename T2>
    struct is_same < typelist<H1,T1>, typelist<H2,T2> >
    {
        enum { value = std::is_same<H1, H2>::value && is_same<T1, T2>::value };
    };        

    // TL::apply1<TLIST, UNARY_FUNCTION>::type
    //
    template <typename TLIST, typename UF> struct apply1;
    template <typename UF>
    struct apply1<null, UF>
    {
        typedef null type;
    };

    template <typename H, typename T, typename UF>
    struct apply1< typelist<H,T>, UF>
    {
        typedef typelist <
        typename UF::template apply<H>::type,
                 typename apply1<T,UF>::type
                 > type;
    };

    // TL::transform<TLIST1, TLIST2, BINARY_FUNCTION>::type
    //
    template <typename T1, typename T2, typename BF> struct transform;
    template <typename BF>
    struct transform<null, null, BF>
    {
        typedef null type;
    };

    template <typename H1, typename T1, typename H2, typename T2, typename BF>
    struct transform< typelist<H1,T1>, typelist<H2,T2>, BF>
    {
        typedef typelist <
        typename BF::template apply<H1,H2>::type,
                 typename transform<T1,T2,BF>::type
                 > type;
    };

}  // namespace TL
}; // namespace more

#endif /* TYPELIST_HH */
