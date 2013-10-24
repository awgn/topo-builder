/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef _TYPEMAP_HPP_
#define _TYPEMAP_HPP_ 

#include <utility>
#include <type_traits>

namespace more { namespace type {

    template <typename ...Ti> struct typemap;  // a typelist of std::pair<key,value>

    // get<key, typemap>::type
    //
    template <typename Tm, typename K, typename E = void> struct get; 
    template <typename K, typename K0, typename V0, typename ...Ki, typename ...Vi>
    struct get<typemap<std::pair<K0, V0>, std::pair<Ki,Vi>...>, K, typename std::enable_if<!std::is_same<K,K0>::value>::type>
    {
        typedef typename get<typemap<std::pair<Ki,Vi>...>, K,void>::type type;
    };
    template <typename K, typename K0, typename V0, typename ...Ki, typename ...Vi>
    struct get<typemap<std::pair<K0, V0>, std::pair<Ki,Vi>...>, K, typename std::enable_if<std::is_same<K,K0>::value>::type>
    {
        typedef V0 type;
    };
    template <typename K>
    struct get<K, typemap<>>
    {
    };

    // size<Typemap>::value
    //
    template <typename Tm> struct size;
    template <typename ...Ti>
    struct size<typemap<Ti...>>
    {
        enum { value = sizeof...(Ti) };
    };

    // append<Typemap, Key, Value>::type
    //
    template <typename Tm, typename K, typename V> struct append;
    template <typename K, typename V, typename ...Ki, typename ...Vi>
    struct append<typemap<std::pair<Ki,Vi>...>, K, V>
    {
        typedef typemap< std::pair<Ki,Vi>..., std::pair<K,V>> type;
    };

    // insert<Typemap, Key, Value>::type
    //
    template <typename Tm, typename K, typename V> struct insert;
    template <typename K, typename V, typename ...Ki, typename ...Vi>
    struct insert<typemap<std::pair<Ki,Vi>...>, K, V>
    {
        typedef typemap< std::pair<K,V>, std::pair<Ki,Vi>...> type;
    };

     // index_of<Typemap, Key>::value
     //
     template <typename Tm, typename Key, typename E = void> struct index_of;

     template <typename K, typename K0, typename V0, typename ...Ki, typename ...Vi>
     struct index_of<typemap< std::pair<K0,V0>, std::pair<Ki,Vi>...>, K, typename std::enable_if<!std::is_same<K0,K>::value>::type>
     {
         enum { value = index_of<typemap< std::pair<Ki,Vi>...>, K>::value == -1 ? -1 : 1 + index_of< typemap<std::pair<Ki,Vi>...>, K>::value };
     };

     template <typename K, typename K0, typename V0, typename ...Ki, typename ...Vi>
     struct index_of<typemap< std::pair<K0,V0>, std::pair<Ki,Vi>...>, K, typename std::enable_if<std::is_same<K0,K>::value>::type>
     {
         enum { value = 0 };
     };

     template <typename K>
     struct index_of<typemap<>, K>
     {
         enum { value = -1 };
     };

}   // namespace type
}   // namespace more

#endif /* _TYPEMAP_HPP_ */
