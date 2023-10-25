#pragma once


// include the full GSL header (C++14). If not defined, uses gsl_lite (c++11)
#define ENABLE_FULL_GSL ON 

// defined if we should use cpp 14 and undefined means cpp 11
#define ENABLE_CPP_14 ON 

// enables the use of assembly based SHA1
/* #undef ENABLE_NASM */

// enable the wolf ssl socket layer.
/* #undef ENABLE_WOLFSSL */

// enable the use of intel SSE instructions.
#define ENABLE_SSE ON

// enable the use of the portable AES implementation.
/* #undef ENABLE_PORTABLE_AES */

#if (defined(_MSC_VER) || defined(__SSE2__)) && defined(ENABLE_SSE)
#define ENABLE_SSE_BLAKE2 ON
#define OC_ENABLE_SSE2 ON
#endif

#define OC_ENABLE_PORTABLE_AES ON
