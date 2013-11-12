/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _TUPLE_EXT_HPP_
#define _TUPLE_EXT_HPP_ 

#include <cstddef>
#include <tuple>
#include <algorithm>

#include <iostream>

namespace more { 
 
    /// Inspired by litb:
    /// http://stackoverflow.com/questions/7858817/unpacking-a-tuple-to-call-a-matching-function-pointer
    ///
    
    /// both gens forward and backward, in Haskell:
    ///
    /// gen_forward 0 xs = xs
    /// gen_forward n xs = gen_forward (n-1) $ (n-1):xs
    /// 
    /// gen_backward x xs = gen_backward' x 0 xs
    ///         where gen_backward' n m xs
    ///                 | n == m = xs
    ///                 | otherwise = gen_backward' n (m+1) (m : xs)

    //////////////////////////////////////////////////////////////
    // numeric sequence utility
    // 

    template <int ...>
    struct seq { };

    //////////////////////////////////////////////////////////////
    // gen_forward
    //

    template <int N, int ...Xs>
    struct gen_forward : gen_forward<N-1, N-1, Xs...> { };

    template <int ...Xs>
    struct gen_forward<0, Xs...> {
        typedef seq<Xs...> type;
    };

    //////////////////////////////////////////////////////////////
    // gen_backward
    //

    template <int N, int M, int ...Xs>
    struct __gen_backward : __gen_backward<N, M+1, M, Xs...> {};
    
    template <int N, int ...Xs> 
    struct __gen_backward<N,N,Xs...>
    {
        typedef seq<Xs...> type;
    };

    template <int N, int ...Xs>
    struct gen_backward : __gen_backward<N, 0, Xs...> {};

    //////////////////////////////////////////////////////////////
    /// tuple_for_each 

    template <typename TupleT, typename Fun>
    void call_for_each(TupleT &&, Fun, seq<>)
    {
    }

    template <typename TupleT, typename Fun, int ...S>
    void call_for_each(TupleT &&tup, Fun fun, seq<S...>)
    {
        // 8.5.4: Within the initializer-list of a braced-init-list, the initializer-clauses, 
        // including any that result from pack expansions (14.5.3), are evaluated in the order in which they appear
        //
        
        auto sink __attribute__((unused)) = { (fun(std::get<S>(tup)),0)... };
    }

    template <typename TupleT, typename Fun>
    void tuple_for_each(TupleT &&tup, Fun fun)
    {
        call_for_each(std::forward<TupleT>(tup), fun, 
                      typename gen_forward<std::tuple_size<typename std::decay<TupleT>::type>::value
                      >::type());
    }   

    //////////////////////////////////////////////////////////////
    /// tuple_for_each2

    template <typename Tp>
    constexpr Tp && min(Tp &&a, Tp &&b)
    {
        return a < b ? a : b;
    }
    
    template <typename Tuple1, typename Tuple2, typename Fun>
    void call_for_each2(Tuple1 &&, Tuple2 &&, Fun, seq<>)
    {
    }
    
    template <typename Tuple1, typename Tuple2, typename Fun, int ...S>
    void call_for_each2(Tuple1 &&t1, Tuple2 &&t2, Fun fun, seq<S...>)
    {
        auto sink __attribute__((unused)) = { (fun(std::get<S>(t1), std::get<S>(t2)),0)... };
    }

    template <typename Tuple1, typename Tuple2, typename Fun>
    void tuple_for_each2(Tuple1 &&t1, Tuple2 &&t2, Fun fun)
    {
        call_for_each2(std::forward<Tuple1>(t1),
                      std::forward<Tuple2>(t2), 
                      fun, typename gen_forward<
                            min(std::tuple_size<typename std::decay<Tuple1>::type>::value,
                                std::tuple_size<typename std::decay<Tuple2>::type>::value) 
                      >::type());
    }   

    //////////////////////////////////////////////////////////////
    /// tuple_map
    
    template <typename Fun, typename TupleT, int ...S>
    auto call_map(Fun fun, TupleT &&tup, seq<S...>)
    -> decltype(std::make_tuple(fun(std::get<S>(tup))...))
    {
        return std::make_tuple(fun(std::get<S>(tup))...);
    }

    template <typename Fun, typename TupleT>
    auto tuple_map(Fun fun, TupleT &&tup)
    -> decltype(call_map(fun, std::forward<TupleT>(tup),                                       
                        typename gen_forward<                                                   
                          std::tuple_size<typename std::decay<TupleT>::type>::value
                        >::type()))                                                             
    {                                               
        return call_map(fun, std::forward<TupleT>(tup), 
                      typename gen_forward<
                        std::tuple_size<typename std::decay<TupleT>::type>::value
                      >::type());
    }   

    //////////////////////////////////////////////////////////////
    /// tuple_apply

    template <typename Fun, typename TupleT, int ...S>
    auto call_apply(Fun fun, TupleT &&tup, seq<S...>)
        -> decltype(fun(std::get<S>(tup)...))
    {
        return fun(std::get<S>(tup)...);
    }

    template <typename Fun, typename TupleT>
    auto tuple_apply(Fun fun, TupleT &&tup)
        -> decltype(call_apply(fun, std::forward<TupleT>(tup), 
                      typename gen_forward<
                        std::tuple_size<typename std::decay<TupleT>::type>::value
                      >::type()))
    {
        return call_apply(fun, std::forward<TupleT>(tup), 
                      typename gen_forward<
                        std::tuple_size<typename std::decay<TupleT>::type>::value
                      >::type());
    }   

} // namespace more


#endif /* _TUPLE_EXT_HPP_ */
