#include "TLSConnection.h"
#include <stdlib.h>
#include <stdio.h>
const static int HTTPS_PORT = 443;

TLSConnection::TLSConnection():
    Socket(),
    Endpoint(),
    _is_connected(false),
    _ssl_ctx(),
    _ssl()
{
}

bool TLSConnection::connect(const char *host)
{
    if (init_socket(SOCK_STREAM) < 0)
        return false;

    if (set_address(host, HTTPS_PORT) != 0)
        return false;

    if (lwip_connect(_sock_fd, (const struct sockaddr *) &_remoteHost, sizeof(_remoteHost)) < 0) {
        close();
        return false;
    }

    if(ssl_ctx_new(&_ssl_ctx, 0, SSL_DEFAULT_CLNT_SESS) != &_ssl_ctx)
        return false;

    _ssl.ssl_ctx = &_ssl_ctx;

    if(ssl_client_new(&_ssl, _sock_fd, NULL, 0) == NULL) {
        close();
        return false;
    }
    if(_ssl.hs_status != SSL_OK) {
        close();
        return false;
    }

    _is_connected = true;

    return true;
}

bool TLSConnection::is_connected(void)
{
    return _is_connected;
}

int TLSConnection::send_all(char *data, int length)
{
    if ((_sock_fd < 0) || !_is_connected)
        return -1;

    return ssl_write(&_ssl, (uint8_t*)data, length);
}

int TLSConnection::receive(char *data, int length)
{
    return ssl_read(&_ssl, (uint8_t*)data, length);
}

bool TLSConnection::close(bool shutdown)
{
    if(!_is_connected)
        return true;

    _is_connected = false;
    ssl_ctx_free(_ssl.ssl_ctx);

    return Socket::close(shutdown) == 0;
}

