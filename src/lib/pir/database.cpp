#include "database.h"
#include <map>

// const double lwe_error_prob = std::pow(2, -40);
#define LWE_ERROR_PROB std::pow(2, -40)

void print(entry_t val) {
    const entry_t zero(0);
    const entry_t base(10);
    std::vector<uint64_t> digits;
    do {
        digits.push_back((val % base).toUnsignedLong());
        val /= base;
    } while (val > zero);

    for (size_t i = digits.size(); i > 0; i--)
        std::cout << digits[i-1];
    // std::cout << std::endl;
}

std::pair<uint64_t, bool> get_num_logp_entries(const uint64_t N, const uint64_t d, const uint64_t logp) {

    uint64_t num_logp_entries;
    bool roundUpEll;

    if (d > logp) {
        // split d bits across several logp entries
        num_logp_entries = N * std::ceil(float(d) / float(logp));
        roundUpEll = true;
    } else {
        // pack several d-bit entries into a single logp element
        // num_logp_entries = std::ceil(float(N*d)/float(logp));
        const uint64_t num_entries_per_logp_elem = std::floor(float(logp)/float(d));
        num_logp_entries = std::ceil((double)N/(double)num_entries_per_logp_elem);
        roundUpEll = false;
    }

    return std::make_pair(num_logp_entries, roundUpEll);
}

uint64_t round_up_ell(const uint64_t ell, const uint64_t d, const uint64_t logp) {
    const uint64_t num_Zp_entries_per_db_entry = std::ceil(float(d) / logp);
    const uint64_t res_ell = (ell % num_Zp_entries_per_db_entry) == 0 ? ell : ell + num_Zp_entries_per_db_entry - (ell % num_Zp_entries_per_db_entry);
    assert(res_ell % num_Zp_entries_per_db_entry == 0); 
    return res_ell;
}

void get_balanced_m_and_ell(
    uint64_t & m, uint64_t & ell,
    const uint64_t N, const uint64_t d, const uint64_t logp, 
    const bool allowTrivial = false,
    const uint64_t batch_size = 1,
    const uint64_t approx_log_ell = 0
) {
    const uint64_t logq = LHE::logq;
    // const uint64_t n = LHE::n;
    const uint64_t stat_param = STAT_SEC_PARAM; // VLHEPIR::stat_sec_param;

    const auto num_entries_and_round = get_num_logp_entries(N, d, logp);
    const uint64_t num_logp_entries = std::get<0>(num_entries_and_round);
    const bool roundUpEll = std::get<1>(num_entries_and_round);

    m = floor(sqrt(double(num_logp_entries*batch_size*logq) / double(batch_size*logq + stat_param*(logp + approx_log_ell))));  // This is the line to change to optimize download balance
    if (m == 0) {
        if (allowTrivial) m = sqrt(num_logp_entries);
        else assert(false);
    } 
    ell = ceil(double(num_logp_entries) / double(m));
    if (roundUpEll) ell = round_up_ell(ell, d, logp);
    if (ell % 8 != 0)
        ell += 8 - (ell % 8);  // round up rows for hardcoded packing
    m = ceil(double(num_logp_entries) / double(ell));

    // non-trivial download check
    // if (!allowTrivial) {
    //     uint64_t hint_size = n*ell*logq;  // size of offline download
    //     if (hint_size >= N*d) {
    //         std::cout << "hitting digest size bound on ell\n";
    //         // ell = 0.99*dbBits//(n*logq)
    //         // m = ceil(float(N) / float(m));
    //         assert(false);
    //     }
    // }
    assert(ell > 0);
    if (ell == 0) {
        std::cout << N << " " << m << std::endl;
        assert(false);
    }

    if (abs(log2(ell) - approx_log_ell) >= 3) return get_balanced_m_and_ell(m, ell, N, d, logp, allowTrivial, batch_size, log2(ell));
}

void get_simple_pir_m_and_ell(
    uint64_t & m, uint64_t & ell,
    const uint64_t N, const uint64_t d, const uint64_t logp
) {
    const auto num_entries_and_round = get_num_logp_entries(N, d, logp);
    const uint64_t num_logp_entries = std::get<0>(num_entries_and_round);
    const bool roundUpEll = std::get<1>(num_entries_and_round);

    m = sqrt(num_logp_entries);
    ell = ceil(double(num_logp_entries) / double(m));
    if (roundUpEll) ell = round_up_ell(ell, d, logp);
    if (ell % 8 != 0)
        ell += 8 - (ell % 8);  // round up rows for hardcoded packing
    m = ceil(double(num_logp_entries) / double(ell));
}

// p modulus constraints

const Elem get_close_to_q() {
    return (1ULL << (LHE::logq-1)) - 1 + (1ULL << (LHE::logq-1));
}

Elem get_max_p_lwe_slack_correct(const uint64_t m, const uint64_t ell, const bool honest_hint) {
    constexpr double sigma = 6.4;   // fixed
    const Elem q = get_close_to_q();
    if (honest_hint) return std::sqrt(q / (std::sqrt(2*m*log(2/LWE_ERROR_PROB)) * sigma));
    else return std::sqrt(q / (std::sqrt(2*m*log(2/LWE_ERROR_PROB)) * sigma * 2 * ell));
}

uint64_t get_max_m_lwe_slack_correct(const Elem p, const uint64_t ell, const bool honest_hint) {
    constexpr double sigma = 6.4;   // fixed
    const Elem q = get_close_to_q();
    if (honest_hint) return pow(q / (sigma * pow(p, 2)), 2) / (2*log(2/LWE_ERROR_PROB));
    else return pow(q / (sigma * 2 * ell * pow(p, 2)), 2) / (2*log(2/LWE_ERROR_PROB));
}

Elem get_right_check_sis() {
    // delta = 1.005
    const uint64_t right_check_exp = 2*sqrt(LHE::n * LHE::logq*log2(1.005));
    const Elem right_check = 1ULL << std::min(LHE::logq, right_check_exp);
    return right_check;
}

Elem get_max_p_sis_hardness(const uint64_t m, const uint64_t ell, Elem right_check = 0) {
    if (right_check == 0)
        right_check = get_right_check_sis();
    return right_check/(4*ell*sqrt(m));
}

// correctness and security checks

bool check_slack_lwe_correctness(
    const Elem p, const uint64_t m, 
    const bool honest_hint = true, const uint64_t ell=1, 
    const bool verbose=false
) {
    constexpr double sigma = 6.4;   // fixed

    // uint64_t lower_bound = sigma * pow(p, 2) * sqrt(2*m*log(2/LWE_ERROR_PROB));
    double log_lower_bound = log2(sigma) + 2*log2(p) + 0.5*log2(2*m*log(2/LWE_ERROR_PROB));
    if (!honest_hint)
        log_lower_bound += 1 + log2(ell);
        // lower_bound *= ell*2;

    // const Elem q = get_close_to_q();
    // if (q < lower_bound) {
    if (LHE::logq < log_lower_bound) {
        if (verbose) 
            std::cout << "slack lwe correctness failed\n";
        return false;
    }

    return true;
}

bool check_sis_hardness(const Elem p, const uint64_t m, const uint64_t ell, const bool verbose) {
    const Elem left = 4*ell*p*sqrt(m);
    const Elem right = get_right_check_sis();
    if (left > right) {
        if (verbose) std::cout << "sis hardness failed\n";
        return false;
    }
    return true;
}

// Communication checks

uint64_t get_per_query_communication_in_bits(const uint64_t m, const uint64_t ell, const uint64_t logp, const bool verbose, const uint64_t batch_size) {
    // minimizing parameters are m ~ sqrt(N/(stat_param+1)) and ell = N/m
    
    const uint64_t stat_param = STAT_SEC_PARAM; // VLHEPIR::stat_sec_param;
    const uint64_t logq = LHE::logq;

    const uint64_t upload = batch_size*m*logq; 
    const uint64_t download = batch_size*ell*logq + m*stat_param*(log2(ell) + logp);
    if (verbose)
        std::cout << "download balance: " << 
            ell*logq << ", " << 
            m*stat_param*(logp + log2(ell)) << ", " << 
            ell*logq / (m*stat_param*(logp + log2(ell))) << std::endl;
    return upload + download;
}

uint64_t get_simplepir_per_query_communication_in_bits(const uint64_t m, const uint64_t ell, const uint64_t logp, const bool verbose, const uint64_t batch_size) {
    const uint64_t logq = LHE::logq;
    const uint64_t upload = batch_size*m*logq; 
    const uint64_t download = batch_size*ell*logq;
    return upload + download;
}


Elem ith_digit_base_p(entry_t x, const uint64_t i, const Elem p) {
    const entry_t big_p(p);
    for (uint64_t j = 0; j < i; j++) {
        x /= big_p;
    }
    return (x % big_p).toUnsignedLong();
}

std::vector<Elem> get_digits_base_p(entry_t x, const Elem p) {
    const entry_t big_p(p);
    std::vector<Elem> result;
    do {
        result.push_back((x % big_p).toUnsignedLong());
        x /= big_p;
    } while(x > 0);
    return result;
}

entry_t reconstruct_base_p(const Elem * vals, const uint64_t num_digits, const Elem p) {
    entry_t res(0);
    const entry_t big_p(p);
    entry_t scale(1);

    for (uint64_t i = 0; i < num_digits; i++) {
        res += entry_t(vals[i]) * scale;
        scale *= big_p;
    }

    return res;
}

Matrix Database::packDataInMatrix(const PlaintextDBParams& params, const bool verbose) const {
    // database consists of N entries of at most d bits

    std::cout << "packing ratio: " << double(N*d) / (double(params.m*params.ell)*log2(params.p)) << std::endl;

    if (floor(log2(params.p)) >= d) {

        if (verbose) std::cout << "pt modulus is at least as large as db entries\n";

        const uint64_t complete_pt_bits = floor(log2(params.p));

        if (complete_pt_bits * params.m * params.ell < N*d) {
            std::cout << complete_pt_bits * params.m * params.ell << " " <<  N*d << std::endl;
            std::cout << "not enough bits in the matrix!\n";
            assert(false);
        }

        // Matrix result(params.ell, params.m);
        Matrix result(params.m, params.ell);  // column-pack data to stay consistent with large-element packing
        
        // at least one entry per Zp element
        const uint64_t elems_per_Zp = complete_pt_bits/d;
        // std::cout << elems_per_Zp << " " << complete_pt_bits << " " << d << std::endl;

        if (double(N)/double(elems_per_Zp) > params.m*params.ell) {
            std::cout << double(N)/double(elems_per_Zp) << " " << params.m*params.ell << std::endl;
            std::cout << "need tighter packing!\n";
            assert(false);
        }

        uint64_t matrixDataInd = 0;
        for (uint64_t firstInd = 0; firstInd < N; firstInd += elems_per_Zp) {
            Elem val = 0;
            for (uint64_t innerElemInd = 0; innerElemInd < elems_per_Zp; innerElemInd++) {
                const uint64_t elem_ind = firstInd + innerElemInd;
                if (elem_ind >= N) break;
                // this conversion is always safe, since we know that the value fits within an Elem
                const Elem dataElem = getDataAtIndex(elem_ind).toUnsignedLong();
                val += (dataElem) << (d * innerElemInd);
            }
            result.data[matrixDataInd] = val;
            matrixDataInd += 1;
        }

        if (verbose) {
            std::cout << "stopped at matrix index " << matrixDataInd << " out of " << params.m*params.ell << "\n"; 
            if ((params.m-1)*(params.ell) >= matrixDataInd) {
                std::cout << "at least one column is empty!\n";
            }
        }

        return transpose(result);  // result is ell x m

    } else {

        if (verbose) std::cout << "splitting database entries across multiple Zp elements.\n";

        // entries should be COLUMN-PACKED to provide answer in a single query
        Matrix resultTranspose(params.m, params.ell);

        // split entries across multiple Zp elements
        const uint64_t Zp_vals_per_elem = ceil(double(d) / log2(params.p));

        // std::cout << Zp_vals_per_elem << " " << d << " " << log2(params.p) << std::endl;

        if (Zp_vals_per_elem > params.ell) {
            std::cout << "elements don't fit in a column\n";
            assert(false);
        }

        uint64_t matrixDataInd = 0;
        for (uint64_t elemInd = 0; elemInd < N; elemInd++) {

            if (params.ell < (matrixDataInd % params.ell) + Zp_vals_per_elem) {
                std::cout << "skipping " << matrixDataInd % params.ell << " entries\n";
                std::cout << matrixDataInd << " " << Zp_vals_per_elem << std::endl;
                matrixDataInd += matrixDataInd % params.ell; // skip partial entries to avoid double column response
            }

            if (matrixDataInd > params.m*params.ell - Zp_vals_per_elem) {
                std::cout << matrixDataInd << " " << params.m*params.ell - Zp_vals_per_elem << " " << Zp_vals_per_elem << std::endl;
                std::cout << "ran out of matrix elements on element index " << elemInd << " out of " << N << std::endl;
                assert(false);
            }

            const entry_t toSplit = getDataAtIndex(elemInd);

            std::vector<Elem> digits = get_digits_base_p(toSplit, params.p);
            assert(digits.size() <= Zp_vals_per_elem);
            for (uint64_t innerElemInd = 0; innerElemInd < digits.size(); innerElemInd++)
                resultTranspose.data[matrixDataInd + innerElemInd] = digits[innerElemInd];
            for (uint64_t innerElemInd = digits.size(); innerElemInd < Zp_vals_per_elem; innerElemInd++)
                resultTranspose.data[matrixDataInd + innerElemInd] = 0;

            // for (uint64_t innerElemInd = 0; innerElemInd < Zp_vals_per_elem; innerElemInd++)
            //     resultTranspose.data[matrixDataInd + innerElemInd] = ith_digit_base_p(toSplit, innerElemInd, params.p);

            matrixDataInd += Zp_vals_per_elem;
        }

        return transpose(resultTranspose);  // actual result is ell x m
    }
}



// Top-level parameter computations

PlaintextDBParams Database::computeParams(const bool allowTrivial, const bool verbose, const bool simplePIR, const uint64_t batchSize, const bool preproc, const bool honest_hint) const {

    if (preproc) {
        assert(!simplePIR);
    }

    if (simplePIR) {
        assert(!preproc);
    }

    const uint64_t logq = LHE::logq; 
    const uint64_t n = LHE::n;

    // const bool honest_hint = false;
    // const bool honest_hint = true;

    if (honest_hint) 
        std::cout << "HONEST HINT ASSUMPTION IS ON\n";

    Elem p = 0;
    uint64_t m = 0;
    uint64_t ell = 0;
    uint64_t total_comm = 0;

    // std::cout << "parameter search for N = " << N << " and d = " << d << std::endl;

    for (uint64_t logp = 1; logp < logq; logp++) {

        // std::cout << "testing logp = " << logp << std::endl;

        // const auto num_entries_and_round = get_num_logp_entries(N, d, logp);
        // const uint64_t num_logp_entries = std::get<0>(num_entries_and_round);
        // const bool roundUpEll = std::get<1>(num_entries_and_round);

        // std::cout << "number of logp entries = " << num_logp_entries << std::endl;

        uint64_t test_m;
        uint64_t test_ell;

        if (!simplePIR && !preproc) get_balanced_m_and_ell(test_m, test_ell, N, d, logp, allowTrivial, batchSize);
        else get_simple_pir_m_and_ell(test_m, test_ell, N, d, logp);

        // std::cout << "test_m, test_ell = " << test_m << " " << test_ell << std::endl;

        if (!allowTrivial) {
            const uint64_t hint_size = n*test_ell*logq;
            if (hint_size >= N*d)
                continue;
                // std::cout << "hint size is greater than the database size. trivial preprocessing flag is set to false\n";
                // std::cout << "test_m, test_ell = " << test_m << " " << test_ell << std::endl;
                // assert(false);
        }

        const uint64_t test_p = 1ULL << logp;

        // while (test_m != 0 && !check_slack_lwe_correctness(test_p, test_m, honest_hint, test_ell, verbose)) {
        //     test_m = get_max_m_lwe_slack_correct(test_p, test_ell, honest_hint);
        //     test_ell = ceil(double(num_logp_entries) / double(test_m));
        //     if (roundUpEll) test_ell = round_up_ell(test_ell, d, logp);
        //     // assert(check_slack_lwe_correctness(test_p, test_m, honest_hint, test_ell, verbose));
        //     // continue;
        //     if (verbose) std::cout << "updated for lwe hardness: " << test_m << " " << test_ell << std::endl;
        // }
        if (!simplePIR) {
            if (!check_slack_lwe_correctness(test_p, test_m, honest_hint, test_ell, verbose)) break;
            if (!check_sis_hardness(test_p, test_m, test_ell, verbose)) break;
        } else {
            if (!check_slack_lwe_correctness(test_p, test_m, true, test_ell, verbose)) break;
        }

        if (test_m == 0) continue;

        uint64_t total;
        if (simplePIR || preproc) total = get_simplepir_per_query_communication_in_bits(test_m, test_ell, logp, verbose, batchSize);
        else total = get_per_query_communication_in_bits(test_m, test_ell, logp, verbose, batchSize);
        if ((total_comm == 0) || (total < total_comm)) {
            if (verbose) {
                std::cout << "total comm (bits): " <<  total << std::endl;
                std::cout << "new opt params " << test_p << " " << test_m << " " << test_ell << std::endl;
            }
            p = test_p;
            m = test_m;
            ell = test_ell;
            total_comm = total;
        }
    }

    assert(p > 0);
    if (!simplePIR) {
        assert(check_slack_lwe_correctness(p, m, honest_hint, ell, verbose));
        assert(check_sis_hardness(p, m, ell, verbose));
    } else 
        assert(check_slack_lwe_correctness(p, m, true, ell, verbose));
    if (N*d > m*ell*log2(p)) {
        std::cout << "db bits " << N*d << std::endl;
        std::cout << "available bits: " << uint64_t(m*ell*log2(p)) << std::endl;
        assert(false);
    }

    PlaintextDBParams result; 
    result.N = N; result.d = d;
    result.p = p;
    result.ell = ell;
    result.m = m;

    return result;
}