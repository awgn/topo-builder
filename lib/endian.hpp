#pragma once 

#include <arpa/inet.h> 

#include <type_traits>
#include <iostream>
#include <cstdint>
#include <string>

typedef int16_t   int16be_t;
typedef int32_t   int32be_t;
typedef int64_t   int64be_t;

typedef uint16_t  uint16be_t;
typedef uint32_t  uint32be_t;
typedef uint64_t  uint64be_t;

typedef int16_t   int16le_t;
typedef int32_t   int32le_t;
typedef int64_t   int64le_t;

typedef uint16_t  uint16le_t;
typedef uint32_t  uint32le_t;
typedef uint64_t  uint64le_t;


namespace details 
{
    template <typename T>
    inline T ntoh(T n, std::integral_constant<size_t, 1>)
    { 
        return n; 
    }

    template <typename T>
    inline T ntoh(T n, std::integral_constant<size_t, 2>)
    { 
        return static_cast<T>(ntohs(static_cast<uint16_t>(n))); 
    }

    template <typename T>
    inline T ntoh(T n, std::integral_constant<size_t, 4>) 
    { 
        return static_cast<T>(ntohl(static_cast<uint32_t>(n))); 
    }

    template <typename T>
    inline T ntoh(T n, std::integral_constant<size_t, 8>) 
    { 
        return (static_cast<T>(ntohl (n)) << 32) + ntohl (n >> 32); 
    }

} // namespace details


template <typename T>
inline T ntoh(T n)
{
    static_assert(std::is_integral<T>::value, "ntoh: T must be integral type");
    return details::ntoh(n, std::integral_constant<size_t, sizeof(T)>());
}


template <typename T>
inline T hton(T n)
{
    static_assert(std::is_integral<T>::value, "hton: T must be integral type");
    return details::ntoh(n, std::integral_constant<size_t, sizeof(T)>());
}

