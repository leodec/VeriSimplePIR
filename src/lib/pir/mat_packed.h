#pragma once

#include "mat.h"
#include "multilimb_lhe.h"

#define STAT_SEC_PARAM 40  // statistical security parameter. number of rows in binary C

struct PackedMatrix {
    Matrix mat;
    const uint64_t orig_rows, orig_cols;  // original dimension of the matrix
    const uint64_t elemBits;  // number of bits per element

    PackedMatrix(const Matrix& m, const uint64_t o_r, const uint64_t o_c, const uint64_t eB) :
        mat(m), orig_rows(o_r), orig_cols(o_c), elemBits(eB) {};

    PackedMatrix() : orig_rows(0), orig_cols(0), elemBits(0) {};
};

PackedMatrix packMatrix(const Matrix& mat, const uint64_t p);
PackedMatrix packMatrixHardCoded(const Matrix& mat, const uint64_t p);
PackedMatrix packMatrixHardCoded(const uint64_t rows, const uint64_t cols, const uint64_t p, const bool random = true);


Matrix matMulLeftBinaryRightColPacked(const BinaryMatrix& binary, const PackedMatrix& b);
Matrix matMulLeftBinaryRightColPacked_Hardcoded(const BinaryMatrix& binary, const PackedMatrix& b, const bool outputPadded = false);
// Matrix matMulLeftBinaryRightColPacked_Hardcoded_v2(const BinaryMatrix& binary, const PackedMatrix& b);

Matrix matVecMulColPacked(const PackedMatrix& packed, const Matrix& vec, const Elem modulus = 0);
Multi_Limb_Matrix matVecMulColPacked(const PackedMatrix& packed, const Multi_Limb_Matrix& vec, const Elem modulus);
Matrix matMulColPacked(const PackedMatrix& a, const Matrix& b);

void matMulVecColPackedInner(Elem *out, const Elem *a, const Elem *b, size_t aRows, size_t aCols);
Matrix simplepir_matVecMulColPacked(const PackedMatrix& a, const Matrix& b);
Matrix simplepir_matVecMulColPacked_variableCompression(const PackedMatrix& a, const Matrix& b);
Matrix simplepir_matVecMulColPacked_variableCompression_noUnroll(const PackedMatrix& a, const Matrix& b);