#include "pir.h"
#include <cmath>
#include <functional>

Matrix VLHEPIR::Init() const {
    return lhe.genPublicA(m);
}

Matrix VLHEPIR::FakeInit() const {
    // return lhe.genPublicA(m);
    Matrix A(m, lhe.n);
    random_fast(A);
    return A;
}

Matrix VLHEPIR::GenerateHint(const Matrix& A, const Matrix& D) const {
    if (D.rows != ell || D.cols != m) {
        std::cout << "database dimension mismatch!\n";
        assert(false);
    }

    Matrix H = matMul(D, A);
    return H;
}

Matrix VLHEPIR::GenerateFakeHint() const {
    Matrix H(ell, lhe.n);
    random_fast(H);
    return H;
}

void VLHEPIR::HashAandH(unsigned char * hash, const Matrix& A, const Matrix& H) const {
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, A.data, A.rows*A.cols*sizeof(Elem));
    SHA256_Update(&sha256, H.data, H.rows*H.cols*sizeof(Elem));
    SHA256_Final(hash, &sha256);
}



std::pair<Matrix, Matrix> VLHEPIR::Query(const Matrix& A, const uint64_t index) const {
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

std::pair<Matrix, Matrix> VLHEPIR::Query(const Matrix& A, const std::vector<uint64_t> indices) const {
    // ciphertext and secret keys are column vectors
    // load data as rows of matrices, then take the transpose at the end

    const uint64_t m = A.rows;

    Matrix ct_transpose(indices.size(), m);
    Matrix sk_transpose(indices.size(), lhe.n);

    for (size_t query_ind = 0; query_ind < indices.size(); query_ind++) {
        auto ct_and_sk = Query(A, indices[query_ind]);
        const Matrix ct = std::get<0>(ct_and_sk);
        const Matrix sk = std::get<1>(ct_and_sk);

        for (uint64_t ct_data_ind = 0; ct_data_ind < m; ct_data_ind++)
            ct_transpose.data[m*query_ind + ct_data_ind] = ct.data[ct_data_ind];
        for (uint64_t sk_data_ind = 0; sk_data_ind < lhe.n; sk_data_ind++)
            sk_transpose.data[lhe.n*query_ind + sk_data_ind] = sk.data[sk_data_ind];
    }

    return std::make_pair(transpose(ct_transpose), transpose(sk_transpose));
}

Matrix VLHEPIR::Answer(const Matrix& ciphertext, const Matrix& D) const {
    if (ciphertext.cols == 1) {
        Matrix ans = matMulVec(D, ciphertext);
        return ans;
    } else {
        Matrix ans = matMul(D, ciphertext);
        return ans;
    }
}

Matrix VLHEPIR::Answer(const Matrix& ciphertext, const PackedMatrix& D_packed) const {
    if (ciphertext.cols == 1) {
        Matrix ans = simplepir_matVecMulColPacked_variableCompression(D_packed, ciphertext);
        return ans;
    } else {
        Matrix ans = matMulColPacked(D_packed, ciphertext);
        return ans;
    }
}


BinaryMatrix VLHEPIR::HashToC(const unsigned char * AandHhash, const Matrix& u, const Matrix& v) const {
    
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, u.data, u.rows*u.cols*sizeof(Elem));
    SHA256_Update(&sha256, v.data, v.rows*v.cols*sizeof(Elem));
    SHA256_Final(hash, &sha256);

    SeedType seed = osuCrypto::toBlock(hash) ^ osuCrypto::toBlock(hash + 16);
    // memcpy((unsigned char *)seed, hash, sizeof(seed)); 
    // std::cout << "Dummy C seed!\n";
    BinaryMatrix C(stat_sec_param, ell);
    pseudorandom(C, seed);
    return C;
}

BinaryMatrix VLHEPIR::BatchHashToC(const unsigned char * AandHhash, const std::vector<Matrix>& u_vec, const std::vector<Matrix>& v_vec) const {
    
    assert(u_vec.size() == v_vec.size());

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);

    for (uint64_t i = 0; i < u_vec.size(); i++) {
        SHA256_Update(&sha256, u_vec[i].data, u_vec[i].rows*u_vec[i].cols*sizeof(Elem));
        SHA256_Update(&sha256, v_vec[i].data, v_vec[i].rows*v_vec[i].cols*sizeof(Elem));
    }
    SHA256_Final(hash, &sha256);

    SeedType seed = osuCrypto::toBlock(hash) ^ osuCrypto::toBlock(hash + 16);
    // memcpy((unsigned char *)seed, hash, sizeof(seed)); 
    // std::cout << "Dummy C seed!\n";
    BinaryMatrix C(stat_sec_param, ell);
    pseudorandom(C, seed);
    return C;
}

// Proves knowledge of solution to D * [A u] = [H v]
// returns Z such that Z * [A u] = C * [H v] 
Matrix VLHEPIR::Prove(
    // const Matrix& A, const Matrix& H, 
    const unsigned char * hash,
    const Matrix& u, const Matrix& v, 
    const Matrix& D
) const {
    BinaryMatrix C = HashToC(hash, u, v);
    Matrix Z = matMulLeftBinary(C, D);
    return Z;
}

Matrix VLHEPIR::Prove(
    // const Matrix& A, const Matrix& H, 
    const unsigned char * hash,
    const Matrix& u, const Matrix& v, 
    const PackedMatrix& D
) const {
    BinaryMatrix C = HashToC(hash, u, v);
    Matrix Z = matMulLeftBinaryRightColPacked_Hardcoded(C, D);
    return Z;
}

Matrix VLHEPIR::BatchProve(
    const unsigned char * hash,
    const std::vector<Matrix>& u, const std::vector<Matrix>& v, 
    const PackedMatrix& D
) const {
    BinaryMatrix C = BatchHashToC(hash, u, v);
    Matrix Z = matMulLeftBinaryRightColPacked_Hardcoded(C, D);
    return Z;
}

void VLHEPIR::Verify(
    const Matrix& A, const Matrix& H, 
    const unsigned char * hash,
    const Matrix& u, const Matrix& v, 
    const Matrix& Z,
    const bool fake
) const {

    const size_t norm_bound = lhe.p*ell;
    const size_t Z_len = Z.rows * Z.cols;
    for (size_t i = 0; i < Z_len; i++) {
        if (Z.data[i] >= norm_bound) {
            if (!fake){
                std::cout << "Z is too big!\n";
                assert(false);
            }
        }
    }

    BinaryMatrix C = HashToC(hash, u, v);
    
    // Matrix leftMat = matMulAppendVec(Z, A, u);
    // Matrix rightMat = binaryMatMulAppendVec(C, H, v);
    Matrix leftMatFixed = matMul(Z, A);
    Matrix rightMatFixed = matMulLeftBinary(C, H);
    if (!eq(leftMatFixed, rightMatFixed, !fake)) {
        if (!fake){
            std::cout << "verify mismatch!\n";
            assert(false);
        }
    }

    Matrix leftMatQ = matMul(Z, u);
    Matrix rightMatQ = matMulLeftBinary(C, v);
    if (!eq(leftMatQ, rightMatQ, !fake)) {
        if (!fake){
            std::cout << "verify mismatch!\n";
            assert(false);
        }
    }
}

void VLHEPIR::BatchVerify(
    const Matrix& A, const Matrix& H, 
    const unsigned char * hash,
    const std::vector<Matrix>& u_vec, const std::vector<Matrix>& v_vec, 
    const Matrix& Z,
    const bool fake
) const {

    const size_t norm_bound = lhe.p*ell;
    const size_t Z_len = Z.rows * Z.cols;
    for (size_t i = 0; i < Z_len; i++) {
        if (Z.data[i] >= norm_bound) {
            if (!fake){
                std::cout << "Z is too big!\n";
                assert(false);
            }
        }
    }

    BinaryMatrix C = BatchHashToC(hash, u_vec, v_vec);
    
    // Matrix leftMat = matMulAppendVec(Z, A, u);
    // Matrix rightMat = binaryMatMulAppendVec(C, H, v);
    Matrix leftMatFixed = matMul(Z, A);
    Matrix rightMatFixed = matMulLeftBinary(C, H);
    if (!eq(leftMatFixed, rightMatFixed, !fake)) {
        if (!fake){
            std::cout << "verify mismatch!\n";
            assert(false);
        }
    }

    for (uint64_t i = 0; i < u_vec.size(); i++) {
        Matrix leftMatQ = matMul(Z, u_vec[i]);
        Matrix rightMatQ = matMulLeftBinary(C, v_vec[i]);
        if (!eq(leftMatQ, rightMatQ, !fake)) {
            if (!fake){
                std::cout << "verify mismatch!\n";
                assert(false);
            }
        }
    }
}

void VLHEPIR::FakeVerify(
    const Matrix& A, const Matrix& H, 
    const unsigned char * hash,
    const Matrix& u, const Matrix& v, 
    const Matrix& Z
) const {
    // Same as real verify but does not crash when it fails
    Verify(A, H, hash, u, v, Z, true);
}

entry_t VLHEPIR::Recover(const Matrix& hint, const Matrix& ciphertext, const Matrix& secretKey, const uint64_t index) const {
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

std::vector<entry_t> VLHEPIR::Recover(const Matrix& hint, const Matrix& ciphertext_batch, const Matrix& secretKey_batch, const std::vector<uint64_t> indices) const {
    // individual ciphertexts and keys should be columns
    if (indices.size() != ciphertext_batch.cols || indices.size() != secretKey_batch.cols) {
        assert(false);
    }

    std::vector<entry_t> result(indices.size());
    for (size_t query_ind = 0; query_ind < indices.size(); query_ind++) {
        const Matrix ct = ciphertext_batch.getColumn(query_ind);
        const Matrix sk = secretKey_batch.getColumn(query_ind);
        result[query_ind] = Recover(hint, ct, sk, indices[query_ind]);
    }
    return result;
}
