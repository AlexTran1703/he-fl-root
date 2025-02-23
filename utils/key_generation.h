#pragma once

#include <iomanip>
#include <tuple>
#include <unistd.h>

#include "openfhe.h"

// header files needed for serialization
#include "ciphertext-ser.h"
#include "cryptocontext-ser.h"
#include "key/key-ser.h"
#include "scheme/bfvrns/bfvrns-ser.h"
#include "utils.h"
#include <vector>

using namespace lbcrypto;

/*
    constant parameters
*/
// const int multDepth    = 5;
// const int scaleModSize = 40;
// const usint batchSize  = 32;

/*
    location for storing keys
    === IMPORTANT ===
    If running locally, you may want to replace the "hardcoded" `DATAFOLDER` with
    the `DATAFOLDER` location below which gets the current working directory
*/

namespace Crypto
{
    class FHE
    {
    protected:
        std::string ccLocation = "/crypto_context.txt";
        std::string pubKeyLocation = "/public_key.txt";  // Pub key
        std::string priKeyLocation = "/private_key.txt"; // Pri key
        std::string multKeyLocation = "/mult_key.txt";
        void load_crypto_context(std::string folder, CryptoContext<DCRTPoly> &CC)
        {
            /*
                Load Crypto Context from file
            */
            CC->ClearEvalMultKeys();
            CC->ClearEvalAutomorphismKeys();

            lbcrypto::CryptoContextFactory<lbcrypto::DCRTPoly>::ReleaseAllContexts();
            if (!Serial::DeserializeFromFile(folder + ccLocation, CC, SerType::BINARY))
            {
                Utils::LOG_ERROR("Can not read serialized data from: " + folder + ccLocation);
                std::cerr << "Cannot read serialization from " << folder + ccLocation << std::endl;
                std::exit(1);
            }
        }

        void load_public_key(std::string folder, KeyPair<DCRTPoly> &KP)
        {
            /*
                Load Public Key
            */
            if (!Serial::DeserializeFromFile(folder + pubKeyLocation, KP.publicKey, SerType::BINARY))
            {
                Utils::LOG_ERROR("Can not read serialized data from: " + folder + pubKeyLocation);
                std::cerr << "Cannot read serialization from " << folder + pubKeyLocation << std::endl;
                std::exit(1);
            }
        }

        void load_priv_key(std::string folder, KeyPair<DCRTPoly> &KP)
        {
            /*
                Load Private Key
            */
            if (!Serial::DeserializeFromFile(folder + priKeyLocation, KP.secretKey, SerType::BINARY))
            {
                Utils::LOG_ERROR("Can not read serialized data from: " + folder + priKeyLocation);
                std::cerr << "Cannot read serialization from " << folder + priKeyLocation << std::endl;
                std::exit(1);
            }
        }

        void load_mult_key(std::string folder, CryptoContext<DCRTPoly> &CC)
        {
            /*
                Load Mult Key
            */
            std::ifstream multKeyIStream(folder + multKeyLocation, std::ios::in | std::ios::binary);
            if (!multKeyIStream.is_open())
            {
                Utils::LOG_ERROR("Can not read serialized data from: " + folder + multKeyLocation);
                std::cerr << "Cannot read serialization from " << folder + multKeyLocation << std::endl;
                std::exit(1);
            }
            if (!CC->DeserializeEvalMultKey(multKeyIStream, SerType::BINARY))
            {
                Utils::LOG_ERROR("Could not deserialize eval mult key file");
                std::cerr << "Could not deserialize eval mult key file" << std::endl;
                std::exit(1);
            }
        }

        void save_crypto_context(std::string folder, CryptoContext<DCRTPoly> &CC)
        {
            // saving CryptoContext
            Utils::folder_exists(folder);
            if (!Serial::SerializeToFile(folder + ccLocation, CC, SerType::BINARY))
            {
                Utils::LOG_ERROR("Error writing serialization of the crypto context to " + folder + ccLocation);
                std::cerr << "Error writing serialization of the crypto context to " << folder + ccLocation << std::endl;
                std::exit(1);
            }
        }

        void save_public_key(std::string folder, KeyPair<DCRTPoly> &KP)
        {
            Utils::folder_exists(folder);
            // saving Public Key
            if (!Serial::SerializeToFile(folder + pubKeyLocation, KP.publicKey, SerType::BINARY))
            {
                Utils::LOG_ERROR("Exception writing public key to" + folder + pubKeyLocation);
                std::cerr << "Exception writing public key to" << folder + pubKeyLocation << std::endl;
                std::exit(1);
            }
        }

        void save_private_key(std::string folder, KeyPair<DCRTPoly> &KP)
        {
            Utils::folder_exists(folder);
            // saving Private Key
            if (!Serial::SerializeToFile(folder + priKeyLocation, KP.secretKey, SerType::BINARY))
            {
                Utils::LOG_ERROR("Exception writing public key to" + folder + priKeyLocation);
                std::cerr << "Exception writing public key to" << folder + priKeyLocation << std::endl;
                std::exit(1);
            }
        }

        void save_mult_key(std::string folder, CryptoContext<DCRTPoly> &CC)
        {
            Utils::folder_exists(folder);
            // saving mult key
            std::ofstream multKeyFile(folder + multKeyLocation, std::ios::out | std::ios::binary);
            if (multKeyFile.is_open())
            {
                if (!CC->SerializeEvalMultKey(multKeyFile, SerType::BINARY))
                {
                    Utils::LOG_ERROR("Error writing eval mult keys");
                    std::cerr << "Error writing eval mult keys" << std::endl;
                    std::exit(1);
                }
                multKeyFile.close();
            }
            else
            {
                Utils::LOG_ERROR("Error serializing EvalMult keys");
                std::cerr << "Error serializing EvalMult keys" << std::endl;
                std::exit(1);
            }
        }
    };
    class FHEBFV : public FHE
    {
    public:
        FHEBFV() = default;
        virtual ~FHEBFV() = default;
        static FHEBFV &Instance()
        {
            static FHEBFV utils_openfhebfv_instance;
            return utils_openfhebfv_instance;
        }
        void set_context(unsigned long plaintextModulus = 1017348097, unsigned long multiplicative_depth = 2)
        {
            CCParams<CryptoContextBFVRNS> params;
            params.SetPlaintextModulus(plaintextModulus);
            params.SetMultiplicativeDepth(multiplicative_depth);
            params.SetSecurityLevel(HEStd_128_classic);
            /*
                initialize crypto context
            */
            CC = GenCryptoContext(params);
            // Enable features that you wish to use
            CC->Enable(PKE);
            CC->Enable(KEYSWITCH);
            CC->Enable(LEVELEDSHE);
            KP = CC->KeyGen();
            CC->EvalMultKeyGen(KP.secretKey);
        }

        void save_keys(std::string key_folder = "keys/bfv")
        {
            save_crypto_context(key_folder, CC);
            save_public_key(key_folder, KP);
            save_private_key(key_folder, KP);
            save_mult_key(key_folder, CC);
        }
        void load_keys(std::string key_folder = "keys/bfv")
        {
            load_crypto_context(key_folder, CC);
            load_public_key(key_folder, KP);
            load_priv_key(key_folder, KP);
            load_mult_key(key_folder, CC);
        }
        Ciphertext<DCRTPoly> encrypt(std::vector<int64_t> data)
        {
            Plaintext plain = CC->MakePackedPlaintext(data);
            auto cipher = CC->Encrypt(KP.publicKey, plain);
            return cipher;
        }

        std::vector<int64_t> decrypt(Ciphertext<DCRTPoly> encrypted_data)
        {
            Plaintext result;
            CC->Decrypt(KP.secretKey, encrypted_data, &result);
            return result->GetPackedValue();
        }
        Ciphertext<DCRTPoly> eval_add(Ciphertext<DCRTPoly> ct1, Ciphertext<DCRTPoly> ct2)
        {
            return CC->EvalAdd(ct1, ct2);
        }

        Ciphertext<DCRTPoly> eval_mult(Ciphertext<DCRTPoly> ct1, Ciphertext<DCRTPoly> ct2)
        {
            return CC->EvalMult(ct1, ct2);
        }

    private:
        CryptoContext<DCRTPoly> CC;
        KeyPair<DCRTPoly> KP;
    };
}
