/*
    This examples shows how to use the HTTPS library 
    in order to download a webpage from twitter.com
    
    Don't forget to download certificates and copy on 
    your mbed before running this program. You can 
    download them from this link:
    http://mbed.org/media/uploads/feb11/certificates-twitter.zip
*/
 
#include "mbed.h"
#include "EthernetInterface.h"
#include "CertificateManager.h"
#include "HTTPSClient.h"
 
const char host[] = "twitter.com";
const char request[] = "https://twitter.com";
LocalFileSystem local("local");
 
int main()
{
    set_time(1378370406);
    
    /* Starting Ethernet */
    EthernetInterface eth;
    if(eth.init() || eth.connect())
    {
        printf("Error with EthernetInterface\n\r");
        return -1;
    }
 
    /* Loading certificates in precomputed mode */
    CertificateManager::add("/local/cert1.der");
    CertificateManager::add("/local/cert2.der");
    CertificateManager::add("/local/cert3.der");
    if(!CertificateManager::load(true))
    {
        printf("Failed to load certificates\n");
        return -1;
    }
 
    /* Now, let's connect to twitter.com */
    HTTPSClient client;
    if(!client.connect(host))    
    {
        printf("Failed to connect to %s\n", host);
        return -1;
    }
    printf("Connected to %s !\n", host);
    
    /*  Don't forget to call this method, it's an 
        easy way to free a few KB of memory */
    CertificateManager::clear();
    
    
    /* Let's send our GET request to get the webpage */
    char buffer[256];
    int bufferLength = sizeof(buffer);
    HTTPHeader header;
    int read = client.get(request, &header, buffer, bufferLength);
    if(header.getStatus() != HTTP_OK || read < 0)
    {
        printf("Failed sending GET request : %s to %s", request, host);
        return -1;
    }
    
    /* index.htm is used to store the webpage */
    FILE *fp = fopen("/local/index.htm", "w");
    if(fp == NULL)
    {
        printf("Failed to open file index.htm\n");
        return -1;
    }
    fwrite(buffer, 1, read, fp);   // writing the first part of the body
    
    int totalRead = read;
    while(totalRead < header.getBodyLength())
    {
        if(bufferLength > header.getBodyLength() - totalRead)
            bufferLength = header.getBodyLength() - totalRead;
        
        /* This function does not send a get request but instead
            just receive data from the host.
        */
        read = client.get("", NULL, buffer, bufferLength);
        if(read < 0)
        {
            printf("Error while getting data from %s\n", host);
            return -1;
        }
        fwrite(buffer, 1, read, fp);
        totalRead += read;
    }
 
    /* We're done, let's close everything */
    fclose(fp);
    printf("Disconnecting from %s\n", host);
    client.disconnect();
    eth.disconnect();
    
    return 0;
}