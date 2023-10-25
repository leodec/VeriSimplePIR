#include "pir/pir.h"
#include "pir/preproc_pir.h"

void benchmark_vlhepir_proof(const uint64_t N, const uint64_t d, const bool verbose = false) {

    double start, end;
    const uint64_t iters = 1;

    std::cout << "Input params: N = " << N << " d = " << d << std::endl;
    std::cout << "database size: " << N*d / (8.0*(1ULL << 20)) << " MiB\n";

    VLHEPIR pir(
        N, d, 
        true, 
        verbose, 
        false, 
        false
    );

    std::cout << "database params: "; pir.dbParams.print();

    // Matrix D = pir.db.packDataInMatrix(pir.dbParams);
    Matrix D(pir.dbParams.ell, pir.dbParams.m);
    // Matrix D; D.init_no_memset(pir.dbParams.ell, pir.dbParams.m);
    random_fast(D, pir.dbParams.p);

    // PackedMatrix D_packed = packMatrix(D, pir.lhe.p);
    // std::cout << "packed_D rows " << D_packed.mat.rows << " cols " << D_packed.mat.cols << std::endl;

    PackedMatrix D_packed_hardcoded = packMatrixHardCoded(D, pir.lhe.p);
    random_fast(D_packed_hardcoded.mat);
    std::cout << "D_packed_hardcoded rows " << D_packed_hardcoded.mat.rows << " cols " << D_packed_hardcoded.mat.cols << std::endl;

    Matrix A = pir.FakeInit();

    Matrix H = pir.GenerateFakeHint();
    std::cout << "offline download size = " << H.rows*H.cols*sizeof(Elem) / (1ULL << 20) << " MiB\n";

    unsigned char hash[SHA256_DIGEST_LENGTH];
    pir.HashAandH(hash, A, H);

    const uint64_t index = 10;
    auto ct_sk = pir.Query(A, index);

    Matrix ct = std::get<0>(ct_sk);
    // Matrix sk = std::get<1>(ct_sk);

    Matrix ans = pir.Answer(ct, D);

    start = currentDateTime();
    for (uint64_t i = 0; i < iters; i++) {
        Matrix Z = pir.Prove(hash, ct, ans, D);
    }
    end = currentDateTime();
    std::cout << "Full Proof generation time: " << (end-start)/iters << " ms\n";

    start = currentDateTime();
    for (uint64_t i = 0; i < iters; i++) {
        Matrix Z = pir.Prove(hash, ct, ans, D_packed_hardcoded);
    }
    end = currentDateTime();
    std::cout << "Full packed Proof generation time: " << (end-start)/iters << " ms\n";


    BinaryMatrix C = pir.HashToC(hash, ct, ans);
    start = currentDateTime();
    for (uint64_t i = 0; i < iters; i++) {
        BinaryMatrix C = pir.HashToC(hash, ct, ans);
    }
    end = currentDateTime();
    std::cout << "Hash to C time: " << (end-start)/iters << " ms\n";


    start = currentDateTime();
    for (uint64_t i = 0; i < iters; i++) {
        Matrix Z = matMulLeftBinary(C, D);
    }
    end = currentDateTime();
    std::cout << "C*D time: " << (end-start)/iters << " ms\n";

    // start = currentDateTime();
    // for (uint64_t i = 0; i < iters; i++) {
    //     Matrix Z = matMulLeftBinaryRightColPacked(C, D_packed);
    // }
    // end = currentDateTime();
    // std::cout << "C*D_packed time: " << (end-start)/iters << " ms\n";

    start = currentDateTime();
    for (uint64_t i = 0; i < iters; i++) {
        Matrix Z = matMulLeftBinaryRightColPacked_Hardcoded(C, D_packed_hardcoded);
    }
    end = currentDateTime();
    std::cout << "C*D_packed_hardcoded time: " << (end-start)/iters << " ms\n";
}

void benchmark_verisimplepir_proof(const uint64_t N, const uint64_t d, const bool verbose = false) {

    double start, end;
    const uint64_t iters = 1;

    std::cout << "Input params: N = " << N << " d = " << d << std::endl;
    std::cout << "database size: " << N*d / (8.0*(1ULL << 20)) << " MiB\n";

    const bool honest_hint = true;
    VLHEPIR pir(N, d, true, verbose, false, false, 1, true);
    VeriSimplePIR pre_pir(N, d, true, verbose, false, false, 1, true, honest_hint);

    pir.m = pre_pir.m;
    pir.ell = pre_pir.ell;
    pir.lhe = pre_pir.lhe;

    std::cout << "database params: "; pre_pir.dbParams.print();

    // Matrix D = pir.db.packDataInMatrix(pir.dbParams);
    // Matrix D(pir.dbParams.ell, pir.dbParams.m);
    Matrix D; D.init_no_memset(pre_pir.dbParams.ell, pre_pir.dbParams.m);
    // random_fast(D, pir.dbParams.p);

    // PackedMatrix D_packed = packMatrix(D, pir.lhe.p);
    // std::cout << "packed_D rows " << D_packed.mat.rows << " cols " << D_packed.mat.cols << std::endl;

    // PackedMatrix D_packed_hardcoded;
    PackedMatrix D_packed_hardcoded = packMatrixHardCoded(D, pre_pir.lhe.p);
    random_fast(D_packed_hardcoded.mat);
    std::cout << "D_packed_hardcoded rows " << D_packed_hardcoded.mat.rows << " cols " << D_packed_hardcoded.mat.cols << std::endl;

    unsigned char hash[SHA256_DIGEST_LENGTH];
    // pir.HashAandH(hash, A, H);

    const uint64_t index = 10;
    // auto ct_sk = pre_pir.Query(A, index);
    Matrix ct(pre_pir.m, 1); random(ct);

    // Matrix ct = std::get<0>(ct_sk);
    // Matrix sk = std::get<1>(ct_sk);

    Matrix ans = pre_pir.Answer(ct, D);
    // Matrix ans()

    start = currentDateTime();
    for (uint64_t i = 0; i < iters; i++) {
        Matrix Z = pir.Prove(hash, ct, ans, D);
    }
    end = currentDateTime();
    std::cout << "Full Proof generation time: " << (end-start)/iters << " ms\n";

    start = currentDateTime();
    for (uint64_t i = 0; i < iters; i++) {
        Matrix Z = pir.Prove(hash, ct, ans, D_packed_hardcoded);
    }
    end = currentDateTime();
    std::cout << "Full packed Proof generation time: " << (end-start)/iters << " ms\n";


    BinaryMatrix C = pir.HashToC(hash, ct, ans);
    start = currentDateTime();
    for (uint64_t i = 0; i < iters; i++) {
        BinaryMatrix C = pir.HashToC(hash, ct, ans);
    }
    end = currentDateTime();
    std::cout << "Hash to C time: " << (end-start)/iters << " ms\n";


    start = currentDateTime();
    for (uint64_t i = 0; i < iters; i++) {
        Matrix Z = matMulLeftBinary(C, D);
    }
    end = currentDateTime();
    std::cout << "C*D time: " << (end-start)/iters << " ms\n";

    // start = currentDateTime();
    // for (uint64_t i = 0; i < iters; i++) {
    //     Matrix Z = matMulLeftBinaryRightColPacked(C, D_packed);
    // }
    // end = currentDateTime();
    // std::cout << "C*D_packed time: " << (end-start)/iters << " ms\n";

    start = currentDateTime();
    for (uint64_t i = 0; i < iters; i++) {
        Matrix Z = matMulLeftBinaryRightColPacked_Hardcoded(C, D_packed_hardcoded);
    }
    end = currentDateTime();
    std::cout << "C*D_packed_hardcoded time: " << (end-start)/iters << " ms\n";
}

int main() {
    // const uint64_t N = 1<<20;
    // // const uint64_t d = 2048;
    // const uint64_t d = 128;

    // const uint64_t N = 1<<30;
    const uint64_t N = 4*(1ULL<<33);
    // const uint64_t N = 8*(1ULL<<33);
    // const uint64_t N = 16*(1ULL<<33);
    const uint64_t d = 1;

    const bool verbose = false;

    // benchmark_vlhepir_proof(N, d, verbose);
    benchmark_verisimplepir_proof(N, d, verbose);
}