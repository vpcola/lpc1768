#ifndef HTTP_HEADER_H
#define HTTP_HEADER_H

#include <string>
#include "HTTPStatus.h"

class HTTPSClient;

class HTTPHeader
{
public :

    friend class HTTPSClient;

    HTTPHeader(HTTPStatus status = HTTP_INVALID);

    static std::string getRequest(const std::string &path, const std::string &host, const int port);

    HTTPStatus getStatus() const;
    int getBodyLength() const;
private :

    HTTPStatus _status;
    int _bodyLength;
};

#endif

