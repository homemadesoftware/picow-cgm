#include "tcp_client.h"
#include "CGM_Display.h"



void MyTcpHandler(TcpConnection *connection, ConnectionEvents eventType)
{
    switch (eventType)
    {
        case ConnectionEvents::Connected :

        break;

        case ConnectionEvents::ReceivedData :
            
        break;
    }
}



int main() {
    stdio_init_all();

    CGM_InitDisplay();

    if (cyw43_arch_init()) 
    {
        CGM_DisplayText("failed to initialise");
        return 1;
    }
    cyw43_arch_enable_sta_mode();

    CGM_ClearScreen();
	
    CGM_DisplayText("Connecting to Wi-Fi...");
    if (cyw43_arch_wifi_connect_timeout_ms("You will be hacked", NULL, CYW43_AUTH_OPEN, 30000)) 
    {
        CGM_ClearScreen();
        CGM_DisplayText("failed to connect.");
        return 1;
    } 
    else 
    {
        CGM_ClearScreen();
        CGM_DisplayText("Wi-Fi Connected.");
    }
    
    sleep_ms(1000);


    CGM_ClearScreen();

    TcpConnection *tcp = new TcpConnection("4.234.223.103", 5001);
    tcp->StartConnect();
    
    while (true)
    {
        TcpUserEvent* nextEvent;
        nextEvent = tcp->DequeueEvent();
        CGM_ClearScreen();
        CGM_printf("ev: %d", nextEvent);

        while (nextEvent != nullptr)
        {
            ConnectionEvents eventType = nextEvent->GetEvent();
            switch (eventType)
            {
                case ConnectionEvents::Connected :
                    CGM_ClearScreen();
                    CGM_DisplayText("Connected.");
                break;
                
                case ConnectionEvents::ReceivedData :
                    CGM_ClearScreen();
                    CGM_printf("Received %s", nextEvent->GetBuffer());
                break;

                case ConnectionEvents::Errored :
                    CGM_ClearScreen();
                    CGM_printf("Errored %d", nextEvent->GetError());
                break;
            }

            //nextEvent->FreeBuffer();
            //delete nextEvent;
            
            sleep_ms(50);
            nextEvent = tcp->DequeueEvent();
        }
        sleep_ms(50);
    }


    //CGM_printf("Silly Ela. your dog has a flat face");
    //cyw43_arch_deinit();


    return 0;

}
