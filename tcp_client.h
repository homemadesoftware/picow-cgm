#include "pico/stdlib.h"
#include "pico/mutex.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include <string.h>
#include <time.h>



class TcpConnection;
class TcpUserEvent;


enum ConnectionEvents
{
    Connected = 1,
    SentData = 2,
    ReceivedData = 3,
    Errored = 4
};



class TcpConnection
{

    public:
        TcpConnection(const char* address, u_int32_t port);
        void StartConnect();
        TcpUserEvent* DequeueEvent();
        void SendData(uint8_t* buffer, uint16_t length);
        void Close();

    private: 
        struct tcp_pcb *pcb;
        ip_addr_t targetAddress;
        u_int32_t targetPort;
        TcpUserEvent* head;
        TcpUserEvent* tail;
        recursive_mutex_t mutex;

        err_t ClientConnected(err_t err);
        void ClientErrored(err_t err);
        err_t DataReceived(struct pbuf *p, err_t err);
        void DataSent();
        
        void QueueEvent(ConnectionEvents event, err_t err, uint8_t* buffer, uint16_t length);

        
        friend err_t tcp_client_data_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
        friend err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err);
        friend err_t tcp_client_polling(void* arg, struct tcp_pcb *tpcb);
        friend err_t tcp_client_data_received(void* arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
        friend void tcp_client_errored(void* arg, err_t err);

};


class TcpUserEvent
{
    public:
        TcpUserEvent(
            ConnectionEvents event,       
            err_t err,
            uint8_t* buffer,
            uint16_t length)
        {
            this->event = event;
            this->error = error;
            this->buffer = buffer;
            this->length = length;
            this->next = nullptr;
        }

        ConnectionEvents GetEvent()
        {
            return event;
        }

        err_t GetError()
        {
            return error;
        }

        uint8_t* GetBuffer()
        {
            return buffer;
        }

        uint16_t GetBufferLength()
        {
            return length;
        }

        void FreeBuffer()
        {
            if (buffer != nullptr)
            {
                free(buffer);
                buffer = nullptr;
                length = 0;
            }
        }

    private:
        ConnectionEvents event;        
        err_t error;
        uint8_t* buffer;
        uint16_t length;
        TcpUserEvent* next;
        friend class TcpConnection;  
    


};