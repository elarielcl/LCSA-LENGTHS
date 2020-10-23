

#include <iostream>
#include <string>
#include <vector>
#include <sdsl/csa_bitcompressed.hpp>
#include <lcsa/LCSALENGTHS.hpp>


int main() {

    std::string input;
    std::ifstream t("../../data/dna");
    std::stringstream buffer;
    buffer << t.rdbuf();
    input = buffer.str();

    sdsl::csa_bitcompressed<> csa;
    sdsl::construct_im(csa, input, 1);
    LCSALENGTHS::LCSALENGTHS* lcsa_lengths = new LCSALENGTHS::LCSALENGTHS(csa.sa_sample, 128); // Fix Sweet point

    int d = 129;
    for (int i = 0; i < csa.size()-d; ++i) {
        auto q = lcsa_lengths->extract(i, i+d);
        for (int j = 0; j< q.size(); ++j) {
            if (q[j] != csa[i+j]) {
                std::cout << "error " << i << ", " << j << std::endl;
                exit(0);
            }

        }
    }

    return 0;
}

