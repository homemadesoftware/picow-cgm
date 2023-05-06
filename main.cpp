#include "tcp_client.h"
#include "CGM_Display.h"

int counter = 0;

int ReadTextStream();
int ReadBitmapStream();



int ReadTextStream()
{
    TcpConnection *tcp = new TcpConnection("4.234.223.103", 5001);
    tcp->StartConnect();
    
    while (true)
    {
        printf(".");
        TcpUserEvent* nextEvent;
        nextEvent = tcp->DequeueEvent();

        while (nextEvent != nullptr)
        {
            ConnectionEvents eventType = nextEvent->GetEvent();          
            printf("Picked event, type: %d, addr: %X\r\n", eventType, nextEvent);

            switch (eventType)
            {
                case ConnectionEvents::Connected :
                    CGM_ClearScreen();
                    CGM_printf("Connected. %d", ++counter);
                break;
                
                case ConnectionEvents::ReceivedData :
                    CGM_ClearScreen();
                    CGM_printf("R:%s, %d", nextEvent->GetBuffer(), ++counter);
                    printf("Closing\r\n");
                    tcp->Close();
                    printf("Closed\r\n");
                break;

                case ConnectionEvents::Errored :
                    CGM_ClearScreen();
                    CGM_printf("Errored %d, %d", nextEvent->GetError(), ++counter);
                break;

            }
            delete nextEvent;

            if (tcp->IsClosed())
            {
                break;
            }
            
            sleep_ms(100);
            nextEvent = tcp->DequeueEvent();
        }
        if (tcp->IsClosed())
        {
            printf("Deleting tcp\r\n");
            delete tcp;
            tcp = nullptr;
            printf("Returning\r\n");
            return 0;
        }

        sleep_ms(100);
    }

}

int ReadBitmapStream()
{
    const int imageSize = (128 / 8) * 250;

    TcpConnection *tcp = new TcpConnection("4.234.223.103", 5002);
    tcp->StartConnect();
    
    unsigned char* combinedBuffer = nullptr;
    unsigned int totalSize = 0;

    while (true)
    {
        printf(".");
        TcpUserEvent* nextEvent;
        nextEvent = tcp->DequeueEvent();

        while (nextEvent != nullptr)
        {
            ConnectionEvents eventType = nextEvent->GetEvent();          
            printf("Picked event, type: %d, addr: %X\r\n", eventType, nextEvent);

            switch (eventType)
            {
                
                case ConnectionEvents::ReceivedData :
                    {
                        unsigned char* partBuffer = nextEvent->GetBuffer();
                        int partLength = nextEvent->GetBufferLength();
                        printf("len: %d", partLength);
                        unsigned char* newBuffer = static_cast<unsigned char*>(malloc(totalSize + partLength));
                        if (combinedBuffer != nullptr)
                        {
                            memcpy(newBuffer, combinedBuffer, totalSize);
                        }
                        memcpy(newBuffer + totalSize, partBuffer, partLength);
                        void* previousBuffer = combinedBuffer;
                        combinedBuffer = newBuffer;
                        totalSize += partLength;
                        if (previousBuffer != nullptr)
                        {
                            free(previousBuffer);
                        }
                        
                        if (totalSize >= imageSize)
                        {
                            //CGM_ClearScreen();
                            CGM_DisplayBitmap(static_cast<unsigned char*>(combinedBuffer));
                            tcp->Close();
                        }
                    }
                break;

                case ConnectionEvents::Errored :
                    CGM_ClearScreen();
                    CGM_printf("Errored %d, %d", nextEvent->GetError(), ++counter);
                    tcp->Close();
                break;

            }
            delete nextEvent;

            if (tcp->IsClosed())
            {
                break;
            }
            
            sleep_ms(100);
            nextEvent = tcp->DequeueEvent();
        }
        if (tcp->IsClosed())
        {
            delete tcp;
            tcp = nullptr;
            if (combinedBuffer != nullptr)
            {
                free(combinedBuffer);
            }
            return 0;
        }

        sleep_ms(100);
    }

}


int main() 
{
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

    while (true)
    {
        CGM_ClearScreen();

        //ReadBitmapStream();
        ReadTextStream();
    }
   

    //CGM_printf("Silly Ela. your dog has a flat face");
    //cyw43_arch_deinit();


    return 0;

}
