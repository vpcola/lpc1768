#ifndef CERT_MANAGER_H
#define CERT_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "axTLS/ssl/crypto_misc.h"


    struct PrecomputedCertificate {
        char *ca_cert_dn[X509_NUM_DN_TYPES];
        char *cert_dn[X509_NUM_DN_TYPES];
        uint8_t *sig;
        uint16_t sig_len;
        uint8_t *mod;
        uint16_t mod_len;
        uint8_t *expn;
        uint16_t expn_len;
        uint8_t *digest;
        uint16_t digest_len;
        PrecomputedCertificate *next;
    };
    typedef struct PrecomputedCertificate PrecomputedCertificate;


    /*
        This is the C API of CertificateManager. These functions are
        used by axTLS to access certificates that have been loaded.
    */
    char is_precomputed(void);
    PrecomputedCertificate get_precomputed_cert(char *cert_dn[], char *ca_cert_dn[]);
    X509_CTX* get_cert(char *ca_cert_dn[]);
    void cert_manager_clear(void);


#ifdef __cplusplus
}
#endif


#endif



