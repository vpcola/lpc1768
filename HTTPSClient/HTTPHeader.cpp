#include "HTTPHeader.h"

HTTPHeader::HTTPHeader(HTTPStatus status):
    _status(status),
    _bodyLength(0)
{
}

std::string HTTPHeader::getRequest(const std::string &path, const std::string &host, const int port)
{
    
    std::string request = "GET ";
    request += path;
    request += " HTTP/1.0\r\nHost: ";
    request += host;
    request += ":";
    request += port;
    request += "\r\n\r\n";
    return request;
}

HTTPStatus HTTPHeader::getStatus() const
{
    return _status;
}

int HTTPHeader::getBodyLength() const
{
    return _bodyLength;
}

