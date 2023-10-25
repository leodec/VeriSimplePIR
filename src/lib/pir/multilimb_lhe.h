#pragma once

/*
    Multi-limb version of LHE.
    Used to run the precomputation and then drop the 
    modulus to a machine word for fast online operations.
*/

#include "math/backend.h"
#include "lhe.h"

struct Multi_Limb_Matrix {
    uint64_t rows, cols;

    Matrix q_data;
    Matrix kappa_data;

    Multi_Limb_Matrix(const uint64_t m, const uint64_t n) :
        rows(m), cols(n), 
        q_data(m, n), kappa_data(m, n) {};

    ~Multi_Limb_Matrix() {};
};

void constant(Multi_Limb_Matrix& mat, const Elem val, const Elem kappa);

Multi_Limb_Matrix matMul(const Matrix &a, const Multi_Limb_Matrix &b, const Elem modulus);
Multi_Limb_Matrix matMulVec(const Matrix &a, const Multi_Limb_Matrix &b, const Elem modulus);
Multi_Limb_Matrix matMulLeftBinary(const BinaryMatrix& binary, const Multi_Limb_Matrix& b, const Elem modulus);

bool eq(const Multi_Limb_Matrix& a, const Multi_Limb_Matrix& b, const bool verbose = false);

class Multi_Limb_LHE {  // linearly homomorphic encryption
public:

    using ui128 = lbcrypto::ui128;
    using ubi = lbcrypto::ubi;

    static constexpr uint64_t n = (sizeof(Elem) <= 4) ? 1024 : 2048;  // number of columns of A. morally, the security parameter
    Elem p;  // plaintext modulus
    // the ciphertext modulus q is implicit in the data type Elem
    //      logq is either 32 or 64 depending on the size of Elem.
    static constexpr uint64_t logq = 8*sizeof(Elem);
    Elem kappa; // full modulus is q * kappa. 

    ui128 big_Q;
    ui128 q_inv_mod_kappa;
    ui128 kappa_inv_mod_q;

    ui128 DeltaBig;
    Elem Delta_q;  // q*kappa/p mod q
    Elem Delta_kappa;  // q*kappa/p mod kappa

    Multi_Limb_LHE () {};

    Multi_Limb_LHE(const Elem p_in, const Elem kappa) : p(p_in), kappa(kappa) {
        assert(p_in >= 2);
        assert(kappa % 2 == 1);  // kappa must be odd for CRT with even modulus
        // std::cout << "kappa = " <<  kappa << std::endl;

        ui128 q = ((ui128)1) << logq;
        big_Q = q * (ui128)kappa;
        DeltaBig = ( q ) * ( (ui128)kappa ) / ((ui128) p);
        Delta_q = (DeltaBig) % q;
        Delta_kappa = (DeltaBig) % kappa;

        ui128 kappa_pow = kappa;
        kappa_inv_mod_q = 1;
        while (kappa_pow != 1) {
            kappa_inv_mod_q = kappa_inv_mod_q * kappa_pow;
            kappa_pow = kappa_pow * kappa_pow;
        }
        assert(kappa_inv_mod_q * kappa == 1);  // mod q is implicit
        // std::cout << "kappa_inv_mod_q in constructor = " << lbcrypto::uint128ToString(kappa_inv_mod_q) << std::endl;

        // Elem q_inv_mod_kappa;
        Elem q_mod_kappa = ((ui128)1LL << logq) % ((ui128) kappa);
        q_inv_mod_kappa = 1;
        Elem q_mod_kappa_pow = q_mod_kappa;
        uint64_t order_of_kappa_in_z_q = 1;
        // std::cout << "kappa = " << kappa << std::endl;
        while (q_mod_kappa_pow != 1) {
            if (order_of_kappa_in_z_q > kappa) {
                std::cout << "order is bigger than the size of the group\n";
                std::cout << "kappa = " << kappa << std::endl;
                assert(false);
            }
            q_inv_mod_kappa = (q_inv_mod_kappa * q_mod_kappa) % kappa;
            q_mod_kappa_pow = (q_mod_kappa_pow * q_mod_kappa) % kappa;
            order_of_kappa_in_z_q += 1;
        }
        // std::cout << "q_mod_kappa = " << q_mod_kappa << std::endl;
        // q_inv_mod_kappa = lbcrypto::mod_pow(q_mod_kappa, kappa-2, kappa);
        // std::cout << "q_inv_mod_kappa = " << q_inv_mod_kappa << std::endl;
        assert(q_inv_mod_kappa * q_mod_kappa % kappa == 1);
    };

    void error(Multi_Limb_Matrix& mat) const;

    Multi_Limb_Matrix genPublicA(uint64_t m) const;  // input is number of rows
    Multi_Limb_Matrix sampleSecretKey() const;  // column vector where # of rows is always n

    void randomPlaintext(Matrix& pt) const;

    // plaintext has length m, where m is in the parameter used to sample m
    Multi_Limb_Matrix encrypt(const Multi_Limb_Matrix& A, const Multi_Limb_Matrix& sk, const Matrix& pt) const;  

    ui128 recombine(const Elem q_elem, const Elem kappa_elem) const;

    // length of ct should match the # of rows of H
    Matrix decrypt(const Multi_Limb_Matrix& H, const Multi_Limb_Matrix& sk, const Multi_Limb_Matrix& ct) const;
};