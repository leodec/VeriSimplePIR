#include "multilimb_lhe.h"
#include "gauss.h"


void constant(Multi_Limb_Matrix& mat, const Elem val, const Elem kappa) {
    size_t len = mat.rows * mat.cols;
    for (size_t i = 0; i < len; i++) {
        mat.q_data.data[i] = val;
        mat.kappa_data.data[i] = val % kappa;
    }
}

Multi_Limb_Matrix matMul(const Matrix &a, const Multi_Limb_Matrix &b, const Elem modulus) {
    Multi_Limb_Matrix result(a.rows, b.cols);

    result.q_data = matMul(a, b.q_data);
    result.kappa_data = matMul(a, b.kappa_data, modulus);

    return result;
}


Multi_Limb_Matrix matMulVec(const Matrix &a, const Multi_Limb_Matrix &b, const Elem modulus) {
    Multi_Limb_Matrix result(a.rows, b.cols);

    result.q_data = matMulVec(a, b.q_data);
    result.kappa_data = matMulVec(a, b.kappa_data, modulus);

    return result;
}

Multi_Limb_Matrix matMulLeftBinary(const BinaryMatrix& binary, const Multi_Limb_Matrix& b, const Elem modulus) {
    Multi_Limb_Matrix result(binary.rows, b.cols);

    result.q_data = matMulLeftBinary(binary, b.q_data);
    result.kappa_data = matMulLeftBinary(binary, b.kappa_data, modulus);

    return result;
}

bool eq(const Multi_Limb_Matrix& a, const Multi_Limb_Matrix& b, const bool verbose) {
    return eq(a.q_data, b.q_data, verbose) && eq(a.kappa_data, b.kappa_data, verbose);
}

Multi_Limb_LHE::ui128 Multi_Limb_LHE::recombine(const Elem q_elem, const Elem kappa_elem) const {
    constexpr ui128 q = ((ui128)1) << logq;
    const ui128 big_modulus = ((ui128)kappa) * q;

    // std::cout << "kappa_inv_mod_q = " << lbcrypto::uint128ToString(kappa_inv_mod_q) << std::endl;

    const ui128 q_1_term = (ui128)q_elem * kappa_inv_mod_q % (ui128)q;
    const ui128 q_2_term = (ui128)kappa_elem * q_inv_mod_kappa % (ui128)kappa;

    // std::cout << "q_1_term = " << lbcrypto::uint128ToString(q_1_term) << std::endl;
    // std::cout << "q_2_term = " << lbcrypto::uint128ToString(q_2_term) << std::endl;

    const ui128 result = (
        (q_1_term * (ui128)kappa) + (q_2_term * (ui128)q) 
        ) % big_modulus;

    // std::cout << "result = " << lbcrypto::uint128ToString(result) << std::endl;

    assert(result % q == (ui128) q_elem);
    assert(result % (ui128)kappa == (ui128) kappa_elem);

    return result;
}

// input is number of rows
Multi_Limb_Matrix Multi_Limb_LHE::genPublicA(uint64_t m) const {
    Multi_Limb_Matrix A(m, n);
    random(A.q_data);
    random(A.kappa_data, kappa);
    return A;
};

// column vector where # of rows is always n
Multi_Limb_Matrix Multi_Limb_LHE::sampleSecretKey() const {
    Multi_Limb_Matrix sk(n, 1);
    random(sk.q_data);
    random(sk.kappa_data, kappa);
    return sk;
};

void Multi_Limb_LHE::randomPlaintext(Matrix& pt) const {
    random(pt, p);
}

void Multi_Limb_LHE::error(Multi_Limb_Matrix& mat) const {
    const size_t len = mat.rows * mat.cols;
    for (size_t i = 0; i < len; i++) {
        const auto elem = GaussSample<int>();
        // const lbcrypto::ui128 big_elem = (elem >= 0) ? elem : big_Q + elem;
        // std::cout << "error elem " << i << " = " << lbcrypto::uint128ToString(big_elem) << " (" << elem << ")" << std::endl;
        mat.q_data.data[i] = elem;  // big_elem;
        mat.kappa_data.data[i] = (kappa + elem) % kappa;   // assumes kappa > elem
        // assert(recombine(mat.q_data.data[i], mat.kappa_data.data[i]) == big_elem);
    }
}

// plaintext has length m, where m is in the parameter used to sample m
// Assuming all elements of pt are less than p;
Multi_Limb_Matrix Multi_Limb_LHE::encrypt(const Multi_Limb_Matrix& A, const Multi_Limb_Matrix& sk, const Matrix& pt) const {
    if (A.rows != pt.rows) {
        std::cout << "Plaintext dimension mismatch!\n";
        assert(false);
    }

    if (sk.cols != 1 || pt.cols != 1) {
        std::cout << "secret key or plaintext are not column vectors!\n";
        assert(false);
    }

    Multi_Limb_Matrix ciphertext(A.rows, 1);
    error(ciphertext);
    // constant(ciphertext, -1, kappa); std::cout << "change me back!\n";

    Multi_Limb_Matrix A_sk(ciphertext.rows, ciphertext.cols);
    A_sk.q_data = matMulVec(A.q_data, sk.q_data); 
    A_sk.kappa_data = matMulVec(A.kappa_data, sk.kappa_data, kappa); 

    matAddInPlace(ciphertext.q_data, A_sk.q_data);
    matAddInPlace(ciphertext.kappa_data, A_sk.kappa_data, kappa);

    // scale plaintext
    Multi_Limb_Matrix pt_scaled(A.rows, 1);
    pt_scaled.q_data = matMulScalar(pt, Delta_q);
    pt_scaled.kappa_data = matMulScalar(pt, Delta_kappa, kappa);

    matAddInPlace(ciphertext.q_data, pt_scaled.q_data);
    matAddInPlace(ciphertext.kappa_data, pt_scaled.kappa_data, kappa);

    return ciphertext;
};

// length of ct should match the # of rows of H
Matrix Multi_Limb_LHE::decrypt(const Multi_Limb_Matrix& H, const Multi_Limb_Matrix& sk, const Multi_Limb_Matrix& ct) const {
    if (H.rows != ct.rows) {
        std::cout << "Ciphertext dimension mismatch!\n";
        assert(false);
    }

    if (sk.cols != 1 || ct.cols != 1) {
        std::cout << "secret key or ciphertext are not column vectors!\n";
        assert(false);
    }

    Multi_Limb_Matrix H_sk(H.rows, 1);
    H_sk.q_data = matMulVec(H.q_data, sk.q_data);
    H_sk.kappa_data = matMulVec(H.kappa_data, sk.kappa_data, kappa);

    Multi_Limb_Matrix scaled_pt(ct.rows, 1);
    scaled_pt.q_data = matSub(ct.q_data, H_sk.q_data);
    scaled_pt.kappa_data = matSub(ct.kappa_data, H_sk.kappa_data, kappa);

    std::vector<ui128> pt_big_scaled(scaled_pt.rows);
    for (uint64_t i = 0; i < scaled_pt.rows; i++) {
        pt_big_scaled[i] = recombine(scaled_pt.q_data.data[i], scaled_pt.kappa_data.data[i]);
    }

    Matrix pt(pt_big_scaled.size(), 1);
    for (size_t i = 0; i < pt.rows; i++) {
        // std::cout << "pt_big_scaled["<<i<<"] = " << lbcrypto::uint128ToString(pt_big_scaled[i]) << std::endl;
        pt.data[i] = (pt_big_scaled[i] + (DeltaBig/2)) / DeltaBig;
        // pt.data[i] %= p;
        if (pt.data[i] == p) pt.data[i] = 0;
    }

    return pt;
}


