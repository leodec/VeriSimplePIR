#include <iostream>
#include "pir/mat.h"

void transpose_consistency() {
    size_t aRows = 10;
    size_t aCols = 5;
    Matrix a(aRows, aCols);
    random(a);

    Matrix aT = transpose(a);
    
    size_t bRows = 5;
    size_t bCols = 7;
    Matrix b(bRows, bCols);
    random(b);

    Matrix bT = transpose(b);

    Matrix res = matMul(a, b);

    Matrix shouldBeResT = matMul(bT, aT);

    Matrix shouldBeRes = transpose(shouldBeResT);

    assert(eq(res, shouldBeRes));
    std::cout << "Basic transpose consistency check passed\n";
}


void matrix_vector_consistency() {
    size_t aRows = 10;
    size_t aCols = 5;
    Matrix a(aRows, aCols);
    random(a);

    size_t bRows = 5;
    size_t bCols = 1;
    Matrix b(bRows, bCols);
    random(b);

    Matrix res = matMul(a, b);

    Matrix shouldBeRes = matMulVec(a, b);

    assert(eq(res, shouldBeRes));
    std::cout << "Basic matrix-vector consistency passed\n";
}

void associativity_test() {

    size_t aRows = 10;
    size_t aCols = 5;
    Matrix a(aRows, aCols);
    random(a);
    // constant(a, 1);
    // std::cout << "a =\n";
    // print(a);
    
    size_t bRows = aCols;
    size_t bCols = 7;
    Matrix b(bRows, bCols);
    random(b);
    // constant(b, 1);
    // std::cout << "b =\n";
    // print(b);

    size_t cRows = bCols;
    size_t cCols = 10;
    Matrix c(cRows, cCols);
    random(c);
    // constant(c, 1);
    // std::cout << "c =\n";
    // print(c);

    Matrix ab = matMul(a, b);
    // std::cout << "ab =\n";
    // print(ab);

    Matrix ab_c = matMul(ab, c);

    Matrix bc = matMul(b, c);

    Matrix a_bc = matMul(a, bc);

    assert(eq(a_bc, ab_c, true));
    std::cout << "Basic associativity test passed\n";
}


int main() {
    transpose_consistency();
    matrix_vector_consistency();
    associativity_test();
}