/*
 * distributiongenerator.cpp
 *
 *  Created on: Aug 25, 2017
 *      Author: chiraag
 *
 */

#include "distributiongenerator.h"
#include "prng.h"
#include <random>

namespace lbcrypto
{

    // thread_local osuCrypto::PRNG aes128_engine::m_prng(_mm_setzero_si128(), 256);
    thread_local osuCrypto::PRNG aes128_engine::m_prng(osuCrypto::sysRandomSeed(), 256);

    thread_local osuCrypto::PRNG aes128_zero_engine::zero_prng(osuCrypto::ZeroBlock, 256);

    aes128_engine &get_prng()
    {
        // C++11 thread-safe static initialization
        // static thread_local std::mt19937_64 prng(std::random_device{}());
        static thread_local aes128_engine prng;
        // static aes128_engine prng;
        return prng;
    }

    aes128_zero_engine &get_zero_prng()
    {
        // static thread_local aes128_zero_engine prng;
        static aes128_zero_engine prng;
        return prng;
    }

    void reset_zero_prng()
    {
        aes128_zero_engine::reset_prng();
    }

    uv64 get_bug_vector(const ui32 size)
    {
        auto &prng = get_prng();
        // osuCrypto::PRNG prng(osuCrypto::sysRandomSeed(), 256);
        auto distribution = std::uniform_int_distribution<ui64>(0, 1);

        uv64 v(size);
        for (ui32 i = 0; i < size; i++)
        {
            v[i] = distribution(prng);
        }
        return v;
    }

    uv64 get_tug_vector(const ui32 size, const ui64 modulus)
    {
        auto &prng = get_prng();
        // osuCrypto::PRNG prng(osuCrypto::sysRandomSeed(), 256);
        auto distribution = std::uniform_int_distribution<si32>(-1, 1);
        ui64 minus1 = modulus - 1;
        uv64 v(size);

        for (ui32 m = 0; m < size; m++)
        {
            si32 rand = distribution(prng);
            switch (rand)
            {
            case -1:
                v[m] = minus1;
                break;
            case 0:
                v[m] = 0;
                break;
            case 1:
                v[m] = 1;
                break;
            }
        }

        return v;
    }

    template ui32 get_dug_element(const ui32 modulus);
    template ui64 get_dug_element(const ui64 modulus);

    template <typename T>
    T get_dug_element(const T modulus)
    {
        auto &prng = get_prng();
        auto distribution = std::uniform_int_distribution<T>(0, modulus - 1);
        return distribution(prng);
    }

    // uv64 get_dug_vector(const ui32 size, const ui64 modulus) {
    template std::vector<ui32> get_dug_vector(const ui32 size, const ui32 modulus);
    template std::vector<ui64> get_dug_vector(const ui32 size, const ui64 modulus);

    template <typename T>
    std::vector<T> get_dug_vector(const ui32 size, const T modulus)
    {
        auto &prng = get_prng();
        auto distribution = std::uniform_int_distribution<T>(0, modulus - 1);

        std::vector<T> v(size);

        for (ui32 i = 0; i < size; i++)
        {
            v[i] = distribution(prng);
        }
        return v;
    }

    template void get_dug_array(ui64 *x, const ui32 size, const ui64 modulus);
    template void get_dug_array(ui32 *x, const ui32 size, const ui32 modulus);
    template <typename T>
    void get_dug_array(T *x, const ui32 size, const T modulus)
    {
        auto &prng = get_prng();
        auto distribution = std::uniform_int_distribution<T>(0, modulus - 1);

        for (ui32 i = 0; i < size; i++)
            x[i] = distribution(prng);
    }

    void get_dug_array_128(ui128 *x, const ui32 size, const ui128 modulus)
    {
        auto &prng = get_prng();
        auto distribution = std::uniform_int_distribution<ui64>(0, 1ULL << 63);

        for (ui32 i = 0; i < size; i++)
        {
            x[i] = ((ui128)distribution(prng)) << 64;
            x[i] += distribution(prng);
            x[i] %= modulus;
        }
    }

    template void get_dug_array_from_seed(ui64 *x, const ui32 size, const osuCrypto::block seed, const ui64 modulus);
    template void get_dug_array_from_seed(ui32 *x, const ui32 size, const osuCrypto::block seed, const ui32 modulus);

    template <typename T>
    void get_dug_array_from_seed(T *x, const ui32 size, const osuCrypto::block seed, const T modulus)
    {
        osuCrypto::PRNG prng(seed);
        auto distribution = std::uniform_int_distribution<T>(0, modulus - 1);

        for (ui32 i = 0; i < size; i++)
            x[i] = distribution(prng);
    }

    // template void get_dug_array_from_prng(ui64 * x, const ui32 size, const osuCrypto::PRNG prng, const ui64 modulus);
    // template void get_dug_array_from_prng(ui32 * x, const ui32 size, const osuCrypto::PRNG prng, const ui32 modulus);

    // template <typename T>
    // void get_dug_array_from_prng(T * x, const ui32 size, const osuCrypto::PRNG prng, const T modulus) {
    //     auto distribution = std::uniform_int_distribution<T>(0, modulus-1);
    //     for (ui32 i = 0; i < size; i++)
    //         x[i] = distribution(prng);
    // }

    template void get_dug_array_from_seed_parallel(ui64 *x, const ui32 size, const osuCrypto::block seed, const ui64 modulus);
    template void get_dug_array_from_seed_parallel(ui32 *x, const ui32 size, const osuCrypto::block seed, const ui32 modulus);

    template <typename T>
    void get_dug_array_from_seed_parallel(T *x, const ui32 size, const osuCrypto::block seed, const T modulus)
    {
        constexpr ui32 numGens = 8;
        ASSERT_DEBUG(size >= numGens);
        ASSERT_DEBUG(size % numGens == 0);
        // osuCrypto::PRNG prng(seed);
        // auto distribution = std::uniform_int_distribution<T>(0, modulus-1);
        std::vector<osuCrypto::PRNG> prngs;
        prngs.reserve(numGens);
        std::vector<std::uniform_int_distribution<T>> dists;
        dists.reserve(numGens);
        for (ui32 i = 0; i < numGens; i++)
        {
            prngs.push_back(osuCrypto::PRNG(seed + osuCrypto::toBlock(i)));
            dists.push_back(std::uniform_int_distribution<T>(0, modulus - 1));
        }

        const ui32 blockSize = size / numGens;
        PARALLEL_FOR
        for (ui32 blockNum = 0; blockNum < numGens; blockNum++)
            for (ui32 i = 0; i < blockSize; i++)
                x[blockNum * blockSize + i] = dists[blockNum](prngs[blockNum]);
    }

    template uv32 get_dgg_testvector(ui32 size, ui32 p, float std_dev);
    template uv64 get_dgg_testvector(ui32 size, ui64 p, float std_dev);
    template <typename T>
    std::vector<T> get_dgg_testvector(ui32 size, T p, float std_dev)
    {
        std::normal_distribution<double> distribution(0, std_dev);
        auto &prng = get_prng();

        std::vector<T> vec(size);
        for (ui32 i = 0; i < size; i++)
        {
            si32 r = std::max(-127, std::min(128, (si32)distribution(prng)));
            vec[i] = (r >= 0) ? r : p + r;
        }
        return vec;
    }

    uv64 get_uniform_testvector(ui32 size, ui64 max)
    {
        std::uniform_int_distribution<ui64> distribution(0, max);
        auto &prng = get_prng();

        uv64 vec(size);
        for (ui32 i = 0; i < size; i++)
        {
            vec[i] = distribution(prng);
        }
        return vec;
    }

    aes128_engine::result_type aes128_engine::operator()()
    {
        return m_prng.get<aes128_engine::result_type>();
    }

    aes128_zero_engine::result_type aes128_zero_engine::operator()()
    {
        return zero_prng.get<aes128_zero_engine::result_type>();
    }

} // namespace lbcrypto
