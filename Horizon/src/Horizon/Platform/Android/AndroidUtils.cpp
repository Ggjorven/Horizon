#include "hzpch.h"

#if defined(HZ_PLATFORM_ANDROID)
#include "AndroidUtils.hpp"

#include "Horizon/Core/Logging.hpp"

namespace Hz
{

	JNIEnv* Utils::GetJNIEnv()
    {
        JavaVM* jvm;
        JNIEnv* env;
        jint result = JavaVM::GetJavaVM(&jvm);

        HZ_ASSERT((result == JNI_OK), "Failed to retrieve JavaVM");

        jvm->AttachCurrentThread(&env, nullptr);
        return env;
    }

}
#endif