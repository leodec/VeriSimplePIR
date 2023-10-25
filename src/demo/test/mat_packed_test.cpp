#include "pir/mat_packed.h"


void test_packed_binary_matrix_mult() {

    // const uint64_t leftRows = 100;
    const uint64_t leftRows = STAT_SEC_PARAM;
    const uint64_t leftCols = 10000;
    // const uint64_t leftCols = 5;
    const uint64_t rightRows = leftCols;
    const uint64_t rightCols = 500;
    // const uint64_t rightCols = 10;

    const uint64_t logp = 2;
    const uint64_t p = 1<<logp;
    // const uint64_t p = 10;

    BinaryMatrix binary(leftRows, leftCols);
    random(binary);
    // print(binary);

    Matrix right(rightRows, rightCols);
    random(right, p);
    // std::cout << "right = \n"; print(right);

    Matrix correct = matMul(binary.asMatrix(), right);
    // std::cout << "correct = \n"; print(correct);

    PackedMatrix rightPacked = packMatrix(right, p);

    Matrix result = matMulLeftBinaryRightColPacked(binary, rightPacked);

    if (!eq(correct, result, true)) {
        assert(false);
    }

    std::cout << "basic column packing binary mat mult test passed\n";

    double start, end;
    const uint32_t iters = 10;

    start = currentDateTime();
    for (uint32_t iter = 0; iter < iters; iter++)
        matMulLeftBinaryRightColPacked(binary, rightPacked);
    end = currentDateTime();
    std::cout << "basic column packing runtime: " << (end - start)/iters << " ms\n";

    PackedMatrix rightPacked_hardcoded = packMatrixHardCoded(right, p);
    Matrix result_hardcoded = matMulLeftBinaryRightColPacked_Hardcoded(binary, rightPacked_hardcoded);
    if (!eq(correct, result_hardcoded, true)) {
        assert(false);
    }

    std::cout << "v1 hardcoded binary mat mult test passed\n";

    start = currentDateTime();
    for (uint32_t iter = 0; iter < iters; iter++)
        matMulLeftBinaryRightColPacked_Hardcoded(binary, rightPacked_hardcoded);
    end = currentDateTime();
    std::cout << "v1 hardcoded packing runtime: " << (end - start)/iters << " ms\n";

    start = currentDateTime();
    for (uint32_t iter = 0; iter < iters; iter++)
        matMulLeftBinaryRightColPacked_Hardcoded(binary, rightPacked_hardcoded, true);
    end = currentDateTime();
    std::cout << "v1 hardcoded packing (padded output) runtime: " << (end - start)/iters << " ms\n";

    // Matrix result_hardcoded_v2 = matMulLeftBinaryRightColPacked_Hardcoded_v2(binary, rightPacked_hardcoded);
    // // std::cout << "result_hardcoded_v2 = \n"; print(result_hardcoded_v2);
    // if (!eq(correct, result_hardcoded_v2, true)) {
    //     assert(false);
    // }

    // std::cout << "v2 hardcoded binary mat mult test passed\n";

    // start = currentDateTime();
    // for (uint32_t iter = 0; iter < iters; iter++)
    //     matMulLeftBinaryRightColPacked_Hardcoded_v2(binary, rightPacked_hardcoded);
    // end = currentDateTime();
    // std::cout << "v2 hardcoded packing runtime: " << (end - start)/iters << " ms\n";
}

void test_packed_mat_vec_mul() {

    const uint64_t leftRows = 104;
    const uint64_t leftCols = 1024;
    const uint64_t rightRows = leftCols;
    const uint64_t rightCols = 1;

    const uint64_t logp = 2;
    const uint64_t p = 1<<logp;
    // const uint64_t p = 10;

    Matrix left(leftRows, leftCols);
    random(left, p);

    Matrix right(rightRows, rightCols);
    random(right);

    Matrix correct = matMulVec(left, right);

    PackedMatrix leftPacked = packMatrix(left, p);

    Matrix result = matVecMulColPacked(leftPacked, right);

    if (!eq(correct, result, true)) {
        assert(false);
    }

    std::cout << "basic column packing mat vec mult test passed\n";

    PackedMatrix leftPackedHardcoded = packMatrixHardCoded(left, p);

    // Matrix simplepir_result = simplepir_matVecMulColPacked(leftPackedHardcoded, right);

    //  if (!eq(simplepir_result, result, true)) {
    //     assert(false);
    // }

    std::cout << "simplepir basic column packing mat vec mult test passed\n";


    Matrix simplepir_variableComp_result = simplepir_matVecMulColPacked_variableCompression(leftPackedHardcoded, right);

     if (!eq(simplepir_variableComp_result, result, true)) {
        assert(false);
    }

    std::cout << "simplepir variable compression basic column packing mat vec mult test passed\n";

    Matrix simplepir_variableComp_nounroll_result = simplepir_matVecMulColPacked_variableCompression_noUnroll(leftPackedHardcoded, right);

     if (!eq(simplepir_variableComp_nounroll_result, result, true)) {
        assert(false);
    }

    std::cout << "simplepir variable compression basic column packing mat vec mult test passed\n";
}

void test_packed_mat_mul() {

    const uint64_t leftRows = 104;
    const uint64_t leftCols = 1024;
    const uint64_t rightRows = leftCols;
    const uint64_t rightCols = 100;

    const uint64_t logp = 2;
    const uint64_t p = 1<<logp;
    // const uint64_t p = 10;

    Matrix left(leftRows, leftCols);
    random(left, p);

    Matrix right(rightRows, rightCols);
    random(right);

    Matrix correct = matMul(left, right);

    // PackedMatrix leftPacked = packMatrix(left, p);
    PackedMatrix leftPackedHardcoded = packMatrixHardCoded(left, p);

    Matrix result = matMulColPacked(leftPackedHardcoded, right);

    if (!eq(correct, result, true)) {
        assert(false);
    }

    std::cout << "basic column packing mat mult test passed\n";
}

void bench_packed_mat_vec_mul() {

    const uint64_t leftRows = 104*8;
    const uint64_t leftCols = 500*30 + 8;
    const uint64_t rightRows = leftCols;
    const uint64_t rightCols = 1;

    const uint64_t logp = 2;
    const uint64_t p = 1<<logp;
    // const uint64_t p = 10;

    Matrix left(leftRows, leftCols);
    random(left, p);

    Matrix right(rightRows, rightCols);
    random(right);

    PackedMatrix leftPacked = packMatrix(left, p);

    double start, end;
    const uint32_t iters = 100;

    start = currentDateTime();
    for (size_t iter = 0; iter < iters; iter++) {
       Matrix result = matVecMulColPacked(leftPacked, right);
    }
    end = currentDateTime();
    std::cout << "our packed mat-vec mult = " << (end-start)/iters << " ms\n"; 

    PackedMatrix leftPackedHardcoded = packMatrixHardCoded(left, p);

    // start = currentDateTime();
    // for (size_t iter = 0; iter < iters; iter++) {
    //     Matrix simplepir_result = simplepir_matVecMulColPacked(leftPackedHardcoded, right);
    // }
    // end = currentDateTime();
    // std::cout << "simplepir packed mat-vec mult = " << (end-start)/iters << " ms\n"; 

    start = currentDateTime();
    for (size_t iter = 0; iter < iters; iter++) {
        Matrix simplepir_result = simplepir_matVecMulColPacked_variableCompression(leftPackedHardcoded, right);
    }
    end = currentDateTime();
    std::cout << "simplepir var comp packed mat-vec mult = " << (end-start)/iters << " ms\n"; 

    start = currentDateTime();
    for (size_t iter = 0; iter < iters; iter++) {
        Matrix simplepir_result = simplepir_matVecMulColPacked_variableCompression_noUnroll(leftPackedHardcoded, right);
    }
    end = currentDateTime();
    std::cout << "simplepir var comp no unroll packed mat-vec mult = " << (end-start)/iters << " ms\n"; 
}

int main() {

    test_packed_binary_matrix_mult();
    test_packed_mat_vec_mul();
    test_packed_mat_mul();
    bench_packed_mat_vec_mul();
}