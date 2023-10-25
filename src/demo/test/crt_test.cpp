#include "pir/multilimb_lhe.h"

void test_crt_recombine() {

    using ui128 = lbcrypto::ui128;

    const Elem p = 2;

    // const ui128 q = ((ui128)1) << 64;

    // const Elem kappa = 1048583;
    const Elem kappa = 25;

    // uint64_t m = 15;

    Multi_Limb_LHE lhe(p, kappa);

    // for (size_t i = 0; i < 10; i++) {
        Elem x_q = 11936820735700446991ULL;
        Elem x_kappa = 236678ULL % kappa;

        auto x_big = lhe.recombine(x_q, x_kappa);
    // }
}

void test_crt_error_recombine() {

    using ui128 = lbcrypto::ui128;

    const Elem p = 2;

    // const ui128 q = ((ui128)1) << 64;

    // const Elem kappa = 1048583;
    const Elem kappa = 25;

    uint64_t m = 15;

    Multi_Limb_LHE lhe(p, kappa);

    auto A = lhe.genPublicA(m);

    Multi_Limb_Matrix ciphertext(A.rows, 1);
    lhe.error(ciphertext);

    std::cout << "big_Q = " << lbcrypto::uint128ToString(lhe.big_Q) << std::endl;
    for (size_t i = 0; i < ciphertext.rows; i++) {
        auto x_big = lhe.recombine(ciphertext.q_data.data[i], ciphertext.kappa_data.data[i]);
        // if (x_big > lhe.big_Q/2) x_big = lhe.big_Q - x_big;
        std::cout << "error elem " << i << " = " << lbcrypto::uint128ToString(x_big) << std::endl;
    }
}

void test_crt_enc_dec() {

    // uint64_t n = 10;
    const Elem p = 1000;
    // const Elem p = 3;
    // const Elem p = 2;

    // const Elem kappa = 1048583;
    const Elem kappa = 25;

    uint64_t m = 15;

    Matrix pt(m, 1);
    // for (size_t i = 0; i < m; i++) {
        // pt.data[i] = 1;
    // }
    // std::cout << "change me back\n";
    random(pt, p);

    // std::cout << "pt =\n"; print(pt);

    Multi_Limb_LHE lhe(p, kappa);

    auto A = lhe.genPublicA(m);

    auto sk = lhe.sampleSecretKey();

    auto ct = lhe.encrypt(A, sk, pt);

    auto res = lhe.decrypt(A, sk, ct);

    // std::cout << "result = \n"; print(res);

    if (!eq(res, pt, true)) {
        assert(false);
    }

    std::cout << "Basic encrypt-decrypt test passed\n";
};

void basic_lhe_ops_test() {

    // uint64_t n = 4;
    Elem p = 10;

    // const Elem kappa = 1048583;
    const Elem kappa = 25;

    uint64_t m = 15;

    Multi_Limb_LHE lhe(p, kappa);

    Matrix pt(m, 1);
    lhe.randomPlaintext(pt);
    // constant(pt, 1);
    // std::cout << "pt =\n";
    // print(pt);

    const uint64_t ell = 12;

    Matrix D(ell, m);
    lhe.randomPlaintext(D);
    // constant(D, 1);
    // std::cout << "D =\n";
    // print(D);

    Matrix correct = matMulVec(D, pt);
    for (size_t i = 0; i < ell; i++) {
        correct.data[i] %= p;
    }
    // std::cout << "correct =\n";
    // print(correct);


    auto A = lhe.genPublicA(m);
    // std::cout << "A =\n";
    // print(A);
    auto H = matMul(D, A, lhe.kappa);

    auto sk = lhe.sampleSecretKey();

    auto ct = lhe.encrypt(A, sk, pt);
    auto ct_res = matMulVec(D, ct, lhe.kappa);

    auto res = lhe.decrypt(H, sk, ct_res);

    // std::cout << "res =\n";
    // print(res);

    if (!eq(res, correct, true)) {
        assert(false);
    }

    std::cout << "Basic lhe linear operations test passed\n";
}

void lhe_binary_mult_test() {

    // uint64_t n = 4;
    Elem p = 10;

    // const Elem kappa = 1048583;
    const Elem kappa = 25;

    uint64_t m = 15;

    Multi_Limb_LHE lhe(p, kappa);

    Matrix pt(m, 1);
    lhe.randomPlaintext(pt);
    // constant(pt, 1);
    // std::cout << "pt =\n";
    // print(pt);

    const uint64_t ell = 12;

    Matrix D(ell, m);
    lhe.randomPlaintext(D);
    // constant(D, 1);
    // std::cout << "D =\n";
    // print(D);

    Matrix correct = matMulVec(D, pt);
    for (size_t i = 0; i < ell; i++) {
        correct.data[i] %= p;
    }
    // std::cout << "correct =\n";
    // print(correct);


    auto A = lhe.genPublicA(m);
    // std::cout << "A =\n";
    // print(A);
    auto H = matMul(D, A, lhe.kappa);

    auto sk = lhe.sampleSecretKey();

    auto ct = lhe.encrypt(A, sk, pt);
    auto ct_res = matMulVec(D, ct, lhe.kappa);

    auto res = lhe.decrypt(H, sk, ct_res);

    // std::cout << "res =\n";
    // print(res);

    if (!eq(res, correct, true)) {
        assert(false);
    }

    std::cout << "Basic lhe linear operations test passed\n";
}


int main() {
    // test_crt_recombine();
    // test_crt_error_recombine();
    test_crt_enc_dec();
    basic_lhe_ops_test();
};