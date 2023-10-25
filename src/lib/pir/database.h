#pragma once

#include "lhe.h"
#include "mat_packed.h"
#include "bigint/BigUnsigned.h"

typedef BigUnsigned entry_t;

void print(entry_t val);

entry_t reconstruct_base_p(const Elem * vals, const uint64_t num_digits, const Elem p);

uint64_t get_per_query_communication_in_bits(const uint64_t m, const uint64_t ell, const uint64_t logp, const bool verbose=false, const uint64_t batch_size = 1);
uint64_t get_simplepir_per_query_communication_in_bits(const uint64_t m, const uint64_t ell, const uint64_t logp, const bool verbose=false,  const uint64_t batch_size = 1);

struct PlaintextDBParams {
    uint64_t N, d;
    Elem p;  // plaintext modulus
    uint64_t ell, m;  // db matrix is ell x m
    // m is size of upload, ell is size of download

    void print() const {
        std::cout << "Plaintext db params: m = " << m 
            << ", ell = " << ell << ", p = " << p << " = 2^" << log2(p) << std::endl; 
    };

    uint64_t indexToColumn(const uint64_t i) const {
        // maps index to column for a query
        // get number of elements per column

        if (floor(log2(p)) >= d) {
            // at least one data element per Zp element
            const uint64_t elems_per_Zp = floor(log2(p))/d;
            const uint64_t elems_per_column = elems_per_Zp * ell;
            return i / elems_per_column;
        } else {
            // get the column with the data
            const uint64_t Zp_vals_per_elem = ceil(double(d) / log2(p));
            const uint64_t elems_per_column = ell / Zp_vals_per_elem;
            return i / elems_per_column;
        }
    }

    uint64_t indexToRow(const uint64_t i) const {
        uint64_t row;
        if (floor(log2(p)) >= d) {
            // at least one data element per Zp element
            const uint64_t elems_per_Zp = floor(log2(p))/d;
            // const uint64_t elems_per_column = elems_per_Zp * ell;
            row = (i/elems_per_Zp) % ell;
        } else {
            // get the column with the data
            const uint64_t Zp_vals_per_elem = ceil(double(d) / log2(p));
            const uint64_t elems_per_column = ell / Zp_vals_per_elem;
            row = (i % elems_per_column)*Zp_vals_per_elem;  // need to get row offset for split entries
        }
        assert(row < ell);
        return row;
    }

    entry_t recover(const Elem * start, const uint64_t i) const {
        // reconstructs entry_t from Elem column
         if (floor(log2(p)) >= d) {
            // std::cout << "logp is larger than d\n";
            const uint64_t elems_per_Zp = floor(log2(p))/d;
            // std::cout << "elems per Zp = " << elems_per_Zp << std::endl;
            const uint64_t innerEntry = i % elems_per_Zp;
            // std::cout << "inner entry index " << innerEntry << std::endl;
            const entry_t mask = (entry_t(1) << d) - entry_t(1);
            return entry_t(start[0] >> d*innerEntry) & mask;
        } else {
            const uint64_t Zp_vals_per_elem = ceil(double(d) / log2(p));
            return reconstruct_base_p(start, Zp_vals_per_elem, p);
        }
    }
};

class Database {
public:

    // N is the number of elements, d is the bitwidth of the elements
    const uint64_t N, d;
    bool alloc = false;
    entry_t * data;

    // PlaintextDBParams computeSimplePIRParams(const bool verbose = false);
    PlaintextDBParams computeParams(const bool allowTrivial, const bool verbose = false, const bool simplePIR = false, const uint64_t batchSize = 1, const bool preproc = false, const bool honestHint=false) const;

    // PlaintextDBParams lookupParams(const bool allowTrivial, const bool verbose = false, const bool simplePIR = false) const;

    entry_t getDataAtIndex(const uint64_t index) const {
        if (index >= N) {
            std::cout << "index is too big\n";
            std::cout << index << " " << N << std::endl;
            assert(false);
        }
        return data[index];
    }

    Matrix packDataInMatrix(const PlaintextDBParams& params, const bool verbose = false) const;

    void loadRandomData() {
        if (!alloc) {
            data = (entry_t*)malloc(N * sizeof(entry_t));
            alloc = true;
        }
        memset(data, 0, N * sizeof(entry_t));

        std::mt19937_64 prng(std::random_device{}());
        std::uniform_int_distribution<unsigned long> dist;

        const uint64_t numBlocks = ceil(float(d) / float(8.0*sizeof(unsigned long))); 
        const BigUnsigned mask = (BigUnsigned(1) << d) - BigUnsigned(1);
        for (uint64_t i = 0; i < N; i++) {
            BigUnsigned val(0);
            for (uint64_t blockInd = 0; blockInd < numBlocks; blockInd++) {
                val.setBlock(blockInd, dist(prng));
            }
            val &= mask;
            data[i] = val;
            // data[i] = BigUnsigned(i);
        }
    }

    void loadIndexData() {
        if (!alloc) {
            data = (entry_t*)malloc(N * sizeof(entry_t));
            alloc = true;
        }
        memset(data, 0, N * sizeof(entry_t));
        entry_t modulus = entry_t(1) << d;
        for (uint64_t i = 0; i < N; i++) {
            data[i] = entry_t(i) % modulus;
        }
    }

    void loadConstantData(const uint64_t v) {
        if (!alloc) {
            data = (entry_t*)malloc(N * sizeof(entry_t));
            alloc = true;
        }
        memset(data, 0, N * sizeof(entry_t));
        entry_t modulus = entry_t(1) << d;
        for (uint64_t i = 0; i < N; i++) {
            data[i] = entry_t(v) % modulus;
        }
    }

    // Database(const uint64_t N_in, const uint64_t d_in, const bool no_alloc = false) : N(N_in), d(d_in) {};
    Database(const uint64_t N_in, const uint64_t d_in) : N(N_in), d(d_in) {};

    ~Database() {
        if (alloc) free(data);
    };

    // copy constructor
    Database(Database& rhs) : N(rhs.N), d(rhs.d) {
        if (rhs.alloc) {
            data = (entry_t*)malloc(N * sizeof(entry_t));
            alloc = true;
            memcpy(data, rhs.data, N * sizeof(entry_t));
        }
    };
};