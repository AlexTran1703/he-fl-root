
#include <iostream>
#include "openfhe.h"
#include "../utils/key_generation.h"

using namespace lbcrypto;

int main() {
    //Crypto::FHEBFV::Instance().set_context();
    // setup CKKS Crypto Context
    Utils::LOG_INFO("Application started");
    Crypto::FHEBFV::Instance().load_keys();

    // Step 3: Encrypt two integers
    int64_t num1 = 7;
    int64_t num2 = 15;
    Ciphertext<DCRTPoly> ct1 = Crypto::FHEBFV::Instance().encrypt({num1});
    Ciphertext<DCRTPoly> ct2 = Crypto::FHEBFV::Instance().encrypt({num2});
    Ciphertext<DCRTPoly> result = Crypto::FHEBFV::Instance().eval_add(ct1, ct2);
    int64_t result_end = Crypto::FHEBFV::Instance().decrypt(result)[0];
    std::cout << "The result of adding " << num1 << " and " << num2 << " is: " << result_end << std::endl;
    Ciphertext<DCRTPoly> result_cipher = Crypto::FHEBFV::Instance().eval_mult(ct1, ct2);
    int64_t result_mul = Crypto::FHEBFV::Instance().decrypt(result_cipher)[0];
    std::cout << "The result of multiply " << num1 << " and " << num2 << " is: " << result_mul << std::endl;
    return 0;
}