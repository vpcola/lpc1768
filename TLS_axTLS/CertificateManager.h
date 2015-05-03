#ifndef CERTIFICATE_MANAGER_H
#define CERTIFICATE_MANAGER_H

#include "mbed.h"
#include <vector>
#include <list>
#include <string>
#include "axTLS/ssl/crypto_misc.h"
#include "cert_manager.h"


/** This class is in charge of loading and storing certificates.

    Example:
    @code
    #include "mbed.h"
    #include "CertificateManager.h
    LocalFileSystem local("/local/");

    int main(void)
    {
        CertificateManager::add("/local/root.der");
        if(!CertificateManager::load())
            printf("Error while loading certificates\n");

        return 0;
    }
    @endcode
*/
class CertificateManager
{
public :

    friend char is_precomputed(void);
    friend PrecomputedCertificate get_precomputed_cert(char *cert_dn[], char *ca_cert_dn[]);
    friend X509_CTX* get_cert(char *ca_cert_dn[]);

    /** Add a certificate to load.

        \param fileName Certificate's filename.
        \note This function does not load the certificate
        and does not check if the file exists.
    */
    static void add(const char *fileName);

    /** Load certificates.

        \param precompute Tells the certificate manager how to load
        certificates.
        \return True if certificates were loaded with
        success, false otherwise.

        \note If the loading fails, everything is cleared. So,
        you have to add again all certificates you need.
    */
    static bool load(const bool precompute = false);

    /** Clear everything.
        \note This function should be called once a TLS
        connection is established with success.
    */
    static void clear();

private :

    CertificateManager();
    ~CertificateManager();
    static CertificateManager& instance();

    bool loadCertificates();
    bool loadPrecomputeCertificates();
    bool check(X509_CTX *cert1, X509_CTX* cert2);

    std::list<std::string> files;
    X509_CTX *certs;
    std::vector<PrecomputedCertificate> precomputedCerts;
};


#endif

