#include "media_scanner_android.h"

#if defined(Q_OS_ANDROID)

#include <QJniEnvironment>
#include <QJniObject>
#include <QNativeInterface>

namespace {

void clear_pending_exception() {
    QJniEnvironment env;
    if (env->ExceptionCheck()) env->ExceptionClear();
}

QStringList scan_collection(JNIEnv* env, jobject resolver, jmethodID query_method,
                            jclass uri_class, jmethodID uri_parse_method,
                            jclass string_class, const char* collection_uri,
                            jstring sort_order) {
    QStringList paths;
    jstring uri_text = env->NewStringUTF(collection_uri);
    jobject uri = env->CallStaticObjectMethod(uri_class, uri_parse_method, uri_text);
    env->DeleteLocalRef(uri_text);
    if (env->ExceptionCheck() || !uri) {
        env->ExceptionClear();
        return paths;
    }

    jobjectArray projection = env->NewObjectArray(1, string_class, nullptr);
    jstring data_column = env->NewStringUTF("_data");
    env->SetObjectArrayElement(projection, 0, data_column);
    env->DeleteLocalRef(data_column);

    jobject cursor = env->CallObjectMethod(
        resolver, query_method, uri, projection, nullptr, nullptr, sort_order);
    env->DeleteLocalRef(projection);
    env->DeleteLocalRef(uri);
    if (env->ExceptionCheck() || !cursor) {
        env->ExceptionClear();
        return paths;
    }

    jclass cursor_class = env->GetObjectClass(cursor);
    const jmethodID get_column_index = env->GetMethodID(
        cursor_class, "getColumnIndex", "(Ljava/lang/String;)I");
    const jmethodID move_to_next = env->GetMethodID(cursor_class, "moveToNext", "()Z");
    const jmethodID get_string = env->GetMethodID(
        cursor_class, "getString", "(I)Ljava/lang/String;");
    const jmethodID close = env->GetMethodID(cursor_class, "close", "()V");

    jstring column_name = env->NewStringUTF("_data");
    const jint data_index = env->CallIntMethod(cursor, get_column_index, column_name);
    env->DeleteLocalRef(column_name);

    if (!env->ExceptionCheck() && data_index >= 0) {
        while (env->CallBooleanMethod(cursor, move_to_next) == JNI_TRUE) {
            if (env->ExceptionCheck()) break;
            auto value = static_cast<jstring>(
                env->CallObjectMethod(cursor, get_string, data_index));
            if (env->ExceptionCheck()) break;
            if (!value) continue;

            const char* utf8 = env->GetStringUTFChars(value, nullptr);
            if (utf8) {
                const QString path = QString::fromUtf8(utf8);
                env->ReleaseStringUTFChars(value, utf8);
                if (!path.isEmpty() && !paths.contains(path)) paths.push_back(path);
            }
            env->DeleteLocalRef(value);
        }
    }

    if (env->ExceptionCheck()) env->ExceptionClear();
    env->CallVoidMethod(cursor, close);
    if (env->ExceptionCheck()) env->ExceptionClear();
    env->DeleteLocalRef(cursor_class);
    env->DeleteLocalRef(cursor);
    return paths;
}

} // namespace

QStringList scan_android_media_files() {
    QJniObject context = QNativeInterface::QAndroidApplication::context();
    if (!context.isValid()) return {};

    QJniEnvironment environment;
    JNIEnv* env = environment.operator->();
    if (!env) return {};

    jclass context_class = env->GetObjectClass(context.object<jobject>());
    const jmethodID get_content_resolver = env->GetMethodID(
        context_class, "getContentResolver", "()Landroid/content/ContentResolver;");
    jobject resolver = env->CallObjectMethod(context.object<jobject>(), get_content_resolver);
    env->DeleteLocalRef(context_class);
    if (env->ExceptionCheck() || !resolver) {
        env->ExceptionClear();
        return {};
    }

    jclass resolver_class = env->GetObjectClass(resolver);
    const jmethodID query_method = env->GetMethodID(
        resolver_class, "query",
        "(Landroid/net/Uri;[Ljava/lang/String;Ljava/lang/String;[Ljava/lang/String;Ljava/lang/String;)Landroid/database/Cursor;");
    env->DeleteLocalRef(resolver_class);

    jclass uri_class = env->FindClass("android/net/Uri");
    const jmethodID uri_parse_method = env->GetStaticMethodID(
        uri_class, "parse", "(Ljava/lang/String;)Landroid/net/Uri;");
    jclass string_class = env->FindClass("java/lang/String");
    jstring sort_order = env->NewStringUTF("date_added DESC");

    if (env->ExceptionCheck() || !uri_class || !string_class ||
        !query_method || !uri_parse_method) {
        env->ExceptionClear();
        if (sort_order) env->DeleteLocalRef(sort_order);
        if (string_class) env->DeleteLocalRef(string_class);
        if (uri_class) env->DeleteLocalRef(uri_class);
        env->DeleteLocalRef(resolver);
        return {};
    }

    QStringList paths;
    paths.append(scan_collection(env, resolver, query_method, uri_class,
                                 uri_parse_method, string_class,
                                 "content://media/external/images/media", sort_order));
    paths.append(scan_collection(env, resolver, query_method, uri_class,
                                 uri_parse_method, string_class,
                                 "content://media/external/video/media", sort_order));
    paths.append(scan_collection(env, resolver, query_method, uri_class,
                                 uri_parse_method, string_class,
                                 "content://media/external/audio/media", sort_order));
    paths.removeDuplicates();

    env->DeleteLocalRef(sort_order);
    env->DeleteLocalRef(string_class);
    env->DeleteLocalRef(uri_class);
    env->DeleteLocalRef(resolver);
    return paths;
}

#endif // Q_OS_ANDROID
