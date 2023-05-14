#include "tcp_client.h"
#include "CGM_Display.h"

int counter = 0;

bool ReadBitmapStream();

extern "C" {
    void tcp_debug_print_pcbs(void);
    extern struct tcp_pcb *tcp_active_pcbs;
}


bool ReadBitmapStream()
{
    tcp_debug_print_pcbs();

    while (tcp_active_pcbs != nullptr)
    {
        printf("Waiting for all tcps to clear\r\n");
        sleep_ms(1000);
    } 

    const int imageSize = (128 / 8) * 250;
    bool hasErrored = false;

    TcpConnection *tcp = new TcpConnection("4.234.223.103", 5002, 30);
    tcp->StartConnect();


    unsigned int totalSize = imageSize;
    unsigned char* combinedBuffer = static_cast<unsigned char *>(malloc(totalSize));
    unsigned int usedSize = 0; 
    
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
                        if (usedSize + partLength > totalSize)
                        {
                            partLength = totalSize - usedSize;
                        }
                       
                        memcpy(combinedBuffer + usedSize, partBuffer, partLength);
                        usedSize += partLength;

                        if (usedSize >= imageSize)
                        {
                            CGM_ClearScreen();
                            CGM_DisplayBitmap(static_cast<unsigned char*>(combinedBuffer));
                            tcp->Close();
                        }
                    }
                break;

                case ConnectionEvents::Errored :
                    CGM_ClearScreen();
                    CGM_printf("Errored %d, %d", nextEvent->GetError(), ++counter);
                    tcp->Close();
                    hasErrored = true;
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

        if (tcp->HasTimedOut())
        {
            CGM_ClearScreen();
            CGM_printf("Timed out");
            tcp->Close();
        }

        if (tcp->IsClosed())
        {
            delete tcp;
            tcp = nullptr;
            free(combinedBuffer);
            return !hasErrored;
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

    bool needToConnect = true;
    while (true)
    {
        if (needToConnect)
        {
            CGM_ClearScreen();
	
            CGM_DisplayText("Connecting to Wi-Fi...");
            if (cyw43_arch_wifi_connect_timeout_ms("HC", "nfwi6536", CYW43_AUTH_WPA2_AES_PSK, 30000)) 
            {
                CGM_ClearScreen();
                CGM_DisplayText("failed to connect.");
            } 
            else 
            {
                CGM_ClearScreen();
                CGM_DisplayText("Wi-Fi Connected.");
                needToConnect = false;
            }
        }
        else
        {
            if (!ReadBitmapStream())
            {
                needToConnect = true;
            }  
            else
            {      
                sleep_ms(1000);
            }
        }
    }
    
   

    while (true)
    {
       

        ReadBitmapStream();
        //ReadTextStream();
    }

    return 0;

}
