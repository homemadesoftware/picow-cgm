#include "tcp_client.h"

#include "CGM_Display.h"


#define MAX_BUFFER_LENGTH 4096


err_t tcp_client_data_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err);
err_t tcp_client_polling(void* arg, struct tcp_pcb *tpcb);
err_t tcp_client_data_received(void* arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
void tcp_client_errored(void* arg, err_t err);




TcpConnection::TcpConnection(const char* address, u_int32_t port)
{
    this->head = nullptr;
    this->tail = nullptr;
    recursive_mutex_init(&mutex);

    ip4addr_aton(address, &this->targetAddress);
    this->targetPort = port;
    this->pcb = tcp_new_ip_type(IP_GET_TYPE());
}


void TcpConnection::StartConnect()
{
    // Set the argument that needs to be called everywhere
    tcp_arg(this->pcb, this);

    // Set the callbacks
    tcp_poll(this->pcb, tcp_client_polling, 10);
    tcp_sent(this->pcb, tcp_client_data_sent);
    tcp_recv(this->pcb, tcp_client_data_received);
    tcp_err(this->pcb, tcp_client_errored);

    cyw43_arch_lwip_begin();
    err_t err = tcp_connect(this->pcb, &this->targetAddress, this->targetPort, tcp_client_connected);
    cyw43_arch_lwip_end();
}

void TcpConnection::SendData(uint8_t* buffer, uint16_t length)
{
    tcp_write(this->pcb, buffer, length, TCP_WRITE_FLAG_COPY);
}

void TcpConnection::Close()
{
    err_t err = tcp_close(this->pcb);
    tcp_arg(this->pcb, NULL);
    tcp_poll(this->pcb, NULL, 0);
    tcp_sent(this->pcb, NULL);
    tcp_recv(this->pcb, NULL);
    tcp_err(this->pcb, NULL);
}

err_t TcpConnection::ClientConnected(err_t err)
{
    cyw43_arch_lwip_check();

    if (err != ERR_OK)
    {
        QueueEvent(ConnectionEvents::Errored, err, nullptr, 0);    
    }
    else
    {
        QueueEvent(ConnectionEvents::Connected, err, nullptr, 0);
    }

    return err;
}

err_t TcpConnection::DataReceived(struct pbuf *p, err_t err)
{
    cyw43_arch_lwip_check();

    if (err != ERR_OK)
    {
        if (p != nullptr)
        {
            pbuf_free(p);
        }
        QueueEvent(ConnectionEvents::Errored, err, nullptr, 0);
        return err;
    }
    
    uint8_t* buffer = (uint8_t*)calloc(1, MAX_BUFFER_LENGTH + 1);
    uint16_t bufferLength = 0;

    if (p->tot_len > 0) 
    {
        // Receive the buffer
        uint16_t buffer_left = MAX_BUFFER_LENGTH - bufferLength;
        if (buffer_left > p->tot_len)
        {
            buffer_left = p->tot_len;
        }

        bufferLength += pbuf_copy_partial(p, 
            buffer + bufferLength,
            buffer_left, 0);

        tcp_recved(this->pcb, p->tot_len);
    }
    pbuf_free(p);

    QueueEvent(ConnectionEvents::ReceivedData, err, buffer, bufferLength);

    return 0;
}

void TcpConnection::DataSent()
{
    cyw43_arch_lwip_check();
}

void TcpConnection::ClientErrored(err_t err)
{
    QueueEvent(ConnectionEvents::Errored, err, nullptr, 0);
}

void TcpConnection::QueueEvent(ConnectionEvents event, err_t err, uint8_t* buffer, uint16_t length)
{
    TcpUserEvent* newEvent = new TcpUserEvent(event, err, buffer, length);

    recursive_mutex_enter_blocking(&mutex);

    if (head == nullptr)
    {
        head = tail = newEvent;
    }
    else
    {
        tail->next = newEvent;
        tail = newEvent;
    }

    recursive_mutex_exit(&mutex);
}

TcpUserEvent* TcpConnection::DequeueEvent()
{
    if (!recursive_mutex_enter_timeout_ms(&mutex, 100))
    {
        return nullptr;
    }

    if (head == nullptr)
    {
        return nullptr;
    }
    TcpUserEvent* recoveredEvent = head;
    if (head == tail)
    {
        head = tail = nullptr;
    }
    head = head->next;

    recursive_mutex_exit(&mutex);

    return recoveredEvent;
}

#define TCP_CONNECTION_FROM_ARG(arg) TcpConnection *tc = (TcpConnection*)arg

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
    tc->ClientErrored(err);
}



