#include "pico/stdlib.h"


class TcpConnection;
class TcpUserEvent;


enum ConnectionEvents
{
    Connected = 1,
    SentData = 2,
    ReceivedData = 3,
    Closed = 4
};



typedef void (*Handler_t)(TcpConnection *connection, ConnectionEvents eventType);


class TcpConnection
{

    public:
        TcpConnection(Handler_t handler);
        void SetRemoteAddressAndPort(const char* address, u_int32_t port);
        void Connect();
        void SendData(uint8_t* buffer, uint16_t length);
        void Close();
        err_t GetError();
        const char* GetBuffer();
        

    private: 
        struct tcp_pcb *pcb;
        ip_addr_t target_address;
        u_int32_t target_port;
        uint8_t* pbuffer;
        uint16_t buffer_length;
        Handler_t handler;
        err_t err;
        int signature;
        TcpUserEvent* head;
        TcpUserEvent* tail;

        err_t ClientConnected(err_t err);
        err_t DataReceived(struct pbuf *p, err_t err);
        void DataSent();
        void QueueEvent(ConnectionEvents event, err_t err, uint8_t* buffer, uint16_t length);
        TcpUserEvent* DequeueEvent();

        
        
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


};