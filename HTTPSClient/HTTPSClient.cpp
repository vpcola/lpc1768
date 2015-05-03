#include "HTTPSClient.h"
#include "HTTPHeader.h"
#include <stdio.h>
#include <string.h>

HTTPSClient::HTTPSClient():
    _con()
{
}

bool HTTPSClient::connect(const std::string& host)
{
    if(_con.is_connected())
        return false;

    return _con.connect(host.c_str());
}

std::string HTTPSClient::readLine()
{
    std::string line;
    char c;
    _con.receive(&c, 1);
    while(c != '\r') {
        line += c;
        _con.receive(&c, 1);
    }
    _con.receive(&c, 1); // skip \n
    return line;
}

HTTPHeader HTTPSClient::readHeader()
{
    HTTPHeader hdr;
    std::string line = readLine();
    sscanf(line.c_str(), "HTTP/1.%*d %d OK", &hdr._status);
    do {
        if(!line.compare(0,strlen("Content-Length"), "Content-Length"))
            sscanf(line.c_str(), "Content-Length: %d", &hdr._bodyLength);
        else if(!line.compare(0,strlen("content-length"), "content-length"))
            sscanf(line.c_str(), "content-length: %d", &hdr._bodyLength);
        line = readLine();
    } while(line.size());
    return hdr;
}

int HTTPSClient::get(const std::string& path, HTTPHeader *hdr)
{
    if(!_con.is_connected())
        return -1;

    const std::string &request = HTTPHeader::getRequest(path, _con.get_address(), 443);

    if(_con.send_all((char*)request.c_str(), request.size()+1) != request.size()+1)
        return -1;

    *hdr = readHeader();
    return hdr->_status == HTTP_OK ? 0 : -1;
}

int HTTPSClient::get(const std::string& path, HTTPHeader *hdr, char *data, int length)
{
    if(!_con.is_connected())
        return -1;

    if(hdr != NULL) {
        const std::string &request = HTTPHeader::getRequest(path, _con.get_address(), 443);
        if(_con.send_all((char*)request.c_str(), request.size()+1) != request.size()+1)
            return -1;
        *hdr = readHeader();
        if(hdr->_status != HTTP_OK)
            return -1;

        if(hdr->_bodyLength > 0)
            return _con.receive(data, hdr->_bodyLength > length ? length : hdr->_bodyLength);

        return 0;
    } else
        return _con.receive(data, length);
}

bool HTTPSClient::disconnect()
{
    if(!_con.is_connected())
        return true;

    return _con.close() == 0;
}
