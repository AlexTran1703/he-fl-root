#include "ws/ws_server.h"
#include <iostream>
#include <thread>
#include "utils_key_generation.h"
#include "he_key_generation.h"
#include <chrono>

using namespace AsymKeyUtils;
using namespace SymKeyUtils;
int main() {
    // Step 1: Generate Key Pairs for Both Parties
    EVP_PKEY* keyA = generate_ec_key();
    EVP_PKEY* keyB = generate_ec_key();

    // Step 2: Extract Public Keys as Byte Arrays (Simulating Key Exchange)
    std::vector<unsigned char> pubkeyA_bytes = get_public_key_bytes(keyA);
    std::vector<unsigned char> pubkeyB_bytes = get_public_key_bytes(keyB);

    // Step 3: Load Public Keys from Byte Arrays
    EVP_PKEY* pubA = load_public_key_from_bytes(pubkeyA_bytes);
    EVP_PKEY* pubB = load_public_key_from_bytes(pubkeyB_bytes);

    // Step 4: Compute Shared Secret
    std::vector<unsigned char> secretA = compute_shared_secret(keyA, pubB);
    std::vector<unsigned char> secretB = compute_shared_secret(keyB, pubA);
    //std::string str1(secretA.begin(), secretA.end());
    //std::string str2(secretB.begin(), secretB.end());
    std::cout << secretA << std::endl;
    std::cout << secretB << std::endl;
    //std::vector<unsigned char> vec1(str1.begin(), str1.end());
    //std::vector<unsigned char> vec2(str2.begin(), str2.end());
    // Step 5: Verify Both Parties Derive the Same Secret
    if (secretA == secretB) {
        std::cout << "ECDH Key Exchange Successful!" << std::endl;
    } else {
        std::cout << "Shared secrets do not match!" << std::endl;
    }
    std::vector<unsigned char> aesKey1 = derive_aes_key(secretA);
    std::vector<unsigned char> aesKey2 = derive_aes_key(secretB);
    std::cout << aesKey1 << std::endl;
    std::cout << aesKey2 << std::endl;
    // Cleanup
    std::string encrypted = aes_gcm_encrypt("Hello world", aesKey1);
    std::cout << aes_gcm_decrypt(encrypted, aesKey2);
    EVP_PKEY_free(keyA);
    EVP_PKEY_free(keyB);
    EVP_PKEY_free(pubA);
    EVP_PKEY_free(pubB);
    Utils::LOG_INFO("Start Server");
    Crypto::FHEBFV::Instance().set_context();
    try {
        WebSocketServer::getInstance(8080);
        std::cout << "WebSocket Server running on port 8080...\n";
        
        std::this_thread::sleep_for(std::chrono::hours(1)); // Keeps main thread alive

    } catch (const std::exception& e) {
        std::cerr << "Server Error: " << e.what() << std::endl;
    }
    return 0;
}
