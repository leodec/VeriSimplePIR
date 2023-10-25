#include "pir/pir.h"

void benchmark_vlhepir_online (const uint64_t N, const uint64_t d, const bool verbose = false) {

    double start, end;

    std::cout << "Input params: N = " << N << " d = " << d << std::endl;
    std::cout << "database size: " << N*d / (8.0*(1ULL<<20)) << " MiB\n";

    VLHEPIR pir(
        N, d, 
        true,       // allowTrivial
        verbose,    // verbose
        false,      // simplePIR
        false,      // random data
        1,          // batch size
        false       // honest hint assumption
    );

    std::cout << "database params: "; pir.dbParams.print();
    

    std::cout << "sampling random db matrix...\n";

    PackedMatrix D_packed = packMatrixHardCoded(pir.dbParams.ell, pir.dbParams.m, pir.dbParams.p);

    const uint64_t index = 1;

    std::cout << "sampling random A matrix...\n";
    Matrix A = pir.FakeInit();

    Matrix H = pir.GenerateFakeHint();
    std::cout << "offline download size = " << H.rows*H.cols*sizeof(Elem) / (1ULL<<20) << " MiB\n";

    auto ct_sk = pir.Query(A, index);
    uint64_t iters = 10;
    start = currentDateTime();
    for (uint64_t i = 0; i < iters; i++) {
        pir.Query(A, index);
    }
    end = currentDateTime();
    std::cout << "Query generation time: " << (end-start)/iters << " ms\n";

    Matrix ct = std::get<0>(ct_sk);
    Matrix sk = std::get<1>(ct_sk);

    Matrix ans = pir.Answer(ct, D_packed);
    start = currentDateTime();
    for (uint64_t i = 0; i < iters; i++) {
        // Matrix ans = pir.Answer(ct, D);
        Matrix ans = pir.Answer(ct, D_packed);
    }
    end = currentDateTime();
    std::cout << "Answer generation time: " << (end-start)/iters << " ms\n";

    unsigned char hash[SHA256_DIGEST_LENGTH];
    pir.HashAandH(hash, A, H);

    iters = 1;

    Matrix Z = pir.Prove(hash, ct, ans, D_packed);
    start = currentDateTime();
    for (uint64_t i = 0; i < iters; i++) {
        Matrix Z = pir.Prove(hash, ct, ans, D_packed);
    }
    end = currentDateTime();
    std::cout << "Proof with packed D generation time: " << (end-start)/iters << " ms\n";

    start = currentDateTime();
    for (uint64_t i = 0; i < iters; i++) {
        pir.FakeVerify(A, H, hash, ct, ans, Z);
    }
    end = currentDateTime();
    std::cout << "Verification time: " << (end-start)/iters << " ms\n";

    entry_t res;
    start = currentDateTime();
    for (uint64_t i = 0; i < iters; i++) {
        res = pir.Recover(H, ans, sk, index);
    }
    end = currentDateTime();
    std::cout << "Recovery time: " << (end-start)/iters << " ms\n";
}

int main() {
    // const uint64_t N = 1ULL<<30;
    // const uint64_t N = 1ULL<<33;
    const uint64_t N = 4*(1ULL<<33);
    // const uint64_t N = 8*(1ULL<<33);
    // const uint64_t N = 16*(1ULL<<33);
    // const uint64_t N = 1ULL<<35;
    // const uint64_t d = 2048;
    // const uint64_t d = 128;
    const uint64_t d = 1;

    // const bool verbose = true;
    const bool verbose = false;

    benchmark_vlhepir_online(N, d, verbose);
}