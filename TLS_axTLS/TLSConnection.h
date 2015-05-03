#ifndef TLSCONNECTION_H
#define TLSCONNECTION_H

#include "Socket/Socket.h"
#include "Socket/Endpoint.h"
#include "axTLS/ssl/ssl.h"

/** This class provides a user-friendly interface for the
    axTLS library.
*/
class TLSConnection : public Socket, public Endpoint
{
public :

    TLSConnection();

    /** This function tries to establish a TLS connection
        with the given host.
        It will first try to establish a TCP connection on
        port 443 with the host. Then, it runs the TLS 
        handshake protocol.

        \param host A valid hostname (e.g. "mbed.org")
        \return True if it managed to establish a connection
        with the host. False otherwise.
    */
    bool connect(const char *host);

    /** Indicates whether a connection is established or not.

        \return true if a connection is established, otherwise
       returns false.
    */
    bool is_connected(void);

    /** Sends some data to the host. This method does not return
        until length bytes have been sent.

        \param data A pointer to some data
        \param length Number of bytes to send
        \return Number of bytes sent, or -1 if an error occured.
    */
    int send_all(char *data, int length);

    /** Receive some data from the host.

        \param data
        \param length Maximum number of bytes to receive
        \return Number of bytes read in range 0..length, or -1
        if an error occured.
    */
    int receive(char *data, int length);

    /** Close the connection.

        \param shutdown
        \return True if the connection was closed with success,
        false otherwise. If no connection was established,
        returns true immediately.
    */
    bool close(bool shutdown = true);

private :

    bool _is_connected;

    SSL_CTX _ssl_ctx;
    SSL _ssl;
};

#endif


