#include "backrooms_logger.h"

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#define LOG_BUF_SIZE 2048

void LogOutput(log_color Color, const char* Message)
{
    PlatformSetLogColor(Color);
    printf("%s\n", Message);
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

    LogOutput(LogColor_CyanInfo, OutputBuffer);
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

    LogOutput(LogColor_YellowWarn, OutputBuffer);
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

    LogOutput(LogColor_ErrorCriticalRed, OutputBuffer);
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

    LogOutput(LogColor_ErrorCriticalRed, OutputBuffer);
    PlatformMessageBox(Buffer, true);
    assert(false);
}