#ifndef __OAUTHDATA_H__
#define __OAUTHDATA_H__

/**
 * Author : Cola Vergil
 * Email  : vpcola@gmail.com
 * Date : Thu Apr 30 2015
 **/

#include <IHTTPData.h>
#include <string>

class OAuthDataOut : public IHTTPDataOut
{
    public:
	/** Create an HTTPText instance for output
	 * @param str String to be transmitted
	 */
	OAuthDataOut(const char * contentType, const char * data);


    protected:
	//IHTTPDataOut
    virtual void readReset();

    virtual int read(char* buf, size_t len, size_t* pReadLen);

    virtual int getDataType(char* type, size_t maxTypeLen); 

    virtual bool getIsChunked(); 

    virtual size_t getDataLen(); 

    private:
	char * m_str;
    std::string m_contentType;
	size_t m_size;
	size_t m_pos;
    bool m_isChunked;
};

class OAuthDataIn : public IHTTPDataIn
{
    public:

    OAuthDataIn(char* str, size_t size);
    std::string getContentType(); 
    std::string getData();

    protected:
	//IHTTPDataIn
    virtual void writeReset();

    virtual int write(const char* buf, size_t len);

    virtual void setDataType(const char* type); //Internet media type for Content-Type header

    virtual void setIsChunked(bool chunked); //For Transfer-Encoding header

    virtual void setDataLen(size_t len); //For Content-Length header

    private:
    char * m_str;
    std::string m_contentType;
   	size_t m_size;
    size_t m_pos;
     bool m_isChunked;
};

#endif

