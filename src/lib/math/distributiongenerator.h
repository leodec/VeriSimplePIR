#ifndef LBCRYPTO_MATH_DISTRIBUTIONGENERATOR_H_
#define LBCRYPTO_MATH_DISTRIBUTIONGENERATOR_H_

// used to define a thread-safe generator
#if defined(_MSC_VER) // Visual studio
//#define thread_local __declspec( thread )
#elif defined(__GCC__) // GCC
#define thread_local __thread
#endif

#include "backend.h"
#include <random>
#include "defines.h"
#include "prng.h"

namespace lbcrypto
{

    // AES Engine
    struct aes128_engine
    {
    private:
        static thread_local osuCrypto::PRNG m_prng;

    public:
        aes128_engine(){};
        ~aes128_engine(){};
        using result_type = uint64_t;
        constexpr static result_type min() { return 0; }
        constexpr static result_type max() { return -1; }

        result_type operator()();
    };

    // AES Engine
    struct aes128_zero_engine
    {
    private:
        static thread_local osuCrypto::PRNG zero_prng;

    public:
        aes128_zero_engine(){};
        ~aes128_zero_engine(){};
        using result_type = uint64_t;
        constexpr static result_type min() { return 0; }
        constexpr static result_type max() { return -1; }

        result_type operator()();

        static void reset_prng()
        {
            zero_prng.SetSeed(osuCrypto::ZeroBlock, 256);
        };
    };

    // TODO: Clean up dead code

    // Return a static generator object
    aes128_engine &get_prng();

    aes128_zero_engine &get_zero_prng();

    void reset_zero_prng();

    uv64 get_bug_vector(const ui32 size);

    uv64 get_tug_vector(const ui32 size, const ui64 modulus);

    template <typename T>
    T get_dug_element(const T modulus);

    template <typename T>
    std::vector<T> get_dug_vector(const ui32 size, const T modulus);
    // uv64 get_dug_vector(const ui32 size, const ui64 modulus);

    template <typename T>
    void get_dug_array(T *x, const ui32 size, const T modulus);

    void get_dug_array_128(ui128 *x, const ui32 size, const ui128 modulus);

    template <typename T>
    void get_dug_array_from_seed(T *x, const ui32 size, const osuCrypto::block seed, const T modulus);

    // template <typename T>
    // void get_dug_array_from_prng(T * x, const ui32 size, const osuCrypto::PRNG prng, const T modulus);

    template <typename T>
    void get_dug_array_from_seed_parallel(T *x, const ui32 size, const osuCrypto::block seed, const T modulus);

    template <typename T>
    std::vector<T> get_dgg_testvector(ui32 size, T p, float std_dev = 40.0);

    uv64 get_uniform_testvector(ui32 size, ui64 max);

    /**
     * @brief Abstract class describing generator requirements.
     *
     * The Distribution Generator defines the methods that must be implemented by a real generator.
     * It also holds the single PRNG, which should be called by all child class when generating a random number is required.
     *
     */

    // Base class for Distribution Generator by type
    class DistributionGenerator
    {
    public:
        DistributionGenerator() {}
        virtual ~DistributionGenerator() {}
    };

} // namespace lbcrypto

#endif // LBCRYPTO_MATH_DISTRIBUTIONGENERATOR_H_
