#pragma once

#include "backrooms_common.h"

struct platform_config
{
    bool Running;
    bool VerticalSync;
    i32 Width;
    i32 Height;
};

extern platform_config PlatformConfiguration;