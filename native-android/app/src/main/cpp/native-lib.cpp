#include <jni.h>

extern "C" JNIEXPORT jstring JNICALL
Java_com_ve_cut_NativeMediaEngine_nativeInitialize(JNIEnv* env, jobject /* thiz */) {
    return env->NewStringUTF("C++ engine loaded (rendering pipeline not implemented yet)");
}
