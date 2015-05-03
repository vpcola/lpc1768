#include "oauth_data.h"

#include <cstring>

#define OK 0

using std::memcpy;
using std::strncpy;
using std::strlen;

#define MIN(x,y) (((x)<(y))?(x):(y))

OAuthDataOut::OAuthDataOut(const char * contentType, const char* data)
    : m_str((char *) data), 
    m_contentType(contentType), 
    m_pos(0), 
    m_isChunked(false)
{
  m_size = strlen(data) + 1;
}

void OAuthDataOut::readReset()
{
    m_pos = 0;
}

// HTTPClient reads a piece of data to be transmitted
// buf - pointer to the buffer on which to copy (destination)
// the data, len is the length of the buffer.
// pReadLen is a pointer a variable on which the actual number 
// of data that was copied
int OAuthDataOut::read(char* buf, size_t len, size_t* pReadLen)
{
    *pReadLen = MIN(len, m_size - 1 - m_pos);
    memcpy(buf, m_str + m_pos, *pReadLen);
    m_pos += *pReadLen;
    return OK;

}
// Copies the content-type of the data 
// type (out) gets the content type of the data stored.
int OAuthDataOut::getDataType(char* type, size_t maxTypeLen)
{
   strncpy(type, m_contentType.c_str(), maxTypeLen-1);
   type[maxTypeLen-1] = 0;
   return OK;
}

bool OAuthDataOut::getIsChunked()
{
    return m_isChunked;
}

size_t OAuthDataOut::getDataLen()
{
    return m_size - 1;
}



OAuthDataIn::OAuthDataIn(char* str, size_t size) 
    : m_str(str), 
    m_contentType("text/plain"), 
    m_size(size), 
    m_pos(0), 
    m_isChunked(false)
{

}

std::string OAuthDataIn::getContentType() 
{
    return m_contentType;
}

std::string OAuthDataIn::getData()
{
    return std::string((const char *)m_str);
}

void OAuthDataIn::writeReset()
{
  m_pos = 0;
}

int OAuthDataIn::write(const char* buf, size_t len)
{
  size_t writeLen = MIN(len, m_size - 1 - m_pos);
  memcpy(m_str + m_pos, buf, writeLen);
  m_pos += writeLen;
  m_str[m_pos] = '\0';
  return OK;
}

void OAuthDataIn::setDataType(const char* type) 
{
    m_contentType = type;
}

void OAuthDataIn::setIsChunked(bool chunked) 
{
    m_isChunked = chunked;
}

void OAuthDataIn::setDataLen(size_t len) 
{
    m_size = len;
}



