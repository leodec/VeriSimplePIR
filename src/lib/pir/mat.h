/*
    Basic matrix definitions and operations
*/
#pragma once

#include <cstdio>
#include <cstdlib>
#include "utils.h"
#include "math/prng.h"

// typedef std::mt19937_64 PRNG;
typedef osuCrypto::PRNG PRNG;
// typedef PRNG::result_type SeedType;
typedef osuCrypto::block SeedType;

/* These are the only tested values of Elem */ 
// typedef uint32_t Elem;
typedef uint64_t Elem;

#define ALIGN 8

class Matrix {
public:
    uint64_t rows, cols;
    Elem* data;  // packed in row-major order by default

    Matrix() {};

    // used on unitialized matrices to write data directly 
    void init_no_memset(uint64_t r, uint64_t c) {
        rows = r;
        cols = c;

        #ifdef ALIGN
        data = (Elem*)aligned_alloc(ALIGN, rows*cols * sizeof(Elem));
        #else
        data = (Elem*)malloc(rows*cols * sizeof(Elem));
        #endif
    }

    Matrix(uint64_t r, uint64_t c, const Elem val = 0) : rows(r), cols(c) {
         #ifdef ALIGN
        data = (Elem*)aligned_alloc(ALIGN, rows*cols * sizeof(Elem));
        #else
        data = (Elem*)malloc(rows*cols * sizeof(Elem));
        #endif

        memset(data, val, rows*cols * sizeof(Elem));
    }

    // ~Matrix() {
    //     free(data);
    // }

    Matrix(const Matrix& rhs) {
        rows = rhs.rows;
        cols = rhs.cols;

        #ifdef ALIGN
        data = (Elem*)aligned_alloc(ALIGN, rows*cols * sizeof(Elem));
        #else
        data = (Elem*)malloc(rows*cols * sizeof(Elem));
        #endif
        
        memcpy(data, rhs.data, rows*cols * sizeof(Elem));
    }

    Elem getElem(const uint64_t row, const uint64_t col) const {
        // assumes row-major order packing
        if (row >= rows) {
            std::cout << "row too big!\n";
            std::cout << row << " " << rows << std::endl;
            assert(false);
        }
        if (col > cols) {
            std::cout << "col too big!\n";
            std::cout << col << " " << cols << std::endl;
            assert(false);
        }
        return data[row*cols + col];
    }

    Matrix getColumn(const uint64_t col) const {
        Matrix res(rows, 1);
        for (uint64_t i = 0; i < rows; i++) 
            res.data[i] = data[i*cols + col];
        return res;
    }
};

class BinaryMatrix {
public:
    uint64_t rows, cols;
    bool* data;  // packed in row-major order by default

    BinaryMatrix() {};

    BinaryMatrix(uint64_t r, uint64_t c, const Elem val = 0) : rows(r), cols(c) {
        #ifdef ALIGN
        data = (bool*)aligned_alloc(ALIGN, rows*cols * sizeof(bool));
        #else
        data = (bool*)malloc(rows*cols * sizeof(bool));
        #endif

        memset(data, val, rows*cols * sizeof(bool));
    }

    ~BinaryMatrix() {
        free(data);
    }

    BinaryMatrix(const BinaryMatrix& rhs) {
        rows = rhs.rows;
        cols = rhs.cols;

        #ifdef ALIGN
        data = (bool*)aligned_alloc(ALIGN, rows*cols * sizeof(bool));
        #else
        data = (bool*)malloc(rows*cols * sizeof(bool));
        #endif
        
        memcpy(data, rhs.data, rows*cols * sizeof(bool));
    }

    // used on unitialized matrices to write data directly 
    void init_no_memset(uint64_t r, uint64_t c) {
        rows = r;
        cols = c;

        #ifdef ALIGN
        data = (bool*)aligned_alloc(ALIGN, rows*cols * sizeof(bool));
        #else
        data = (Elem*)malloc(rows*cols * sizeof(Elem));
        #endif
    }


    Matrix asMatrix() const {
        Matrix res; res.init_no_memset(rows, cols);
        for (size_t i = 0; i < rows*cols; i++)
            res.data[i] = (Elem)data[i];
        return res;
    }

    // Matrix& operator&() {
    //     return (Matrix&)(*this);
    // }
};

void random(Matrix& mat, const Elem max = 0);
void random(BinaryMatrix& mat);
void random_fast(Matrix& mat, const Elem modulus = 0);

void pseudorandom(Matrix& mat, const SeedType& seed, const Elem max = 0);
void pseudorandom(BinaryMatrix& mat, const SeedType& seed);

void error(Matrix& mat);

template <typename MatrixType>
void constant(MatrixType& mat, const Elem val = 0);

bool eq(const Matrix& a, const Matrix& b, const bool verbose = false);

template <typename MatType>
void print(const MatType& mat);

template <typename MatrixType>
MatrixType transpose(const MatrixType& in) {
    const size_t rows = in.rows;
    const size_t cols = in.cols;

    MatrixType out; out.init_no_memset(cols, rows);

    for (size_t i = 0; i < rows; i++)
    {
        for (size_t j = 0; j < cols; j++)
        {
            out.data[j * rows + i] = in.data[i * cols + j];
        }
    }

    return out;
}

template BinaryMatrix transpose<BinaryMatrix>(const BinaryMatrix& in);
template Matrix transpose<Matrix>(const Matrix& in);


Matrix matAdd(const Matrix &a, const Matrix &b);
void matAddInPlace(Matrix& a, const Matrix& b, const Elem modulus = 0);

Matrix matSub(const Matrix &a, const Matrix &b, const Elem modulus = 0);
// void matSubInPlace(Matrix& a, const Matrix& b);

Matrix matMul(const Matrix &a, const Matrix &b, const Elem modulus = 0);
Matrix matMulLeftBinary(const BinaryMatrix& binary, const Matrix& b, const Elem modulus = 0);
Matrix matMulRightBinary(const Matrix& b, const BinaryMatrix& binary, const Elem modulus = 0);


// produces the product a*[b | c], where c is a vector
Matrix matMulAppendVec(const Matrix &a, const Matrix &b, const Matrix &c);
Matrix binaryMatMulAppendVec(const BinaryMatrix &binary, const Matrix &b, const Matrix &c);

Matrix matMulScalar(const Matrix &a, const Elem b, const Elem modulus = 0);
Matrix matDivScalar(const Matrix &a, const Elem b);

Matrix matMulVec(const Matrix &a, const Matrix &b, const Elem modulus = 0);
Matrix matBinaryMulVec(const BinaryMatrix& a, const Matrix& b);

