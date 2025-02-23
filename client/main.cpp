
#include <iostream>
#include "openfhe.h"
#include "../utils/key_generation.h"

using namespace lbcrypto;

int main() {
    Crypto::FHEBFV::Instance().set_BFV_Context();
    // setup CKKS Crypto Context

    // saving keys
    Crypto::FHEBFV::Instance().save_crypto_context();
    Crypto::FHEBFV::Instance().save_public_key();
    Crypto::FHEBFV::Instance().save_private_key();
    Crypto::FHEBFV::Instance().save_mult_key();
    return 0;
}