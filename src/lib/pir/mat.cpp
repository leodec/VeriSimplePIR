#include "mat.h"
#include "gauss.h"

#include <fstream>
#include <iostream>


void randomInner(Matrix& mat, PRNG& prng, const Elem max) {
    std::uniform_int_distribution<Elem> dist;
    if (max == 0) {
        dist = std::uniform_int_distribution<Elem>(0);  // default max is maximum value of type
    } else {
        dist = std::uniform_int_distribution<Elem>(0, max-1); 
    }
    const size_t len = mat.rows * mat.cols;
    for (size_t i = 0; i < len; i++) {
        mat.data[i] = dist(prng);
    }
}

void randomInner(BinaryMatrix& mat, PRNG& prng) {
    auto dist = std::uniform_int_distribution<>(0, 1); 
    const size_t len = mat.rows * mat.cols;
    for (size_t i = 0; i < len; i++) {
        mat.data[i] = dist(prng);
    }
}

void random_fast(Matrix& mat, const Elem modulus) {
    auto fd = fopen("/dev/random", "r");
    // memcpy(fd, mat.data, mat.rows*mat.cols*sizeof(Elem));
    fgets((char *)mat.data, mat.rows*mat.cols*sizeof(Elem), fd);
    fclose(fd);

    if (modulus > 0) {
        for (size_t i = 0; i < mat.rows*mat.cols; i++)
            mat.data[i] %= modulus;
    }
}

void random(Matrix& mat, const Elem max) {
    PRNG prng(osuCrypto::sysRandomSeed());
    randomInner(mat, prng, max);
}

void random(BinaryMatrix& mat) {
    PRNG prng(osuCrypto::sysRandomSeed());
    randomInner(mat, prng);
}

void pseudorandom(Matrix& mat, const SeedType& seed, const Elem max) {
    PRNG prng(seed);
    randomInner(mat, prng, max);
}

void pseudorandom(BinaryMatrix& mat, const SeedType& seed) {
    PRNG prng(seed);
    randomInner(mat, prng);
}

void error(Matrix& mat) {
    const size_t len = mat.rows * mat.cols;
    for (size_t i = 0; i < len; i++) {
        mat.data[i] = GaussSample<Elem>();
    }
}

template <typename MatrixType>
void constant(MatrixType& mat, const Elem val) {
    size_t len = mat.rows * mat.cols;
    for (size_t i = 0; i < len; i++) {
        mat.data[i] = val;
    }
}

template void constant(Matrix& mat, const Elem val);
template void constant(BinaryMatrix& mat, const Elem val);

bool eq(const Matrix& a, const Matrix& b, const bool verbose) {
    if (a.rows != b.rows || a.cols != b.cols) {
        if (verbose) {
            std::cout << "Dimension mismatch!\n";
            std::cout << a.rows << " x " << a.cols << " vs. " << b.rows << " x " << b.cols << std::endl;
        } 
        return false;
    }
    size_t len = a.rows * b.cols;
    for (size_t i = 0; i < len; i++) {
        if (a.data[i] != b.data[i]) {
            if (verbose) {
                std::cout << "Data mismatch!\n";
                std::cout << i << " " << a.data[i] << " " << b.data[i] << std::endl;
            }
            return false;
        }
    }
    return true;
}

template <typename MatType>
void print(const MatType& mat) {
    for (size_t r = 0; r < mat.rows; r++) {
        for (size_t c = 0; c < mat.cols; c++) {
            std::cout << mat.data[r*mat.cols + c] << " ";
        }
        std::cout << std::endl;
    }
}

template void print(const Matrix& mat);
template void print(const BinaryMatrix& mat); 

void matAddInPlace(Matrix& a, const Matrix& b, const Elem modulus) {
    const size_t rows = a.rows;
    const size_t cols = a.cols;

    if (rows != b.rows || cols != b.cols) {
        std::cout << "dimension mismatch\n!";
        assert(false);
    }

    const size_t len = rows * cols;

    if (modulus == 0) {
        for (size_t i = 0; i < len; i++) {
            a.data[i] += b.data[i];
        }
    } else {
        for (size_t i = 0; i < len; i++) {
            a.data[i] += b.data[i];
            a.data[i] %= modulus;
        }
    }
}

Matrix matAdd(const Matrix& a, const Matrix& b) {
    const size_t rows = a.rows;
    const size_t cols = a.cols;

    if (rows != b.rows || cols != b.cols) {
        std::cout << "Input dimension mismatch\n!";
        assert(false);
    }

    Matrix out; out.init_no_memset(rows, cols);

    const size_t len = rows * cols;
    for (size_t i = 0; i < len; i++) {
        out.data[i] = a.data[i] + b.data[i];
    }

    return out;
}

Matrix matSub(const Matrix& a, const Matrix& b, const Elem modulus) {
    const size_t rows = a.rows;
    const size_t cols = a.cols;

    if (rows != b.rows || cols != b.cols) {
        std::cout << "Input dimension mismatch\n!";
        assert(false);
    }

    Matrix out; out.init_no_memset(rows, cols);

    const size_t len = rows * cols;

    if (modulus == 0) {
        for (size_t i = 0; i < len; i++) {
            out.data[i] = a.data[i] - b.data[i];
        }
    } else {
        for (size_t i = 0; i < len; i++) {
            out.data[i] = a.data[i] + modulus - b.data[i];
            out.data[i] -= (out.data[i] >= modulus) ? modulus : 0;
        }
    }

    return out;
}

Matrix matMulScalar(const Matrix& a, const Elem b, const Elem modulus) {
    Matrix out; out.init_no_memset(a.rows, a.cols);
    const size_t len = a.rows * a.cols;
    if (modulus == 0) {
        for (size_t i = 0; i < len; i++) {
            out.data[i] = a.data[i] * b;
        }
    } else {
        for (size_t i = 0; i < len; i++) {
            out.data[i] = a.data[i] * b;
            out.data[i] %= modulus;
        }   
    }
    return out;
}

// Performs the divide and round step
Matrix matDivScalar(const Matrix& a, const Elem b) {
    Matrix out; out.init_no_memset(a.rows, a.cols);
    const size_t len = a.rows * a.cols;
    for (size_t i = 0; i < len; i++) {
        out.data[i] = a.data[i] / b;
        if (a.data[i] % b >= b/2) {
            out.data[i] += 1;
        }
    }
    return out;
}

Matrix matMul(const Matrix& a, const Matrix& b, const Elem modulus) {
    const size_t aRows = a.rows;
    const size_t aCols = a.cols;
    const size_t bCols = b.cols;

    if (aCols != b.rows) {
        std::cout << aCols << " " << b.rows << std::endl;
        std::cout << "Dimension mismatch!\n";
        assert(false);
    }

    Matrix out(aRows, bCols);  // memset values to zero

    if (modulus == 0) {

        for (size_t i = 0; i < aRows; i++) {
            for (size_t k = 0; k < aCols; k++) {
                for (size_t j = 0; j < bCols; j++) {
                    out.data[bCols * i + j] += a.data[aCols * i + k] * b.data[bCols * k + j];
                }
            }
        }

    } else {

        for (size_t i = 0; i < aRows; i++) {
            for (size_t k = 0; k < aCols; k++) {
                for (size_t j = 0; j < bCols; j++) {
                    out.data[bCols * i + j] += a.data[aCols * i + k] * b.data[bCols * k + j];
                    out.data[bCols * i + j] %= modulus;
                }
            }
        }

    }

    return out;
}

// produces the product a*[b | c], where c is a vector
// assume that c is a vector
Matrix matMulAppendVec(const Matrix &a, const Matrix &b, const Matrix &c) {
    const size_t aRows = a.rows;
    const size_t aCols = a.cols;
    const size_t bCols = b.cols;

    if (aCols != b.rows) {
        std::cout << "Dimension mismatch!\n";
        std::cout << "trying to multiply ";
        std::cout << a.rows << " x " << a.cols << " times " << b.rows << " x " << b.cols << std::endl; 
        assert(false);
    }

    Matrix out(aRows, bCols + c.cols);  // memset values to zero

    for (size_t i = 0; i < aRows; i++) {
        for (size_t k = 0; k < aCols; k++) {
            const Elem val = a.data[aCols * i + k];
            for (size_t j = 0; j < bCols; j++) 
                out.data[out.cols * i + j] += val * b.data[bCols * k + j];
            for (size_t j = 0; j < c.cols; j++) 
                out.data[out.cols * i + bCols + j] += val * c.data[c.cols * k + j];
        }
    }

    return out;
}

Matrix binaryMatMulAppendVec(const BinaryMatrix &binary, const Matrix &b, const Matrix &c) {
    const size_t aRows = binary.rows;
    const size_t aCols = binary.cols;
    const size_t bCols = b.cols;

    if (aCols != b.rows) {
        std::cout << "Dimension mismatch!\n";
        assert(false);
    }

    Matrix out(aRows, bCols + c.cols);  // memset values to zero

    for (size_t i = 0; i < aRows; i++) {
        for (size_t k = 0; k < aCols; k++) {
            if (binary.data[aCols * i + k]) {
                for (size_t j = 0; j < bCols; j++) 
                    out.data[out.cols * i + j] += b.data[bCols * k + j];
                
                for (size_t j = 0; j < c.cols; j++) 
                    out.data[out.cols * i + bCols + j] += c.data[c.cols * k + j];
            }
        }
    }

    return out;
}

Matrix matMulLeftBinary(const BinaryMatrix& binary, const Matrix& b, const Elem modulus) {
    const size_t aRows = binary.rows;
    const size_t aCols = binary.cols;
    const size_t bCols = b.cols;

    if (aCols != b.rows) {
        std::cout << "Dimension mismatch!\n";
        assert(false);
    }

    Matrix out(aRows, bCols);  // memset values to zero

    if (modulus == 0) {

        for (size_t i = 0; i < aRows; i++) {
            for (size_t k = 0; k < aCols; k++) {
                if (binary.data[aCols*i + k]) {
                    for (size_t j = 0; j < bCols; j++) {
                        out.data[bCols * i + j] += b.data[bCols * k + j];
                    }
                }
            }
        }

    } else {

        for (size_t i = 0; i < aRows; i++) {
            for (size_t k = 0; k < aCols; k++) {
                if (binary.data[aCols*i + k]) {
                    for (size_t j = 0; j < bCols; j++) {
                        out.data[bCols * i + j] += b.data[bCols * k + j];
                        out.data[bCols * i + j] %= modulus;
                    }
                }
            }
        }

    }

    return out;
}

Matrix matMulRightBinary(const Matrix& b, const BinaryMatrix& binary, const Elem modulus) {
    Matrix rightMat = binary.asMatrix();
    return matMul(b, rightMat);
}

Matrix matMulVec(const Matrix& a, const Matrix& b, const Elem modulus) {
    const size_t aRows = a.rows;
    const size_t aCols = a.cols;
    const size_t bRows = b.rows;

    if (aCols != bRows || b.cols != 1) {
        std::cout << "Input dimension mismatch!\n";
        assert(false);
    }

    Matrix out; out.init_no_memset(aRows, 1);

    Elem tmp;

    if (modulus == 0) {  // machine word modulus

        for (size_t i = 0; i < aRows; i++)
        {
            tmp = 0;
            for (size_t j = 0; j < aCols; j++)
            {
                tmp += a.data[aCols * i + j] * b.data[j];
            }
            out.data[i] = tmp;
        }

    } else {

        for (size_t i = 0; i < aRows; i++)
        {
            tmp = 0;
            for (size_t j = 0; j < aCols; j++)
            {
                tmp += a.data[aCols * i + j] * b.data[j];
            }
            out.data[i] = tmp % modulus;
        }

    }

    return out;
}

Matrix matBinaryMulVec(const BinaryMatrix& a, const Matrix& b) {
    const size_t aRows = a.rows;
    const size_t aCols = a.cols;
    const size_t bRows = b.rows;

    if (aCols != bRows || b.cols != 1) {
        std::cout << "Input dimension mismatch!\n";
        assert(false);
    }

    Matrix out; out.init_no_memset(aRows, 1);

    Elem tmp;
    for (size_t i = 0; i < aRows; i++)
    {
        tmp = 0;
        for (size_t j = 0; j < aCols; j++)
        {
            tmp += (a.data[aCols * i + j]) ? b.data[j] : 0;
        }
        out.data[i] = tmp;
    }

    return out;
}


