#include "pir/database.h"

void test_packing_one_elem_per_Zp(const bool simplePIR = false) {
    // const uint64_t N = 1ULL<<20;
    // const uint64_t N = 1ULL<<10;
    const uint64_t N = 100;
    // const uint64_t N = 25;
    const uint64_t d = 8;

    Database db(N, d); 

    db.loadRandomData();

    // for (uint64_t i = 0; i < 10; i++) {
    //     std::cout << "data["<<i<<"] = " << db.getDataAtIndex(i).toUnsignedLong() << std::endl;
    // }

    PlaintextDBParams dbParams = db.computeParams(true, false, simplePIR);
    dbParams.print();

    Matrix packed_db = db.packDataInMatrix(dbParams);

    // print(packed_db);

    for (uint64_t i = 0; i < N; i++) {
        const uint64_t column = dbParams.indexToColumn(i);
        const uint64_t row = dbParams.indexToRow(i);

        Elem element = packed_db.getElem(row, column);
        entry_t toCheck = dbParams.recover(&element, i);
        if (toCheck != db.data[i]) {
            // std::cout << i << std::endl;
            // print(db.data[i]);
            // std::cout << row << " " << column << std::endl;
            // std::cout << element << std::endl;
            // print(toCheck);
            // assert(false);
            std::cout << "index " << i << std::endl;
            std::cout << "column " << column << std::endl; 
            std::cout << "row " << row << std::endl;
            std::cout << "element " << element << std::endl;
            print(toCheck); std::cout << std::endl;
            print(db.data[i]); std::cout << std::endl;
            assert(false);
        }
    } 

    std::cout << "basic database packing test passed\n\n";
}

void test_packing_many_elem_per_Zp(const bool simplePIR = false) {
    const uint64_t N = 1ULL<<20;
    // const uint64_t N = 25;
    const uint64_t d = 1;

    Database db(N, d); 

    db.loadRandomData();

    // for (uint64_t i = 0; i < 10; i++) {
    //     std::cout << "data["<<i<<"] = " << db.getDataAtIndex(i).toUnsignedLong() << std::endl;
    // }

    PlaintextDBParams dbParams = db.computeParams(true, false, simplePIR);
    dbParams.print();

    Matrix packed_db = db.packDataInMatrix(dbParams);

    // print(packed_db);

    for (uint64_t i = 0; i < N; i++) {
        const uint64_t column = dbParams.indexToColumn(i);
        const uint64_t row = dbParams.indexToRow(i);

        Elem element = packed_db.getElem(row, column);
        entry_t toCheck = dbParams.recover(&element, i);
        if (toCheck != db.data[i]) {
            std::cout << "index " << i << std::endl;
            std::cout << "column " << column << std::endl; 
            std::cout << "row " << row << std::endl;
            std::cout << "element " << element << std::endl;
            print(toCheck); std::cout << std::endl;
            print(db.data[i]); std::cout << std::endl;
            assert(false);
        }
    } 

    std::cout << "small element database packing test passed\n\n";
}

void test_packing_no_elem_per_Zp(const bool simplePIR = false) {
    const uint64_t N = 1ULL<<8;
    // const uint64_t N = 25;
    const uint64_t d = 39;

    Database db(N, d); 

    db.loadRandomData();
    // db.loadIndexData();

    // for (uint64_t i = 0; i < 10; i++) {
    //     std::cout << "data["<<i<<"] = " << db.getDataAtIndex(i) << std::endl;
    // }

    PlaintextDBParams dbParams = db.computeParams(true, false, simplePIR);
    dbParams.print();

    Matrix packed_db = db.packDataInMatrix(dbParams);

    // print(packed_db);

    for (uint64_t i = 0; i < N; i++) {
        const uint64_t column = dbParams.indexToColumn(i);
        const uint64_t row = dbParams.indexToRow(i);

        Matrix result = packed_db.getColumn(column);  // query response
        entry_t toCheck = dbParams.recover((result.data + row), i);
        if (toCheck != db.data[i]) {
            std::cout << "index " << i << std::endl;
            std::cout << "column " << column << std::endl; print(result);
            std::cout << "beginning at row " << row << std::endl;
            print(toCheck); std::cout << std::endl;
            print(db.data[i]); std::cout << std::endl;
            assert(false);
        }
    } 

    std::cout << "large element database packing test passed\n\n";
}

int main() {

    test_packing_one_elem_per_Zp();
    test_packing_one_elem_per_Zp(true);

    test_packing_many_elem_per_Zp();
    test_packing_many_elem_per_Zp(true);

    test_packing_no_elem_per_Zp();
    test_packing_no_elem_per_Zp(true);
}