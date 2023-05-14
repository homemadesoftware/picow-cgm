#include "tcp_client.h"
#include "time.h"


#define MAX_BUFFER_LENGTH 4096


err_t tcp_client_data_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err);
err_t tcp_client_polling(void* arg, struct tcp_pcb *tpcb);
err_t tcp_client_data_received(void* arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
void tcp_client_errored(void* arg, err_t err);

static TcpConnectionStaticData staticallyInitialisedData;

static u_int32_t idCounter;

TcpConnection::TcpConnection(const char* address, u_int32_t port, int timeOutSeconds)
{
    this->timesOutAt = clock() + timeOutSeconds * CLOCKS_PER_SEC;
    this->head = nullptr;
    this->tail = nullptr;
    this->pcb  = nullptr;

    ip4addr_aton(address, &this->targetAddress);
    this->targetPort = port;
    this->pcb = tcp_new_ip_type(IP_GET_TYPE());
    this->id = ++idCounter;
    printf("newtcp: %X, id:%d timesOut: %ld\r\n", this->pcb, this->id, this->timesOutAt / CLOCKS_PER_SEC);
}


TcpConnection::~TcpConnection()
{
    while (TcpUserEvent* pe = DequeueEvent())
    {
        delete pe;
    }
}

bool TcpConnection::HasTimedOut()
{
    return clock() >= this->timesOutAt;
}

bool TcpConnection::IsClosed()
{
    return pcb == nullptr; 
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
    if (this->pcb != nullptr)
    {
        tcp_poll(this->pcb, nullptr, 0);
        tcp_sent(this->pcb, nullptr);
        tcp_recv(this->pcb, nullptr);
        tcp_err(this->pcb, nullptr);
        
        tcp_close(this->pcb);
        this->pcb  = nullptr;   
    }
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
        return ERR_OK;
    }
    
    uint8_t* buffer = nullptr;
    uint16_t bufferLength = 0;

    if (p != nullptr) 
    {
        if (p->tot_len > 0)
        {
            // Receive the buffer
            uint16_t bytesToCopy = p->tot_len;
            if (bytesToCopy > MAX_BUFFER_LENGTH)
            {
                bytesToCopy = MAX_BUFFER_LENGTH;
            }

            buffer = (uint8_t*)calloc(bytesToCopy + 1, sizeof(uint8_t));
            bufferLength = bytesToCopy;

            pbuf_copy_partial(p, buffer, bytesToCopy, 0);

            tcp_recved(this->pcb, p->tot_len);
                
        }
        pbuf_free(p);
    }

    QueueEvent(ConnectionEvents::ReceivedData, err, buffer, bufferLength);

    return ERR_OK;
}

void TcpConnection::DataSent()
{
    cyw43_arch_lwip_check();
}

void TcpConnection::ClientErrored(err_t err)
{
    // turns out, the pcb is already freed by the time this is called 
    this->pcb  = nullptr;   

    QueueEvent(ConnectionEvents::Errored, err, nullptr, 0);
}

void TcpConnection::QueueEvent(ConnectionEvents event, err_t err, uint8_t* buffer, uint16_t length)
{
    TcpUserEvent* newEvent = new TcpUserEvent(event, err, buffer, length);

    recursive_mutex_enter_blocking(&staticallyInitialisedData.mutex);

    if (tail == nullptr)
    {
        head = tail = newEvent;
    }
    else
    {
        tail->next = newEvent;
        tail = newEvent;
    }

    recursive_mutex_exit(&staticallyInitialisedData.mutex);
}

TcpUserEvent* TcpConnection::DequeueEvent()
{
    //printf("try enter mutex\r\n");

    if (!recursive_mutex_enter_timeout_ms(&staticallyInitialisedData.mutex, 100))
    {
        //printf("failed enter mutex\r\n");

        return nullptr;
    }

    TcpUserEvent* recoveredEvent = nullptr;
    if (head != nullptr)
    {
        recoveredEvent = head;
        if (head == tail)
        {
            head = tail = nullptr;
        }
        else
        {
            head = head->next;
        }
    }

    recursive_mutex_exit(&staticallyInitialisedData.mutex);
    //printf("left mutex, %x\r\n", recoveredEvent);


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
    printf("cp: %X", arg);
    return 1;
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
    printf("err: %X", arg);
    tc->ClientErrored(err);
}



