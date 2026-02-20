#include "Crypto.h"
#include <mbedtls/ecdh.h>
#include <mbedtls/gcm.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/md.h>
#include <esp_random.h>
#include <cstring>

static mbedtls_entropy_context entropy;
static mbedtls_ctr_drbg_context ctr_drbg;
static bool rng_initialized = false;

static int init_rng() {
    if (rng_initialized) return 0;
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    int ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func,
                                     &entropy, nullptr, 0);
    if (ret == 0) rng_initialized = true;
    return ret;
}

bool Crypto::begin() {
    _hasKeyPair = false;
    return init_rng() == 0;
}

// Use the high-level ECDH TLS 1.3 API which is cleaner
bool Crypto::generateKeyPair(uint8_t pubkey[ECDH_PUBKEY_SIZE]) {
    mbedtls_ecp_group grp;
    mbedtls_mpi d;
    mbedtls_ecp_point Q;

    mbedtls_ecp_group_init(&grp);
    mbedtls_mpi_init(&d);
    mbedtls_ecp_point_init(&Q);

    bool ok = false;
    do {
        if (mbedtls_ecp_group_load(&grp, MBEDTLS_ECP_DP_CURVE25519) != 0) break;
        if (mbedtls_ecdh_gen_public(&grp, &d, &Q,
                                     mbedtls_ctr_drbg_random, &ctr_drbg) != 0) break;

        // Export private key (32 bytes)
        if (mbedtls_mpi_write_binary(&d, _privkey, 32) != 0) break;

        // For Curve25519, public key is Q.X coordinate (32 bytes)
        if (mbedtls_mpi_write_binary(&Q.X, pubkey, ECDH_PUBKEY_SIZE) != 0) break;

        _hasKeyPair = true;
        ok = true;
    } while (0);

    mbedtls_ecp_point_free(&Q);
    mbedtls_mpi_free(&d);
    mbedtls_ecp_group_free(&grp);
    return ok;
}

bool Crypto::deriveSharedKey(const uint8_t peer_pubkey[ECDH_PUBKEY_SIZE],
                              uint8_t shared_key[AES_KEY_SIZE]) {
    if (!_hasKeyPair) return false;

    mbedtls_ecp_group grp;
    mbedtls_mpi d, z;
    mbedtls_ecp_point Qp;

    mbedtls_ecp_group_init(&grp);
    mbedtls_mpi_init(&d);
    mbedtls_mpi_init(&z);
    mbedtls_ecp_point_init(&Qp);

    bool ok = false;
    do {
        if (mbedtls_ecp_group_load(&grp, MBEDTLS_ECP_DP_CURVE25519) != 0) break;

        // Import our private key
        if (mbedtls_mpi_read_binary(&d, _privkey, 32) != 0) break;

        // Import peer public key as point (Curve25519: X coordinate only)
        if (mbedtls_mpi_read_binary(&Qp.X, peer_pubkey, ECDH_PUBKEY_SIZE) != 0) break;
        if (mbedtls_mpi_lset(&Qp.Z, 1) != 0) break;

        // Compute shared secret
        if (mbedtls_ecdh_compute_shared(&grp, &z, &Qp, &d,
                                         mbedtls_ctr_drbg_random, &ctr_drbg) != 0) break;

        // Extract raw shared secret
        uint8_t raw_secret[32];
        if (mbedtls_mpi_write_binary(&z, raw_secret, 32) != 0) break;

        // Simple key derivation: use first 16 bytes of SHA-256(shared_secret)
        uint8_t hash[32];
        mbedtls_md_context_t md_ctx;
        mbedtls_md_init(&md_ctx);
        mbedtls_md_setup(&md_ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 0);
        mbedtls_md_starts(&md_ctx);
        mbedtls_md_update(&md_ctx, raw_secret, 32);
        mbedtls_md_finish(&md_ctx, hash);
        mbedtls_md_free(&md_ctx);

        memcpy(shared_key, hash, AES_KEY_SIZE);
        ok = true;
    } while (0);

    mbedtls_ecp_point_free(&Qp);
    mbedtls_mpi_free(&z);
    mbedtls_mpi_free(&d);
    mbedtls_ecp_group_free(&grp);
    return ok;
}

size_t Crypto::encrypt(const uint8_t* key, const uint8_t* plain, size_t plain_len,
                        uint8_t* out, size_t out_max) {
    size_t needed = GCM_IV_SIZE + plain_len + GCM_TAG_SIZE;
    if (out_max < needed) return 0;

    // Generate random IV
    uint8_t iv[GCM_IV_SIZE];
    esp_fill_random(iv, GCM_IV_SIZE);
    memcpy(out, iv, GCM_IV_SIZE);

    mbedtls_gcm_context gcm;
    mbedtls_gcm_init(&gcm);

    size_t result = 0;
    if (mbedtls_gcm_setkey(&gcm, MBEDTLS_CIPHER_ID_AES, key, AES_KEY_SIZE * 8) == 0) {
        uint8_t tag[GCM_TAG_SIZE];
        if (mbedtls_gcm_crypt_and_tag(&gcm, MBEDTLS_GCM_ENCRYPT,
                                       plain_len, iv, GCM_IV_SIZE,
                                       nullptr, 0,
                                       plain, out + GCM_IV_SIZE,
                                       GCM_TAG_SIZE, tag) == 0) {
            memcpy(out + GCM_IV_SIZE + plain_len, tag, GCM_TAG_SIZE);
            result = needed;
        }
    }

    mbedtls_gcm_free(&gcm);
    return result;
}

size_t Crypto::decrypt(const uint8_t* key, const uint8_t* in, size_t in_len,
                        uint8_t* plain, size_t plain_max) {
    if (in_len < GCM_IV_SIZE + GCM_TAG_SIZE) return 0;
    size_t cipher_len = in_len - GCM_IV_SIZE - GCM_TAG_SIZE;
    if (plain_max < cipher_len) return 0;

    const uint8_t* iv   = in;
    const uint8_t* ct   = in + GCM_IV_SIZE;
    const uint8_t* tag  = in + GCM_IV_SIZE + cipher_len;

    mbedtls_gcm_context gcm;
    mbedtls_gcm_init(&gcm);

    size_t result = 0;
    if (mbedtls_gcm_setkey(&gcm, MBEDTLS_CIPHER_ID_AES, key, AES_KEY_SIZE * 8) == 0) {
        if (mbedtls_gcm_auth_decrypt(&gcm, cipher_len,
                                      iv, GCM_IV_SIZE,
                                      nullptr, 0,
                                      tag, GCM_TAG_SIZE,
                                      ct, plain) == 0) {
            result = cipher_len;
        }
    }

    mbedtls_gcm_free(&gcm);
    return result;
}
