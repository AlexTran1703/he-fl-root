#include "utils_key_generation.h"
#include "utils.h"
#include <openssl/evp.h>
#include <openssl/param_build.h>
#include <openssl/core_names.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/rand.h>
#include <openssl/kdf.h>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>

namespace AsymKeyUtils
{
    using namespace Utils;

    // Generate EC Key Pair
    EVP_PKEY *generate_ec_key()
    {
        EVP_PKEY_CTX *pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, NULL);
        if (!pctx)
            handleErrors();

        if (EVP_PKEY_keygen_init(pctx) <= 0)
            handleErrors();
        if (EVP_PKEY_CTX_set_ec_paramgen_curve_nid(pctx, NID_X9_62_prime256v1) <= 0)
            handleErrors();

        EVP_PKEY *key = nullptr;
        if (EVP_PKEY_keygen(pctx, &key) <= 0)
            handleErrors();

        EVP_PKEY_CTX_free(pctx);
        return key;
    }

    // Extract Public Key as Byte Array
    std::vector<unsigned char> get_public_key_bytes(EVP_PKEY *key)
    {
        int len = i2d_PUBKEY(key, NULL); // Get required buffer size
        if (len <= 0)
            handleErrors();

        std::vector<unsigned char> pubkey_bytes(len);
        unsigned char *temp = pubkey_bytes.data();
        if (i2d_PUBKEY(key, &temp) <= 0)
            handleErrors();

        return pubkey_bytes;
    }

    // Load Public Key from Byte Array
    EVP_PKEY *load_public_key_from_bytes(const std::vector<unsigned char> &pubkey_bytes)
    {
        const unsigned char *temp = pubkey_bytes.data();
        EVP_PKEY *key = d2i_PUBKEY(NULL, &temp, pubkey_bytes.size());
        if (!key)
            handleErrors();
        return key;
    }

    // Compute Shared Secret
    std::vector<unsigned char> compute_shared_secret(EVP_PKEY *private_key, EVP_PKEY *peer_public_key)
    {
        EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(private_key, NULL);
        if (!ctx)
            handleErrors();

        if (EVP_PKEY_derive_init(ctx) <= 0)
            handleErrors();
        if (EVP_PKEY_derive_set_peer(ctx, peer_public_key) <= 0)
            handleErrors();

        size_t secret_len;
        if (EVP_PKEY_derive(ctx, NULL, &secret_len) <= 0)
            handleErrors();

        std::vector<unsigned char> shared_secret(secret_len);
        if (EVP_PKEY_derive(ctx, shared_secret.data(), &secret_len) <= 0)
            handleErrors();

        EVP_PKEY_CTX_free(ctx);
        shared_secret.resize(secret_len); // Adjust size to actual secret length
        return shared_secret;
    }
    // Derive AES-256 Key using HKDF
    std::vector<unsigned char> derive_aes_key(const std::vector<unsigned char>& shared_secret) {
        std::vector<unsigned char> aesKey(32);  // AES-256 key (32 bytes)
        
        EVP_PKEY_CTX* pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_HKDF, nullptr);
        if (!pctx) handleErrors();
    
        if (EVP_PKEY_derive_init(pctx) <= 0) handleErrors();
        
        if (EVP_PKEY_CTX_set_hkdf_mode(pctx, EVP_KDF_HKDF_MODE_EXTRACT_AND_EXPAND) <= 0) handleErrors();
        if (EVP_PKEY_CTX_set_hkdf_md(pctx, EVP_sha256()) <= 0) handleErrors();
        
        // No salt (optional)
        if (EVP_PKEY_CTX_set1_hkdf_salt(pctx, nullptr, 0) <= 0) handleErrors();
        
        // Set input keying material (shared secret)
        if (EVP_PKEY_CTX_set1_hkdf_key(pctx, shared_secret.data(), shared_secret.size()) <= 0) handleErrors();
        
        // Context-specific info (optional)
        const unsigned char info[] = "ECDH AES Key";
        if (EVP_PKEY_CTX_add1_hkdf_info(pctx, info, sizeof(info)) <= 0) handleErrors();
        
        // Derive the key
        size_t outLen = aesKey.size();
        if (EVP_PKEY_derive(pctx, aesKey.data(), &outLen) <= 0) handleErrors();
        
        EVP_PKEY_CTX_free(pctx);
        return aesKey;
    }

}

namespace SymKeyUtils
{
    using namespace Utils;
    // AES-GCM Encryption (Returns Base64 String)
    std::string aes_gcm_encrypt(const std::string &plaintext, const std::vector<unsigned char> &key)
    {
        EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
        if (!ctx)
            handleErrors();

        std::vector<unsigned char> iv(12);  // 96-bit IV
        std::vector<unsigned char> tag(16); // 128-bit tag

        if (!RAND_bytes(iv.data(), iv.size()))
            handleErrors(); // Generate random IV

        if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key.data(), iv.data()) != 1)
            handleErrors();

        std::vector<unsigned char> ciphertext(plaintext.size());
        int len;

        if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, (unsigned char *)plaintext.data(), plaintext.size()) != 1)
            handleErrors();

        int ciphertext_len = len;

        if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1)
            handleErrors();

        ciphertext_len += len;
        ciphertext.resize(ciphertext_len);

        // Get authentication tag
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag.data()) != 1)
            handleErrors();

        EVP_CIPHER_CTX_free(ctx);

        // Concatenate IV + Ciphertext + Tag
        std::vector<unsigned char> encrypted_data;
        encrypted_data.insert(encrypted_data.end(), iv.begin(), iv.end());
        encrypted_data.insert(encrypted_data.end(), ciphertext.begin(), ciphertext.end());
        encrypted_data.insert(encrypted_data.end(), tag.begin(), tag.end());

        // Convert to Base64 for easy storage/transmission
        return base64_encode(encrypted_data);
    }
    // AES-GCM Decryption (Takes Base64 String)
    std::string aes_gcm_decrypt(const std::string &base64_ciphertext, const std::vector<unsigned char> &key)
    {
        std::vector<unsigned char> encrypted_data = base64_decode(base64_ciphertext);

        if (encrypted_data.size() < 12 + 16)
        {
            std::cerr << "Invalid ciphertext format!" << std::endl;
            exit(EXIT_FAILURE);
        }

        // Extract IV, Ciphertext, and Tag
        std::vector<unsigned char> iv(encrypted_data.begin(), encrypted_data.begin() + 12);
        std::vector<unsigned char> tag(encrypted_data.end() - 16, encrypted_data.end());
        std::vector<unsigned char> ciphertext(encrypted_data.begin() + 12, encrypted_data.end() - 16);

        EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
        if (!ctx)
            handleErrors();

        if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key.data(), iv.data()) != 1)
            handleErrors();

        std::vector<unsigned char> plaintext(ciphertext.size());
        int len;

        if (EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data(), ciphertext.size()) != 1)
            handleErrors();

        int plaintext_len = len;

        // Set authentication tag for verification
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, tag.data()) != 1)
            handleErrors();

        if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len) <= 0)
        {
            std::cerr << "Decryption failed! Authentication failed." << std::endl;
            exit(EXIT_FAILURE);
        }

        plaintext_len += len;
        plaintext.resize(plaintext_len);

        EVP_CIPHER_CTX_free(ctx);

        return vector_to_string(plaintext);
    }
}