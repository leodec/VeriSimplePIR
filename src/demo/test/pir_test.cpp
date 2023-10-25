#include "pir/pir.h"
#include "pir/preproc_pir.h"

void basic_pir_test(const uint64_t N, const uint64_t d, const bool verbose = false) {

    // set simplepir to true
    VLHEPIR pir(N, d, true, verbose, true, true);
    std::cout << "database size: " << N*d / (8.0*(1ULL << 20)) << " MiB\n";
    pir.dbParams.print();

    Matrix D = pir.db.packDataInMatrix(pir.dbParams, verbose);
    // Matrix D(pir.ell, pir.m);
    // pir.lhe.randomPlaintext(D);
    // std::cout << "D =\n";
    // print(D);

    const uint64_t index = 1;
    const entry_t correct = pir.db.getDataAtIndex(index);

    Matrix A = pir.Init();

    Matrix H = pir.GenerateHint(A, D);

    auto ct_sk = pir.Query(A, index);

    Matrix ct = std::get<0>(ct_sk);
    Matrix sk = std::get<1>(ct_sk);

    Matrix ans = pir.Answer(ct, D);

    const entry_t res = pir.Recover(H, ans, sk, index);

    if (res != correct) {
        std::cout << "pir mismatch!\n";
        print(correct); std::cout << std::endl;
        print(res); std::cout << std::endl;
        assert(false);
    }

    std::cout << "Basic PIR test passed\n\n";
}

void basic_verifiable_pir_test(const uint64_t N, const uint64_t d, const bool verbose = false) {
    VLHEPIR pir(N, d, true, verbose);
    std::cout << "database size: " << N*d / (8.0*(1ULL << 20)) << " MiB\n";
    pir.dbParams.print();

    std::cout << "packing data into plaintext matrix...\n";
    Matrix D = pir.db.packDataInMatrix(pir.dbParams, verbose);
    // std::cout << "D =\n"; print(D);

    const uint64_t index = 1;
    const entry_t correct = pir.db.getDataAtIndex(index);

    Matrix A = pir.Init();

    std::cout << "generating hint...\n";
    Matrix H = pir.GenerateHint(A, D);

    unsigned char hash[SHA256_DIGEST_LENGTH];
    pir.HashAandH(hash, A, H);

    std::cout << "generating query...\n";
    auto ct_sk = pir.Query(A, index);

    Matrix ct = std::get<0>(ct_sk);
    Matrix sk = std::get<1>(ct_sk);

    std::cout << "generating answer...\n";
    Matrix ans = pir.Answer(ct, D);

    std::cout << "generating proof...\n";
    Matrix Z = pir.Prove(hash, ct, ans, D);

    std::cout << "verifying proof...\n";
    pir.Verify(A, H, hash, ct, ans, Z);

    std::cout << "recovering result...\n";
    entry_t res = pir.Recover(H, ans, sk, index);

    if (res != correct) {
        std::cout << "pir mismatch!\n";
        print(correct); std::cout << std::endl;
        print(res); std::cout << std::endl;
        assert(false);
    }

    std::cout << "Basic VLHE PIR test passed\n\n";
}


void basic_verifiable_pir_test_packed_db(const uint64_t N, const uint64_t d, const bool verbose = false) {
    VLHEPIR pir(N, d, true, verbose);
    std::cout << "database size: " << N*d / (8.0*(1ULL << 20)) << " MiB\n";
    pir.dbParams.print();

    std::cout << "packing data into plaintext matrix...\n";
    Matrix D = pir.db.packDataInMatrix(pir.dbParams, verbose);
    // std::cout << "D =\n";
    // print(D);

    PackedMatrix D_packed = packMatrixHardCoded(D, pir.lhe.p);

    const uint64_t index = 1;
    const entry_t correct = pir.db.getDataAtIndex(index);

    Matrix A = pir.Init();

    std::cout << "generating hint...\n";
    Matrix H = pir.GenerateHint(A, D);

    unsigned char hash[SHA256_DIGEST_LENGTH];
    pir.HashAandH(hash, A, H);

    std::cout << "generating query...\n";
    auto ct_sk = pir.Query(A, index);

    Matrix ct = std::get<0>(ct_sk);
    Matrix sk = std::get<1>(ct_sk);


    std::cout << "generating answer...\n";
    // Matrix ans = pir.Answer(ct, D);
    Matrix ans = pir.Answer(ct, D_packed);


    std::cout << "generating proof...\n";
    Matrix Z = pir.Prove(hash, ct, ans, D_packed);

    std::cout << "verifying proof...\n";
    pir.Verify(A, H, hash, ct, ans, Z);

    std::cout << "recovering result...\n";
    entry_t res = pir.Recover(H, ans, sk, index);

    if (res != correct) {
        std::cout << "pir mismatch!\n";
        print(correct); std::cout << std::endl;
        print(res); std::cout << std::endl;
        assert(false);
    }

    std::cout << "Basic VLHEPIR test with packed database passed\n\n";
}

void batch_verifiable_pir_test_packed_db(const uint64_t N, const uint64_t d, const bool verbose = false) {
    VLHEPIR pir(N, d, true, verbose);
    std::cout << "database size: " << N*d / (8.0*(1ULL << 20)) << " MiB\n";
    pir.dbParams.print();

    std::cout << "packing data into plaintext matrix...\n";
    Matrix D = pir.db.packDataInMatrix(pir.dbParams, verbose);
    // std::cout << "D =\n";
    // print(D);

    PackedMatrix D_packed = packMatrixHardCoded(D, pir.lhe.p);

    const std::vector<uint64_t> indices = {1, 10, 20};
    std::vector<entry_t> correct(indices.size());
    for (size_t i = 0; i < indices.size(); i++)
        correct[i] = pir.db.getDataAtIndex(indices[i]);

    Matrix A = pir.Init();

    std::cout << "generating hint...\n";
    Matrix H = pir.GenerateHint(A, D);

    unsigned char hash[SHA256_DIGEST_LENGTH];
    pir.HashAandH(hash, A, H);

    std::cout << "generating query...\n";
    auto ct_sk = pir.Query(A, indices);

    Matrix ct = std::get<0>(ct_sk);
    Matrix sk = std::get<1>(ct_sk);


    std::cout << "generating answer...\n";
    // Matrix ans = pir.Answer(ct, D);
    Matrix ans = pir.Answer(ct, D_packed);


    std::cout << "generating proof...\n";
    Matrix Z = pir.Prove(hash, ct, ans, D_packed);

    std::cout << "verifying proof...\n";
    pir.Verify(A, H, hash, ct, ans, Z);

    std::cout << "recovering result...\n";
    std::vector<entry_t> res = pir.Recover(H, ans, sk, indices);

    for (size_t i = 0; i < correct.size(); i++) {
        if (res[i] != correct[i]) {
            std::cout << "pir mismatch!\n";
            std::cout << "batch index " << i << " out of " << correct.size() << std::endl;
            std::cout << "db index: " << indices[i] << std::endl;
            print(correct[i]); std::cout << std::endl;
            print(res[i]); std::cout << std::endl;
            assert(false);
        }
    }

    std::cout << "Batch VLHEPIR test with packed database passed\n\n";
}


void basic_preproc_pir_test(const uint64_t N, const uint64_t d, const bool verbose = false) {

    // set simplepir to true
    VeriSimplePIR pir(N, d, true, verbose, false, true, 1, true);
    std::cout << "database size: " << N*d / (8.0*(1ULL << 20)) << " MiB\n";
    pir.dbParams.print();

    Matrix D = pir.db.packDataInMatrix(pir.dbParams, verbose);
    // Matrix D(pir.ell, pir.m);
    // pir.lhe.randomPlaintext(D);
    // std::cout << "D =\n";
    // print(D);

    const uint64_t index = 10;
    const entry_t correct = pir.db.getDataAtIndex(index);

    Matrix A = pir.Init();

    Matrix H = pir.GenerateHint(A, D);

    auto ct_sk = pir.Query(A, index);

    Matrix ct = std::get<0>(ct_sk);
    Matrix sk = std::get<1>(ct_sk);

    Matrix ans = pir.Answer(ct, D);

    const entry_t res = pir.Recover(H, ans, sk, index);

    if (res != correct) {
        std::cout << "pir mismatch!\n";
        print(correct); std::cout << std::endl;
        print(res); std::cout << std::endl;
        assert(false);
    }

    std::cout << "Basic preprocessed PIR test passed\n\n";
}

void full_preproc_pir_test(const uint64_t N, const uint64_t d, const bool verbose = false) {

    VeriSimplePIR pir(
        N, d, 
        true,   // allow preprocessing download to be larger than the database
        verbose, 
        false,  // SimplePIR
        true,   // randomData
        1,      // batch size
        true,    // preprocessed
        false   // honest hint 
    );
    std::cout << "database size: " << N*d / (8.0*(1ULL << 20)) << " MiB\n";
    pir.dbParams.print();

    Matrix D = pir.db.packDataInMatrix(pir.dbParams, verbose);
    Matrix D_T = transpose(D);
    PackedMatrix D_T_packed = packMatrixHardCoded(D_T, pir.dbParams.p);
    // Matrix D(pir.ell, pir.m);
    // pir.lhe.randomPlaintext(D);
    // std::cout << "D =\n";
    // print(D);

    const Matrix A = pir.Init();
    const Matrix H = pir.GenerateHint(A, D);

    // Preprocessing phase

    const Multi_Limb_Matrix A_2 = pir.PreprocInit();
    const Multi_Limb_Matrix H_2 = pir.PreprocGenerateHint(A_2, D_T);
    unsigned char preproc_hash[SHA256_DIGEST_LENGTH];
    pir.HashAandH(preproc_hash, A_2, H_2);

    const BinaryMatrix C = pir.PreprocSampleC();

    const Matrix correct_Z = matMulLeftBinary(C, D);

    pir.VerifyPreprocZ(correct_Z, A, C, H);
    std::cout << "plaintext Z verified\n";

    // const BinaryMatrix C_T = transpose(C);
    // const Matrix Z_T = matMulRightBinary(D_T, C_T);
    // const Matrix correct_Z_from_T = transpose(Z_T);

    // pir.VerifyPreprocZ(correct_Z_from_T, A, C, H);
    // std::cout << "plaintext Z from transpose verified\n";

    // assert(eq(correct_Z, correct_Z_from_T));

    const auto preproc_ct_sk_pair = pir.PreprocClientMessage(A_2, C);

    const auto preproc_cts = std::get<0>(preproc_ct_sk_pair);
    const auto preproc_sks = std::get<1>(preproc_ct_sk_pair);

    const auto preproc_res_cts = pir.PreprocAnswer(preproc_cts, D_T);

    const auto preproc_Z = pir.PreprocProve(preproc_hash, preproc_cts, preproc_res_cts, D_T);

    pir.PreprocVerify(A_2, H_2, preproc_hash, preproc_cts, preproc_res_cts, preproc_Z);

    const Matrix Z = pir.PreprocRecoverZ(H_2, preproc_sks, preproc_res_cts);

    if (!eq(Z, correct_Z, true)) {
        std::cout << "Z is not correct\n";
        std::cout << Z.rows << " x " << Z.cols << std::endl;
        print(Z);
        assert(false);
    }

    pir.VerifyPreprocZ(Z, A, C, H);

    // Online phase

    const uint64_t index = 1;
    const entry_t correct = pir.db.getDataAtIndex(index);

    auto ct_sk = pir.Query(A, index);

    Matrix ct = std::get<0>(ct_sk);
    Matrix sk = std::get<1>(ct_sk);

    Matrix ans = pir.Answer(ct, D);

    pir.PreVerify(ct, ans, Z, C);

    const entry_t res = pir.Recover(H, ans, sk, index);

    if (res != correct) {
        std::cout << "pir mismatch!\n";
        print(correct); std::cout << std::endl;
        print(res); std::cout << std::endl;
        assert(false);
    }

    std::cout << "Verified Preprocessed PIR test passed\n\n";
}


int main() {

    
    // const uint64_t N = 100;
    // const uint64_t d = 1;

    // const uint64_t N = 1ULL<<10;
    const uint64_t N = 1ULL<<20;
    const uint64_t d = 8;
    // const uint64_t d = 1;

    // const uint64_t N = 1<<10;
    // const uint64_t d = 2048;

    // const bool verbose = true;
    const bool verbose = false;

    basic_pir_test(N, d, verbose);
    basic_verifiable_pir_test(N, d, verbose);

    basic_preproc_pir_test(N, d, verbose);
    full_preproc_pir_test(N, d, verbose);


    // basic_verifiable_pir_test_packed_db(N, d);

    // batch_verifiable_pir_test_packed_db(N, d);
}