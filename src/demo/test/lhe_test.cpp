#include "pir/lhe.h"

void basic_enc_dec_test() {
    // uint64_t n = 10;
    Elem p = 1000;

    uint64_t m = 15;

    Matrix pt(m, 1);
    // for (size_t i = 0; i < m; i++) {
    //     pt.data[i] = i;
    // }
    random(pt, p);

    LHE lhe(p);

    Matrix A = lhe.genPublicA(m);

    Matrix sk = lhe.sampleSecretKey();

    Matrix ct = lhe.encrypt(A, sk, pt);

    Matrix res = lhe.decrypt(A, sk, ct);

    if (!eq(res, pt, true)) {
        assert(false);
    }

    std::cout << "Basic encrypt-decrypt test passed\n";
}

void basic_lhe_test() {

    // uint64_t n = 4;
    Elem p = 10;

    LHE lhe(p);

    uint64_t m = 10;

    Matrix pt(m, 1);
    lhe.randomPlaintext(pt);
    // constant(pt, 1);
    // std::cout << "pt =\n";
    // print(pt);

    uint64_t ell = 12;

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



    Matrix A = lhe.genPublicA(m);
    // std::cout << "A =\n";
    // print(A);
    Matrix H = matMul(D, A);

    Matrix sk = lhe.sampleSecretKey();

    Matrix ct = lhe.encrypt(A, sk, pt);
    Matrix ct_res = matMulVec(D, ct);

    Matrix res = lhe.decrypt(H, sk, ct_res);

    // std::cout << "res =\n";
    // print(res);

    if (!eq(res, correct, true)) {
        assert(false);
    }

    std::cout << "Basic lhe test passed\n";
}

int main() {

    basic_enc_dec_test();
    basic_lhe_test();

};