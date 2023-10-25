#include "preproc_pir.h"
#include <cmath>
#include <functional>

// Operations for preprocessing phase

// Sample A_2 for the preprocessing
Multi_Limb_Matrix VeriSimplePIR::PreprocInit() const {
    return preproc_lhe.genPublicA(ell);
};

Multi_Limb_Matrix VeriSimplePIR::PreprocFakeInit() const {
    Multi_Limb_Matrix A(ell, lhe.n);
    random_fast(A.q_data);
    random_fast(A.kappa_data, preproc_lhe.kappa);
    return A;
}

Multi_Limb_Matrix VeriSimplePIR::PreprocGenerateHint(const Multi_Limb_Matrix& A, const Matrix& D) const {
    if (D.rows != m || D.cols != ell) {
        std::cout << "database dimension mismatch! input should be D^T\n";
        assert(false);
    }

    Multi_Limb_Matrix H = matMul(D, A, preproc_lhe.kappa);
    return H;
}

Multi_Limb_Matrix VeriSimplePIR::PreprocGenerateFakeHint() const {
    Multi_Limb_Matrix H(m, lhe.n);
    random_fast(H.q_data);
    random_fast(H.kappa_data, preproc_lhe.kappa);
    return H;
}

// samples the plaintext C to be encrypted
BinaryMatrix VeriSimplePIR::PreprocSampleC() const {
    BinaryMatrix C(stat_sec_param, ell);
    random(C);
    // constant(C, 0); std::cout << "change me back\n";
    return C;
}

// Encrypts C with a fresh key. output is list of ciphertexts and secret keys pair  
std::pair<std::vector<Multi_Limb_Matrix>, std::vector<Multi_Limb_Matrix>>
VeriSimplePIR::PreprocClientMessage(
    const Multi_Limb_Matrix& A, const BinaryMatrix& C
) const {
    if (C.rows != stat_sec_param || C.cols != ell) {
        std::cout << "plaintext matrix dimension mismatch!\n";
        assert(false);
    }

    std::vector<Multi_Limb_Matrix> sks; 
    sks.reserve(C.rows);
    for (uint64_t i = 0; i < C.rows; i++)
        sks.push_back(preproc_lhe.sampleSecretKey());

    std::vector<Multi_Limb_Matrix> result_cts; result_cts.reserve(C.rows);
    // encrypt each row of C
    for (uint64_t row_ind = 0; row_ind < C.rows; row_ind++) {
        Matrix pt(C.cols, 1);
        for (uint64_t i = 0; i < C.cols; i++)
            pt.data[i] = C.data[row_ind*C.cols + i];

        Multi_Limb_Matrix ct = preproc_lhe.encrypt(A, sks[row_ind], pt);
        result_cts.push_back(ct);
    }

    assert(result_cts.size() == sks.size());

    return make_pair(result_cts, sks);
}

std::pair<std::vector<Multi_Limb_Matrix>, std::vector<Multi_Limb_Matrix>>
VeriSimplePIR::PreprocFakeClientMessage() const {
    std::vector<Multi_Limb_Matrix> sks; 
    sks.reserve(stat_sec_param);
    for (uint64_t i = 0; i < stat_sec_param; i++)
        sks.push_back(preproc_lhe.sampleSecretKey());

    std::vector<Multi_Limb_Matrix> result_cts; result_cts.reserve(sks.size());
    // encrypt each row of C
    for (uint64_t row_ind = 0; row_ind < sks.size(); row_ind++) {
        Multi_Limb_Matrix ct(ell, 1);
        random_fast(ct.q_data); 
        random_fast(ct.kappa_data, preproc_lhe.kappa); 
        result_cts.push_back(ct);
    }

    assert(result_cts.size() == sks.size());

    return make_pair(result_cts, sks);
}

// Just multiply input ciphertexts by D^T
std::vector<Multi_Limb_Matrix> VeriSimplePIR::PreprocAnswer(
    const std::vector<Multi_Limb_Matrix>& in_cts, 
    const Matrix& D
    // const PackedMatrix& D
) const {
    // if (D.orig_rows != m || D.orig_cols != ell) {
    //     std::cout << "plaintext matrix dimension mismatch!\n";
    //     assert(false);
    // }

    std::vector<Multi_Limb_Matrix> result_cts; 
    result_cts.reserve(in_cts.size());
    for (uint64_t i = 0; i < in_cts.size(); i++)
        result_cts.push_back(matMulVec(D, in_cts[i], preproc_lhe.kappa));
        // result_cts.push_back(matVecMulColPacked(D, in_cts[i], preproc_lhe.kappa));

    return result_cts;
}

// takes in one row of D^T and multiplies it m times
// used when database is too big for benchmarking machine
std::vector<Multi_Limb_Matrix> VeriSimplePIR::PreprocFakeComputeAnswer(
    const std::vector<Multi_Limb_Matrix>& in_cts, 
    const Matrix& D
) const {
    std::vector<Multi_Limb_Matrix> result_cts; 
    result_cts.reserve(in_cts.size());
    for (uint64_t i = 0; i < in_cts.size(); i++) {
        Multi_Limb_Matrix ct_elem(1, 1);
        for (uint64_t j = 0; j < m; j++) {
            ct_elem = matMulVec(D, in_cts[i], preproc_lhe.kappa);
        }
        Multi_Limb_Matrix ct(ell, 1);
        for (uint64_t j = 0; j < ell; j++) {
            ct.q_data.data[j] = ct_elem.q_data.data[0];
            ct.kappa_data.data[j] = ct_elem.kappa_data.data[0];
        }
        result_cts.push_back(ct);
    }

    assert(result_cts.size() == in_cts.size());
    return result_cts;
}


std::vector<Multi_Limb_Matrix> VeriSimplePIR::PreprocFakeAnswer() const {
    std::vector<Multi_Limb_Matrix> result_cts; 
    result_cts.reserve(stat_sec_param);
    for (uint64_t i = 0; i < stat_sec_param; i++) {
        Multi_Limb_Matrix ct(m, 1);
        random_fast(ct.q_data); 
        random_fast(ct.kappa_data, preproc_lhe.kappa); 
        result_cts.push_back(ct);
    }
    return result_cts;
}

void VeriSimplePIR::HashAandH(unsigned char * hash, const Multi_Limb_Matrix& A, const Multi_Limb_Matrix& H) const {
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, A.q_data.data, A.rows*A.cols*sizeof(Elem));
    SHA256_Update(&sha256, A.kappa_data.data, A.rows*A.cols*sizeof(Elem));
    SHA256_Update(&sha256, H.q_data.data, H.rows*H.cols*sizeof(Elem));
    SHA256_Update(&sha256, H.kappa_data.data, H.rows*H.cols*sizeof(Elem));
    SHA256_Final(hash, &sha256);
}

// This is the C used to prove the correctness of the preprocessed computation. 
// The dimension is lambda x m
BinaryMatrix VeriSimplePIR::BatchHashToC(const unsigned char * AandHhash, const std::vector<Multi_Limb_Matrix>& u_vec, const std::vector<Multi_Limb_Matrix>& v_vec) const {
    
    assert(u_vec.size() == v_vec.size());

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);

    for (uint64_t i = 0; i < u_vec.size(); i++) {
        SHA256_Update(&sha256, u_vec[i].q_data.data, u_vec[i].rows*u_vec[i].cols*sizeof(Elem));
        SHA256_Update(&sha256, u_vec[i].kappa_data.data, u_vec[i].rows*u_vec[i].cols*sizeof(Elem));
        SHA256_Update(&sha256, v_vec[i].q_data.data, v_vec[i].rows*v_vec[i].cols*sizeof(Elem));
        SHA256_Update(&sha256, v_vec[i].kappa_data.data, v_vec[i].rows*v_vec[i].cols*sizeof(Elem));
    }
    SHA256_Final(hash, &sha256);

    SeedType seed = osuCrypto::toBlock(hash) ^ osuCrypto::toBlock(hash + 16);
    // memcpy((unsigned char *)seed, hash, sizeof(seed)); 
    // std::cout << "Dummy C seed!\n";
    BinaryMatrix C(stat_sec_param, m);
    pseudorandom(C, seed);
    return C;
}

// Prove that the computation in PreprocAnswer was correct.
Matrix VeriSimplePIR::PreprocProve(
    const unsigned char * hash,
    const std::vector<Multi_Limb_Matrix>& u, const std::vector<Multi_Limb_Matrix>& v, 
    // const PackedMatrix& D
    const Matrix& D
) const {
    BinaryMatrix C = BatchHashToC(hash, u, v);
    // Matrix Z = matMulLeftBinaryRightColPacked_Hardcoded(C, D);
    Matrix Z = matMulLeftBinary(C, D);
    return Z;
}

Matrix VeriSimplePIR::PreprocFakeProve() const {
    Matrix Z(stat_sec_param, ell);
    random(Z, lhe.p*m);
    return Z;
}

// Verifies preprocessed Z
void VeriSimplePIR::PreprocVerify(
    const Multi_Limb_Matrix& A, const Multi_Limb_Matrix& H, 
    const unsigned char * hash,
    const std::vector<Multi_Limb_Matrix>& u, const std::vector<Multi_Limb_Matrix>& v, 
    const Matrix& Z, const bool fake
) const {
    const size_t norm_bound = lhe.p*m;  // D^T rows have length m
    const size_t Z_len = Z.rows * Z.cols;
    for (size_t i = 0; i < Z_len; i++) {
        if (Z.data[i] >= norm_bound) {
            if (!fake){
                std::cout << "Z is too big!\n";
                assert(false);
            }
        }
    }

    BinaryMatrix C = BatchHashToC(hash, u, v);
    
    Multi_Limb_Matrix leftMatFixed = matMul(Z, A, preproc_lhe.kappa);
    Multi_Limb_Matrix rightMatFixed = matMulLeftBinary(C, H, preproc_lhe.kappa);
    if (!eq(leftMatFixed, rightMatFixed, !fake)) {
        if (!fake){
            std::cout << "preproc verify mismatch!\n";
            assert(false);
        }
    }

    for (uint64_t i = 0; i < u.size(); i++) {
        Multi_Limb_Matrix leftMatQ = matMul(Z, u[i], preproc_lhe.kappa);
        Multi_Limb_Matrix rightMatQ = matMulLeftBinary(C, v[i], preproc_lhe.kappa);
        if (!eq(leftMatQ, rightMatQ, !fake)) {
            if (!fake){
                std::cout << "preproc verify mismatch!\n";
                assert(false);
            }
        }
    }
}

// Decrypts result Z
// Check Z against A_1 and plaintext C
Matrix VeriSimplePIR::PreprocRecoverZ(
    const Multi_Limb_Matrix& H_2,
    const std::vector<Multi_Limb_Matrix>& sks,
    const std::vector<Multi_Limb_Matrix>& res_ct
) const {
    assert(res_ct.size() == stat_sec_param);
    assert(res_ct.size() == sks.size());
    Matrix Z(stat_sec_param, m);

    for (uint64_t row_ind = 0; row_ind < res_ct.size(); row_ind++) {
        Matrix Z_row = preproc_lhe.decrypt(H_2, sks[row_ind], res_ct[row_ind]);
        assert(Z_row.rows == m); assert(Z_row.cols == 1);
        
        for (uint64_t col_ind = 0; col_ind < m; col_ind++)
            Z.data[row_ind*m + col_ind] = Z_row.data[col_ind];
    }

    return Z;
}

void VeriSimplePIR::VerifyPreprocZ(
    const Matrix& Z,
    const Matrix& A_1, const BinaryMatrix& C, const Matrix& H_1,
    const bool fake
) const {
    // Verify Z against A_1 and H_1
    // std::cout << "Z = "; print(Z); 

    const size_t norm_bound = lhe.p*ell; 
    const size_t Z_len = Z.rows * Z.cols;
    for (size_t i = 0; i < Z_len; i++) {
        if (Z.data[i] >= norm_bound && !fake) {
            std::cout << "Z is too big!\n";
            assert(false);
        }
    }

    Matrix leftMat = matMul(Z, A_1);
    Matrix rightMat = matMulLeftBinary(C, H_1);
    if (!eq(leftMat, rightMat) && !fake) {
        std::cout << "plaintext verify mismatch!\n";
        assert(false);
    }
}


// Operations for online phase

Matrix VeriSimplePIR::Init() const {
    return lhe.genPublicA(m);
}

Matrix VeriSimplePIR::FakeInit() const {
    // return lhe.genPublicA(m);
    Matrix A(m, lhe.n);
    random_fast(A);
    return A;
}

Matrix VeriSimplePIR::GenerateHint(const Matrix& A, const Matrix& D) const {
    if (D.rows != ell || D.cols != m) {
        std::cout << "database dimension mismatch!\n";
        assert(false);
    }

    Matrix H = matMul(D, A);
    return H;
}

Matrix VeriSimplePIR::GenerateFakeHint() const {
    Matrix H(ell, lhe.n);
    random_fast(H);
    return H;
}

std::pair<Matrix, Matrix> VeriSimplePIR::Query(const Matrix& A, const uint64_t index) const {
    if (index >= N) {
        std::cout << "index out of range!\n";
        assert(false);
    }

    const uint64_t index_col = dbParams.indexToColumn(index);
    // std::cout << "Query index column = " << index_col << std::endl;

    Matrix pt(m, 1);
    constant(pt, 0);
    pt.data[index_col] = 1;

    Matrix secretKey = lhe.sampleSecretKey();

    Matrix ciphertext = lhe.encrypt(A, secretKey, pt);

    return std::make_pair(ciphertext, secretKey);
}

Matrix VeriSimplePIR::Answer(const Matrix& ciphertext, const Matrix& D) const {
    if (ciphertext.cols == 1) {
        Matrix ans = matMulVec(D, ciphertext);
        return ans;
    } else {
        Matrix ans = matMul(D, ciphertext);
        return ans;
    }
}

Matrix VeriSimplePIR::Answer(const Matrix& ciphertext, const PackedMatrix& D_packed) const {
    if (ciphertext.cols == 1) {
        Matrix ans = simplepir_matVecMulColPacked_variableCompression(D_packed, ciphertext);
        return ans;
    } else {
        Matrix ans = matMulColPacked(D_packed, ciphertext);
        return ans;
    }
}

void VeriSimplePIR::PreVerify(const Matrix& u, const Matrix& v, const Matrix& Z, const BinaryMatrix& C, const bool fake) const {
    const auto left = matMulVec(Z, u);
    const auto right = matBinaryMulVec(C, v);
    if (!eq(left, right)) {
        if (!fake) {
            std::cout << "verify mismatch!\n";
            assert(false);
        }
    }
    return;
}

void VeriSimplePIR::FakePreVerify(const Matrix& u, const Matrix& v, const Matrix& Z, const BinaryMatrix& C) const {
    PreVerify(u, v, Z, C, true);
}

std::pair<BinaryMatrix, Matrix> VeriSimplePIR::SampleFakeCandZ() const {
    // C is binary with dimension lambda x ell
    // Z is random lambda x m with inf norm at most p*ell
    BinaryMatrix C(stat_sec_param, ell); random(C);
    Matrix Z(stat_sec_param, m); random(Z, lhe.p*ell);

    return std::make_pair(C, Z);
}

entry_t VeriSimplePIR::Recover(const Matrix& hint, const Matrix& ciphertext, const Matrix& secretKey, const uint64_t index) const {
    // const uint64_t index_row = index / m;
    const uint64_t index_row = dbParams.indexToRow(index);
    // std::cout << "Recover index row = " << index_row << std::endl;
    if (index_row >= ell) {
        std::cout << "index row is too big!\n";
        assert(false);
    }
  
    Matrix pt = lhe.decrypt(hint, secretKey, ciphertext);
    // std::cout << "Recover pt =\n"; print(pt);
    return dbParams.recover(&pt.data[index_row], index);
}