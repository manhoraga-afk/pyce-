#include "../../include/builtins.h"
#include <jni.h>

extern JavaVM* g_jvm;

Value platform_ask_file() {
    JNIEnv* env;
    g_jvm->AttachCurrentThread(&env, NULL);

    jclass activityClass = env->FindClass("com/pycee/runtime/PyceeActivity");
    jmethodID method = env->GetStaticMethodID(activityClass, "askFile", "()Ljava/lang/String;");
    jstring result = (jstring)env->CallStaticObjectMethod(activityClass, method);

    if (result) {
        const char* chars = env->GetStringUTFChars(result, NULL);
        Value v = make_text(chars);
        env->ReleaseStringUTFChars(result, chars);
        return v;
    }

    return make_text("");
}

Value platform_get_camera() {
    JNIEnv* env;
    g_jvm->AttachCurrentThread(&env, NULL);

    jclass activityClass = env->FindClass("com/pycee/runtime/PyceeActivity");
    jmethodID method = env->GetStaticMethodID(activityClass, "getCamera", "()Ljava/lang/String;");
    jstring result = (jstring)env->CallStaticObjectMethod(activityClass, method);

    if (result) {
        const char* chars = env->GetStringUTFChars(result, NULL);
        Value v = make_text(chars);
        env->ReleaseStringUTFChars(result, chars);
        return v;
    }

    return make_text("");
}