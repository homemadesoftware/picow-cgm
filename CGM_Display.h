#ifdef __cplusplus
extern "C" {
#endif

int CGM_InitDisplay();
int CGM_DisplayText(const char* message);
int CGM_printf(const char *fmt, ...);
int CGM_ClearScreen();

#ifdef __cplusplus
}
#endif