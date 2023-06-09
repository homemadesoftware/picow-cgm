#include "tcp_client.h"
#include "CGM_Display.h"
#include "hardware/watchdog.h"


int counter = 0;
long lastKnownVersion = 0;
bool watchDogReboot = false;

bool ReadBitmapStream();

extern "C" {
    void tcp_debug_print_pcbs(void);
    extern struct tcp_pcb *tcp_active_pcbs;
}

typedef struct tagWifiPoint
{
    char ssid[48];
    char password[32];
} WifiPoint;

WifiPoint wifiPoints[] = 
{
    {"WIFIHUB_d82510", "ntydtnkv"},
    {"FRITZ!Box 7430 WO", "80031011246998990386"},
    {"You will be hacked", ""},
    {"HC", "nfwi6536"},
    {""},
};

bool ReadBitmapStream()
{
    tcp_debug_print_pcbs();

    while (tcp_active_pcbs != nullptr)
    {
        printf("Waiting for all tcps to clear\r\n");
        sleep_ms(1000);
    } 

    const int imageSize = (128 / 8) * 250;
    const int versionSize = sizeof(uint32_t);
    bool hasErrored = false;

    //TcpConnection *tcp = new TcpConnection("4.234.223.103", 5002, 30);  // Azure Server
    TcpConnection *tcp = new TcpConnection("81.187.255.239", 5002, 30); // Raspberry Pi at Toadhall

    tcp->StartConnect();


    unsigned int totalSize = imageSize + versionSize;
    unsigned char* combinedBuffer = static_cast<unsigned char *>(malloc(totalSize));
    unsigned int usedSize = 0; 
    
    while (true)
    {
        watchdog_update();
        printf(watchDogReboot ? "¬" : ".");
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

                        if (usedSize >= totalSize)
                        {
                            long version = *(reinterpret_cast<long*>(combinedBuffer));
                            printf("Version %ld", version);
                            if (lastKnownVersion != version)
                            {
                                lastKnownVersion = version;
                                CGM_ClearScreen();
                                CGM_DisplayBitmap(static_cast<unsigned char*>(combinedBuffer + versionSize));
                            }
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

bool ConnectToWifi()
{
    for (int i = 0; wifiPoints[i].ssid[0] != 0; ++i)
    {
        const char *ssid = wifiPoints[i].ssid;
        const char *password = wifiPoints[i].password; 

        CGM_ClearScreen();
   
        if (cyw43_arch_init()) 
        {
            CGM_DisplayText("failed to initialise");
            return 1;
        }
        cyw43_arch_enable_sta_mode();

        CGM_printf("Trying: %s", ssid);
        uint32_t authLevel = CYW43_AUTH_WPA2_AES_PSK;
        if (*password == 0)
        {
            authLevel = CYW43_AUTH_OPEN;
            password = nullptr;
        }
        for (int i = 0; i < 30; ++i)
        {
            if (cyw43_arch_wifi_connect_timeout_ms(ssid, password, authLevel, 1000) == PICO_OK) 
            {
                CGM_ClearScreen();
                CGM_DisplayText("Wi-Fi Connected.");
                return true;
            } 
            watchdog_update();
        }
       cyw43_wifi_leave(&cyw43_state, CYW43_ITF_STA);
       cyw43_arch_deinit();
    }

    return false;
}


int main() 
{
    stdio_init_all();

    if (watchdog_caused_reboot()) 
    {
        printf("Rebooted by Watchdog!\n");
        watchDogReboot = true;
    } 
    else 
    {
        printf("Clean boot\n");
    }
    
    watchdog_enable(8000, 1);

    CGM_InitDisplay();


    bool connected = false;
    while (true)
    {
        if (!connected)
        {
            connected = ConnectToWifi();
        }
        else
        {
            if (!ReadBitmapStream())
            {
                connected = false;
            }  
            else
            {      
                sleep_ms(1000);
            }
        }
        watchdog_update();

    }
    

    return 0;

}
