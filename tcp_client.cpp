#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#define MAX_BUFFER_LENGTH 4096



err_t tcp_client_data_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err);
err_t tcp_client_polling(void* arg, struct tcp_pcb *tpcb);
err_t tcp_client_data_received(void* arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
void tcp_client_errored(void* arg, err_t err);

class TCPConnection
{
    public:
        TCPConnection();
        void SetRemoteAddressAndPort(char* address, u_int32_t port);
        void Connect();
        void 

    private: 
        struct tcp_pcb *pcb;
        ip_addr_t target_address;
        u_int32_t target_port;
        uint8_t* pbuffer;
        uint16_t buffer_length;

        err_t DataReceived(struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
        
        friend err_t tcp_client_data_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
        friend err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err);
        friend err_t tcp_client_polling(void* arg, struct tcp_pcb *tpcb);
        friend err_t tcp_client_data_received(void* arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
        friend void tcp_client_errored(void* arg, err_t err);

};

TCPConnection::TCPConnection()
{
    
    this->pbuffer = (uint8_t*)calloc(1, MAX_BUFFER_LENGTH);
    this->buffer_length = 0;
}


void TCPConnection::Connect()
{
    this->buffer_length = 0;

    // Set the argument that needs to be called everywhere
    tcp_arg(this->pcb, this);

    // Set the callbacks
    tcp_poll(this->pcb, tcp_client_polling, 10);
    tcp_sent(this->pcb, tcp_client_data_sent);
    tcp_recv(this->pcb, tcp_client_data_received);
    tcp_err(this->pcb, tcp_client_errored);

    cyw43_arch_lwip_begin();
    err_t err = tcp_connect(this->pcb, &this->target_address, this->target_port, tcp_client_connected);
    cyw43_arch_lwip_end();
}

void TCPConnection::SetRemoteAddressAndPort(char* address, u_int32_t port)
{
    ip4addr_aton(address, &this->target_address);
    this->target_port = port;
    this->pcb = tcp_new_ip_type(IP_GET_TYPE(tcp_c->remote_addr));
}

err_t TCPConnection::DataReceived(struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    cyw43_arch_lwip_check();

    if (p->tot_len > 0) 
    {
        // Receive the buffer
        uint16_t buffer_left = MAX_BUFFER_LENGTH - this->buffer_length;
        if (buffer_left > p->tot_len)
        {
            buffer_left = p->tot_len;
        }

        this->buffer_length += pbuf_copy_partial(p, 
            this->pbuffer + this->buffer_length,
            buffer_left, 0);

        tcp_recved(tpcb, p->tot_len);
    }

    pbuf_free(p);
    return 0;
}

#define TCP_CONNECTION_FROM_ARG(arg) TCPConnection *tc = (TCPConnection*)arg

err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err) 
{
    TCP_CONNECTION_FROM_ARG(arg);
    return tc->ClientConnected(tpcb, p, err);
}


err_t tcp_client_data_received(void* arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) 
{
    TCP_CONNECTION_FROM_ARG(arg);
    return tc->DataReceived(tpcb, p, err);
}

err_t tcp_client_polling(void* arg, struct tcp_pcb *tpcb) 
{
    TCP_CONNECTION_FROM_ARG(arg);
    return 0;
}

err_t tcp_client_data_sent(void* arg, struct tcp_pcb *tpcb, u16_t len) 
{
    TCP_CONNECTION_FROM_ARG(arg);
    return 0;
}

void tcp_client_errored(void* arg, err_t err) 
{
    TCP_CONNECTION_FROM_ARG(arg);
}



