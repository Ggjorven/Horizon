#pragma once

#if defined(HZ_PLATFORM_ANDROID)
#include "Horizon/Core/Core.hpp"
#include "Horizon/Core/Logging.hpp"

#include <jni.h>

namespace Hz
{

    // Note: This function might need to be improved since
    // now we retrieve every time, instead of once and caching.
    class Utils
    {
    public:
        static JNIEnv* GetJNIEnv();
    };

}
#endif