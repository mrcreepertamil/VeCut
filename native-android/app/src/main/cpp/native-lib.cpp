#include <jni.h>
#include "timeline.h"
#include <cstdint>
#include <string>

namespace {
vecut::Timeline g_timeline;

std::string to_string(JNIEnv* env, jstring value) {
    if (value == nullptr) return {};
    const char* chars = env->GetStringUTFChars(value, nullptr);
    if (chars == nullptr) return {};
    std::string result(chars);
    env->ReleaseStringUTFChars(value, chars);
    return result;
}

jstring to_java_string(JNIEnv* env, const std::string& value) {
    return env->NewStringUTF(value.c_str());
}
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_ve_cut_NativeMediaEngine_nativeInitialize(JNIEnv* env, jobject) {
    return to_java_string(env, "VeCut native timeline engine ready");
}

extern "C" JNIEXPORT jint JNICALL
Java_com_ve_cut_NativeMediaEngine_nativeAddClip(JNIEnv* env, jobject, jstring uri, jstring mime_type) {
    return static_cast<jint>(g_timeline.add_clip(to_string(env, uri), to_string(env, mime_type)));
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_ve_cut_NativeMediaEngine_nativeRemoveClip(JNIEnv*, jobject, jint clip_id) {
    return g_timeline.remove_clip(static_cast<int>(clip_id)) ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_ve_cut_NativeMediaEngine_nativeSetClipTransition(JNIEnv*, jobject, jint clip_id,
                                                           jint type, jlong duration_ms) {
    return g_timeline.set_transition(static_cast<int>(clip_id), static_cast<int>(type),
                                     static_cast<int64_t>(duration_ms)) ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT jlongArray JNICALL
Java_com_ve_cut_NativeMediaEngine_nativeGetClipSettings(JNIEnv* env, jobject, jint clip_id) {
    int64_t source_in = 0;
    int64_t source_out = -1;
    int transition_type = 0;
    int64_t transition_duration = 0;
    if (!g_timeline.get_clip_settings(static_cast<int>(clip_id), &source_in, &source_out,
                                      &transition_type, &transition_duration)) {
        return nullptr;
    }
    const jlong values[] = {
        static_cast<jlong>(source_in),
        static_cast<jlong>(source_out),
        static_cast<jlong>(transition_type),
        static_cast<jlong>(transition_duration)
    };
    jlongArray result = env->NewLongArray(4);
    if (result != nullptr) env->SetLongArrayRegion(result, 0, 4, values);
    return result;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_ve_cut_NativeMediaEngine_nativeSetClipDuration(JNIEnv*, jobject, jint clip_id, jlong duration_ms) {
    return g_timeline.set_clip_duration(static_cast<int>(clip_id),
                                        static_cast<int64_t>(duration_ms)) ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_ve_cut_NativeMediaEngine_nativeTrimClip(JNIEnv*, jobject, jint clip_id, jlong in_ms, jlong out_ms) {
    return g_timeline.trim_clip(static_cast<int>(clip_id), static_cast<int64_t>(in_ms),
                                static_cast<int64_t>(out_ms)) ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_ve_cut_NativeMediaEngine_nativeMoveClip(JNIEnv*, jobject, jint clip_id, jint target_index) {
    return g_timeline.move_clip(static_cast<int>(clip_id),
                                static_cast<int>(target_index)) ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_ve_cut_NativeMediaEngine_nativeSplitClip(JNIEnv*, jobject, jint clip_id, jlong split_at_ms) {
    return static_cast<jint>(g_timeline.split_clip(static_cast<int>(clip_id),
                                                   static_cast<int64_t>(split_at_ms)));
}

extern "C" JNIEXPORT jint JNICALL
Java_com_ve_cut_NativeMediaEngine_nativeGetClipCount(JNIEnv*, jobject) {
    return static_cast<jint>(g_timeline.clip_count());
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_ve_cut_NativeMediaEngine_nativeGetTimelineSummary(JNIEnv* env, jobject) {
    return to_java_string(env, g_timeline.summary());
}

extern "C" JNIEXPORT void JNICALL
Java_com_ve_cut_NativeMediaEngine_nativeClearTimeline(JNIEnv*, jobject) {
    g_timeline.clear();
}
