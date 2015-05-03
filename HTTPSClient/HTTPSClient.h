#ifndef HTTPS_CLIENT_H
#define HTTPS_CLIENT_H

#include <string>
#include "TLSConnection.h"
#include "HTTPHeader.h"


/** Simple wrapper of TLS library to send GET request to
    a server and receive some web pages.
*/
class HTTPSClient
{
public :

    HTTPSClient();

    /** Connnect to the given host.

        \param host A valid hostname (e.g. "mbed.org")
        \return True if the connection was established with
        success, false otherwise.

        \note If the client is already connected, it returns false.
    */
    bool connect(const std::string& host);

    /** Send a GET request to the host.

        \param path
        \param hdr A pointer to an HTTPHeader object.
        \return Returns 0 if the request was sent with success,
        or -1 if an error occured.
    */
    int get(const std::string& path, HTTPHeader *hdr);

    /** Send a get request and reads (partially) the body

        \param path
        \param hdr      A pointer to an HTTPHeader object.
        \param data     A pointer to some buffer
        \param length   Maximum number of bytes to read
        \return Number of bytes stored in data in range 0..length, or -1
        if an error occured.

        \note To check whether this request is sent with success, you must check
        the status of the HTTPHeader and the value of the integer returned by this
        function.

        Example:
        @code
        // Assuming that an HTTPSClient object c is connected to a host
        HTTPHeader hdr;
        char buffer[256];
        int n = c.get("/", &hdr, buffer, 256);
        if(n > 0 && hdr.getStatus() == HTTP_OK) // GET request sent with success
        {
            // do some stuff here..
        }
        else
            printf("Failed to send get request\n");
        @endcode


        \note If hdr is null, this function does not send anything and directly writes
        bytes in data. This is particularly useful if you expect large answers (such as
        webpages) from the host.
    */
    int get(const std::string& path, HTTPHeader *hdr, char *data, int length);

    /** Disconnect from host

        \return True if the client disconnected with success, false otherwise.
    */
    bool disconnect();

private :

    std::string readLine();
    HTTPHeader readHeader();

    TLSConnection _con;

};

#endif

