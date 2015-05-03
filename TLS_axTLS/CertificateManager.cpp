#include "CertificateManager.h"
#include "axTLS/crypto/bigint.h"
#include "cert_manager.h"
#include "axTLS/ssl/crypto_misc.h"

CertificateManager::CertificateManager():
    files(),
    certs(NULL),
    precomputedCerts()
{
}

CertificateManager::~CertificateManager()
{
    clear();
}

CertificateManager& CertificateManager::instance()
{
    static CertificateManager cm;
    return cm;
}

void CertificateManager::add(const char *fileName)
{
    CertificateManager::instance().files.push_back(fileName);
}

bool CertificateManager::load(bool precompute)
{
    if(precompute)
        return CertificateManager::instance().loadPrecomputeCertificates();
    else
        return CertificateManager::instance().loadCertificates();
}

bool CertificateManager::loadCertificates()
{
    X509_CTX *cert = certs;
    for(std::list<std::string>::iterator itor = files.begin();
            itor != files.end();
            ++itor) {
        FILE *fp = fopen(itor->c_str(), "r");
        if(fp == NULL)
            return false;

        fseek(fp, 0, SEEK_END);
        int length = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        uint8_t *data = new uint8_t[length];
        fread(data, sizeof(uint8_t), length, fp);
        fclose(fp);

        if(x509_new(data, NULL, &cert) != 0) {
            x509_free(certs);
            delete[] data;
            return false;
        }

        delete[] data;
        cert = cert->next;
    }
    files.clear();
    return true;
}

/* Check cert1 with cert2
*/
bool CertificateManager::check(X509_CTX *cert1, X509_CTX* cert2)
{
    if(asn1_compare_dn(cert1->ca_cert_dn, cert2->cert_dn))
        return false;
    if (time(NULL) < cert1->not_before)
        return false;
    if (time(NULL) > cert1->not_after)
        return false;

    /* Cannot check : takes too much memory
    BI_CTX *ctx = cert2->rsa_ctx->bi_ctx;
    bigint *mod = bi_clone(ctx, cert2->rsa_ctx->m);
    bigint *expn = bi_clone(ctx , cert2->rsa_ctx->e);
    bigint *digest = sig_verify(ctx, cert1->signature, cert1->sig_len, mod, expn);
    if(digest && cert1->digest)
    {
        if(bi_compare(digest, cert1->digest) != 0)
        {
            bi_free(ctx, digest);
            return false;
        }
        bi_free(ctx, digest);
    }
    else
        return false;
    */
    return true;
}

bool CertificateManager::loadPrecomputeCertificates()
{
    if(files.size() < 2)
        return false;
    precomputedCerts.reserve(files.size()-1);
    X509_CTX *cert1 = NULL, *cert2 = NULL;

    // load cert1
    FILE *fp = fopen(files.front().c_str(), "r");
    if(fp == NULL) {
        printf("Could not open file %s\n", files.front().c_str());
        clear();
        return false;
    }
    files.pop_front();

    fseek(fp, 0, SEEK_END);
    int length = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    uint8_t *data = new uint8_t[length];
    fread(data, sizeof(uint8_t), length, fp);
    fclose(fp);

    if(x509_new(data, NULL, &cert1) != 0) {
        delete[] data;
        clear();
        return false;
    }
    delete[] data;

    while(!files.empty()) {
        // load cert2
        fp = fopen(files.front().c_str(), "r");
        if(fp == NULL) {
            printf("Could not open file %s\n", files.front().c_str());
            x509_free(cert1);
            clear();
            return false;
        }
        files.pop_front();

        fseek(fp, 0, SEEK_END);
        length = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        data = new uint8_t[length];
        fread(data, sizeof(uint8_t), length, fp);
        fclose(fp);

        if(x509_new(data, NULL, &cert2) != 0) {
            x509_free(cert1);
            delete[] data;
            clear();
            return false;
        }
        delete[] data;


        if(!check(cert1, cert2)) {
            printf("Certificates verification failed\n");
            x509_free(cert1);
            x509_free(cert2);
            clear();
            return false;
        }


        // Create certificate
        PrecomputedCertificate pc;
        memset(&pc, 0, sizeof(PrecomputedCertificate));
        for(int i = 0; i < X509_NUM_DN_TYPES; ++i) {
            if(cert1->ca_cert_dn[i] != NULL) {
                pc.ca_cert_dn[i] = new char[strlen(cert1->ca_cert_dn[i])+1];
                strcpy(pc.ca_cert_dn[i], cert1->ca_cert_dn[i]);
            }
            if(cert1->cert_dn[i] != NULL) {
                pc.cert_dn[i] = new char[strlen(cert1->cert_dn[i])+1];
                strcpy(pc.cert_dn[i], cert1->cert_dn[i]);
            }
        }

        uint8_t buffer[512];
        uint16_t paddingLength;
        bi_permanent(cert1->digest);
        bi_export(cert1->rsa_ctx->bi_ctx, cert1->digest, buffer, 512);
        bi_depermanent(cert1->digest);

        paddingLength = 0;
        while(buffer[paddingLength] == 0 && paddingLength < 512) paddingLength++;
        pc.digest_len = 512 - paddingLength;
        pc.digest = new uint8_t[pc.digest_len];
        memcpy(pc.digest, &buffer[paddingLength], pc.digest_len);

        bi_export(cert2->rsa_ctx->bi_ctx, cert2->rsa_ctx->m, buffer, 512);

        paddingLength = 0;
        while(buffer[paddingLength] == 0) paddingLength++;
        pc.mod_len = 512 - paddingLength;
        pc.mod = new uint8_t[pc.mod_len];
        memcpy(pc.mod, &buffer[paddingLength], pc.mod_len);


        bi_export(cert2->rsa_ctx->bi_ctx, cert2->rsa_ctx->e, buffer, 512);
        paddingLength = 0;
        while(buffer[paddingLength] == 0) paddingLength++;
        pc.expn_len = 512 - paddingLength;
        pc.expn = new uint8_t[pc.expn_len];
        memcpy(pc.expn, &buffer[paddingLength], pc.expn_len);

        pc.sig = new uint8_t[cert1->sig_len];
        pc.sig_len = cert1->sig_len;
        memcpy(pc.sig, cert1->signature, pc.sig_len);


        precomputedCerts.push_back(pc);

        x509_free(cert1);
        cert1 = cert2;
    }
    x509_free(cert2);
    certs = NULL;
    return true;
}

void CertificateManager::clear()
{
    CertificateManager &cm = CertificateManager::instance();
    cm.files.clear();
    x509_free(cm.certs);
    cm.certs = NULL;
    for(int i = 0; i < cm.precomputedCerts.size(); ++i) {
        for(int j = 0; j < X509_NUM_DN_TYPES; ++j) {
            delete cm.precomputedCerts[i].cert_dn[j];
            delete cm.precomputedCerts[i].ca_cert_dn[j];
        }

        delete[] cm.precomputedCerts[i].sig;
        delete[] cm.precomputedCerts[i].mod;
        delete[] cm.precomputedCerts[i].expn;
        delete[] cm.precomputedCerts[i].digest;
    }
    cm.precomputedCerts.clear();
}

extern "C" char is_precomputed(void)
{
    return CertificateManager::instance().precomputedCerts.size() > 0 ? 1 : 0;
}

extern "C" PrecomputedCertificate get_precomputed_cert(char *cert_dn[], char *ca_cert_dn[])
{
    std::vector<PrecomputedCertificate> &precomputedCerts = CertificateManager::instance().precomputedCerts;

    for(int i = 0; i < precomputedCerts.size(); ++i) {

        PrecomputedCertificate pc = precomputedCerts[i];
        int j = 0;
        for(; j < X509_NUM_DN_TYPES; ++j) {

            if( (cert_dn[j] == NULL && pc.cert_dn[j] != NULL)
                    ||  (cert_dn[j] != NULL && pc.cert_dn[j] == NULL)
                    ||  (ca_cert_dn[j] == NULL && pc.ca_cert_dn[j] != NULL)
                    ||  (ca_cert_dn[j] != NULL && pc.ca_cert_dn[j] == NULL))
                break;

            if(cert_dn[j] && pc.cert_dn[j]) {
                if(strcmp(cert_dn[j], pc.cert_dn[j]))
                    break;
            }

            if(ca_cert_dn[j] && pc.ca_cert_dn[j]) {
                if(strcmp(ca_cert_dn[j], pc.ca_cert_dn[j]))
                    break;
            }
        }
        if(j == X509_NUM_DN_TYPES)
            return pc;
    }

    PrecomputedCertificate pc;
    memset(&pc, 0, sizeof(PrecomputedCertificate));
    return pc;
}


extern "C" X509_CTX* get_cert(char *ca_cert_dn[])
{
    X509_CTX *cert = CertificateManager::instance().certs;

    while(1) {
        if(cert == NULL)
            return NULL;
        int i = 0;
        for(; i < X509_NUM_DN_TYPES; ++i) {
            if(strcmp(ca_cert_dn[i], cert->cert_dn[i]))
                break;
        }
        if(i == X509_NUM_DN_TYPES)
            return cert;
        cert = cert->next;
    }
}


extern "C" void cert_manager_clear(void)
{
    CertificateManager::clear();
}



