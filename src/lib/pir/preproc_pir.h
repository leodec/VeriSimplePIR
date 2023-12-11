#pragma once

#include "database.h"
#include "multilimb_lhe.h"
#include <utility>
#include <openssl/sha.h>


class VeriSimplePIR {
public:

    static constexpr uint64_t stat_sec_param = STAT_SEC_PARAM;
    
    uint64_t N, d;  // database has N entries of length d
    Database db;
    PlaintextDBParams dbParams;
    uint64_t m, ell;  // dimensions of the database

    Multi_Limb_LHE preproc_lhe;  // lhe for the offline phase
    LHE lhe;  // lhe for online phase

    // const bool simplePIR = false;
    // SHA256_CTX sha256;  // Fiat Shamir hash

    VeriSimplePIR(const uint64_t N_in, const uint64_t d_in,
        const bool allowTrivial = true, const bool verbose = false, 
        const bool simplePIR = false,
        const bool randomData = true, 
        const uint64_t batchSize = 1,
        const bool preproc = false,
        const bool honestHint = false
    ) : N(N_in), d(d_in), db(N_in, d_in)
    {
        dbParams = db.computeParams(allowTrivial, verbose, simplePIR, batchSize, preproc, honestHint);
        lhe = LHE(dbParams.p);

        m = dbParams.m;
        ell = dbParams.ell;

        // q * q' ≥ σ · 2`2 p2√2m ln(2/δ).

        // should essentially just be ell since ell = m
        Elem kappa = (Elem)std::ceil(std::sqrt(m * ell));
        if (kappa % 2 == 0) kappa += 1;

        // std::cout << "ell, m, kappa: " << ell << " " << m << " " << kappa << std::endl; 

        preproc_lhe = Multi_Limb_LHE(dbParams.p*ell, kappa);

        if (!simplePIR)
            checkSISHardness();

        if (randomData) db.loadRandomData();
        // std::cout << "change me back!\n";
        // db.loadConstantData(1);
    };

    void checkSISHardness() {
        const float left = float(2*ell * lhe.p) * std::sqrt(float(m));
        const float right = std::exp2(2*std::sqrt(float(lhe.n * lhe.logq * std::log2(1.005))));
        if ((left > right) || (std::log2(left) > lhe.logq)) {
            std::cout << "SIS is not hard!\n";
            assert(false);
        }
    }

    // Preprocessing Operations

    Multi_Limb_Matrix PreprocInit() const;  // Sample A_2 for the preprocessing
    Multi_Limb_Matrix PreprocFakeInit() const;

    Multi_Limb_Matrix PreprocGenerateHint(const Multi_Limb_Matrix& A, const Matrix& D) const;
    Multi_Limb_Matrix PreprocGenerateFakeHint() const;

    // this samples the plaintext C to be used in the online phase
    BinaryMatrix PreprocSampleC() const;

    // Encrypts C with a fresh key. output is list of ciphertexts and secret keys pair  
    std::pair<std::vector<Multi_Limb_Matrix>, std::vector<Multi_Limb_Matrix>> 
    PreprocClientMessage(const Multi_Limb_Matrix& A, const BinaryMatrix& C) const;

    std::pair<std::vector<Multi_Limb_Matrix>, std::vector<Multi_Limb_Matrix>>
    PreprocFakeClientMessage() const;

    std::vector<Multi_Limb_Matrix> PreprocAnswer(
        const std::vector<Multi_Limb_Matrix>& ciphertext, 
        // const PackedMatrix& D
        const Matrix& D
    ) const;

    std::vector<Multi_Limb_Matrix> PreprocFakeComputeAnswer(
        const std::vector<Multi_Limb_Matrix>& in_cts, 
        const Matrix& D
    ) const;

    std::vector<Multi_Limb_Matrix> PreprocFakeAnswer() const;

    void HashAandH(unsigned char * hash, const Multi_Limb_Matrix& A, const Multi_Limb_Matrix& H) const;

    // this generates the C used to prove the correctness of the preprocessing
    // BinaryMatrix VeriSimplePIR::HashToC(const unsigned char * AandHhash, const Matrix& u, const Matrix& v) const;
    BinaryMatrix BatchHashToC(const unsigned char * AandHhash, const std::vector<Multi_Limb_Matrix>& u_vec, const std::vector<Multi_Limb_Matrix>& v_vec) const;

    // Prove that the computation in PreprocAnswer was correct.
    Matrix PreprocProve(
        const unsigned char * hash,
        const std::vector<Multi_Limb_Matrix>& u, const std::vector<Multi_Limb_Matrix>& v, 
        // const PackedMatrix& D
        const Matrix& D
    ) const;

    Matrix PreprocFakeProve() const;

    // Verifies preprocessed Z
    void PreprocVerify(
        const Multi_Limb_Matrix& A, const Multi_Limb_Matrix& H, 
        const unsigned char * hash,
        const std::vector<Multi_Limb_Matrix>& u, const std::vector<Multi_Limb_Matrix>& v, 
        const Matrix& Z, const bool fake = false) const;

    // Decrypts result Z
    Matrix PreprocRecoverZ(
        const Multi_Limb_Matrix& H_2,
        const std::vector<Multi_Limb_Matrix>& sks,
        const std::vector<Multi_Limb_Matrix>& res_ct
    ) const;

    void VerifyPreprocZ(
        const Matrix& Z,
        const Matrix& A_1, const BinaryMatrix& C, const Matrix& H_1,
        const bool fake = false
    ) const;


    // Operations for Online Phase
    // Fake operations are used in benchmarking

    Matrix Init() const;  // Sample public parameters
    Matrix FakeInit() const;
    
    Matrix GenerateHint(const Matrix& A, const Matrix& D) const;
    Matrix GenerateFakeHint() const;

    std::pair<Matrix, Matrix> Query(const Matrix& A, const uint64_t index) const;  
    // batch query. output is still ciphertext and secret key pair  
    // std::pair<Matrix, Matrix> Query(const Matrix& A, const std::vector<uint64_t> indices) const;
    
    Matrix Answer(const Matrix& ciphertext, const Matrix& D) const;
    Matrix Answer(const Matrix& ciphertext, const PackedMatrix& D_packed) const;

    void PreVerify(const Matrix& u, const Matrix& v, const Matrix& Z, const BinaryMatrix& C, const bool fake = false) const;
    void FakePreVerify(const Matrix& u, const Matrix& v, const Matrix& Z, const BinaryMatrix& C) const;

    std::pair<BinaryMatrix, Matrix> SampleFakeCandZ() const;

    entry_t Recover(
        const Matrix& hint, const Matrix& ciphertext, 
        const Matrix& secretKey, const uint64_t index) const;
};


