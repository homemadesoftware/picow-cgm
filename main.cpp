/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include <string.h>
#include <time.h>



#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"


#include "tcp_client.h"
#include "CGM_Display.h"



void MyTcpHandler(TcpConnection *connection, ConnectionEvents eventType)
{
    switch (eventType)
    {
        case ConnectionEvents::Connected :

        break;

        case ConnectionEvents::ReceivedData :
            CGM_ClearScreen();
            CGM_printf("R: %s", connection->GetBuffer());
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

    CGM_DisplayText("connecting...");
	
	
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

    TcpConnection *tcp = new TcpConnection(MyTcpHandler);
    tcp->SetRemoteAddressAndPort("4.234.223.103", 5001);
    tcp->Connect();
    
    while (true)
    {
                sleep_ms(1000);

        /*
        // main loop (not from a timer) to check for Wi-Fi driver or lwIP work that needs to be done.
        cyw43_arch_poll();
        // you can poll as often as you like, however if you have nothing else to do you can
        // choose to sleep until either a specified time, or cyw43_arch_poll() has work to do:
        cyw43_arch_wait_for_work_until(make_timeout_time_ms(1000));*/
        CGM_ClearScreen();
    }


    //CGM_printf("Silly Ela. your dog has a flat face");
    //cyw43_arch_deinit();



    return 0;

}
