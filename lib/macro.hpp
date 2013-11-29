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
#endif

#ifndef PASTE
#define PASTE(a,b)      a ## b
#define XPASTE(a,b)     PASTE(a,b)
#endif

#define UNPACK_ARGS(...)    __VA_ARGS__
#define UNPACK(x)           UNPACK_ARGS x
#define APPLY(f, ...)       f(__VA_ARGS__)

#define CONCAT_COMMA(x, ...)        x, __VA_ARGS__
#define CONCAT_SEMICOLON(x, ...)    x; __VA_ARGS__
#define CONCAT_SPACE(x, ...)        x  __VA_ARGS__

#define FOR_EACH_1(c, f, x)        APPLY(f,UNPACK(x))
#define FOR_EACH_2(c, f, x, ...)   c(APPLY(f,UNPACK(x)), FOR_EACH_1(c, f, __VA_ARGS__))
#define FOR_EACH_3(c, f, x, ...)   c(APPLY(f,UNPACK(x)), FOR_EACH_2(c, f, __VA_ARGS__))
#define FOR_EACH_4(c, f, x, ...)   c(APPLY(f,UNPACK(x)), FOR_EACH_3(c, f, __VA_ARGS__))
#define FOR_EACH_5(c, f, x, ...)   c(APPLY(f,UNPACK(x)), FOR_EACH_4(c, f, __VA_ARGS__))
#define FOR_EACH_6(c, f, x, ...)   c(APPLY(f,UNPACK(x)), FOR_EACH_5(c, f, __VA_ARGS__))
#define FOR_EACH_7(c, f, x, ...)   c(APPLY(f,UNPACK(x)), FOR_EACH_6(c, f, __VA_ARGS__))
#define FOR_EACH_8(c, f, x, ...)   c(APPLY(f,UNPACK(x)), FOR_EACH_7(c, f, __VA_ARGS__))
#define FOR_EACH_9(c, f, x, ...)   c(APPLY(f,UNPACK(x)), FOR_EACH_8(c, f, __VA_ARGS__))
#define FOR_EACH_10(c, f, x, ...)  c(APPLY(f,UNPACK(x)), FOR_EACH_9(c, f, __VA_ARGS__))
#define FOR_EACH_11(c, f, x, ...)  c(APPLY(f,UNPACK(x)), FOR_EACH_10(c, f, __VA_ARGS__))
#define FOR_EACH_12(c, f, x, ...)  c(APPLY(f,UNPACK(x)), FOR_EACH_11(c, f, __VA_ARGS__))
#define FOR_EACH_13(c, f, x, ...)  c(APPLY(f,UNPACK(x)), FOR_EACH_12(c, f, __VA_ARGS__))
#define FOR_EACH_14(c, f, x, ...)  c(APPLY(f,UNPACK(x)), FOR_EACH_13(c, f, __VA_ARGS__))
#define FOR_EACH_15(c, f, x, ...)  c(APPLY(f,UNPACK(x)), FOR_EACH_14(c, f, __VA_ARGS__))
#define FOR_EACH_16(c, f, x, ...)  c(APPLY(f,UNPACK(x)), FOR_EACH_15(c, f, __VA_ARGS__))
#define FOR_EACH_17(c, f, x, ...)  c(APPLY(f,UNPACK(x)), FOR_EACH_16(c, f, __VA_ARGS__))
#define FOR_EACH_18(c, f, x, ...)  c(APPLY(f,UNPACK(x)), FOR_EACH_17(c, f, __VA_ARGS__))
#define FOR_EACH_19(c, f, x, ...)  c(APPLY(f,UNPACK(x)), FOR_EACH_18(c, f, __VA_ARGS__))
#define FOR_EACH_20(c, f, x, ...)  c(APPLY(f,UNPACK(x)), FOR_EACH_19(c, f, __VA_ARGS__))

#define FOR_EACH(f, ...)            XPASTE(FOR_EACH_, PP_NARG(__VA_ARGS__))(CONCAT_SPACE    , f, __VA_ARGS__)
#define FOR_EACH_COMMA(f, ...)      XPASTE(FOR_EACH_, PP_NARG(__VA_ARGS__))(CONCAT_COMMA    , f, __VA_ARGS__)
#define FOR_EACH_SEMICOLON(f, ...)  XPASTE(FOR_EACH_, PP_NARG(__VA_ARGS__))(CONCAT_SEMICOLON, f, __VA_ARGS__)

