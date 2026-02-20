#pragma once
#include "Protocol.h"
#include <cstdint>
#include <cstddef>

class Crypto {
public:
    bool begin();

    // ECDH key exchange
    bool generateKeyPair(uint8_t pubkey[ECDH_PUBKEY_SIZE]);
    bool deriveSharedKey(const uint8_t peer_pubkey[ECDH_PUBKEY_SIZE],
                         uint8_t shared_key[AES_KEY_SIZE]);

    // AES-128-GCM encrypt: plaintext -> iv(12) + ciphertext + tag(16)
    // Returns total output length, or 0 on failure
    size_t encrypt(const uint8_t* key, const uint8_t* plain, size_t plain_len,
                   uint8_t* out, size_t out_max);

    // AES-128-GCM decrypt: iv(12) + ciphertext + tag(16) -> plaintext
    // Returns plaintext length, or 0 on failure
    size_t decrypt(const uint8_t* key, const uint8_t* in, size_t in_len,
                   uint8_t* plain, size_t plain_max);

private:
    uint8_t _privkey[32];
    bool _hasKeyPair;
};
