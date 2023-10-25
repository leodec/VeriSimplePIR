#include "pir/pir.h"

void benchmark_verisimplepir_verify(const uint64_t N, const uint64_t d, const bool verbose = false) {

    double start, end;
    const uint64_t iters = 10;

    std::cout << "Input params: N = " << N << " d = " << d << std::endl;
    std::cout << "database size: " << N*d / (8.0*(1ULL << 20)) << " MiB\n";

    VLHEPIR pir(N, d, true, verbose, false, false);

    std::cout << "database params: "; pir.dbParams.print();

    // Matrix D = pir.db.packDataInMatrix(pir.dbParams);
    Matrix D(pir.dbParams.ell, pir.dbParams.m);
    random(D, pir.dbParams.p);

    const uint64_t index = 1;

    Matrix A = pir.Init();

    Matrix H = pir.GenerateFakeHint();
    std::cout << "offline download size = " << H.rows*H.cols*sizeof(Elem) / (1ULL << 20) << " MiB\n";

    unsigned char hash[SHA256_DIGEST_LENGTH];
    pir.HashAandH(hash, A, H);

    auto ct_sk = pir.Query(A, index);

    Matrix ct = std::get<0>(ct_sk);
    // Matrix sk = std::get<1>(ct_sk);

    Matrix ans = pir.Answer(ct, D);

    Matrix Z = pir.Prove(hash, ct, ans, D);

    pir.FakeVerify(A, H, hash, ct, ans, Z);
    start = currentDateTime();
    for (uint64_t i = 0; i < iters; i++) {
        pir.FakeVerify(A, H, hash, ct, ans, Z);
    }
    end = currentDateTime();
    std::cout << "Full Verification time: " << (end-start)/iters << " ms\n";
}

int main() {
    // const uint64_t N = 1<<20;
    // // const uint64_t d = 2048;
    // const uint64_t d = 128;

    const uint64_t N = 1<<30;
    const uint64_t d = 1;

    const bool verbose = false;

    benchmark_verisimplepir_verify(N, d, verbose);
}