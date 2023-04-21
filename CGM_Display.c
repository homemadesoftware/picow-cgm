#include <stdio.h>
#include <stdarg.h>


#include "EPD_Test.h"
#include "EPD_2in13_V3.h"

#include "CGM_Display.h"

UBYTE *BlackImage;

int CGM_InitDisplay()
{
    if(DEV_Module_Init()!=0){
        return -1;
    }

    printf("e-Paper Init and Clear...\r\n");
	EPD_2in13_V3_Init();
    EPD_2in13_V3_Clear();

    //Create a new image cache
    UWORD Imagesize = ((EPD_2in13_V3_WIDTH % 8 == 0)? (EPD_2in13_V3_WIDTH / 8 ): (EPD_2in13_V3_WIDTH / 8 + 1)) * EPD_2in13_V3_HEIGHT;
    if((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL) {
        printf("Failed to apply for black memory...\r\n");
        return -1;
    }
    printf("Paint_NewImage\r\n");
    Paint_NewImage(BlackImage, EPD_2in13_V3_WIDTH, EPD_2in13_V3_HEIGHT, 90, WHITE);
	Paint_Clear(WHITE);


	Paint_NewImage(BlackImage, EPD_2in13_V3_WIDTH, EPD_2in13_V3_HEIGHT, 90, WHITE);  	
    printf("Drawing\r\n");
    //1.Select Image
    Paint_SelectImage(BlackImage);
    Paint_Clear(WHITE);
	
    return 0;
}

int CGM_DisplayText(const char* message)
{
    Paint_DrawString_EN(0, 0, message, &Font16, WHITE, BLACK);

    EPD_2in13_V3_Display_Base(BlackImage);
    DEV_Delay_ms(1000);

    return 0;

}

int CGM_ClearScreen()
{
    Paint_Clear(WHITE);

    return 0;
}

int CGM_printf(const char *fmt, ...)
{
    char buffer[128];
    
    va_list argptr;
    va_start(argptr, fmt);
    vsnprintf(buffer, 128, fmt, argptr);
    va_end(argptr);
    CGM_DisplayText(buffer);

    return 0;
}