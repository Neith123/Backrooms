#pragma once

#include "backrooms_common.h"

void LogOutput(const char* Color, const char* Message);

void LogInfo(const char* Format, ...);
void LogWarn(const char* Format, ...);
void LogError(const char* Format, ...);
void LogCritical(const char* Format, ...);