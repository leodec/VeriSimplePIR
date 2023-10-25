#pragma once
#include "mat.h"

class LHE {  // linearly homomorphic encryption
public:
    static constexpr uint64_t n = (sizeof(Elem) <= 4) ? 1024 : 2048;  // number of columns of A. morally, the security parameter
    Elem p;  // plaintext modulus
    // the ciphertext modulus q is implicit in the data type Elem
    //      logq is either 32 or 64 depending on the size of Elem.
    static constexpr uint64_t logq = 8*sizeof(Elem);
    Elem Delta;  // q/p

    LHE () {};

    // LHE(uint64_t n_in, Elem p_in) : n(n_in), p(p_in) {
    LHE(const Elem p_in) : p(p_in) {
        assert(p_in >= 2);
        Elem q_half = 1ULL << (logq - 1);
        Delta = q_half/p;
        Delta <<= 1;
    };

    Matrix genPublicA(uint64_t m) const;  // input is number of rows
    Matrix sampleSecretKey() const;  // column vector where # of rows is always n

    void randomPlaintext(Matrix& pt) const;

    // plaintext has length m, where m is in the parameter used to sample m
    Matrix encrypt(const Matrix& A, const Matrix& sk, const Matrix& pt) const;  

    // length of ct should match the # of rows of H
    Matrix decrypt(const Matrix& H, const Matrix& sk, const Matrix& ct) const;
};
