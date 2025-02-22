#include <iostream>
#include "seal/seal.h"

using namespace std;
using namespace seal;

int main() {
    // Step 1: Create encryption parameters
    EncryptionParameters parms(scheme_type::bfv);  // Updated to "bfv" instead of "BFV"

    // Set up parameters (you can change them depending on your needs)
    parms.set_poly_modulus_degree(8192); // Polynomial modulus degree
    parms.set_coeff_modulus(CoeffModulus::Create(8192, { 60, 40, 60 })); // Coefficients modulus

    std::cout << "Hello \r\n";

    return 0;
}
