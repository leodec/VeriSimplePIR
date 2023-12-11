#include "mat_packed.h"

// #define BASIS 2
// #define BASIS 4
// #define BASIS 8
// #define BASIS 9
// #define BASIS 16
// #define BASIS 20
#define BASIS 26
#define COMPRESSION (sizeof(Elem)*8/BASIS)
#define MASK (1ULL << BASIS) - 1


PackedMatrix packMatrix(const Matrix& mat, const uint64_t p) {
    // column-packed matrix 
    const uint64_t elemWidth = ceil(log2(p));
    const uint64_t numEntriesPerElem = floor(double(8*sizeof(Elem)) / double(elemWidth));
    const uint64_t numPackedCols = ceil(double(mat.cols) / double(numEntriesPerElem));

    Matrix result(mat.rows, numPackedCols);
    for (uint64_t i = 0; i < mat.rows; i++) {
        uint64_t real_col_ind = 0;
        for (uint64_t packed_col_ind = 0; packed_col_ind < numPackedCols; packed_col_ind++) {
            for (uint64_t packed_elem_ind = 0; packed_elem_ind < numEntriesPerElem; packed_elem_ind++) {
                if (real_col_ind >= mat.cols) break;
                result.data[i*numPackedCols + packed_col_ind] += mat.data[i*mat.cols + real_col_ind] << (elemWidth * packed_elem_ind);
                real_col_ind++;
            }
        }
    }

    return PackedMatrix(result, mat.rows, mat.cols, elemWidth);
}

PackedMatrix packMatrixHardCoded(const Matrix& mat, const uint64_t p) {
    assert(BASIS*COMPRESSION <= sizeof(Elem)*8);

    // column-packed matrix 
    if (ceil(log2(p)) > BASIS) {
        std::cout << "width must be at most the hardcoded value\n";
        std::cout << ceil(log2(p)) << " " << BASIS << std::endl;
        assert(false);
    }
    constexpr uint64_t elemWidth = BASIS;

    constexpr uint64_t numEntriesPerElem = COMPRESSION;
    const uint64_t numPackedCols = ceil(double(mat.cols) / double(numEntriesPerElem));

    Matrix result(mat.rows, numPackedCols);
    for (uint64_t i = 0; i < mat.rows; i++) {
        uint64_t real_col_ind = 0;
        for (uint64_t packed_col_ind = 0; packed_col_ind < numPackedCols; packed_col_ind++) {
            for (uint64_t packed_elem_ind = 0; packed_elem_ind < numEntriesPerElem; packed_elem_ind++) {
                if (real_col_ind >= mat.cols) break;
                result.data[i*numPackedCols + packed_col_ind] += mat.data[i*mat.cols + real_col_ind] << (elemWidth * packed_elem_ind);
                real_col_ind++;
            }
        }
    }

    return PackedMatrix(result, mat.rows, mat.cols, elemWidth);
}

PackedMatrix packMatrixHardCoded(const uint64_t rows, const uint64_t cols, const uint64_t p, const bool random) {
     assert(BASIS*COMPRESSION <= sizeof(Elem)*8);

    // column-packed matrix 
    if (ceil(log2(p)) > BASIS) {
        std::cout << "width must be at most the hardcoded value\n";
        std::cout << ceil(log2(p)) << " " << BASIS << std::endl;
        assert(false);
    }
    constexpr uint64_t elemWidth = BASIS;

    constexpr uint64_t numEntriesPerElem = COMPRESSION;
    const uint64_t numPackedCols = ceil(double(cols) / double(numEntriesPerElem));

    Matrix result; result.init_no_memset(rows, numPackedCols);
    if (random) random_fast(result);
    return PackedMatrix(result, rows, cols, elemWidth);
}

Matrix matMulLeftBinaryRightColPacked(const BinaryMatrix& binary, const PackedMatrix& b) {
    const size_t aRows = binary.rows;
    const size_t aCols = binary.cols;
    const size_t bCols = b.orig_cols;

    if (aCols != b.orig_rows) {
        std::cout << "Dimension mismatch!\n";
        assert(false);
    }

    const uint64_t numEntriesPerElem = floor(double(8*sizeof(Elem)) / double(b.elemBits));
    // constexpr uint64_t numEntriesPerElem = COMPRESSION;
    const Elem mask = (1 << b.elemBits) - 1;
    // constexpr Elem mask = MASK;

    Matrix outPadded(aRows, bCols + (b.orig_cols%numEntriesPerElem));  // memset values to zero

    for (size_t i = 0; i < aRows; i++) {
        for (size_t k = 0; k < b.mat.rows; k++) {
            if (binary.data[aCols*i + k]) {
                
                uint64_t real_col_ind = 0;
                for (size_t packed_col_ind = 0; packed_col_ind < b.mat.cols; packed_col_ind++) {
                    const Elem packed_elem = b.mat.data[k*b.mat.cols + packed_col_ind];
                    for (uint64_t packed_elem_ind = 0; packed_elem_ind < numEntriesPerElem; packed_elem_ind++) {
                        // if (real_col_ind >= b.orig_cols) break;
                        outPadded.data[i*outPadded.cols + real_col_ind] += (packed_elem >> (packed_elem_ind*b.elemBits)) & mask;
                        real_col_ind++;
                    }
                }
                
            }
        }
    }

    // return outPadded;
    // seemingly small performance difference based on experiments....

    Matrix out;
    out.init_no_memset(aRows, bCols);
    for (uint32_t i = 0; i < aRows; i++)
        for (uint32_t j = 0; j < bCols; j++)
            out.data[i*bCols + j] = outPadded.data[i*outPadded.cols + j];

    return out;
}

Matrix matMulLeftBinaryRightColPacked_Hardcoded(const BinaryMatrix& binary, const PackedMatrix& b, const bool outputPadded) {
    // const size_t aRows = binary.rows;
    if (binary.rows != STAT_SEC_PARAM) {
        std::cout << "hardcoded function must use statistical security parameter for rows.\n";
        assert(false);
    }
    constexpr size_t aRows = STAT_SEC_PARAM;
    const size_t aCols = binary.cols;
    const size_t bCols = b.orig_cols;

    if (aCols != b.orig_rows) {
        std::cout << "Dimension mismatch!\n";
        assert(false);
    }

    constexpr uint64_t numEntriesPerElem = COMPRESSION;
    constexpr Elem mask = MASK;
    constexpr uint32_t basis = BASIS;
    assert(basis == b.elemBits);

    Matrix outPadded(aRows, bCols + (b.orig_cols%numEntriesPerElem));  // memset values to zero

    for (size_t k = 0; k < b.mat.rows; k++) {
        for (size_t i = 0; i < aRows; i++) {
            if (binary.data[aCols*i + k]) {
                
                uint64_t real_col_ind = 0;
                for (size_t packed_col_ind = 0; packed_col_ind < b.mat.cols; packed_col_ind++) {
                    const Elem packed_elem = b.mat.data[k*b.mat.cols + packed_col_ind];
                    for (uint64_t packed_elem_ind = 0; packed_elem_ind < numEntriesPerElem; packed_elem_ind++) {
                        outPadded.data[i*outPadded.cols + real_col_ind] += (packed_elem >> (packed_elem_ind*basis)) & mask;
                        real_col_ind++;
                    }
                }
                
            }
        }
    }

    if (outputPadded) return outPadded;

    Matrix out;
    out.init_no_memset(aRows, bCols);
    for (uint32_t i = 0; i < aRows; i++)
        for (uint32_t j = 0; j < bCols; j++)
            out.data[i*bCols + j] = outPadded.data[i*outPadded.cols + j];

    return out;
}

Matrix matMulLeftBinaryRightColPacked_Hardcoded_v2(const BinaryMatrix& binary, const PackedMatrix& b) {

    std::cout << "this function is a dud. no locality means slower speeds even with max compression.\n";
    assert(false);

    // const size_t aRows = binary.rows;
    constexpr size_t aRows = STAT_SEC_PARAM;
    const size_t aCols = binary.cols;
    const size_t bCols = b.orig_cols;

    if (aCols != b.orig_rows) {
        std::cout << "Dimension mismatch!\n";
        assert(false);
    }

    constexpr uint64_t numEntriesPerElem = COMPRESSION;
    constexpr Elem mask = MASK;
    constexpr uint32_t basis = BASIS;
    assert(basis == b.elemBits);

    Matrix outPadded(aRows, bCols + (b.orig_cols%numEntriesPerElem));  // memset values to zero

    // read in each column of D
    // D is column packed
    // then, multiply with each row of C

    Elem tmp[COMPRESSION];  // these are the output elements
    // column of the packed matrix and column of the output
    for (size_t packed_col_ind = 0; packed_col_ind < b.mat.cols; packed_col_ind++) {
        // multiply with each row of C
        // reads through each row of C. this is also the row of the output
        for (size_t binary_row_ind = 0; binary_row_ind < binary.rows; binary_row_ind++) {

            for (size_t i = 0; i < COMPRESSION; i++) tmp[i] = 0;

            // reading through the each entry of the column of D
            // note that we're actually reading COMPRESION columns at once
            // each row also corresponds to a column of C
            for (size_t b_row_ind = 0; b_row_ind < b.mat.rows; b_row_ind++) {
                const bool toAdd = binary.data[binary_row_ind*binary.cols + b_row_ind];
                if (toAdd) {
                    // this has COMPRESSION columns
                    const Elem packed_elem = b.mat.data[b_row_ind*b.mat.cols + packed_col_ind];
                
                    for (size_t packed_elem_ind = 0; packed_elem_ind < numEntriesPerElem; packed_elem_ind++) {
                        tmp[packed_elem_ind] += (packed_elem >> (packed_elem_ind*basis)) & mask;
                    }
                }
            }

            for (size_t i = 0; i < COMPRESSION; i++)
                outPadded.data[binary_row_ind*outPadded.cols + packed_col_ind*numEntriesPerElem + i] = tmp[i];

        }
    }

    Matrix out;
    out.init_no_memset(aRows, bCols);
    for (uint32_t i = 0; i < out.rows; i++)
        for (uint32_t j = 0; j < out.cols; j++)
            out.data[i*out.cols + j] = outPadded.data[i*outPadded.cols + j];

    return out;
}

Matrix matVecMulColPacked(const PackedMatrix& packed, const Matrix& vec, const Elem modulus) {
    if (packed.orig_cols != vec.rows) {
        std::cout << "Dimension mismatch!\n";
        assert(false);
    }

    const uint64_t numEntriesPerElem = floor(double(8*sizeof(Elem)) / double(packed.elemBits));
    // const uint64_t numEntriesPerElem = COMPRESSION;
    const Elem mask = (1 << packed.elemBits) - 1;
    // const Elem mask = MASK;


    Matrix out(packed.orig_rows, 1);  // memset values to zero

    if (modulus == 0) {

        for (size_t i = 0; i < packed.orig_rows; i++) {
            Elem tmp = 0;
            for (size_t k = 0; k < packed.mat.cols; k++) {
                const Elem packed_elem = packed.mat.data[i*packed.mat.cols + k];
                for (uint64_t packed_elem_ind = 0; packed_elem_ind < numEntriesPerElem; packed_elem_ind++) {
                    const Elem unpacked_val = (packed_elem >> (packed_elem_ind*packed.elemBits)) & mask;
                    tmp += unpacked_val * vec.data[numEntriesPerElem*k + packed_elem_ind];
                }
            }
            out.data[i] = tmp;
        }

    } else {

        for (size_t i = 0; i < packed.orig_rows; i++) {
            Elem tmp = 0;
            for (size_t k = 0; k < packed.mat.cols; k++) {
                const Elem packed_elem = packed.mat.data[i*packed.mat.cols + k];
                for (uint64_t packed_elem_ind = 0; packed_elem_ind < numEntriesPerElem; packed_elem_ind++) {
                    const Elem unpacked_val = (packed_elem >> (packed_elem_ind*packed.elemBits)) & mask;
                    tmp += unpacked_val * vec.data[numEntriesPerElem*k + packed_elem_ind] % modulus;
                }
            }
            out.data[i] = tmp % modulus;
        }

    }

    return out;
}

Multi_Limb_Matrix matVecMulColPacked(const PackedMatrix& packed, const Multi_Limb_Matrix& vec, const Elem modulus) {
    Multi_Limb_Matrix result(packed.orig_rows, vec.cols);
    result.q_data = matVecMulColPacked(packed, vec.q_data);
    result.kappa_data = matVecMulColPacked(packed, vec.kappa_data, modulus);
    return result;
}


Matrix simplepir_matVecMulColPacked(const PackedMatrix& a, const Matrix& b) {
    Matrix out(a.mat.rows, 1);

    const size_t aRows = a.mat.rows;
    const size_t aCols = a.mat.cols;

    Elem db, db2, db3, db4, db5, db6, db7, db8;
    Elem val, val2, val3, val4, val5, val6, val7, val8;
    Elem tmp, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;
    size_t index = 0;
    size_t index2;

    assert(COMPRESSION == 3);

    for (size_t i = 0; i < aRows; i += 8)
    {
        tmp = 0;
        tmp2 = 0;
        tmp3 = 0;
        tmp4 = 0;
        tmp5 = 0;
        tmp6 = 0;
        tmp7 = 0;
        tmp8 = 0;

        index2 = 0;
        for (size_t j = 0; j < aCols; j++)
        {
            db = a.mat.data[index];
            db2 = a.mat.data[index + 1 * aCols];
            db3 = a.mat.data[index + 2 * aCols];
            db4 = a.mat.data[index + 3 * aCols];
            db5 = a.mat.data[index + 4 * aCols];
            db6 = a.mat.data[index + 5 * aCols];
            db7 = a.mat.data[index + 6 * aCols];
            db8 = a.mat.data[index + 7 * aCols];

            val = db & MASK;
            val2 = db2 & MASK;
            val3 = db3 & MASK;
            val4 = db4 & MASK;
            val5 = db5 & MASK;
            val6 = db6 & MASK;
            val7 = db7 & MASK;
            val8 = db8 & MASK;
            tmp += val * b.data[index2];
            tmp2 += val2 * b.data[index2];
            tmp3 += val3 * b.data[index2];
            tmp4 += val4 * b.data[index2];
            tmp5 += val5 * b.data[index2];
            tmp6 += val6 * b.data[index2];
            tmp7 += val7 * b.data[index2];
            tmp8 += val8 * b.data[index2];
            index2 += 1;

            val = (db >> BASIS) & MASK;
            val2 = (db2 >> BASIS) & MASK;
            val3 = (db3 >> BASIS) & MASK;
            val4 = (db4 >> BASIS) & MASK;
            val5 = (db5 >> BASIS) & MASK;
            val6 = (db6 >> BASIS) & MASK;
            val7 = (db7 >> BASIS) & MASK;
            val8 = (db8 >> BASIS) & MASK;
            tmp += val * b.data[index2];
            tmp2 += val2 * b.data[index2];
            tmp3 += val3 * b.data[index2];
            tmp4 += val4 * b.data[index2];
            tmp5 += val5 * b.data[index2];
            tmp6 += val6 * b.data[index2];
            tmp7 += val7 * b.data[index2];
            tmp8 += val8 * b.data[index2];
            index2 += 1;

            val = (db >> (2*BASIS)) & MASK;
            val2 = (db2 >> (2*BASIS)) & MASK;
            val3 = (db3 >> (2*BASIS)) & MASK;
            val4 = (db4 >> (2*BASIS)) & MASK;
            val5 = (db5 >> (2*BASIS)) & MASK;
            val6 = (db6 >> (2*BASIS)) & MASK;
            val7 = (db7 >> (2*BASIS)) & MASK;
            val8 = (db8 >> (2*BASIS)) & MASK;
            tmp += val * b.data[index2];
            tmp2 += val2 * b.data[index2];
            tmp3 += val3 * b.data[index2];
            tmp4 += val4 * b.data[index2];
            tmp5 += val5 * b.data[index2];
            tmp6 += val6 * b.data[index2];
            tmp7 += val7 * b.data[index2];
            tmp8 += val8 * b.data[index2];
            index2 += 1;
            index += 1;
        }
        out.data[i] += tmp;
        out.data[i + 1] += tmp2;
        out.data[i + 2] += tmp3;
        out.data[i + 3] += tmp4;
        out.data[i + 4] += tmp5;
        out.data[i + 5] += tmp6;
        out.data[i + 6] += tmp7;
        out.data[i + 7] += tmp8;
        index += aCols * 7;
    }

    return out;
}

Matrix simplepir_matVecMulColPacked_variableCompression(const PackedMatrix& a, const Matrix& b) {
    Matrix out(a.mat.rows, 1);

    const size_t aRows = a.mat.rows;
    const size_t aCols = a.mat.cols;

    Elem db, db2, db3, db4, db5, db6, db7, db8;
    Elem val, val2, val3, val4, val5, val6, val7, val8;
    Elem tmp, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;
    size_t index = 0;
    size_t index2;

    // constexpr uint64_t numElemsPerEntry = COMPRESSION;
    // constexpr Elem mask = MASK;
    // constexpr uint64_t basis = BASIS;
    // assert(a.elemBits == BASIS);
    assert(a.mat.rows % 8 == 0);

    for (size_t i = 0; i < aRows; i += 8)
    {
        tmp = 0;
        tmp2 = 0;
        tmp3 = 0;
        tmp4 = 0;
        tmp5 = 0;
        tmp6 = 0;
        tmp7 = 0;
        tmp8 = 0;

        index2 = 0;
        for (size_t j = 0; j < aCols; j++)
        {
            // assert(index + 7*aCols < a.mat.rows * a.mat.cols);
            db = a.mat.data[index];
            db2 = a.mat.data[index + 1 * aCols];
            db3 = a.mat.data[index + 2 * aCols];
            db4 = a.mat.data[index + 3 * aCols];
            db5 = a.mat.data[index + 4 * aCols];
            db6 = a.mat.data[index + 5 * aCols];
            db7 = a.mat.data[index + 6 * aCols];
            db8 = a.mat.data[index + 7 * aCols];

            for (size_t compInd = 0; compInd < COMPRESSION; compInd++) {

                const uint32_t shift = compInd * BASIS;

                val = (db >> shift) & MASK;
                val2 = (db2 >> shift) & MASK;
                val3 = (db3 >> shift) & MASK;
                val4 = (db4 >> shift) & MASK;
                val5 = (db5 >> shift) & MASK;
                val6 = (db6 >> shift) & MASK;
                val7 = (db7 >> shift) & MASK;
                val8 = (db8 >> shift) & MASK;

                // assert(index2 < b.rows);
                tmp += val * b.data[index2];
                tmp2 += val2 * b.data[index2];
                tmp3 += val3 * b.data[index2];
                tmp4 += val4 * b.data[index2];
                tmp5 += val5 * b.data[index2];
                tmp6 += val6 * b.data[index2];
                tmp7 += val7 * b.data[index2];
                tmp8 += val8 * b.data[index2];
                index2 += 1;
            }
            index += 1;
        }
        out.data[i] += tmp;
        out.data[i + 1] += tmp2;
        out.data[i + 2] += tmp3;
        out.data[i + 3] += tmp4;
        out.data[i + 4] += tmp5;
        out.data[i + 5] += tmp6;
        out.data[i + 6] += tmp7;
        out.data[i + 7] += tmp8;
        index += aCols * 7;
    }

    return out;
}

Matrix simplepir_matVecMulColPacked_variableCompression_noUnroll(const PackedMatrix& a, const Matrix& b) {
    Matrix out(a.mat.rows, 1);

    const size_t aRows = a.mat.rows;
    const size_t aCols = a.mat.cols;

    Elem db;
    Elem val;
    Elem tmp;
    size_t index = 0;
    size_t index2;

    for (size_t i = 0; i < aRows; i++) {
        tmp = 0;
        index2 = 0;
        for (size_t j = 0; j < aCols; j++) {
            db = a.mat.data[index];
            for (size_t compInd = 0; compInd < COMPRESSION; compInd++) {
                const uint32_t shift = compInd * BASIS;
                val = (db >> shift) & MASK;
                tmp += val * b.data[index2];
                index2 += 1;
            }
            index += 1;
        }
        out.data[i] += tmp;
    }

    return out;
}

Matrix matMulColPacked(const PackedMatrix& a, const Matrix& b) {

    const size_t aRows = a.mat.rows;
    const size_t aCols = a.mat.cols;
    const size_t bCols = b.cols;

    // assert(a.mat.cols*COMPRESSION == b.rows);
    Matrix out(a.mat.rows, b.cols);

    Elem db;
    Elem val;
    Elem tmp;

    for (size_t i = 0; i < aRows; i++) {
        for (size_t k = 0; k < bCols; k++) {
            tmp = 0;
            uint64_t real_row_ind = 0;
            for (size_t j = 0; j < aCols; j++) {  // iterating over packed columns
                db = a.mat.data[i*aCols + j];
                for (size_t compInd = 0; compInd < COMPRESSION; compInd++) {
                    if (real_row_ind >= b.rows) break;
                    const uint32_t shift = compInd * BASIS;
                    val = (db >> shift) & MASK;
                    tmp += val * b.data[real_row_ind*b.cols + k];
                    real_row_ind++;
                }
            }
            out.data[i*out.cols + k] = tmp;
        }
    }

    return out;
}
