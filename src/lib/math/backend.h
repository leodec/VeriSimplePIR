#ifndef LBCRYPTO_MATH_BACKEND_H
#define LBCRYPTO_MATH_BACKEND_H

#include <inttypes.h>
#include <complex>
#include <vector>
#include <assert.h>
#include <cinttypes>
#include <stdio.h>
#include <memory>
#include<algorithm>

// #include <boost/multiprecision/float128.hpp>

#include "bigint/BigIntegerLibrary.h"

// #define STRING_CONCAT(S1, S2) S1 ## S2
// #define STRING_CONCAT(S1, S2, S3) S1 ## S2 ## S3
#define STRING_CONCAT(S1, S2, S3) S1 S2 S3

// #define MULTI_THREAD // undefine to remove all multithreading from the library
#ifdef MULTI_THREAD
#define PARALLEL_FOR _Pragma("omp parallel for")
// #define PARALLEL_FOR_PRIVATE(x) _Pragma("omp parallel for private(" + #x + ")")
// #define PARALLEL_FOR_PRIVATE(x) _Pragma(STRING_CONCAT("omp parallel for private(",#x ,")"))
#define PRAGMAFY(x) _Pragma(x)
#else
#define PARALLEL_FOR
// #define PARALLEL_FOR_PRIVATE(x)
#define PRAGMAFY(x)
#endif

// #define ASSERT_DEBUG_FLAG

#ifdef ASSERT_DEBUG_FLAG
#define ASSERT_DEBUG(x) assert(x)
#define CHECK_DEBUG std::cout << "DEBUG: Debugging is ON\n";
#define CHECK_DEBUG_VERBOSE CHECK_DEBUG
#define DEBUG_FUNC(x) x
#else
#define ASSERT_DEBUG(x)
#define CHECK_DEBUG
#define CHECK_DEBUG_VERBOSE std::cout << "DEBUG: Debugging is OFF\n";
#define DEBUG_FUNC(x)
#endif

#define ENFORCE_STRICT_MOD
#ifdef ENFORCE_STRICT_MOD
#define STRICT_MOD(x, y) x -= (x >= y) ? y : 0
#else
#define STRICT_MOD(x, y)
#endif

#define MAX(x, y) ((x > y) ? x : y)
#define MIN(x, y) ((x > y) ? y : x)

using namespace std;

/**
 * @namespace lbcrypto
 * The namespace of lbcrypto
 */
namespace lbcrypto
{
    typedef int16_t si16;
    typedef uint16_t ui16;
    typedef int32_t si32;
    typedef uint32_t ui32;
    typedef int64_t si64;
    typedef uint64_t ui64;
    typedef __uint128_t ui128;

    typedef std::vector<si16> sv16;
    typedef std::vector<ui16> uv16;
    typedef std::vector<si32> sv32;
    typedef std::vector<ui32> uv32;
    typedef std::vector<si64> sv64;
    typedef std::vector<ui64> uv64;
    typedef std::vector<ui128> uv128;

    typedef std::vector<sv16> sm16;
    typedef std::vector<uv16> um16;
    typedef std::vector<sv32> sm32;
    typedef std::vector<uv32> um32;
    typedef std::vector<sv64> sm64;
    typedef std::vector<uv64> um64;
    typedef std::vector<uv128> um128;

    typedef double fl32;
    typedef std::vector<double> fv32;
    typedef long double fl64;
    typedef std::vector<fl64> fv64;
    typedef __float128 fl128;
    // typedef float128 fl128;
    typedef std::vector<fl128> fv128;

    typedef BigUnsigned ubi;
    typedef std::vector<ubi> uvbi;

    typedef complex<double> cd;
    typedef std::vector<cd> cdv;

    template <typename T>
    inline constexpr std::shared_ptr<T> allocate_shared_ptr() { return std::shared_ptr<T>(new T()); };

    template <typename T, typename = enable_if_t<is_unsigned<T>::value>>
    T abs(const T &x) { return x; };

    template <typename T, typename = enable_if_t<is_signed<T>::value>, typename = enable_if_t<is_signed<T>::value>>
    T abs(const T &x) { return std::abs(x); };

    /**
     * @brief Lists all modes for RLWE schemes, such as BGV and FV
     */
    enum MODE
    {
        RLWE = 0,     // Discrete Gaussian Sampling
        OPTIMIZED = 1 // Ternary Error Sampling
    };

    template<typename T>
    T mod_pow(T a, T b, T m)
    {
        if(b == 0) return 1;

        a = a % m;
        T temp = mod_pow(a, b/2, m);
        if(b&1)
            return (((temp * temp) % m) * a) % m;
        else
            return (temp * temp) % m;
    }

    std::string uint128ToString(const ui128& value);

} // namespace lbcrypto ends

#endif
