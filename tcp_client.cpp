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

enum ConnectionEvents
{
    Connected = 1,
    SentData = 2,
    ReceivedData = 3,
    Closed = 4
};

class TCPConnection;

typedef void (*Handler_t)(TCPConnection *connection, ConnectionEvents eventType);

class TCPConnection
{

    public:
        TCPConnection(Handler_t handler);
        void SetRemoteAddressAndPort(char* address, u_int32_t port);
        void Connect();
        void SendData(uint8_t* buffer, uint16_t length);
        void Close();
        

    private: 
        struct tcp_pcb *pcb;
        ip_addr_t target_address;
        u_int32_t target_port;
        uint8_t* pbuffer;
        uint16_t buffer_length;
        Handler_t handler;

        err_t ClientConnected(err_t err);
        err_t DataReceived(struct pbuf *p, err_t err);
        void DataSent();
        
        
        friend err_t tcp_client_data_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
        friend err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err);
        friend err_t tcp_client_polling(void* arg, struct tcp_pcb *tpcb);
        friend err_t tcp_client_data_received(void* arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
        friend void tcp_client_errored(void* arg, err_t err);

};

TCPConnection::TCPConnection(Handler_t handler)
{
    this->handler = handler;
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

void TCPConnection::SendData(uint8_t* buffer, uint16_t length)
{
    tcp_write(this->pcb, buffer, length, TCP_WRITE_FLAG_COPY);
}

void TCPConnection::Close()
{
    err_t err = tcp_close(this->pcb);
    tcp_arg(this->pcb, NULL);
    tcp_poll(this->pcb, NULL, 0);
    tcp_sent(this->pcb, NULL);
    tcp_recv(this->pcb, NULL);
    tcp_err(this->pcb, NULL);
}

err_t TCPConnection::ClientConnected(err_t err)
{
    cyw43_arch_lwip_check();

    this->handler(this, ConnectionEvents::Connected);

    return 0;
}

err_t TCPConnection::DataReceived(struct pbuf *p, err_t err)
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

        tcp_recved(this->pcb, p->tot_len);
    }
    pbuf_free(p);

    this->handler(this, ConnectionEvents::ReceivedData);

    return 0;
}

void TCPConnection::DataSent()
{
    cyw43_arch_lwip_check();
    this->handler(this, ConnectionEvents::SentData);
}

#define TCP_CONNECTION_FROM_ARG(arg) TCPConnection *tc = (TCPConnection*)arg

err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err) 
{
    TCP_CONNECTION_FROM_ARG(arg);
    return tc->ClientConnected(err);
}

err_t tcp_client_data_received(void* arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) 
{
    TCP_CONNECTION_FROM_ARG(arg);
    return tc->DataReceived(p, err);
}

err_t tcp_client_polling(void* arg, struct tcp_pcb *tpcb) 
{
    TCP_CONNECTION_FROM_ARG(arg);
    return 0;
}

err_t tcp_client_data_sent(void* arg, struct tcp_pcb *tpcb, u16_t len) 
{
    TCP_CONNECTION_FROM_ARG(arg);
    tc->DataSent();
    return 0;
}

void tcp_client_errored(void* arg, err_t err) 
{
    TCP_CONNECTION_FROM_ARG(arg);
}



