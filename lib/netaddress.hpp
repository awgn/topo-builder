/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
*/

#pragma once 

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <iostream>
#include <string>
#include <stdexcept>
#include <functional>
#include <cassert>

#include <nettypes.hpp>

namespace net { 

    class address
    { 
    public:
        address()
        : addr_(), mask_()
        {}

        address(const char *addr, const char *mask)
        : addr_(), mask_()
        {
            if (inet_pton(AF_INET, addr, &addr_) <= 0) 
                throw std::runtime_error(std::string("net::address: ").append(addr).append(" invalid address"));

            if (inet_pton(AF_INET, mask, &mask_) <= 0)
                throw std::runtime_error(std::string("net::address: ").append(mask).append(" invalid address"));
        }

        address(const char *addr, size_t n=32)
        : addr_(), mask_()
        {
            if (inet_pton(AF_INET, addr, &addr_) <= 0)
                throw std::runtime_error(std::string("net::address: ").append(addr).append(" invalid address"));

            mask_ = prefix2mask(n);
        }

        address(const in_addr& a, const in_addr &m)
        : addr_(a), mask_(m)
        {
        }

        address(const in_addr& a, size_t n=32)
        : addr_(a), mask_(prefix2mask(n))
        {
        }

        address(ipv4_t addr, size_t n=32)
        : mask_(prefix2mask(n))
        {
            addr_.s_addr = addr.value;
        } 

        ~address()
        {}  

        const in_addr &
        addr() const
        {
            return addr_;
        }

        const in_addr &
        mask() const
        {
            return mask_;
        }

        ipv4_t
        addr_ip() const
        {
            return ipv4_t {addr_.s_addr};
        }
        
        ipv4_t
        mask_ip() const
        {
            return ipv4_t {mask_.s_addr};
        }

        size_t
        prefix() const
        {
            return mask2prefix(mask_);
        }

        bool operator==(const address &other) const
        {
            return addr_.s_addr == other.addr_.s_addr &&
                    mask_.s_addr == other.mask_.s_addr;
        }

        bool operator!=(const address &other) const
        {
            return !(*this == other);
        }

        static in_addr prefix2mask(size_t n)
        {
            struct in_addr res;
            res.s_addr =  htonl(~((1ULL << (32-n)) - 1)); 
            return res;
        } 

        static size_t mask2prefix(const in_addr &m)
        {
            return static_cast<size_t>(__builtin_popcount(m.s_addr));
        }

    private:
        struct in_addr addr_;
        struct in_addr mask_;
    };

    // comparisons by means of prefix...
    // 
    static inline
    bool operator<(const address &lhs, const address &rhs)
    {
        return (lhs.prefix() < rhs.prefix() ||
               (lhs.prefix() == rhs.prefix() &&
                  ntohl(lhs.addr().s_addr) < ntohl(rhs.addr().s_addr)));
    }
    static inline
    bool operator>(const address &lhs, const address &rhs)
    {
        return rhs < lhs;
    }
    static inline
    bool operator<=(const address &lhs, const address &rhs)
    {
        return !(lhs > rhs);
    }
    static inline
    bool operator>=(const address &lhs, const address &rhs)
    {
        return !(lhs < rhs);
    }

    // check whether the lhs net address is included into the
    // rhs net address.
    //
    static inline
    bool is_included(const address &lhs, const address &rhs)
    {
       auto lp = lhs.prefix(), rp = rhs.prefix();

       if (lp < rp)
            return false;
       
       auto mask = net::address::prefix2mask(rp);
       return (lhs.addr().s_addr & mask.s_addr) ==
               (rhs.addr().s_addr & mask.s_addr);  
    }

    template <typename CharT, typename Traits>
    typename std::basic_ostream<CharT, Traits> &
    operator<<(std::basic_ostream<CharT,Traits> &out, const address& other)
    {
        char ip[16];
        if (inet_ntop(AF_INET, &other.addr(), ip, sizeof(ip)) == nullptr) 
            throw std::runtime_error("address::operator<<");

        if (other.prefix() == 32)
            return out << ip;
        else
            return out << ip << '/' << other.prefix();
    }

    inline std::string
    show(const address &addr)
    {
        char buf[16] = { '\0' };
        inet_ntop(AF_INET, &addr.addr(), buf, sizeof(buf));

        if (addr.prefix() == 32)
            return std::string(buf);
        else
            return std::string(buf) + '/' + std::to_string(addr.prefix());
    }

    template <typename CharT, typename Traits>
    typename std::basic_istream<CharT, Traits> &
    operator>>(std::basic_istream<CharT,Traits>& in, address& out)
    {
        struct rollback
        {   
            rollback(std::basic_istream<CharT,Traits> &ref)
            : err_(true), ref_(ref), pos_(ref.tellg())
            {}

            ~rollback()
            {
                if (err_) {

                    if (!ref_.seekg(pos_))
                        ref_.setstate(std::ios_base::badbit);         
                    else 
                        ref_.setstate(std::ios_base::failbit);         
                }
            }    

            bool err_;
            std::basic_istream<CharT,Traits> &ref_;
            std::streampos pos_;

        } return_(in);

        std::string addr;
        if (!(in >> addr))
        {
            return in;
        }

        if (addr.front() == '"' || addr.front() == '\'')
        {
            auto tmp = std::string(addr.begin() + 1, addr.end() - 1);
            addr = std::move(tmp);
        }

        auto pos = addr.find('/');

        auto it  = pos == std::string::npos ? std::end(addr) : 
                                              std::next(std::begin(addr), pos);

        in_addr a, m; 

        std::string addr_str(addr.begin(), it);

        if (inet_pton(AF_INET, addr_str.c_str(), &a) <= 0)
        {
            throw std::runtime_error("net::address: address");
        }

        if (pos == std::string::npos)
        {
            return_.err_ = false;
            return (out = address(a, 32)), in;
        }

        auto prefix = std::make_pair(0, false); 

        std::string mask_str(++it, addr.end());
       
        if (inet_pton(AF_INET, mask_str.c_str(), &m) <= 0)
        {
            // numeric mask
            
            prefix.second = true;
            size_t len;
            prefix.first = std::stoul(mask_str, &len);
            if (prefix.first > 32 || len != mask_str.size())
                throw std::runtime_error("net::address: mask");
        }

        return_.err_ = false;
        return (prefix.second ? (out = address(a,prefix.first)): (out = address(a,m))), in;
    } 

} // namespace net


namespace std { 

    // std::hash specialization for net::address class
    //
    
    template <>
    struct hash<net::address>
    {
        // Before to insert elements, we must determine (a priori) the minimum prefix admitted 
        // for the hash table. An utility function will enforce this constraint. 
        //
        // ie: 
        // 192.168.0.0/16
        // 192.168.10.0/24  n -> 16.

        hash(size_t n = 0)
        : min_(n)
        {}

        size_t
        operator()(const net::address& value) const
        {
            auto m = min_ == 0 ? value.prefix() : min_;
            assert( value.prefix() >= m );
            return value.addr().s_addr & net::address::prefix2mask(m).s_addr;
        }

        size_t min_;    
    };

} // namespace std


