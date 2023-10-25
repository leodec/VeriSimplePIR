#include "pir/pir.h"

void benchmark_batch_verisimplepir_online (const uint64_t N, const uint64_t d, const uint64_t batchSize, const bool verbose = false) {

    double start, end;

    std::cout << "Input params: N = " << N << " d = " << d << std::endl;
    std::cout << "database size: " << N*d / (8.0*(1ULL<<20)) << " MiB\n";

    VLHEPIR pir(N, d, true, verbose, false, false, batchSize);

    std::cout << "database params: "; pir.dbParams.print();
    std::cout << "sampling random db matrix...\n";

    PackedMatrix D_packed = packMatrixHardCoded(pir.dbParams.ell, pir.dbParams.m, pir.dbParams.p);

    const uint64_t index = 1;

    std::cout << "sampling random A matrix...\n";
    Matrix A = pir.FakeInit();

    Matrix H = pir.GenerateFakeHint();
    std::cout << "offline download size = " << H.rows*H.cols*sizeof(Elem) / (1ULL << 20) << " MiB\n";

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

    iters = 1;

    Matrix ans = pir.Answer(ct, D_packed);
    start = currentDateTime();
    for (uint64_t i = 0; i < iters; i++) {
        for (uint64_t j = 0; j < batchSize; j++)
        Matrix ans = pir.Answer(ct, D_packed);
    }
    end = currentDateTime();
    std::cout << "Batch Answer generation time: " << (end-start)/iters << " ms\n";
    std::cout << "\t per-query Answer generation time: " << (end-start)/iters/batchSize << " ms\n";

    unsigned char hash[SHA256_DIGEST_LENGTH];
    pir.HashAandH(hash, A, H);


    std::vector<Matrix> ct_vec(batchSize);
    std::vector<Matrix> ans_vec(batchSize);
    for (uint32_t i = 0; i < batchSize; i++) {
        ct_vec[i] = ct;
        ans_vec[i] = ans;
    }

    Matrix Z = pir.BatchProve(hash, ct_vec, ans_vec, D_packed);
    start = currentDateTime();
    for (uint64_t i = 0; i < iters; i++) {
        Matrix Z = pir.BatchProve(hash, ct_vec, ans_vec, D_packed);
    }
    end = currentDateTime();
    std::cout << "Proof with packed D generation time: " << (end-start)/iters << " ms\n";
    std::cout << "\t per-query proof generation time: " << (end-start)/iters/batchSize << " ms\n";

    // start = currentDateTime();
    // for (uint64_t i = 0; i < iters; i++) {
    //     Z = pir.Prove(hash, ct, ans, D);
    // }
    // end = currentDateTime();
    // std::cout << "Proof generation time: " << (end-start)/iters << " ms\n";

    start = currentDateTime();
    for (uint64_t i = 0; i < iters; i++) {
        pir.BatchVerify(A, H, hash, ct_vec, ans_vec, Z, true);
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

    const uint64_t batchSize = 20;

    benchmark_batch_verisimplepir_online(N, d, batchSize, verbose);
}