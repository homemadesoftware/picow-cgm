#include "pico/stdlib.h"


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

        err_t ClientConnected(err_t err);
        err_t DataReceived(struct pbuf *p, err_t err);
        void DataSent();
        
        
        friend err_t tcp_client_data_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
        friend err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err);
        friend err_t tcp_client_polling(void* arg, struct tcp_pcb *tpcb);
        friend err_t tcp_client_data_received(void* arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
        friend void tcp_client_errored(void* arg, err_t err);

};