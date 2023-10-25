#include "database.h"
#include <utility>
#include <openssl/sha.h>


class VLHEPIR {
public:

    static constexpr uint64_t stat_sec_param = STAT_SEC_PARAM;
    
    uint64_t N, d;  // database has N entries of length d
    Database db;
    PlaintextDBParams dbParams;
    LHE lhe;
    uint64_t m, ell;  // dimensions of the database

    // const bool simplePIR = false;
    // SHA256_CTX sha256;  // Fiat Shamir hash

    VLHEPIR(const uint64_t N_in, const uint64_t d_in,
        const bool allowTrivial = true, const bool verbose = false, 
        const bool simplePIR = false,
        const bool randomData = true, 
        const uint64_t batchSize = 1,
        const bool honestHint = false
    ) : N(N_in), d(d_in), db(N_in, d_in)
    {
        dbParams = db.computeParams(allowTrivial, verbose, simplePIR, batchSize, false, honestHint);
        lhe = LHE(dbParams.p);
        m = dbParams.m;
        ell = dbParams.ell;
        if (!simplePIR)
            checkSISHardness();

        if (randomData) db.loadRandomData();
    };

    void checkSISHardness() {
        const float left = float(2*ell * lhe.p) * std::sqrt(float(m));
        const float right = std::exp2(2*std::sqrt(float(lhe.n * lhe.logq * std::log2(1.005))));
        if ((left > right) || (std::log2(left) > lhe.logq)) {
            std::cout << "SIS is not hard!\n";
            assert(false);
        }
    }

    Matrix Init() const;  // Sample public parameters
    Matrix FakeInit() const;
    
    Matrix GenerateHint(const Matrix& A, const Matrix& D) const;
    Matrix GenerateFakeHint() const;

    void HashAandH(unsigned char * hash, const Matrix& A, const Matrix& H) const;

    std::pair<Matrix, Matrix> Query(const Matrix& A, const uint64_t index) const;  
    // batch query. output is still ciphertext and secret key pair  
    std::pair<Matrix, Matrix> Query(const Matrix& A, const std::vector<uint64_t> indices) const;
    
    Matrix Answer(const Matrix& ciphertext, const Matrix& D) const;
    Matrix Answer(const Matrix& ciphertext, const PackedMatrix& D_packed) const;

    // Matrix HashToC(const Matrix& A, const Matrix& H, const Matrix& u, const Matrix& v);
    BinaryMatrix HashToC(const unsigned char * AandHhash, const Matrix& u, const Matrix& v) const;

    BinaryMatrix BatchHashToC(const unsigned char * AandHhash, const std::vector<Matrix>& u_vec, const std::vector<Matrix>& v_vec) const;
    
    Matrix Prove(
        // const Matrix& A, const Matrix& H, 
        const unsigned char * hash,
        const Matrix& u, const Matrix& v, 
        const Matrix& D) const;

    Matrix Prove(
        // const Matrix& A, const Matrix& H, 
        const unsigned char * hash,
        const Matrix& u, const Matrix& v, 
        const PackedMatrix& D) const;

    Matrix BatchProve(
        // const Matrix& A, const Matrix& H, 
        const unsigned char * hash,
        const std::vector<Matrix>& u, const std::vector<Matrix>& v, 
        const PackedMatrix& D) const;

    void Verify(
        const Matrix& A, const Matrix& H, 
        const unsigned char * hash,
        const Matrix& u, const Matrix& v, 
        const Matrix& Z, const bool fake = false) const;
    void FakeVerify(
        const Matrix& A, const Matrix& H, 
        const unsigned char * hash,
        const Matrix& u, const Matrix& v, 
        const Matrix& Z) const;

    void BatchVerify(
        const Matrix& A, const Matrix& H, 
        const unsigned char * hash,
        const std::vector<Matrix>& u, const std::vector<Matrix>& v, 
        const Matrix& Z,
        const bool fake) const;

    entry_t Recover(
        const Matrix& hint, const Matrix& ciphertext, 
        const Matrix& secretKey, const uint64_t index) const;
    // batch recover
    std::vector<entry_t> Recover(
        const Matrix& hint, const Matrix& ciphertext, 
        const Matrix& secretKey, const std::vector<uint64_t> indices) const;
};


