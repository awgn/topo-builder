/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _TYPE_TRAITS_HPP_
#define _TYPE_TRAITS_HPP_ 

#include <iostream>
#include <type_traits>   
#include <utility>
#include <tuple>         
#include <vector>
#include <array>

namespace more 
{
    namespace traits {

    // For use in __is_convertible_simple.
    struct __sfinae_types
    {
      typedef char __one;
      typedef struct { char __arr[2]; } __two;
    };

    // is_class_or_union (using SFINAE... Vandevoorde/Josuttis)
    template <typename T>
    class __is_class_or_union_helper : public __sfinae_types
    {
        template <typename C> static __one test(int C::*);
        template <typename C> static __two test(...);

    public:
        enum { value = sizeof(test<T>(0)) == sizeof(__one) };
    };

    template <typename T>
    struct is_class_or_union : public std::integral_constant<bool, __is_class_or_union_helper<T>::value>
    {};
    
    // has member type helper (using SFINAE... Vandevoorde/Josuttis)
    #define HAS_MEMBER_HELPER(abc) \
    template <typename T>   \
    class __has_ ## abc ## _helper : public __sfinae_types   \
    {   \
        template <typename C> static __one test(typename std::remove_reference<typename C::abc>::type *);  \
        template <typename C> static __two test(...);   \
    \
    public: \
        enum { value = sizeof(test<T>(0)) == sizeof(__one) };   \
    }

    HAS_MEMBER_HELPER(value_type);
    HAS_MEMBER_HELPER(key_type);
    HAS_MEMBER_HELPER(mapped_type);
    HAS_MEMBER_HELPER(container_type);

    HAS_MEMBER_HELPER(pointer);
    HAS_MEMBER_HELPER(const_pointer);
    HAS_MEMBER_HELPER(reference);
    HAS_MEMBER_HELPER(const_reference);
    HAS_MEMBER_HELPER(iterator);
    HAS_MEMBER_HELPER(const_iterator);
    HAS_MEMBER_HELPER(reverse_iterator);
    HAS_MEMBER_HELPER(const_reverse_iterator);
    HAS_MEMBER_HELPER(size_type);
    HAS_MEMBER_HELPER(difference_type);

    template <typename T>
    struct has_value_type : public std::integral_constant<bool, __has_value_type_helper<T>::value>
    {};

    template <typename t>
    struct has_key_type : public std::integral_constant<bool, __has_key_type_helper<t>::value>
    {};

    template <typename t>
    struct has_mapped_type : public std::integral_constant<bool, __has_mapped_type_helper<t>::value>
    {};

    template <typename t>
    struct has_container_type : public std::integral_constant<bool, __has_container_type_helper<t>::value>
    {};

    template <typename T>
    struct has_pointer : public std::integral_constant<bool, __has_pointer_helper<T>::value>
    {};

    template <typename T>
    struct has_const_pointer : public std::integral_constant<bool, __has_const_pointer_helper<T>::value>
    {};
    
    template <typename T>
    struct has_reference : public std::integral_constant<bool, __has_reference_helper<T>::value>
    {};

    template <typename T>
    struct has_const_reference : public std::integral_constant<bool, __has_const_reference_helper<T>::value>
    {};

    template <typename T>
    struct has_iterator : public std::integral_constant<bool, __has_iterator_helper<T>::value>
    {};

    template <typename T>
    struct has_const_iterator : public std::integral_constant<bool, __has_const_iterator_helper<T>::value>
    {};

    template <typename T>
    struct has_reverse_iterator : public std::integral_constant<bool, __has_reverse_iterator_helper<T>::value>
    {};

    template <typename T>
    struct has_const_reverse_iterator : public std::integral_constant<bool, __has_const_reverse_iterator_helper<T>::value>
    {};
    
    template <typename T>
    struct has_size_type : public std::integral_constant<bool, __has_size_type_helper<T>::value>
    {};
    
    template <typename T>
    struct has_difference_type : public std::integral_constant<bool, __has_difference_type_helper<T>::value>
    {};
    

    // is_container 
    //
    
    template <typename T>
    struct is_container : public std::integral_constant<bool, __has_value_type_helper<T>::value && 
                                                              __has_reference_helper<T>::value &&  
                                                              __has_const_reference_helper<T>::value &&  
                                                              __has_iterator_helper<T>::value && 
                                                              __has_const_iterator_helper<T>::value && 
                                                              __has_pointer_helper<T>::value &&  
                                                              __has_const_pointer_helper<T>::value &&  
                                                              __has_size_type_helper<T>::value &&  
                                                              __has_difference_type_helper<T>::value 
                                                               >
    {};


    // is_associative_container 
    //
    
    template <typename T>
    struct is_associative_container : std::integral_constant<bool, is_container<T>::value &&
                                                                   __has_key_type_helper<T>::value &&
                                                                   __has_mapped_type_helper<T>::value>
    {};


    // is_vector_like
    //
    
    template <typename C>
    struct __is_vector_like
    {
        enum { value = false };
    };

    template <typename T>
    struct __is_vector_like<std::vector<T>>
    {
        enum { value = true };
    };
    template <typename T, size_t N>
    struct __is_vector_like<std::array<T, N>>
    {
        enum { value = true };
    };
    template <typename T, size_t N>
    struct __is_vector_like<T[N]>
    {
        enum { value = true };
    };

    template <typename T>
    struct is_vector_like: public std::integral_constant<bool, __is_vector_like<T>::value>
    {};
    
    
    // not_type
    //
    
    template <typename Trait, bool V = Trait::value> 
    struct not_type : std::false_type {};

    template <typename Trait> 
    struct not_type<Trait, false> : std::true_type {};
    
    // is_tuple 
    //
    
    template <typename T>
    struct is_tuple : public std::integral_constant<bool, false>
    {};

    template <typename ...Ti>
    struct is_tuple<std::tuple<Ti...>> : public std::integral_constant<bool, true>
    {};

    // is_pair
    //
    
    template <typename T>
    struct is_pair : public std::integral_constant<bool, false>
    {};

    template <typename T, typename U>
    struct is_pair<std::pair<T,U>> : public std::integral_constant<bool, true>
    {};

    // has_insertion_operator: operator<<()
    //
    
    template <typename T>
    class __has_insertion_operator_helper : public __sfinae_types
    {
        template <typename C> static __one test(typename std::remove_reference< decltype((std::cout << std::declval<C>())) >::type *);
        template <typename C> static __two test(...);
    public:    
        enum { value = sizeof(test<T>(0)) == sizeof(__one) };
    };

    template <typename T>
    struct has_insertion_operator : public std::integral_constant<bool, __has_insertion_operator_helper<T>::value>
    {};

    // has_extraction_operator: operator>>()
    //
    
    template <typename T>
    class __has_extraction_operator_helper : public __sfinae_types
    {
        template <typename C> static __one test(typename std::remove_reference< decltype((std::cin >> std::declval<C &>())) >::type *);
        template <typename C> static __two test(...);
    public:    
        enum { value = sizeof(test<T>(0)) == sizeof(__one) };
    };

    template <typename T>
    struct has_extraction_operator : public std::integral_constant<bool, __has_extraction_operator_helper<T>::value>
    {};

    // is_callable:
    //

    template <typename T>
    class __is_callable_helper : public __sfinae_types
    {
        template <typename C> static __one test(typename std::remove_reference< decltype((std::declval<C>()())) >::type *);
        template <typename C> static __two test(...);
    public:    
        enum { value = sizeof(test<T>(0)) == sizeof(__one) };
    };

    template <typename T>
    struct is_callable : public std::integral_constant<bool, __is_callable_helper<T>::value>
    {};

    // is_copy_constructing:
    //

    template <typename T, typename ...U>
    struct is_copy_constructing : std::false_type {};
    
    template <typename T, typename U>
    struct is_copy_constructing<T,U> : std::is_same<typename std::decay<T>::type, typename std::decay<U>::type> {};

    // is_not_copy_constructing: trait useful to disable universal constructor when 
    //                       copy constructing:
    //
    //  template <typename ...Ts, typename V = std::enable_if<is_not_copy_constructing<ThisClass, Ts...>::value>::type> 
    //  ThisClass(Ts&& ...args) 
    //
    
    template <typename T, typename ...U>
    struct is_not_copy_constructing : not_type<is_copy_constructing<T,U...>> {};
    

    } // namespace traits

} // namespace more 

#endif /* _TYPE_TRAITS_HPP_ */
