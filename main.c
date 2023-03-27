
#include "EPD_Test.h"
#include "EPD_2in13_V3.h"


//#include "EPD_Test.h"   //Examples

int EPD_2in13_V3_test2(char* message)
{
    printf("EPD_2in13_V3_test Demo\r\n");
    if(DEV_Module_Init()!=0){
        return -1;
    }

    printf("e-Paper Init and Clear...\r\n");
	EPD_2in13_V3_Init();
    EPD_2in13_V3_Clear();

    //Create a new image cache
    UBYTE *BlackImage;
    UWORD Imagesize = ((EPD_2in13_V3_WIDTH % 8 == 0)? (EPD_2in13_V3_WIDTH / 8 ): (EPD_2in13_V3_WIDTH / 8 + 1)) * EPD_2in13_V3_HEIGHT;
    if((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL) {
        printf("Failed to apply for black memory...\r\n");
        return -1;
    }
    printf("Paint_NewImage\r\n");
    Paint_NewImage(BlackImage, EPD_2in13_V3_WIDTH, EPD_2in13_V3_HEIGHT, 90, WHITE);
	Paint_Clear(WHITE);


#if 1  // Drawing on the image
	Paint_NewImage(BlackImage, EPD_2in13_V3_WIDTH, EPD_2in13_V3_HEIGHT, 90, WHITE);  	
    printf("Drawing\r\n");
    //1.Select Image
    Paint_SelectImage(BlackImage);
    Paint_Clear(WHITE);
	
    // 2.Drawing on the image


    Paint_DrawString_EN(20, 15, message, &Font16, BLACK, WHITE);

    EPD_2in13_V3_Display_Base(BlackImage);
    DEV_Delay_ms(3000);
#endif


}

/*

int main(void)
{
	 while(1) {
        EPD_2in13_V3_test2();
		
	    DEV_Delay_ms(2000); 
	}
	DEV_Delay_ms(500); 
	// EPD_2in9_V2_test();
    // EPD_2in9bc_test();
    // EPD_2in9b_V3_test();
    // EPD_2in9d_test();

    // EPD_2in13_V2_test();
	// EPD_2in13_V3_test();
    // EPD_2in13bc_test();
    // EPD_2in13b_V3_test();
    // EPD_2in13b_V4_test();
    // EPD_2in13d_test();
    
    // EPD_2in66_test();
    // EPD_2in66b_test();
    
    // EPD_2in7_test();

    // EPD_3in7_test();
	
	// EPD_4in2_test();
    // EPD_4in2b_V2_test();
    // EPD_5in65f_test();

    // EPD_5in83_V2_test();
    // EPD_5in83b_V2_test();

    // EPD_7in5_V2_test();
    // EPD_7in5b_V2_test();

    

    return 0;
}
*/