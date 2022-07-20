#include "backrooms_logger.h"
#include "backrooms_platform.h"

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#define LOG_BUF_SIZE 2048

#define LOG_RESET   "\033[0m"
#define LOG_BLACK   "\033[30m"     
#define LOG_RED     "\033[31m"     
#define LOG_GREEN   "\033[32m"     
#define LOG_YELLOW  "\033[33m"     
#define LOG_BLUE    "\033[34m"     
#define LOG_MAGENTA "\033[35m"     
#define LOG_CYAN    "\033[36m"     
#define LOG_WHITE   "\033[37m"     
#define LOG_BOLD_BLACK   "\033[1m\033[30m"      
#define LOG_BOLD_RED     "\033[1m\033[31m"      
#define LOG_BOLD_GREEN   "\033[1m\033[32m"      
#define LOG_BOLD_YELLOW  "\033[1m\033[33m"      
#define LOG_BOLD_BLUE    "\033[1m\033[34m"      
#define LOG_BOLD_MAGENTA "\033[1m\033[35m"      
#define LOG_BOLD_CYAN    "\033[1m\033[36m"      
#define LOG_BOLD_WHITE   "\033[1m\033[37m"     

void LogOutput(const char* Color, const char* Message)
{
    printf(LOG_RESET);
    printf(Color);
    printf("%s\n", Message);
    printf(LOG_RESET);
}

void LogInfo(const char* Format, ...)
{
    char Buffer[LOG_BUF_SIZE];    
    va_list List;
    va_start(List, Format);
    vsnprintf(Buffer, sizeof(Buffer), Format, List);
    va_end(List);

    char OutputBuffer[LOG_BUF_SIZE];
    sprintf(OutputBuffer, "[INFO] %s", Buffer);

    LogOutput(LOG_CYAN, OutputBuffer);
}

void LogWarn(const char* Format, ...)
{
    char Buffer[LOG_BUF_SIZE];    
    va_list List;
    va_start(List, Format);
    vsnprintf(Buffer, sizeof(Buffer), Format, List);
    va_end(List);

    char OutputBuffer[LOG_BUF_SIZE];
    sprintf(OutputBuffer, "[WARN] %s", Buffer);

    LogOutput(LOG_YELLOW, OutputBuffer);
}

void LogError(const char* Format, ...)
{
    char Buffer[LOG_BUF_SIZE];    
    va_list List;
    va_start(List, Format);
    vsnprintf(Buffer, sizeof(Buffer), Format, List);
    va_end(List);

    char OutputBuffer[LOG_BUF_SIZE];
    sprintf(OutputBuffer, "[ERROR] %s", Buffer);

    LogOutput(LOG_RED, OutputBuffer);
}

void LogCritical(const char* Format, ...)
{
    char Buffer[LOG_BUF_SIZE];    
    va_list List;
    va_start(List, Format);
    vsnprintf(Buffer, sizeof(Buffer), Format, List);
    va_end(List);

    char OutputBuffer[LOG_BUF_SIZE];
    sprintf(OutputBuffer, "[FATAL] %s", Buffer);

    LogOutput(LOG_RED, OutputBuffer);
    PlatformMessageBox(Buffer, true);
    assert(false);
}