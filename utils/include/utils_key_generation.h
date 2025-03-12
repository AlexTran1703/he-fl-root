#pragma once

#include "utils.h"
#include <openssl/evp.h>
#include <openssl/param_build.h>
#include <openssl/core_names.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>

namespace AsymKeyUtils{
    std::vector<unsigned char> get_public_key_bytes(EVP_PKEY *key);
    EVP_PKEY *generate_ec_key();
    EVP_PKEY *load_public_key_from_bytes(const std::vector<unsigned char> &pubkey_bytes);
    std::vector<unsigned char> compute_shared_secret(EVP_PKEY *private_key, EVP_PKEY *peer_public_key);
    std::vector<unsigned char> derive_aes_key(const std::vector<unsigned char> &shared_secret);
}

namespace SymKeyUtils{
    std::string aes_gcm_encrypt(const std::string &plaintext, const std::vector<unsigned char> &key);
    std::string aes_gcm_decrypt(const std::string& base64_ciphertext, const std::vector<unsigned char>& key);
}
    