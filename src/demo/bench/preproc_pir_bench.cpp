#include "pir/preproc_pir.h"

void benchmark_verisimplepir_offline_server_compute(const VeriSimplePIR& pir, const bool verbose = false) {

    double start, end;

    // Matrix D = pir.db.packDataInMatrix(pir.dbParams);
    std::cout << "sampling random db matrix...\n";
    Matrix D_T(pir.dbParams.m, pir.dbParams.ell);
    random_fast(D_T, pir.dbParams.p);
    std::cout << "sampled database.\n";

    const uint64_t index = 1;

    unsigned char preproc_hash[SHA256_DIGEST_LENGTH];
    // pir.HashAandH(preproc_hash, A_2, H_2);

    // Offline phase
    uint64_t iters = 1;

    std::cout << "generating fake client message...\n";
    const auto preproc_ct_sk_pair = pir.PreprocFakeClientMessage();

    const auto preproc_cts = std::get<0>(preproc_ct_sk_pair);
    const auto preproc_sks = std::get<1>(preproc_ct_sk_pair);

    std::cout << "beginning server compute benchmarks\n";

    std::vector<Multi_Limb_Matrix> preproc_res_cts;  //  = pir.PreprocAnswer(preproc_cts, D_T);
    start = currentDateTime();
    for (uint64_t i = 0; i < iters; i++) {
        preproc_res_cts = pir.PreprocAnswer(preproc_cts, D_T);
    }
    end = currentDateTime();
    std::cout << "Preproc answer generation time: " << (end-start)/iters << " ms\n";

    Matrix preproc_Z;
    start = currentDateTime();
    for (uint64_t i = 0; i < iters; i++) {
        preproc_Z = pir.PreprocProve(preproc_hash, preproc_cts, preproc_res_cts, D_T);
    }
    end = currentDateTime();
    std::cout << "Preproc proof generation time: " << (end-start)/iters << " ms\n";

    std::cout << std::endl;
}

void benchmark_verisimplepir_offline_client_compute(const VeriSimplePIR& pir, const bool verbose = false) {

    double start, end;

    const uint64_t index = 1;

    // std::cout << "sampling random A matrix...\n";
    Matrix A_1 = pir.FakeInit();
    Multi_Limb_Matrix A_2 = pir.PreprocFakeInit();

    Matrix H_1 = pir.GenerateFakeHint();
    Multi_Limb_Matrix H_2 = pir.PreprocGenerateFakeHint();

    unsigned char preproc_hash[SHA256_DIGEST_LENGTH];
    // pir.HashAandH(preproc_hash, A_2, H_2);

    // Offline phase
    uint64_t iters = 1;

    const BinaryMatrix C = pir.PreprocSampleC();

    std::cout << "generating client message...\n";
    const auto preproc_ct_sk_pair = pir.PreprocClientMessage(A_2, C);
    start = currentDateTime();
    for (uint64_t i = 0; i < iters; i++) {
        pir.PreprocClientMessage(A_2, C);
    }
    end = currentDateTime();
    std::cout << "Client message generation time: " << (end-start)/iters << " ms\n";

    const auto preproc_cts = std::get<0>(preproc_ct_sk_pair);
    const auto preproc_sks = std::get<1>(preproc_ct_sk_pair);

    const auto preproc_res_cts = pir.PreprocFakeAnswer();
    const auto preproc_Z = pir.PreprocFakeProve();

    const bool fake = true;
    start = currentDateTime();
    for (uint64_t i = 0; i < iters; i++) {
        pir.PreprocVerify(A_2, H_2, preproc_hash, preproc_cts, preproc_res_cts, preproc_Z, fake);
    }
    end = currentDateTime();
    std::cout << "Preproc verification time: " << (end-start)/iters << " ms\n";

    const Matrix Z = pir.PreprocRecoverZ(H_2, preproc_sks, preproc_res_cts);
    start = currentDateTime();
    for (uint64_t i = 0; i < iters; i++) {
        pir.PreprocRecoverZ(H_2, preproc_sks, preproc_res_cts);
    }
    end = currentDateTime();
    std::cout << "Preproc decryption time: " << (end-start)/iters << " ms\n";

    start = currentDateTime();
    for (uint64_t i = 0; i < iters; i++) {
        pir.VerifyPreprocZ(Z, A_1, C, H_1, fake);
    }
    end = currentDateTime();
    std::cout << "Precompute proof check time: " << (end-start)/iters << " ms\n";

    std::cout << std::endl;
}

void benchmark_verisimplepir_online(const VeriSimplePIR& pir, const bool verbose = false) {

    double start, end;

    std::cout << "database params: "; pir.dbParams.print();
    std::cout << "sampling random db matrix...\n";

    PackedMatrix D_packed = packMatrixHardCoded(pir.dbParams.ell, pir.dbParams.m, pir.dbParams.p);

    const uint64_t index = 1;

    std::cout << "sampling random A matrix...\n";
    Matrix A = pir.FakeInit();

    Matrix H = pir.GenerateFakeHint();
    std::cout << "offline download size = " << H.rows*H.cols*sizeof(Elem) / (1ULL << 20) << " MiB\n";

    auto C_and_Z = pir.SampleFakeCandZ();

    const BinaryMatrix C = std::get<0>(C_and_Z);
    const Matrix Z = std::get<1>(C_and_Z);

    auto ct_sk = pir.Query(A, index);
    uint64_t iters = 10;
    start = currentDateTime();
    for (uint64_t i = 0; i < iters; i++) {
        pir.Query(A, index);
    }
    end = currentDateTime();
    double query_time = (end-start)/iters;
    std::cout << "Query generation time: " << query_time << " ms\n";

    Matrix ct = std::get<0>(ct_sk);
    Matrix sk = std::get<1>(ct_sk);

    Matrix ans = pir.Answer(ct, D_packed);
    start = currentDateTime();
    for (uint64_t i = 0; i < iters; i++) {
        // Matrix ans = pir.Answer(ct, D);
        Matrix ans = pir.Answer(ct, D_packed);
    }
    end = currentDateTime();
    double answer_time = (end-start)/iters;
    std::cout << "Answer generation time: " << answer_time << " ms\n";

    start = currentDateTime();
    for (uint64_t i = 0; i < iters; i++) {
        pir.FakePreVerify(ct, ans, Z, C);
    }
    end = currentDateTime();
    double verify_time = (end-start)/iters;
    std::cout << "Verification time: " << verify_time << " ms\n";

    entry_t res;
    start = currentDateTime();
    for (uint64_t i = 0; i < iters; i++) {
        res = pir.Recover(H, ans, sk, index);
    }
    end = currentDateTime();
    double recovery_time = (end-start)/iters;
    std::cout << "Recovery time: " << recovery_time << " ms\n";

    double total_time = query_time + answer_time + verify_time + recovery_time;
    std::cout << "Total time: " << total_time << " ms\n";
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

    std::cout << "Input params: N = " << N << " d = " << d << std::endl;
    std::cout << "database size: " << N*d / (8.0*(1ULL<<20)) << " MiB\n";

    // const bool honest_hint = true;
    const bool honest_hint = false;
    VeriSimplePIR pir(
        N, d, 
        true,   // allowTrivial
        verbose, 
        false,  // SimplePIR
        false,  // random data
        1,   // batch size
        true,  // preprocessed
        honest_hint);

    std::cout << "database params: "; pir.dbParams.print();

    // benchmark_verisimplepir_offline_server_compute(pir, verbose);
    // benchmark_verisimplepir_offline_client_compute(pir, verbose);
    benchmark_verisimplepir_online(pir, verbose);
}