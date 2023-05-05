#ifdef __cplusplus
extern "C" {
#endif

int CGM_InitDisplay();
int CGM_DisplayText(const char* message);
int CGM_printf(const char *fmt, ...);
int CGM_ClearScreen();
int CGM_DisplayBitmap(const unsigned char* image);

#ifdef __cplusplus
}
#endif