#include "pir/pir.h"

void benchmark_simplepir_online (const uint64_t N, const uint64_t d, const bool verbose = false) {

    double start, end;

    std::cout << "Input params: N = " << N << " d = " << d << std::endl;
    std::cout << "database size: " << N*d / (8.0*(1ULL<<30)) << " GiB\n";

    const bool simplePIR = true;
    VLHEPIR pir(N, d, true, verbose, simplePIR, false);

    std::cout << "database params: "; pir.dbParams.print();

    PackedMatrix D_packed = packMatrixHardCoded(pir.dbParams.ell, pir.dbParams.m, pir.dbParams.p);

    const uint64_t index = 1;
    const uint64_t iters = 10;

  
    std::pair<Matrix, Matrix> ct_sk;
    {
        std::cout << "sampling random A matrix...\n";
        Matrix A = pir.FakeInit();
        start = currentDateTime();
        for (uint64_t i = 0; i < iters; i++) {
            ct_sk = pir.Query(A, index);
        }
        end = currentDateTime();
        std::cout << "Query generation time: " << (end-start)/iters << " ms\n";
    }

    Matrix ct = std::get<0>(ct_sk);
    Matrix sk = std::get<1>(ct_sk);

    Matrix ans;
    start = currentDateTime();
    for (uint64_t i = 0; i < iters; i++) {
        ans = pir.Answer(ct, D_packed);
    }
    end = currentDateTime();
    std::cout << "Answer generation time: " << (end-start)/iters << " ms\n";

    {
         Matrix H = pir.GenerateFakeHint();
        std::cout << "offline download size = " << H.rows*H.cols*sizeof(Elem) / (1ULL  << 20) << " MiB\n";

        entry_t res;
        start = currentDateTime();
        for (uint64_t i = 0; i < iters; i++) {
            res = pir.Recover(H, ans, sk, index);
        }
        end = currentDateTime();
        std::cout << "Recovery time: " << (end-start)/iters << " ms\n";
    }
}

int main() {
    // const uint64_t N = 1ULL<<30;
    const uint64_t N = 1ULL<<33;
    // const uint64_t N = 4*(1ULL<<33);
    // const uint64_t N = 8*(1ULL<<33);
    // const uint64_t N = 16*(1ULL<<33);
    // const uint64_t N = 32*(1ULL<<33);
    // const uint64_t N = 64*(1ULL<<33);
    // const uint64_t N = 128*(1ULL<<33);
    // const uint64_t N = 256*(1ULL<<33);
    // const uint64_t N = 1ULL<<35;
    // const uint64_t d = 2048;
    // const uint64_t d = 128;
    const uint64_t d = 1;

    // const bool verbose = true;
    const bool verbose = false;

    benchmark_simplepir_online(N, d, verbose);
}